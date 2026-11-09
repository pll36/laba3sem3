#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "../common/TestUtils.h"
#include "ds/Queue.h"

using ds::Queue;
using Strings = std::vector<std::string>;

TEST(QueueTest, NewQueueIsEmpty)
{
    const Queue queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.toString(), "(empty)");
    EXPECT_EQ(queue.type(), ds::StructureType::Queue);
}

TEST(QueueTest, FifoOrder)
{
    Queue queue;
    queue.push("1");
    queue.push("2");
    queue.push("3");
    EXPECT_EQ(queue.front(), "1");
    EXPECT_EQ(queue.pop(), "1");
    EXPECT_EQ(queue.pop(), "2");
    EXPECT_EQ(queue.pop(), "3");
    EXPECT_TRUE(queue.empty());
}

TEST(QueueTest, EmptyQueueThrows)
{
    Queue queue;
    EXPECT_THROW(queue.pop(), std::out_of_range);
    EXPECT_THROW(queue.front(), std::out_of_range);
}

TEST(QueueTest, QueueRemainsUsableAfterBecomingEmpty)
{
    Queue queue;
    queue.push("a");
    queue.pop();
    queue.push("b"); // rear_ должен быть сброшен при опустошении
    EXPECT_EQ(queue.toVector(), (Strings{"b"}));
    EXPECT_EQ(queue.toString(), "Queue(front->rear): b");
}

TEST(QueueTest, ClearAndSize)
{
    Queue queue;
    queue.push("a");
    queue.push("b");
    EXPECT_EQ(queue.size(), 2U);
    queue.clear();
    EXPECT_EQ(queue.size(), 0U);
    queue.push("c");
    EXPECT_EQ(queue.front(), "c");
}

TEST(QueueTest, CopyMoveAssign)
{
    Queue original;
    original.push("a");
    original.push("b");
    Queue copy(original);
    copy.push("c");
    EXPECT_EQ(original.size(), 2U);
    EXPECT_EQ(copy.size(), 3U);

    Queue moved(std::move(copy));
    EXPECT_EQ(moved.toVector(), (Strings{"a", "b", "c"}));
    EXPECT_TRUE(copy.empty()); // NOLINT: состояние после перемещения определено

    Queue assigned;
    assigned = original;
    EXPECT_EQ(assigned.toVector(), original.toVector());
    const Queue& alias = assigned;
    assigned = alias;
    EXPECT_EQ(assigned.size(), 2U);
}

TEST(QueueTest, SerializationKeepsOrder)
{
    Queue queue;
    queue.push("first");
    queue.push("sec ond");
    queue.push("");
    EXPECT_EQ(testutil::roundTripBinary(queue).toVector(), queue.toVector());
    EXPECT_EQ(testutil::roundTripText(queue).toVector(), queue.toVector());
}
