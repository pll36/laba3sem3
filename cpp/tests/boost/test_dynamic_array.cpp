#include <boost/test/unit_test.hpp>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../common/TestUtils.h"
#include "ds/DynamicArray.h"
#include "ds/Queue.h"
#include "ds/Serialization.h"

using ds::DynamicArray;
using Strings = std::vector<std::string>;

BOOST_AUTO_TEST_SUITE(DynamicArraySuite)

BOOST_AUTO_TEST_CASE(new_array_is_empty)
{
    const DynamicArray array;
    BOOST_CHECK_EQUAL(array.size(), 0U);
    BOOST_CHECK(array.empty());
    BOOST_CHECK_EQUAL(array.capacity(), DynamicArray::kInitialCapacity);
    BOOST_CHECK_EQUAL(array.toString(), "(empty)");
    BOOST_CHECK(array.type() == ds::StructureType::Array);
}

BOOST_AUTO_TEST_CASE(push_back_and_get)
{
    DynamicArray array;
    array.pushBack("a");
    array.pushBack("b");
    BOOST_CHECK_EQUAL(array.size(), 2U);
    BOOST_CHECK_EQUAL(array.get(0), "a");
    BOOST_CHECK_EQUAL(array.get(1), "b");
    BOOST_CHECK_EQUAL(array.toString(), "[a, b]");
}

BOOST_AUTO_TEST_CASE(push_front_and_insert_keep_order)
{
    DynamicArray array;
    array.pushBack("b");
    array.pushFront("a");
    array.insertAt(2, "d");
    array.insertAt(2, "c");
    BOOST_CHECK(array.toVector() == (Strings{"a", "b", "c", "d"}));
}

