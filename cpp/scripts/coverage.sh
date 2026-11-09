#!/usr/bin/env bash
# Сборка с покрытием, запуск всех тестов и HTML-отчёт: build-cov/coverage/index.html
set -e
cd "$(dirname "$0")/.."
cmake -S . -B build-cov -G Ninja -DCMAKE_BUILD_TYPE=Debug -DLAB3_COVERAGE=ON -DLAB3_BUILD_BENCH=OFF
cmake --build build-cov --target coverage
echo "Отчёт: $(pwd)/build-cov/coverage/index.html"
