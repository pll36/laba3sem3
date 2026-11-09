#include <catch2/catch_test_macros.hpp>

#include <climits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../common/TestUtils.h"
#include "ds/HashTable.h"
#include "ds/Serialization.h"

using ds::HashTable;

TEST_CASE("folding hash sums groups of three digits", "[hash]")
{
    // пример из задания: 523 + 456 + 795 = 1774
    CHECK(HashTable::foldingHash(523456795, 100000) == 1774);
    CHECK(HashTable::foldingHash(523456795, 1000) == 774);
    CHECK(HashTable::foldingHash(0, 11) == 0);
    CHECK(HashTable::foldingHash(5, 11) == 5);
    CHECK(HashTable::foldingHash(1234, 1000) == 235);  // 1 + 234
    CHECK(HashTable::foldingHash(-1234, 1000) == 235); // знак игнорируется
    CHECK(HashTable::foldingHash(INT_MIN, 1000) == (2 + 147 + 483 + 648) % 1000);
    CHECK(HashTable::foldingHash(42, 0) == 0);
}

TEST_CASE("new table is empty and reports its parameters", "[hash]")
{
    const HashTable table;
    CHECK(table.empty());
    CHECK(table.capacity() == HashTable::kDefaultCapacity);
    CHECK(table.mode() == HashTable::Mode::Linear);
    CHECK(table.loadFactor() == 0.0);
    CHECK(table.type() == ds::StructureType::Hash);
    CHECK_THROWS_AS(HashTable(HashTable::Mode::Linear, 0), std::invalid_argument);
}

TEST_CASE("insert, contains and erase work in both modes", "[hash]")
{
    for (const auto mode : {HashTable::Mode::Linear, HashTable::Mode::Double})
    {
        HashTable table(mode, 13);
        CHECK(table.insert(15));
        CHECK(table.insert(28)); // 28 % 13 == 2, 15 % 13 == 2 -> коллизия
        CHECK(table.contains(15));
        CHECK(table.contains(28));
        CHECK_FALSE(table.contains(41));
        CHECK(table.size() == 2);

        CHECK(table.erase(15));
        CHECK_FALSE(table.contains(15));
        CHECK(table.contains(28)); // поиск проходит через удалённую ячейку
        CHECK_FALSE(table.erase(15));
        CHECK(table.size() == 1);
    }
}

TEST_CASE("inserting an existing key does not change the size", "[hash]")
{
    HashTable table;
    CHECK(table.insert(5));
    CHECK(table.insert(5));
    CHECK(table.size() == 1);
}

TEST_CASE("deleted slots are reused", "[hash]")
{
    HashTable table(HashTable::Mode::Linear, 5);
    for (const int key : {1, 2, 3, 4, 0})
    {
        REQUIRE(table.insert(key));
    }
    REQUIRE(table.erase(3));
    CHECK(table.insert(8)); // свободна только удалённая ячейка (8 % 5 == 3)
    CHECK(table.contains(8));
    CHECK(table.size() == 5);
}

TEST_CASE("insert reuses first deleted slot when probing passes it", "[hash]")
{
    HashTable table(HashTable::Mode::Linear, 7);
    table.insert(1);
    table.insert(8);         // коллизия с 1 -> ячейка 2
    table.insert(15);        // -> ячейка 3
    table.erase(8);          // ячейка 2 помечена удалённой
    CHECK(table.insert(22)); // 22 % 7 == 1, проход через 1, 2(deleted), 3 -> пустая 4; берём 2
    CHECK(table.contains(22));
    CHECK(table.contains(15));
    CHECK(table.contains(1));
}

TEST_CASE("full table rejects new keys", "[hash]")
{
    for (const auto mode : {HashTable::Mode::Linear, HashTable::Mode::Double})
    {
        HashTable table(mode, 4);
        for (int key = 0; key < 4; ++key)
        {
            REQUIRE(table.insert(key));
        }
        CHECK(table.loadFactor() == 1.0);
        CHECK_FALSE(table.insert(100));
        CHECK_FALSE(table.contains(100)); // проверены все ячейки
        CHECK_FALSE(table.erase(100));
        CHECK(table.contains(3));
        CHECK(table.size() == 4);
    }
}

TEST_CASE("tables of capacity one and two work", "[hash]")
{
    for (const std::size_t capacity : {std::size_t{1}, std::size_t{2}})
    {
        HashTable table(HashTable::Mode::Double, capacity);
        CHECK(table.insert(10));
        CHECK(table.contains(10));
        CHECK(table.erase(10));
    }
}

TEST_CASE("all keys remain reachable with many collisions", "[hash]")
{
    for (const auto mode : {HashTable::Mode::Linear, HashTable::Mode::Double})
    {
        HashTable table(mode, 101);
        std::set<int> keys;
        for (int i = 0; i < 90; ++i)
        {
            const int key = i * 1000 + 7; // свёртка даёт похожие значения -> много коллизий
            keys.insert(key);
            REQUIRE(table.insert(key));
        }
        for (const int key : keys)
        {
            CHECK(table.contains(key));
        }
        CHECK(table.size() == keys.size());
    }
}

