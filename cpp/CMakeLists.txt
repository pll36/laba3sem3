cmake_minimum_required(VERSION 3.16)
project(Lab3DataStructures VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LAB3_BUILD_TESTS "Build unit tests (Boost.Test, GoogleTest, Catch2)" ON)
option(LAB3_BUILD_BENCH "Build benchmarks (Google Benchmark)" ON)
option(LAB3_COVERAGE "Instrument code for coverage (gcc/clang)" OFF)
option(LAB3_CLANG_TIDY "Run clang-tidy during the build" OFF)
option(LAB3_WERROR "Treat warnings as errors" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------- библиотека структур данных
add_library(ds STATIC
    src/Serialization.cpp
    src/IStructure.cpp
    src/DynamicArray.cpp
    src/SinglyLinkedList.cpp
    src/DoublyLinkedList.cpp
    src/Stack.cpp
    src/Queue.cpp
    src/CompleteBinaryTree.cpp
    src/HashTable.cpp
    src/StructureFactory.cpp
    src/StructureManager.cpp
    src/CommandLog.cpp)
target_include_directories(ds PUBLIC include)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ds PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(LAB3_WERROR)
        target_compile_options(ds PRIVATE -Werror)
    endif()
endif()

if(LAB3_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    set_target_properties(ds PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

if(LAB3_COVERAGE)
    # -O0 и --coverage нужны, чтобы счётчики строк были точными
    target_compile_options(ds PUBLIC --coverage -O0 -g)
    target_link_options(ds PUBLIC --coverage)
endif()

# ---------------------------------------------------------------- консольное приложение
add_executable(lab3_app app/main.cpp)
target_link_libraries(lab3_app PRIVATE ds)

# ---------------------------------------------------------------- тесты
if(LAB3_BUILD_TESTS)
    enable_testing()
    find_package(Boost REQUIRED COMPONENTS unit_test_framework)
    find_package(GTest REQUIRED)
    find_package(Catch2 3 REQUIRED)

    # 1) Boost.Test: массив, односвязный и двусвязный списки
    add_executable(test_boost
        tests/boost/test_main.cpp
        tests/boost/test_dynamic_array.cpp
        tests/boost/test_singly_list.cpp
        tests/boost/test_doubly_list.cpp)
    target_compile_definitions(test_boost PRIVATE BOOST_TEST_DYN_LINK)
    target_link_libraries(test_boost PRIVATE ds Boost::unit_test_framework)

    # 2) GoogleTest: стек, очередь, полное бинарное дерево
    add_executable(test_gtest
        tests/gtest/test_stack.cpp
        tests/gtest/test_queue.cpp
        tests/gtest/test_tree.cpp)
    target_link_libraries(test_gtest PRIVATE ds GTest::gtest_main)

    # 3) Catch2: хеш-таблица, сериализация, интерпретатор команд, журнал команд
    add_executable(test_catch2
        tests/catch2/test_hash_table.cpp
        tests/catch2/test_serialization.cpp
        tests/catch2/test_manager.cpp)
    target_link_libraries(test_catch2 PRIVATE ds Catch2::Catch2WithMain)

    add_test(NAME boost_tests COMMAND test_boost)
    add_test(NAME gtest_tests COMMAND test_gtest)
    add_test(NAME catch2_tests COMMAND test_catch2)

    # ----- HTML-отчёт о покрытии: cmake --build build --target coverage
    if(LAB3_COVERAGE)
        find_program(GCOVR_EXE NAMES gcovr REQUIRED)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_EXE} --root ${CMAKE_SOURCE_DIR}
                    --filter ${CMAKE_SOURCE_DIR}/src --filter ${CMAKE_SOURCE_DIR}/include
                    --object-directory ${CMAKE_BINARY_DIR}
                    --html-details ${CMAKE_BINARY_DIR}/coverage/index.html
                    --print-summary --fail-under-line 85
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS test_boost test_gtest test_catch2
            COMMENT "Running tests and generating HTML coverage report")
    endif()
endif()

# ---------------------------------------------------------------- бенчмарки
if(LAB3_BUILD_BENCH)
    find_package(benchmark REQUIRED)
    add_executable(bench_ds bench/bench_ds.cpp)
    target_link_libraries(bench_ds PRIVATE ds benchmark::benchmark)
endif()
