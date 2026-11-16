#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
mkdir -p build
compiler="${CXX:-g++}"
for name in 01_threads 02_race 03_mutex_atomic 04_async 05_stop_token 06_platform generate_logs logscan; do
    optimization=-O2
    [[ "$name" == 02_race ]] && optimization=-O0
    "$compiler" -std=c++20 "$optimization" -pthread -Wall -Wextra -Wpedantic "src/$name.cpp" -o "build/$name"
done
printf 'Build complete.\n'
