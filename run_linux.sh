#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
bash build_linux.sh
mkdir -p results
for name in 01_threads 02_race 03_mutex_atomic 04_async 05_stop_token 06_platform; do
    printf '\n=== %s ===\n' "$name"
    "./build/$name"
done
if [[ ! -d logs ]]; then ./build/generate_logs logs; fi
./build/logscan logs ERROR 7 | tee results/benchmark_run.txt