BOOST_AUTO_TEST_CASE(insert_at_invalid_index_throws)
{
    DynamicArray array;
    BOOST_CHECK_THROW(array.insertAt(1, "x"), std::out_of_range);
    array.pushBack("a");
    BOOST_CHECK_NO_THROW(array.insertAt(1, "x"));
    BOOST_CHECK_THROW(array.insertAt(5, "x"), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(capacity_doubles_when_full)
{
    DynamicArray array;
    for (int i = 0; i < 25; ++i)
    {
        array.pushBack(std::to_string(i));
    }
    BOOST_CHECK_EQUAL(array.size(), 25U);
    BOOST_CHECK_EQUAL(array.capacity(), 40U);
    for (std::size_t i = 0; i < 25; ++i)
    {
        BOOST_CHECK_EQUAL(array.get(i), std::to_string(i));
    }
}

BOOST_AUTO_TEST_CASE(insert_into_full_array_grows)
{
    DynamicArray array;
    for (std::size_t i = 0; i < DynamicArray::kInitialCapacity; ++i)
    {
        array.pushBack("v");
    }
    array.insertAt(0, "first");
    BOOST_CHECK_EQUAL(array.capacity(), 2 * DynamicArray::kInitialCapacity);
    BOOST_CHECK_EQUAL(array.get(0), "first");
}

BOOST_AUTO_TEST_CASE(remove_shifts_elements)
{
    DynamicArray array;
    for (const char* value : {"a", "b", "c", "d"})
    {
        array.pushBack(value);
    }
    array.removeAt(1);
    BOOST_CHECK(array.toVector() == (Strings{"a", "c", "d"}));
    array.popFront();
    array.popBack();
    BOOST_CHECK(array.toVector() == (Strings{"c"}));
}

BOOST_AUTO_TEST_CASE(invalid_access_throws)
{
    DynamicArray array;
    BOOST_CHECK_THROW(array.popBack(), std::out_of_range);
    BOOST_CHECK_THROW(array.popFront(), std::out_of_range);
    BOOST_CHECK_THROW(array.removeAt(0), std::out_of_range);
    BOOST_CHECK_THROW(array.get(0), std::out_of_range);
    BOOST_CHECK_THROW(array.set(0, "x"), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(set_replaces_value)
{
    DynamicArray array;
    array.pushBack("old");
    array.set(0, "new");
    BOOST_CHECK_EQUAL(array.get(0), "new");
    BOOST_CHECK_EQUAL(array.size(), 1U);
}

BOOST_AUTO_TEST_CASE(clear_resets_size_but_keeps_array_usable)
{
    DynamicArray array;
    array.pushBack("a");
    array.pushBack("b");
    array.clear();
    BOOST_CHECK(array.empty());
    array.pushBack("c");
    BOOST_CHECK_EQUAL(array.toString(), "[c]");
}

BOOST_AUTO_TEST_CASE(copy_is_deep)
{
    DynamicArray original;
    original.pushBack("a");
    DynamicArray copy(original);
    copy.set(0, "changed");
    copy.pushBack("b");
    BOOST_CHECK_EQUAL(original.get(0), "a");
    BOOST_CHECK_EQUAL(original.size(), 1U);
    BOOST_CHECK_EQUAL(copy.size(), 2U);
}

BOOST_AUTO_TEST_CASE(copy_assignment_and_self_assignment)
{
    DynamicArray a;
    a.pushBack("x");
    DynamicArray b;
    b.pushBack("1");
    b.pushBack("2");
    b = a;
    BOOST_CHECK_EQUAL(b.toString(), "[x]");
    const DynamicArray& alias = b;
    b = alias;
    BOOST_CHECK_EQUAL(b.toString(), "[x]");
}

BOOST_AUTO_TEST_CASE(move_leaves_source_usable)
{
    DynamicArray source;
    source.pushBack("a");
    DynamicArray target(std::move(source));
    BOOST_CHECK_EQUAL(target.toString(), "[a]");
    BOOST_CHECK_EQUAL(source.size(), 0U); // NOLINT: состояние после перемещения определено
    source.pushBack("again");
    BOOST_CHECK_EQUAL(source.get(0), "again");
    DynamicArray assigned;
    assigned = std::move(target);
    BOOST_CHECK_EQUAL(assigned.toString(), "[a]");
}

BOOST_AUTO_TEST_CASE(binary_round_trip_keeps_special_strings)
{
    DynamicArray array;
    for (const std::string value :
         {std::string("plain"), std::string(), std::string("with space"), std::string("line\nbreak\r\n"),
          std::string("back\\slash"), std::string("\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82")})
    {
        array.pushBack(value);
    }
    const DynamicArray restored = testutil::roundTripBinary(array);
    BOOST_CHECK(restored.toVector() == array.toVector());
}

BOOST_AUTO_TEST_CASE(text_round_trip_keeps_special_strings)
{
    DynamicArray array;
    for (const std::string value : {std::string("plain"), std::string(), std::string("line\nbreak\r\n"),
                                    std::string("back\\slash"), std::string("null")})
    {
        array.pushBack(value);
    }
    const DynamicArray restored = testutil::roundTripText(array);
    BOOST_CHECK(restored.toVector() == array.toVector());
}

BOOST_AUTO_TEST_CASE(binary_format_layout)
{
    DynamicArray array;
    array.pushBack("ab");
    const std::string bytes = testutil::binaryOf(array);
    const std::string expected =
        std::string("DSB1") + '\x01' + std::string("\x01\x00\x00\x00", 4) + std::string("\x02\x00\x00\x00", 4) + "ab";
    BOOST_CHECK_EQUAL(bytes, expected);
}

BOOST_AUTO_TEST_CASE(text_format_layout)
{
    DynamicArray array;
    array.pushBack("a b");
    array.pushBack("x\ny");
    BOOST_CHECK_EQUAL(testutil::textOf(array), "DSTXT 1\nTYPE array\nCOUNT 2\na b\nx\\ny\n");
}

BOOST_AUTO_TEST_CASE(truncated_data_is_rejected_and_array_unchanged)
{
    DynamicArray source;
    source.pushBack("hello");
    source.pushBack("world");
    std::string bytes = testutil::binaryOf(source);
    bytes.resize(bytes.size() - 3);

    DynamicArray target;
    target.pushBack("keep");
    std::istringstream in(bytes, std::ios::binary);
    BOOST_CHECK_THROW(target.loadBinary(in), ds::SerializationError);
    BOOST_CHECK_EQUAL(target.toString(), "[keep]");
}

BOOST_AUTO_TEST_CASE(wrong_signature_or_type_is_rejected)
{
    DynamicArray target;
    std::istringstream badMagic(std::string("XXXX\x01\x00\x00\x00\x00", 9), std::ios::binary);
    BOOST_CHECK_THROW(target.loadBinary(badMagic), ds::SerializationError);

    ds::Queue queue;
    queue.push("q");
    std::istringstream wrongType(testutil::binaryOf(queue), std::ios::binary);
    BOOST_CHECK_THROW(target.loadBinary(wrongType), ds::SerializationError);

    std::istringstream wrongText(testutil::textOf(queue));
    BOOST_CHECK_THROW(target.loadText(wrongText), ds::SerializationError);
}

BOOST_AUTO_TEST_CASE(malformed_text_is_rejected)
{
    DynamicArray target;
    std::istringstream missingLines("DSTXT 1\nTYPE array\nCOUNT 3\nonly one\n");
    BOOST_CHECK_THROW(target.loadText(missingLines), ds::SerializationError);
    std::istringstream badCount("DSTXT 1\nTYPE array\nCOUNT abc\n");
    BOOST_CHECK_THROW(target.loadText(badCount), ds::SerializationError);
    std::istringstream badEscape("DSTXT 1\nTYPE array\nCOUNT 1\nbad\\q\n");
    BOOST_CHECK_THROW(target.loadText(badEscape), ds::SerializationError);
}

BOOST_AUTO_TEST_SUITE_END()