TEST_CASE("double hashing needs fewer probes on clustered keys", "[hash]")
{
    HashTable linear(HashTable::Mode::Linear, 101);
    HashTable doubled(HashTable::Mode::Double, 101);
    // ключи a*1000 + (500 - a) сворачиваются в одно значение 500 -> все коллизируют в одной ячейке
    auto key = [](int a) { return a * 1000 + (500 - a); };
    for (int a = 0; a < 60; ++a)
    {
        REQUIRE(HashTable::foldingHash(key(a), 101) == HashTable::foldingHash(key(0), 101));
        linear.insert(key(a));
        doubled.insert(key(a));
    }
    std::size_t linearProbes = 0;
    std::size_t doubleProbes = 0;
    for (int a = 0; a < 60; ++a)
    {
        linearProbes += linear.probeCount(key(a));
        doubleProbes += doubled.probeCount(key(a));
    }
    CHECK(doubleProbes < linearProbes);
    CHECK(linear.probeCount(-12345) >= 1);
}

TEST_CASE("clear keeps capacity and mode", "[hash]")
{
    HashTable table(HashTable::Mode::Double, 17);
    table.insert(1);
    table.insert(2);
    table.clear();
    CHECK(table.empty());
    CHECK(table.capacity() == 17);
    CHECK(table.mode() == HashTable::Mode::Double);
    CHECK_FALSE(table.contains(1));
}

TEST_CASE("toString lists occupied slots", "[hash]")
{
    HashTable table(HashTable::Mode::Linear, 11);
    table.insert(3);
    table.insert(5);
    table.insert(9);
    table.erase(5);
    CHECK(table.toString() == "Hash(linear, capacity=11, size=2)\n[3] 3\n[9] 9");
    CHECK(HashTable(HashTable::Mode::Double, 7).toString() == "Hash(double, capacity=7, size=0)");
}

TEST_CASE("copy keeps the layout", "[hash]")
{
    HashTable original(HashTable::Mode::Double, 11);
    original.insert(4);
    original.insert(15);
    HashTable copy(original);
    copy.insert(99);
    CHECK(original.size() == 2);
    CHECK(copy.size() == 3);
    CHECK(copy.mode() == HashTable::Mode::Double);
}

TEST_CASE("serialization preserves slots including deleted ones", "[hash][io]")
{
    HashTable table(HashTable::Mode::Double, 11);
    for (const int key : {1, 12, 23, -7, 523456795})
    {
        table.insert(key);
    }
    table.erase(12);

    const HashTable fromBinary = testutil::roundTripBinary(table);
    const HashTable fromText = testutil::roundTripText(table);
    for (const HashTable* restored : {&fromBinary, &fromText})
    {
        CHECK(restored->toString() == table.toString());
        CHECK(restored->mode() == HashTable::Mode::Double);
        CHECK(restored->capacity() == 11);
        CHECK(restored->size() == table.size());
        CHECK(restored->contains(23)); // найден через удалённую ячейку
        CHECK_FALSE(restored->contains(12));
    }
}

TEST_CASE("text format layout", "[hash][io]")
{
    HashTable table(HashTable::Mode::Linear, 3);
    table.insert(1);
    table.insert(2);
    table.erase(2);
    CHECK(testutil::textOf(table) == "DSTXT 1\nTYPE hash\nMODE linear\nCAPACITY 3\nE 0\nO 1\nD 2\n");
}

TEST_CASE("corrupted hash data is rejected", "[hash][io]")
{
    HashTable table;
    table.insert(77);
    const std::string before = table.toString();

    auto loadText = [&table](const std::string& body)
    {
        std::istringstream in("DSTXT 1\nTYPE hash\n" + body);
        table.loadText(in);
    };
    CHECK_THROWS_AS(loadText("MODE triple\nCAPACITY 1\nE 0\n"), ds::SerializationError);
    CHECK_THROWS_AS(loadText("MODE linear\nCAPACITY 0\n"), ds::SerializationError);
    CHECK_THROWS_AS(loadText("MODE linear\nCAPACITY 2\nE 0\n"), ds::SerializationError); // не хватает строк
    CHECK_THROWS_AS(loadText("MODE linear\nCAPACITY 1\nX 0\n"), ds::SerializationError); // неизвестное состояние
    CHECK_THROWS_AS(loadText("MODE linear\nCAPACITY 1\nO\n"), ds::SerializationError);   // нет ключа
    CHECK_THROWS_AS(loadText("MODE linear\nCAPACITY 1\nO x\n"), ds::SerializationError); // ключ не число
    CHECK(table.toString() == before);

    auto loadBinary = [&table](const std::string& bytes)
    {
        std::istringstream in(std::string("DSB1") + '\x07' + bytes, std::ios::binary);
        table.loadBinary(in);
    };
    CHECK_THROWS_AS(loadBinary(std::string("\x05\x01\x00\x00\x00", 5)), ds::SerializationError); // режим 5
    CHECK_THROWS_AS(loadBinary(std::string("\x00\x00\x00\x00\x00", 5)), ds::SerializationError); // ёмкость 0
    CHECK_THROWS_AS(loadBinary(std::string("\x00\x01\x00\x00\x00\x09\x00\x00\x00\x00", 10)),
                    ds::SerializationError); // состояние 9
    CHECK_THROWS_AS(loadBinary(std::string("\x00\x02\x00\x00\x00\x01", 6)), ds::SerializationError); // обрыв данных
    CHECK(table.toString() == before);
}
