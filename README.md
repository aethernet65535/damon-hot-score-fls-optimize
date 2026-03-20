# Result and Environment
## My Environment
### Hardware
CPU: AMD Ryzen 5 5600H  
CPUs: 12    
RAM: 16GB DDR4  

### Software
VM-CPUs: 1  
VM-RAM: 1GB 
KERNEL: v7.0.0-rc4 x86-64   
GCC: gcc (GCC) 15.2.1 20260209  
CLANG: clang version 22.1.1 

## The result on my machine
The detailed results are here [1]; I will include a portion of them
here.
```
# fls

DAMON Perf Test: Starting 10000000 iterations
=============================================
 Total Iterations : 10000000
 Average Latency  : 1 ns
 P95 Latency      : 41 ns
 P99 Latency      : 41 ns
---------------------------------------------
 Range (ns)      | Count        | Percent
---------------------------------------------
 0-19            | 0            |      0%
 20-39           | 2762000      |     27%
 40-59           | 7237000      |     72%
 60-79           | 1000         |      0%
 80-99           | 0            |      0%
 100+            | 0            |      0%
=============================================
```
```
# ilog2

DAMON Perf Test: Starting 10000000 iterations
=============================================
 Total Iterations : 10000000
 Average Latency  : 1 ns
 P95 Latency      : 41 ns
 P99 Latency      : 41 ns
---------------------------------------------
 Range (ns)      | Count        | Percent
---------------------------------------------
 0-19            | 0            |      0%
 20-39           | 2625000      |     26%
 40-59           | 7374000      |     73%
 60-79           | 0            |      0%
 80-99           | 0            |      0%
 100+            | 1000         |      0%
=============================================
```
```
# for-loop

DAMON Perf Test: Starting 10000000 iterations
=============================================
 Total Iterations : 10000000
 Average Latency  : 12 ns
 P95 Latency      : 51 ns
 P99 Latency      : 60 ns
---------------------------------------------
 Range (ns)      | Count        | Percent
---------------------------------------------
 0-19            | 0            |      0%
 20-39           | 0            |      0%
 40-59           | 9862000      |     98%
 60-79           | 135000       |      1%
 80-99           | 1000         |      0%
 100+            | 2000         |      0%
=============================================
```

[1] https://github.com/aethernet65535/damon-hot-score-fls-optimize/tree/master/result-raw

# Usage
## Quick Reproduction
Copy the command here:
```sh
# host machine
sudo cpupower frequency-set -g performance
sync; echo 3 | sudo tee /proc/sys/vm/drop_caches
vng --cpu 1 --memory 2G --verbose

# vitrual machine
git clone https://github.com/aethernet65535/damon-hot-score-fls-optimize.git
cd damon-hot-score-fls-optimize
sudo insmod test-kernel-module/fls.ko
```

### Export Symbol
Due to this test kernel module uses some functions that are not exported
by default in the kernel, you must ensure that 'damon_new_ctx' and
'damon_destroy_ctx' have been exported in your kernel, or you can try
using this patch. [1]

[1] https://github.com/aethernet65535/damon-hot-score-fls-optimize/blob/master/test-kernel-module/0001-damon-hot-score-fls-optimize.patch

### How to change the algorithm?
Simply come here, uncomment the algorithms you want, and comment out the ones you don't.
```c
/* ------------------------------------------------------------------------- */
/* Algorithm A: Original For Loop */
// for (age_in_log = 0; age_in_log < MY_DAMON_MAX_AGE_IN_LOG && age_in_sec;
//      age_in_log++, age_in_sec >>= 1)
// 	;

/* Algorithm B: fls */
// age_in_log = min_t(int, fls(age_in_sec), MY_DAMON_MAX_AGE_IN_LOG);
/* ------------------------------------------------------------------------- */
```
