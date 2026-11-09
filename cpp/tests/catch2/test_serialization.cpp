#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>
#include <vector>

#include "../common/TestUtils.h"
#include "ds/CompleteBinaryTree.h"
#include "ds/DoublyLinkedList.h"
#include "ds/DynamicArray.h"
#include "ds/HashTable.h"
#include "ds/Queue.h"
#include "ds/Serialization.h"
#include "ds/SinglyLinkedList.h"
#include "ds/Stack.h"
#include "ds/StructureFactory.h"

using namespace ds;

TEST_CASE("binary integers are little-endian", "[io]")
{
    std::ostringstream out(std::ios::binary);
    io::writeU32(out, 0x01020304U);
    io::writeI32(out, -2);
    io::writeU8(out, 0xAB);
    CHECK(out.str() == std::string("\x04\x03\x02\x01\xFE\xFF\xFF\xFF\xAB", 9));

    std::istringstream in(out.str(), std::ios::binary);
    CHECK(io::readU32(in) == 0x01020304U);
    CHECK(io::readI32(in) == -2);
    CHECK(io::readU8(in) == 0xAB);
    CHECK_THROWS_AS(io::readU8(in), SerializationError);
}

TEST_CASE("binary strings round trip", "[io]")
{
    const std::string big(10000, 'x'); // больше размера внутреннего буфера чтения
    std::ostringstream out(std::ios::binary);
    io::writeString(out, "");
    io::writeString(out, "abc");
    io::writeString(out, big);
    std::istringstream in(out.str(), std::ios::binary);
    CHECK(io::readString(in).empty());
    CHECK(io::readString(in) == "abc");
    CHECK(io::readString(in) == big);
}

TEST_CASE("string length larger than the data is rejected without huge allocation", "[io]")
{
    std::ostringstream out(std::ios::binary);
    io::writeU32(out, 0xFFFFFFFFU);
    out << "abc";
    std::istringstream in(out.str(), std::ios::binary);
    CHECK_THROWS_AS(io::readString(in), SerializationError);
}

TEST_CASE("sequence of strings round trips in both formats", "[io]")
{
    const std::vector<std::string> values = {"a", "", "b c", "x\ny"};
    std::stringstream binary(std::ios::in | std::ios::out | std::ios::binary);
    io::writeStrings(binary, values);
    CHECK(io::readStrings(binary) == values);

    std::stringstream text;
    io::writeStringsText(text, values);
    CHECK(text.str() == "COUNT 4\na\n\nb c\nx\\ny\n");
    CHECK(io::readStringsText(text) == values);
}

TEST_CASE("escaping covers backslash, newline and carriage return", "[io]")
{
    const std::string raw = "a\\b\nc\rd";
    const std::string escaped = io::escapeLine(raw);
    CHECK(escaped == "a\\\\b\\nc\\rd");
    CHECK(io::unescapeLine(escaped) == raw);
    CHECK(io::unescapeLine("plain") == "plain");
    CHECK_THROWS_AS(io::unescapeLine("bad\\"), SerializationError);
    CHECK_THROWS_AS(io::unescapeLine("bad\\x"), SerializationError);
}

TEST_CASE("readLine strips carriage return and reports end of data", "[io]")
{
    std::istringstream in("first\r\nsecond\n");
    CHECK(io::readLine(in) == "first");
    CHECK(io::readLine(in) == "second");
    CHECK_THROWS_AS(io::readLine(in), SerializationError);
}

TEST_CASE("readField checks the key", "[io]")
{
    std::istringstream in("COUNT 5\nCOUNT\nCOUNTER 1\nOTHER 1\n");
    CHECK(io::readField(in, "COUNT") == "5");
    CHECK(io::readField(in, "COUNT").empty());
    CHECK_THROWS_AS(io::readField(in, "COUNT"), SerializationError); // COUNTER - другой ключ
    CHECK_THROWS_AS(io::readField(in, "COUNT"), SerializationError); // OTHER
}

TEST_CASE("number parsing is strict", "[io]")
{
    CHECK(io::parseInt32("-15") == -15);
    CHECK(io::parseInt32("2147483647") == 2147483647);
    CHECK(io::parseUint32("4294967295") == 4294967295U);
    CHECK_THROWS_AS(io::parseInt32(""), SerializationError);
    CHECK_THROWS_AS(io::parseInt32("12x"), SerializationError);
    CHECK_THROWS_AS(io::parseInt32("2147483648"), SerializationError);
    CHECK_THROWS_AS(io::parseUint32("-1"), SerializationError);
    CHECK_THROWS_AS(io::parseUint32(" 1"), SerializationError);
}

