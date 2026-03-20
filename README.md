# Result and Environment
## My Environment
CPU: AMD Ryzen 5 5600H
RAM: 16GB DDR4

## The result on my machine
```
=============================================
 FILE: result-raw/fls/combined_file.txt
=============================================
 Total Samples : 1072
 Average       : 43.19 ns
 P95 Latency   : 60 ns
 P99 Latency   : 69 ns
---------------------------------------------
 Range (ns)      | Count    | Percent   
---------------------------------------------
 20-39           | 12       |   1.12%
 40-59           | 978      |  91.23%
 60-79           | 82       |   7.65%
=============================================

=============================================
 FILE: result-raw/loop/combined_file.txt
=============================================
 Total Samples : 674
 Average       : 39.99 ns
 P95 Latency   : 50 ns
 P99 Latency   : 70 ns
---------------------------------------------
 Range (ns)      | Count    | Percent   
---------------------------------------------
 20-39           | 124      |  18.40%
 40-59           | 526      |  78.04%
 60-79           | 19       |   2.82%
 80-99           | 3        |   0.45%
 100-119         | 1        |   0.15%
 200-219         | 1        |   0.15%
=============================================
```

# Usage
## How I test?
```sh
# on host machine
sudo cpupower frequency-set -g performance
sync; echo 3 | sudo tee /proc/sys/vm/drop_caches
vng --cpu 1 --memory 2G --verbose

# on VM (virtme-ng)
cdwork
cd tmp/damon
sudo ./lru_sort.sh
stress-ng --vm 4 --vm-bytes 70% --vm-hang 20 --timeout 10m
sudo cat /sys/kernel/tracing/trace | grep DAMON_BENCH
```

## How to get the result?
```sh
python analyze.py <file1> [file-2]
```
