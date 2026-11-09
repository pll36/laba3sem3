#!/usr/bin/env bash
# Сборка (Release) и запуск бенчмарков; результаты также в bench_result.json
set -e
cd "$(dirname "$0")/.."
cmake -S . -B build-bench -G Ninja -DCMAKE_BUILD_TYPE=Release -DLAB3_BUILD_TESTS=OFF
cmake --build build-bench --target bench_ds
./build-bench/bench_ds --benchmark_out=bench_result.json --benchmark_out_format=json "$@"
