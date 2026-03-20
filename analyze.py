import sys
import re
from collections import defaultdict

def analyze_raw_trace(filename):
    latencies = []
    buckets = defaultdict(int)
    total_count = 0
    
    # Regex to capture the number between 'cost' and 'ns'
    # Works with: "fls cost 91 ns" or "loop cost 45 ns"
    pattern = re.compile(r"cost\s+(\d+)\s+ns")

    try:
        with open(filename, 'r') as f:
            for line in f:
                match = pattern.search(line)
                if match:
                    val = int(match.group(1))
                    latencies.append(val)
                    # Group by 20ns steps
                    bucket_start = (val // 20) * 20
                    buckets[bucket_start] += 1
                    total_count += 1
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        return

    if total_count == 0:
        print(f"Warning: No valid data found in '{filename}'.")
        return

    # Sort latencies for percentile calculation
    latencies.sort()
    p95 = latencies[int(total_count * 0.95)]
    p99 = latencies[int(total_count * 0.99)]
    avg = sum(latencies) / total_count

    # Print Summary Table
    print(f"\n{'='*45}")
    print(f" FILE: {filename}")
    print(f"{'='*45}")
    print(f" Total Samples : {total_count}")
    print(f" Average       : {avg:.2f} ns")
    print(f" P95 Latency   : {p95} ns")
    print(f" P99 Latency   : {p99} ns")
    print(f"{'-'*45}")
    print(f" {'Range (ns)':<15} | {'Count':<8} | {'Percent':<10}")
    print(f"{'-'*45}")
    
    for start_node in sorted(buckets.keys()):
        count = buckets[start_node]
        percentage = (count / total_count) * 100
        label = f"{start_node}-{start_node + 19}"
        print(f" {label:<15} | {count:<8} | {percentage:>6.2f}%")
    print(f"{'='*45}\n")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 analyze.py <file1> [file2 ...]")
    else:
        for arg in sys.argv[1:]:
            analyze_raw_trace(arg)
