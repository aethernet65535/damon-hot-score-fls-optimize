#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/damon.h>
#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/sort.h>

/* Macro definitions for iteration and sampling */
#define TEST_LOOPS 10000000
#define SAMPLE_COUNT 10000
#define SAMPLE_STRIDE (TEST_LOOPS / SAMPLE_COUNT)

/* Mock constants for DAMON internal scoring */
#define MY_DAMON_MAX_SUBSCORE (100)
#define MY_DAMOS_MAX_SCORE (99)
#define MY_DAMON_MAX_AGE_IN_LOG (32)

static unsigned int my_max_nr_accesses(struct damon_attrs *attrs)
{
	unsigned int max_nr_accesses =
		attrs->aggr_interval / attrs->sample_interval;
	return max_nr_accesses ? max_nr_accesses : 1;
}

static int my_damon_hot_score(struct damon_ctx *c, struct damon_region *r,
			      struct damos *s)
{
	int freq_subscore, age_in_log, age_subscore, hotness;
	unsigned int age_in_sec, d_age_in_sec;
	unsigned int freq_weight = s->quota.weight_nr_accesses;
	unsigned int age_weight = s->quota.weight_age;

	freq_subscore = r->nr_accesses * MY_DAMON_MAX_SUBSCORE /
			my_max_nr_accesses(&c->attrs);
	d_age_in_sec = age_in_sec = (unsigned long)r->age * c->attrs.aggr_interval / 1000000;

	/* ------------------------------------------------------------------------- */
	/* Algorithm A: Original For-Loop */
	// for (age_in_log = 0; age_in_log < MY_DAMON_MAX_AGE_IN_LOG && age_in_sec;
	//      age_in_log++, age_in_sec >>= 1)
	// 	;

	/* Algorithm B: fls */
	// age_in_log = min_t(int, fls(d_age_in_sec), MY_DAMON_MAX_AGE_IN_LOG);

	/* Algorithm C: ilog2 (recommend by SeongJae) */
	age_in_log = min_t(int, ilog2(d_age_in_sec), MY_DAMON_MAX_AGE_IN_LOG);
	if (age_in_log)
		age_in_log++;
	
	/* Compare: A:B */
	// if (age_in_log != min_t(int, fls(d_age_in_sec), MY_DAMON_MAX_AGE_IN_LOG))
	// 	pr_info("AB: FALSE!\n");

	/* Compare: A:C (DC?) */
	// if (age_in_log != min_t(int, ilog2(d_age_in_sec), MY_DAMON_MAX_AGE_IN_LOG))
	// 	pr_info("ACDC: FALSE!\n");
	/* ------------------------------------------------------------------------- */

	if (freq_subscore == 0)
		age_in_log *= -1;

	age_in_log += MY_DAMON_MAX_AGE_IN_LOG;
	age_subscore = (long)age_in_log * MY_DAMON_MAX_SUBSCORE /
		       MY_DAMON_MAX_AGE_IN_LOG / 2;

	hotness = (freq_weight * freq_subscore + age_weight * age_subscore);
	if (freq_weight + age_weight)
		hotness /= freq_weight + age_weight;

	return (long)hotness * MY_DAMOS_MAX_SCORE / MY_DAMON_MAX_SUBSCORE;
}

static int cmp_u64(const void *a, const void *b)
{
	u64 l = *(u64 *)a, r = *(u64 *)b;
	return (l > r) ? 1 : ((l < r) ? -1 : 0);
}

