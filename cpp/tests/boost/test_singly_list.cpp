#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>
#include <vector>

#include "../common/TestUtils.h"
#include "ds/SinglyLinkedList.h"

using ds::SinglyLinkedList;
using Strings = std::vector<std::string>;

namespace
{

SinglyLinkedList makeList(const Strings& values)
{
    SinglyLinkedList list;
    for (const std::string& value : values)
    {
        list.pushBack(value);
    }
    return list;
}

} // namespace

BOOST_AUTO_TEST_SUITE(SinglyLinkedListSuite)

BOOST_AUTO_TEST_CASE(new_list_is_empty)
{
    const SinglyLinkedList list;
    BOOST_CHECK(list.empty());
    BOOST_CHECK_EQUAL(list.toString(), "(empty)");
    BOOST_CHECK(list.type() == ds::StructureType::SinglyList);
}

BOOST_AUTO_TEST_CASE(push_front_and_back)
{
    SinglyLinkedList list;
    list.pushBack("b");
    list.pushFront("a");
    list.pushBack("c");
    BOOST_CHECK(list.toVector() == (Strings{"a", "b", "c"}));
    BOOST_CHECK_EQUAL(list.size(), 3U);
    BOOST_CHECK_EQUAL(list.toString(), "a -> b -> c");
}

BOOST_AUTO_TEST_CASE(push_before)
{
    SinglyLinkedList list = makeList({"a", "c"});
    BOOST_CHECK(list.pushBefore("c", "b"));
    BOOST_CHECK(list.pushBefore("a", "start"));
    BOOST_CHECK(!list.pushBefore("zzz", "x"));
    BOOST_CHECK(list.toVector() == (Strings{"start", "a", "b", "c"}));
    BOOST_CHECK_EQUAL(list.size(), 4U);
}

BOOST_AUTO_TEST_CASE(push_after)
{
    SinglyLinkedList list = makeList({"a", "c"});
    BOOST_CHECK(list.pushAfter("a", "b"));
    BOOST_CHECK(list.pushAfter("c", "end"));
    BOOST_CHECK(!list.pushAfter("zzz", "x"));
    BOOST_CHECK(list.toVector() == (Strings{"a", "b", "c", "end"}));
}

BOOST_AUTO_TEST_CASE(pop_front_and_back)
{
    SinglyLinkedList list = makeList({"a", "b", "c"});
    list.popFront();
    list.popBack();
    BOOST_CHECK(list.toVector() == (Strings{"b"}));
    list.popBack(); // единственный элемент
    BOOST_CHECK(list.empty());
    BOOST_CHECK_THROW(list.popFront(), std::out_of_range);
    BOOST_CHECK_THROW(list.popBack(), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(pop_before)
{
    SinglyLinkedList list = makeList({"a", "b", "c", "d"});
    BOOST_CHECK(list.popBefore("c")); // удаляем "b" (есть элемент перед ним)
    BOOST_CHECK(list.toVector() == (Strings{"a", "c", "d"}));
    BOOST_CHECK(list.popBefore("c")); // удаляем голову "a"
    BOOST_CHECK(list.toVector() == (Strings{"c", "d"}));
    BOOST_CHECK(!list.popBefore("c")); // перед головой ничего нет
    BOOST_CHECK(!list.popBefore("zzz"));
    BOOST_CHECK_EQUAL(list.size(), 2U);
}

BOOST_AUTO_TEST_CASE(pop_after)
{
    SinglyLinkedList list = makeList({"a", "b", "c"});
    BOOST_CHECK(list.popAfter("a"));
    BOOST_CHECK(list.toVector() == (Strings{"a", "c"}));
    BOOST_CHECK(!list.popAfter("c")); // после хвоста ничего нет
    BOOST_CHECK(!list.popAfter("zzz"));
    BOOST_CHECK_EQUAL(list.size(), 2U);
}

BOOST_AUTO_TEST_CASE(remove_by_value_head_middle_tail)
{
    SinglyLinkedList list = makeList({"a", "b", "c", "d"});
    BOOST_CHECK(list.removeByValue("a"));
    BOOST_CHECK(list.removeByValue("c"));
    BOOST_CHECK(list.removeByValue("d"));
    BOOST_CHECK(!list.removeByValue("zzz"));
    BOOST_CHECK(list.toVector() == (Strings{"b"}));
    BOOST_CHECK_EQUAL(list.size(), 1U);
}

BOOST_AUTO_TEST_CASE(remove_by_value_removes_only_first_occurrence)
{
    SinglyLinkedList list = makeList({"x", "y", "x"});
    BOOST_CHECK(list.removeByValue("x"));
    BOOST_CHECK(list.toVector() == (Strings{"y", "x"}));
}

BOOST_AUTO_TEST_CASE(search)
{
    const SinglyLinkedList list = makeList({"a", "b", "c"});
    BOOST_CHECK(list.contains("b"));
    BOOST_CHECK(!list.contains("zzz"));
    BOOST_CHECK_EQUAL(list.indexOf("c"), 2);
    BOOST_CHECK_EQUAL(list.indexOf("zzz"), -1);
}

BOOST_AUTO_TEST_CASE(print_from_element)
{
    const SinglyLinkedList list = makeList({"a", "b", "c"});
    BOOST_CHECK_EQUAL(list.toStringFrom("b"), "b -> c");
    BOOST_CHECK_EQUAL(list.toStringFrom("zzz"), "");
}

BOOST_AUTO_TEST_CASE(clear_empties_list)
{
    SinglyLinkedList list = makeList({"a", "b"});
    list.clear();
    BOOST_CHECK(list.empty());
    list.pushBack("c");
    BOOST_CHECK_EQUAL(list.toString(), "c");
}

BOOST_AUTO_TEST_CASE(copy_move_and_assign)
{
    SinglyLinkedList original = makeList({"a", "b"});
    SinglyLinkedList copy(original);
    copy.pushBack("c");
    BOOST_CHECK_EQUAL(original.size(), 2U);
    BOOST_CHECK_EQUAL(copy.size(), 3U);

    SinglyLinkedList moved(std::move(copy));
    BOOST_CHECK_EQUAL(moved.size(), 3U);
    BOOST_CHECK(copy.empty()); // NOLINT: состояние после перемещения определено

    SinglyLinkedList assigned;
    assigned = original;
    BOOST_CHECK(assigned.toVector() == original.toVector());
    const SinglyLinkedList& alias = assigned;
    assigned = alias;
    BOOST_CHECK_EQUAL(assigned.size(), 2U);
    assigned = makeList({"z"});
    BOOST_CHECK_EQUAL(assigned.toString(), "z");
}

BOOST_AUTO_TEST_CASE(serialization_round_trips)
{
    const SinglyLinkedList list = makeList({"a b", "", "multi\nline"});
    BOOST_CHECK(testutil::roundTripBinary(list).toVector() == list.toVector());
    BOOST_CHECK(testutil::roundTripText(list).toVector() == list.toVector());

    const SinglyLinkedList empty;
    BOOST_CHECK(testutil::roundTripBinary(empty).empty());
    BOOST_CHECK(testutil::roundTripText(empty).empty());
}

BOOST_AUTO_TEST_SUITE_END()
