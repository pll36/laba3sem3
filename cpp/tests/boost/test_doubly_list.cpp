#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>
#include <vector>

#include "../common/TestUtils.h"
#include "ds/DoublyLinkedList.h"

using ds::DoublyLinkedList;
using Strings = std::vector<std::string>;

namespace
{

DoublyLinkedList makeList(const Strings& values)
{
    DoublyLinkedList list;
    for (const std::string& value : values)
    {
        list.pushBack(value);
    }
    return list;
}

} // namespace

BOOST_AUTO_TEST_SUITE(DoublyLinkedListSuite)

BOOST_AUTO_TEST_CASE(new_list_is_empty)
{
    const DoublyLinkedList list;
    BOOST_CHECK(list.empty());
    BOOST_CHECK_EQUAL(list.toString(), "(empty)");
    BOOST_CHECK_EQUAL(list.toStringBackward(), "(empty)");
    BOOST_CHECK(list.type() == ds::StructureType::DoublyList);
}

BOOST_AUTO_TEST_CASE(push_front_and_back_keep_both_directions)
{
    DoublyLinkedList list;
    list.pushBack("b");
    list.pushFront("a");
    list.pushBack("c");
    BOOST_CHECK(list.toVector() == (Strings{"a", "b", "c"}));
    BOOST_CHECK(list.toVectorBackward() == (Strings{"c", "b", "a"}));
    BOOST_CHECK_EQUAL(list.toString(), "a <-> b <-> c");
    BOOST_CHECK_EQUAL(list.toStringBackward(), "c <-> b <-> a");
}

BOOST_AUTO_TEST_CASE(push_before_and_after)
{
    DoublyLinkedList list = makeList({"b", "d"});
    BOOST_CHECK(list.pushBefore("b", "a")); // перед головой
    BOOST_CHECK(list.pushBefore("d", "c")); // в середину
    BOOST_CHECK(list.pushAfter("d", "e"));  // после хвоста
    BOOST_CHECK(list.pushAfter("a", "a2")); // в середину
    BOOST_CHECK(!list.pushBefore("zzz", "x"));
    BOOST_CHECK(!list.pushAfter("zzz", "x"));
    BOOST_CHECK(list.toVector() == (Strings{"a", "a2", "b", "c", "d", "e"}));
    BOOST_CHECK(list.toVectorBackward() == (Strings{"e", "d", "c", "b", "a2", "a"}));
    BOOST_CHECK_EQUAL(list.size(), 6U);
}

BOOST_AUTO_TEST_CASE(pop_front_and_back)
{
    DoublyLinkedList list = makeList({"a", "b", "c"});
    list.popFront();
    list.popBack();
    BOOST_CHECK(list.toVector() == (Strings{"b"}));
    list.popBack();
    BOOST_CHECK(list.empty());
    BOOST_CHECK_THROW(list.popFront(), std::out_of_range);
    BOOST_CHECK_THROW(list.popBack(), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(pop_before_and_after)
{
    DoublyLinkedList list = makeList({"a", "b", "c", "d"});
    BOOST_CHECK(list.popBefore("c")); // удаляется "b"
    BOOST_CHECK(list.popAfter("c"));  // удаляется "d"
    BOOST_CHECK(list.toVector() == (Strings{"a", "c"}));
    BOOST_CHECK(!list.popBefore("a"));
    BOOST_CHECK(!list.popAfter("c"));
    BOOST_CHECK(!list.popBefore("zzz"));
    BOOST_CHECK(!list.popAfter("zzz"));
    BOOST_CHECK(list.popBefore("c")); // удаляется голова
    BOOST_CHECK(list.toVectorBackward() == (Strings{"c"}));
}

BOOST_AUTO_TEST_CASE(remove_by_value)
{
    DoublyLinkedList list = makeList({"a", "b", "c", "d"});
    BOOST_CHECK(list.removeByValue("a"));
    BOOST_CHECK(list.removeByValue("d"));
    BOOST_CHECK(list.removeByValue("b"));
    BOOST_CHECK(!list.removeByValue("zzz"));
    BOOST_CHECK(list.toVector() == (Strings{"c"}));
    BOOST_CHECK(list.toVectorBackward() == (Strings{"c"}));
}

BOOST_AUTO_TEST_CASE(search_and_print_from)
{
    const DoublyLinkedList list = makeList({"a", "b", "c"});
    BOOST_CHECK(list.contains("b"));
    BOOST_CHECK(!list.contains("zzz"));
    BOOST_CHECK_EQUAL(list.indexOf("c"), 2);
    BOOST_CHECK_EQUAL(list.indexOf("zzz"), -1);
    BOOST_CHECK_EQUAL(list.toStringFrom("b"), "b <-> c");
    BOOST_CHECK_EQUAL(list.toStringFromBackward("b"), "b <-> a");
    BOOST_CHECK_EQUAL(list.toStringFrom("zzz"), "");
    BOOST_CHECK_EQUAL(list.toStringFromBackward("zzz"), "");
}

BOOST_AUTO_TEST_CASE(clear_resets_both_ends)
{
    DoublyLinkedList list = makeList({"a", "b"});
    list.clear();
    BOOST_CHECK(list.empty());
    list.pushFront("x");
    BOOST_CHECK_EQUAL(list.toStringBackward(), "x");
}

BOOST_AUTO_TEST_CASE(copy_move_and_assign)
{
    DoublyLinkedList original = makeList({"a", "b"});
    DoublyLinkedList copy(original);
    copy.pushFront("z");
    BOOST_CHECK_EQUAL(original.size(), 2U);
    BOOST_CHECK(copy.toVectorBackward() == (Strings{"b", "a", "z"}));

    DoublyLinkedList moved(std::move(copy));
    BOOST_CHECK_EQUAL(moved.size(), 3U);
    BOOST_CHECK(copy.empty()); // NOLINT: состояние после перемещения определено

    DoublyLinkedList assigned;
    assigned = original;
    BOOST_CHECK(assigned.toVector() == original.toVector());
    const DoublyLinkedList& alias = assigned;
    assigned = alias;
    BOOST_CHECK_EQUAL(assigned.size(), 2U);
}

BOOST_AUTO_TEST_CASE(serialization_round_trips)
{
    const DoublyLinkedList list = makeList({"a b", "", "multi\nline"});
    const DoublyLinkedList fromBinary = testutil::roundTripBinary(list);
    const DoublyLinkedList fromText = testutil::roundTripText(list);
    BOOST_CHECK(fromBinary.toVector() == list.toVector());
    BOOST_CHECK(fromText.toVector() == list.toVector());
    BOOST_CHECK(fromBinary.toVectorBackward() == list.toVectorBackward());
}

BOOST_AUTO_TEST_SUITE_END()
