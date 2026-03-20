#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/damon.h>
#include <linux/bitops.h>

// 模拟内核中的宏
#define MY_DAMON_MAX_SUBSCORE (100)
#define MY_DAMOS_MAX_SCORE (99)
#define MY_DAMON_MAX_AGE_IN_LOG (64)

/* 
 * 模拟内核中的 damon_max_nr_accesses 逻辑
 * 避免因为该函数是 static 而导致链接失败
 */
static unsigned int my_max_nr_accesses(struct damon_attrs *attrs)
{
	unsigned int max_nr_accesses = attrs->aggr_interval / attrs->sample_interval;
	return max_nr_accesses ? max_nr_accesses : 1;
}

/* 目标函数副本 */
static noinline int my_damon_hot_score(struct damon_ctx *c, struct damon_region *r,
			struct damos *s)
{
	int freq_subscore;
	unsigned int age_in_sec;
	int age_in_log, age_subscore;
	unsigned int freq_weight = s->quota.weight_nr_accesses;
	unsigned int age_weight = s->quota.weight_age;
	int hotness;

	/* 修正点：使用模拟的 max_nr_accesses */
	freq_subscore = r->nr_accesses * MY_DAMON_MAX_SUBSCORE /
			my_max_nr_accesses(&c->attrs);

	age_in_sec = (unsigned long)r->age * c->attrs.aggr_interval / 1000000;

	/* Loop */
	// for (age_in_log = 0; age_in_log < DAMON_MAX_AGE_IN_LOG && age_in_sec;
	//      age_in_log++, age_in_sec >>= 1)
	// 	;

	/* Fls */
	age_in_log = min_t(int, fls(age_in_sec), MY_DAMON_MAX_AGE_IN_LOG);

	if (freq_subscore == 0)
		age_in_log *= -1;

	age_in_log += MY_DAMON_MAX_AGE_IN_LOG;
	age_subscore = (long)age_in_log * MY_DAMON_MAX_SUBSCORE / MY_DAMON_MAX_AGE_IN_LOG / 2;

	hotness = (freq_weight * freq_subscore + age_weight * age_subscore);
	if (freq_weight + age_weight)
		hotness /= freq_weight + age_weight;

	hotness = (long)hotness * MY_DAMOS_MAX_SCORE / MY_DAMON_MAX_SUBSCORE;
	return hotness;
}

static int __init damon_perf_test_init(void)
{
	struct damon_ctx *c;
	struct damon_region r = {0};
	struct damos s = {0};
	ktime_t start, end;
	int i;
	volatile int score;
	const int loops = 10000000; // 一千万次压测

	pr_info("DAMON Perf Test: Starting...\n");

	c = damon_new_ctx();
	if (!c) return -ENOMEM;

	/* * 正确初始化 attrs：
	 * 采样间隔 5ms, 聚合间隔 100ms -> 最大访问次数为 20 次
	 */
	c->attrs.sample_interval = 5000;
	c->attrs.aggr_interval = 100000;

	/* 设置测试区域：访问了 15 次，年龄 500 */
	r.nr_accesses = 15;
	r.age = 500;

	/* 权重分配 */
	s.quota.weight_nr_accesses = 50;
	s.quota.weight_age = 50;

	// 压测开始
	start = ktime_get();
	for (i = 0; i < loops; i++) {
		score = my_damon_hot_score(c, &r, &s);
	}
	end = ktime_get();

	pr_info("DAMON Perf Test: %d loops done.\n", loops);
	pr_info("DAMON Perf Test: Total time: %lld ns\n", ktime_to_ns(ktime_sub(end, start)));
	pr_info("DAMON Perf Test: Avg latency: %lld ns\n", ktime_to_ns(ktime_sub(end, start)) / loops);

	damon_destroy_ctx(c);
	return 0;
}

static void __exit damon_perf_test_exit(void)
{
	pr_info("DAMON Perf Test: Exit.\n");
}

module_init(damon_perf_test_init);
module_exit(damon_perf_test_exit);
MODULE_LICENSE("GPL");
