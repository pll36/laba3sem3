#!/usr/bin/env bash
# Запуск линтеров: cppcheck и clang-tidy (из каталога cpp/)
set -e
cd "$(dirname "$0")/.."
cppcheck --enable=warning,style,performance,portability --std=c++20 --inline-suppr \
         --suppress=missingIncludeSystem --error-exitcode=1 -Iinclude src app
cmake -S . -B build-lint -G Ninja -DLAB3_BUILD_TESTS=OFF -DLAB3_BUILD_BENCH=OFF -DLAB3_CLANG_TIDY=ON
cmake --build build-lint 2>&1 | tee build-lint/tidy.log
if grep -q "warning:" build-lint/tidy.log; then echo "clang-tidy: найдены замечания"; exit 1; fi
echo "Линтеры: замечаний нет"
