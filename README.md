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
# fls-result.txt

[    4.751571] DAMON Perf Test: Starting 10000000 iterations...
[    4.770561] =============================================
[    4.770669]  Total Iterations : 10000000
[    4.770754]  Average Latency  : 1 ns
[    4.770818]  P95 Latency      : 40 ns
[    4.770875]  P99 Latency      : 41 ns
[    4.770994] ---------------------------------------------
[    4.771081]  Range (ns)      | Count    | Percent   
[    4.771153] ---------------------------------------------
[    4.771236]  20-39           | 3522000  |   35%
[    4.771307]  40-59           | 6478000  |   64%
[    4.771377]  60-79           | 0        |    0%
[    4.771446] =============================================
```
```
# loop-result.txt

[   11.495899] DAMON Perf Test: Starting 10000000 iterations...
[   11.596104] =============================================
[   11.596217]  Total Iterations : 10000000
[   11.596571]  Average Latency  : 9 ns
[   11.596680]  P95 Latency      : 51 ns
[   11.596766]  P99 Latency      : 60 ns
[   11.596835] ---------------------------------------------
[   11.596910]  Range (ns)      | Count    | Percent   
[   11.597009] ---------------------------------------------
[   11.597084]  20-39           | 0        |    0%
[   11.597157]  40-59           | 9894000  |   98%
[   11.597228]  60-79           | 98000    |    0%
[   11.597297] =============================================
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
