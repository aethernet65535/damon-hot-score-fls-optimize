# How I Test
Follow this:
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
