#!/bin/bash

cd /sys/module/damon_lru_sort/parameters
echo 500 > hot_thres_access_freq
echo 120000000 > cold_min_age
echo 10 > quota_ms
echo 1000 > quota_reset_interval_ms
echo 500 > wmarks_high
echo 400 > wmarks_mid
echo 200 > wmarks_low
echo Y > enabled