TEST_CASE("type names are bidirectional", "[io]")
{
    for (const auto type : {StructureType::Array, StructureType::SinglyList, StructureType::DoublyList,
                            StructureType::Stack, StructureType::Queue, StructureType::Tree, StructureType::Hash})
    {
        CHECK(parseTypeName(typeName(type)) == type);
    }
    CHECK_THROWS_AS(parseTypeName("graph"), SerializationError);
    CHECK(std::string(typeName(static_cast<StructureType>(99))) == "unknown");
}

TEST_CASE("headers identify the structure type", "[io]")
{
    Stack stack;
    stack.push("x");
    std::istringstream binary(testutil::binaryOf(stack), std::ios::binary);
    CHECK(readBinaryType(binary) == StructureType::Stack);
    std::istringstream text(testutil::textOf(stack));
    CHECK(readTextType(text) == StructureType::Stack);

    std::istringstream badTag(std::string("DSB1\x63", 5), std::ios::binary);
    CHECK_THROWS_AS(readBinaryType(badTag), SerializationError);
    std::istringstream shortMagic("DS", std::ios::binary);
    CHECK_THROWS_AS(readBinaryType(shortMagic), SerializationError);
    std::istringstream badText("HELLO\nTYPE stack\n");
    CHECK_THROWS_AS(readTextType(badText), SerializationError);
    std::istringstream badTextType("DSTXT 1\nTYPE graph\n");
    CHECK_THROWS_AS(readTextType(badTextType), SerializationError);
}

TEST_CASE("factory creates every structure type", "[io]")
{
    for (const auto type : {StructureType::Array, StructureType::SinglyList, StructureType::DoublyList,
                            StructureType::Stack, StructureType::Queue, StructureType::Tree, StructureType::Hash})
    {
        const std::unique_ptr<IStructure> item = makeStructure(type);
        REQUIRE(item != nullptr);
        CHECK(item->type() == type);
        CHECK(item->empty());
    }
    CHECK(makeStructure(static_cast<StructureType>(99)) == nullptr);
}

TEST_CASE("every structure can be saved and loaded polymorphically", "[io]")
{
    DynamicArray array;
    array.pushBack("1");
    SinglyLinkedList slist;
    slist.pushBack("2");
    DoublyLinkedList dlist;
    dlist.pushBack("3");
    Stack stack;
    stack.push("4");
    Queue queue;
    queue.push("5");
    CompleteBinaryTree tree;
    tree.insert(6);
    HashTable hash;
    hash.insert(7);

    const std::vector<const IStructure*> items = {&array, &slist, &dlist, &stack, &queue, &tree, &hash};
    for (const IStructure* item : items)
    {
        std::stringstream binary(std::ios::in | std::ios::out | std::ios::binary);
        item->saveBinary(binary);
        auto fromBinary = makeStructure(item->type());
        fromBinary->loadBinary(binary);
        CHECK(fromBinary->toString() == item->toString());

        std::stringstream text;
        item->saveText(text);
        auto fromText = makeStructure(item->type());
        fromText->loadText(text);
        CHECK(fromText->toString() == item->toString());
    }
}

TEST_CASE("loading into a structure of another type fails", "[io]")
{
    Stack stack;
    stack.push("x");
    HashTable hash;
    std::istringstream binary(testutil::binaryOf(stack), std::ios::binary);
    CHECK_THROWS_AS(hash.loadBinary(binary), SerializationError);
    std::istringstream text(testutil::textOf(stack));
    CHECK_THROWS_AS(hash.loadText(text), SerializationError);
}

TEST_CASE("writing to a failed stream reports an error", "[io]")
{
    std::ostringstream out;
    out.setstate(std::ios::badbit);
    CHECK_THROWS_AS(io::writeU8(out, 1), SerializationError);
    CHECK_THROWS_AS(io::writeString(out, "abc"), SerializationError);
    CHECK_THROWS_AS(io::writeStringsText(out, {"abc"}), SerializationError);
}