static int damon_perf_test(void)
{
	struct damon_ctx *c;
	struct damon_region r = { 0 };
	struct damos s = { 0 };
	u64 *samples;
	ktime_t start, end, t1, t2;
	u64 total_ns = 0, diff;
	int i, sample_idx = 0;
	int cnt_0_19 = 0, cnt_20_39 = 0;
	int cnt_40_59 = 0, cnt_60_79 = 0;
	int cnt_80_99 = 0, cnt_100 = 0;

	samples = kmalloc_array(SAMPLE_COUNT, sizeof(u64), GFP_KERNEL);
	if (!samples)
		return -ENOMEM;

	c = damon_new_ctx();
	if (!c) {
		kfree(samples);
		return -ENOMEM;
	}

	c->attrs.sample_interval = 5000;
	c->attrs.aggr_interval = 100000;
	s.quota.weight_nr_accesses = 50;
	s.quota.weight_age = 50;
	r.nr_accesses = 15;

	pr_info("DAMON Perf Test: Starting %d iterations\n", TEST_LOOPS);

	start = ktime_get();
	for (i = 0; i < TEST_LOOPS; i++) {
		r.age = i;

		if (unlikely(i % SAMPLE_STRIDE == 0 &&
			     sample_idx < SAMPLE_COUNT)) {
			t1 = ktime_get();
			my_damon_hot_score(c, &r, &s);
			t2 = ktime_get();
			diff = ktime_to_ns(ktime_sub(t2, t1));

			samples[sample_idx++] = diff;
			if (diff >= 0 && diff <= 19)
				cnt_0_19++;
			else if (diff >= 20 && diff <= 39)
				cnt_20_39++;
			else if (diff >= 40 && diff <= 59)
				cnt_40_59++;
			else if (diff >= 60 && diff <= 79)
				cnt_60_79++;
			else if (diff >= 80 && diff <= 99)
				cnt_80_99++;
			else
				cnt_100++;
		} else {
			my_damon_hot_score(c, &r, &s);
		}
	}
	end = ktime_get();
	total_ns = ktime_to_ns(ktime_sub(end, start));

	sort(samples, SAMPLE_COUNT, sizeof(u64), cmp_u64, NULL);

	pr_info("=============================================\n");
	pr_info(" Total Iterations : %d\n", TEST_LOOPS);
	pr_info(" Average Latency  : %llu ns\n", total_ns / TEST_LOOPS);
	pr_info(" P95 Latency      : %llu ns\n",
		samples[(SAMPLE_COUNT * 95) / 100]);
	pr_info(" P99 Latency      : %llu ns\n",
		samples[(SAMPLE_COUNT * 99) / 100]);
	pr_info("---------------------------------------------\n");
	pr_info(" Range (ns)      | Count        | Percent\n");
	pr_info("---------------------------------------------\n");
	pr_info(" 0-19            | %-8d     |  %5d%%\n", cnt_0_19 * SAMPLE_STRIDE,
		(cnt_0_19 * 100) / SAMPLE_COUNT);
	pr_info(" 20-39           | %-8d     |  %5d%%\n", cnt_20_39 * SAMPLE_STRIDE,
		(cnt_20_39 * 100) / SAMPLE_COUNT);
	pr_info(" 40-59           | %-8d     |  %5d%%\n", cnt_40_59 * SAMPLE_STRIDE,
		(cnt_40_59 * 100) / SAMPLE_COUNT);
	pr_info(" 60-79           | %-8d     |  %5d%%\n", cnt_60_79 * SAMPLE_STRIDE,
		(cnt_60_79 * 100) / SAMPLE_COUNT);
	pr_info(" 80-99           | %-8d     |  %5d%%\n", cnt_80_99 * SAMPLE_STRIDE,
		(cnt_80_99 * 100) / SAMPLE_COUNT);
	pr_info(" 100+            | %-8d     |  %5d%%\n", cnt_100 * SAMPLE_STRIDE,
		(cnt_100 * 100) / SAMPLE_COUNT);
	pr_info("=============================================\n");
	pr_info("\n");

	damon_destroy_ctx(c);
	kfree(samples);
	return 0;
}

static int __init damon_perf_test_init(void)
{
	int i;

	for (i = 0; i < 5; i++) {
		damon_perf_test();
	}

	return 0;
}

static void __exit damon_perf_test_exit(void)
{
	pr_info("DAMON Perf Test: Module exit.\n");
}

module_init(damon_perf_test_init);
module_exit(damon_perf_test_exit);
MODULE_LICENSE("GPL");
