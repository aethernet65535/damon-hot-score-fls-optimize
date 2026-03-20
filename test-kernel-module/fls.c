#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/damon.h>
#include <linux/bitops.h>

// 模拟 damon.h 或 core.c 中的宏（如果你的环境没包含，这里手动定义）
#ifndef DAMON_MAX_SUBSCORE
#define DAMON_MAX_SUBSCORE (100)
#endif
#ifndef DAMOS_MAX_SCORE
#define DAMOS_MAX_SCORE (99)
#endif
#ifndef DAMON_MAX_AGE_IN_LOG
#define DAMON_MAX_AGE_IN_LOG (32)
#endif

/* * 将目标函数拷贝出来，方便在模块中独立测试。
 * 这样你可以随意修改里面的逻辑（比如对比 fls vs for 循环）
 */
static noinline int my_damon_hot_score(struct damon_ctx *c, struct damon_region *r,
			struct damos *s)
{
	int freq_subscore;
	unsigned int age_in_sec;
	int age_in_log, age_subscore;
	unsigned int freq_weight = s->quota.weight_nr_accesses;
	unsigned int age_weight = s->quota.weight_age;
	int hotness;

	freq_subscore = r->nr_accesses * DAMON_MAX_SUBSCORE /
			damon_max_nr_accesses(&c->attrs);

	age_in_sec = (unsigned long)r->age * c->attrs.aggr_interval / 1000000;

	// for (age_in_log = 0; age_in_log < DAMON_MAX_AGE_IN_LOG && age_in_sec;
	//      age_in_log++, age_in_sec >>= 1)
	// 	;

	// 这里是你想测试的新逻辑
	age_in_log = min_t(int, fls(age_in_sec), DAMON_MAX_AGE_IN_LOG);

	if (freq_subscore == 0)
		age_in_log *= -1;

	age_in_log += DAMON_MAX_AGE_IN_LOG;
	age_subscore = age_in_log * DAMON_MAX_SUBSCORE / DAMON_MAX_AGE_IN_LOG / 2;

	hotness = (freq_weight * freq_subscore + age_weight * age_subscore);
	if (freq_weight + age_weight)
		hotness /= freq_weight + age_weight;

	hotness = hotness * DAMOS_MAX_SCORE / DAMON_MAX_SUBSCORE;
	return hotness;
}

static int __init damon_perf_test_init(void)
{
	struct damon_ctx c = {0};
	struct damon_region r = {0};
	struct damos s = {0};
	ktime_t start, end;
	int i;
	volatile int score;
	const int loops = 10000000; // 跑一千万次

	pr_info("DAMON Perf Test: Starting performance test...\n");

	// 初始化测试数据
	c.attrs.aggr_interval = 1000000; // 1s
	c.attrs.sample_interval = 5000;
	c.attrs.aggr_interval = 100000;
	// 假设最大访问次数为 100
	c.attrs.min_nr_accesses = 0;
	c.attrs.max_nr_accesses = 100;

	r.nr_accesses = 50;
	r.age = 1000;
	s.quota.weight_nr_accesses = 50;
	s.quota.weight_age = 50;

	// 开始计时
	start = ktime_get();
	for (i = 0; i < loops; i++) {
		score = my_damon_hot_score(&c, &r, &s);
	}
	end = ktime_get();

	pr_info("DAMON Perf Test: %d loops took %lld ns\n", 
		loops, ktime_to_ns(ktime_sub(end, start)));
	pr_info("DAMON Perf Test: Average latency: %lld ns\n", 
		ktime_to_ns(ktime_sub(end, start)) / loops);

	return 0;
}

static void __exit damon_perf_test_exit(void)
{
	pr_info("DAMON Perf Test: Module unloaded.\n");
}

module_init(damon_perf_test_init);
module_exit(damon_perf_test_exit);
MODULE_LICENSE("GPL");
