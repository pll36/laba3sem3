// Бенчмарки структур данных (Google Benchmark).
// Запуск: ./bench_ds [--benchmark_filter=Array] [--benchmark_format=json]
#include <benchmark/benchmark.h>

#include <sstream>
#include <string>

#include "ds/CompleteBinaryTree.h"
#include "ds/DoublyLinkedList.h"
#include "ds/DynamicArray.h"
#include "ds/HashTable.h"
#include "ds/Queue.h"
#include "ds/SinglyLinkedList.h"
#include "ds/Stack.h"

namespace
{

// ---- Массив ----
void ArrayPushBack(benchmark::State& state)
{
    for (auto _ : state)
    {
        ds::DynamicArray array;
        for (int i = 0; i < state.range(0); ++i)
        {
            array.pushBack("value");
        }
        benchmark::DoNotOptimize(array.size());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(ArrayPushBack)->RangeMultiplier(4)->Range(64, 1 << 14)->Complexity();

void ArrayPushFront(benchmark::State& state)
{
    for (auto _ : state)
    {
        ds::DynamicArray array;
        for (int i = 0; i < state.range(0); ++i)
        {
            array.pushFront("value");
        }
        benchmark::DoNotOptimize(array.size());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(ArrayPushFront)->RangeMultiplier(4)->Range(64, 1 << 12)->Complexity();

void ArrayRandomAccess(benchmark::State& state)
{
    ds::DynamicArray array;
    for (int i = 0; i < state.range(0); ++i)
    {
        array.pushBack(std::to_string(i));
    }
    std::size_t index = 0;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(array.get(index));
        index = (index + 7) % array.size();
    }
}
BENCHMARK(ArrayRandomAccess)->Arg(1 << 10)->Arg(1 << 14);

// ---- Списки: добавление в конец (односвязный - O(N), двусвязный - O(1)) ----
void SinglyListPushBack(benchmark::State& state)
{
    for (auto _ : state)
    {
        ds::SinglyLinkedList list;
        for (int i = 0; i < state.range(0); ++i)
        {
            list.pushBack("value");
        }
        benchmark::DoNotOptimize(list.size());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(SinglyListPushBack)->RangeMultiplier(4)->Range(64, 1 << 12)->Complexity();

void SinglyListPushFront(benchmark::State& state)
{
    for (auto _ : state)
    {
        ds::SinglyLinkedList list;
        for (int i = 0; i < state.range(0); ++i)
        {
            list.pushFront("value");
        }
        benchmark::DoNotOptimize(list.size());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(SinglyListPushFront)->RangeMultiplier(4)->Range(64, 1 << 14)->Complexity();

void DoublyListPushBack(benchmark::State& state)
{
    for (auto _ : state)
    {
        ds::DoublyLinkedList list;
        for (int i = 0; i < state.range(0); ++i)
        {
            list.pushBack("value");
        }
        benchmark::DoNotOptimize(list.size());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(DoublyListPushBack)->RangeMultiplier(4)->Range(64, 1 << 14)->Complexity();

void DoublyListSearch(benchmark::State& state)
{
    ds::DoublyLinkedList list;
    for (int i = 0; i < state.range(0); ++i)
    {
        list.pushBack(std::to_string(i));
    }
    const std::string last = std::to_string(state.range(0) - 1);
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(list.contains(last));
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(DoublyListSearch)->RangeMultiplier(4)->Range(64, 1 << 14)->Complexity();

// ---- Стек и очередь ----
void StackPushPop(benchmark::State& state)
{
    for (auto _ : state)
    {
        ds::Stack stack;
        for (int i = 0; i < state.range(0); ++i)
        {
            stack.push("value");
        }
        while (!stack.empty())
        {
            benchmark::DoNotOptimize(stack.pop());
        }
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(StackPushPop)->RangeMultiplier(4)->Range(64, 1 << 14)->Complexity();

void QueuePushPop(benchmark::State& state)
{
    for (auto _ : state)
    {
        ds::Queue queue;
        for (int i = 0; i < state.range(0); ++i)
        {
            queue.push("value");
        }
        while (!queue.empty())
        {
            benchmark::DoNotOptimize(queue.pop());
        }
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(QueuePushPop)->RangeMultiplier(4)->Range(64, 1 << 14)->Complexity();

// ---- Дерево: вставка и поиск - O(N) (обход в ширину) ----
void TreeInsert(benchmark::State& state)
{
    for (auto _ : state)
    {
        ds::CompleteBinaryTree tree;
        for (int i = 0; i < state.range(0); ++i)
        {
            tree.insert(i);
        }
        benchmark::DoNotOptimize(tree.size());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(TreeInsert)->RangeMultiplier(4)->Range(64, 1 << 11)->Complexity();

void TreeContains(benchmark::State& state)
{
    ds::CompleteBinaryTree tree;
    for (int i = 0; i < state.range(0); ++i)
    {
        tree.insert(i);
    }
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(tree.contains(-1)); // худший случай: полный обход
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(TreeContains)->RangeMultiplier(4)->Range(64, 1 << 14)->Complexity();

// ---- Хеш-таблица: линейное пробирование против двойного хеширования ----
// Аргумент - коэффициент заполнения в процентах при ёмкости 10007
void HashFill(benchmark::State& state, ds::HashTable::Mode mode)
{
    constexpr std::size_t capacity = 10007;
    const auto keys = static_cast<int>(capacity * static_cast<std::size_t>(state.range(0)) / 100);
    for (auto _ : state)
    {
        ds::HashTable table(mode, capacity);
        for (int i = 0; i < keys; ++i)
        {
            table.insert(i * 1000 + 7); // ключи с близкой свёрткой - много коллизий
        }
        benchmark::DoNotOptimize(table.size());
    }
}
void HashLinearFill(benchmark::State& state)
{
    HashFill(state, ds::HashTable::Mode::Linear);
}
void HashDoubleFill(benchmark::State& state)
{
    HashFill(state, ds::HashTable::Mode::Double);
}
BENCHMARK(HashLinearFill)->Arg(30)->Arg(60)->Arg(90);
BENCHMARK(HashDoubleFill)->Arg(30)->Arg(60)->Arg(90);

// ---- Сериализация: бинарный формат против текстового ----
ds::DynamicArray makeArray(int count)
{
    ds::DynamicArray array;
    for (int i = 0; i < count; ++i)
    {
        array.pushBack("element number " + std::to_string(i));
    }
    return array;
}

void ArraySaveBinary(benchmark::State& state)
{
    const ds::DynamicArray array = makeArray(static_cast<int>(state.range(0)));
    for (auto _ : state)
    {
        std::ostringstream out(std::ios::binary);
        array.saveBinary(out);
        benchmark::DoNotOptimize(out.str().size());
    }
}
BENCHMARK(ArraySaveBinary)->Arg(1000)->Arg(10000);

void ArraySaveText(benchmark::State& state)
{
    const ds::DynamicArray array = makeArray(static_cast<int>(state.range(0)));
    for (auto _ : state)
    {
        std::ostringstream out;
        array.saveText(out);
        benchmark::DoNotOptimize(out.str().size());
    }
}
BENCHMARK(ArraySaveText)->Arg(1000)->Arg(10000);

void ArrayLoadBinary(benchmark::State& state)
{
    std::ostringstream out(std::ios::binary);
    makeArray(static_cast<int>(state.range(0))).saveBinary(out);
    const std::string bytes = out.str();
    for (auto _ : state)
    {
        std::istringstream in(bytes, std::ios::binary);
        ds::DynamicArray array;
        array.loadBinary(in);
        benchmark::DoNotOptimize(array.size());
    }
}
BENCHMARK(ArrayLoadBinary)->Arg(1000)->Arg(10000);

void ArrayLoadText(benchmark::State& state)
{
    std::ostringstream out;
    makeArray(static_cast<int>(state.range(0))).saveText(out);
    const std::string text = out.str();
    for (auto _ : state)
    {
        std::istringstream in(text);
        ds::DynamicArray array;
        array.loadText(in);
        benchmark::DoNotOptimize(array.size());
    }
}
BENCHMARK(ArrayLoadText)->Arg(1000)->Arg(10000);

} // namespace

BENCHMARK_MAIN();
