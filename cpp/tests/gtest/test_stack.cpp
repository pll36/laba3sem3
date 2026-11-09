#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../common/TestUtils.h"
#include "ds/Serialization.h"
#include "ds/Stack.h"

using ds::Stack;
using Strings = std::vector<std::string>;

TEST(StackTest, NewStackIsEmpty)
{
    const Stack stack;
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0U);
    EXPECT_EQ(stack.toString(), "(empty)");
    EXPECT_EQ(stack.type(), ds::StructureType::Stack);
}

TEST(StackTest, PopsInReverseOrder)
{
    Stack stack;
    stack.push("1");
    stack.push("2");
    stack.push("3");
    EXPECT_EQ(stack.pop(), "3");
    EXPECT_EQ(stack.pop(), "2");
    EXPECT_EQ(stack.pop(), "1");
    EXPECT_TRUE(stack.empty());
}

TEST(StackTest, PeekDoesNotRemove)
{
    Stack stack;
    stack.push("a");
    stack.push("b");
    EXPECT_EQ(stack.peek(), "b");
    EXPECT_EQ(stack.size(), 2U);
}

TEST(StackTest, EmptyStackThrows)
{
    Stack stack;
    EXPECT_THROW(stack.pop(), std::out_of_range);
    EXPECT_THROW(stack.peek(), std::out_of_range);
}

TEST(StackTest, ToStringAndToVectorOrder)
{
    Stack stack;
    stack.push("a");
    stack.push("b");
    EXPECT_EQ(stack.toString(), "Stack(top->bottom): b, a");
    EXPECT_EQ(stack.toVector(), (Strings{"a", "b"})); // от дна к вершине
}

TEST(StackTest, ClearRemovesEverything)
{
    Stack stack;
    stack.push("a");
    stack.clear();
    EXPECT_TRUE(stack.empty());
    stack.push("b");
    EXPECT_EQ(stack.peek(), "b");
}

TEST(StackTest, CopyIsIndependentAndKeepsOrder)
{
    Stack original;
    original.push("a");
    original.push("b");
    Stack copy(original);
    copy.push("c");
    EXPECT_EQ(original.size(), 2U);
    EXPECT_EQ(copy.pop(), "c");
    EXPECT_EQ(copy.pop(), "b");
}

TEST(StackTest, MoveAndAssignment)
{
    Stack source;
    source.push("a");
    Stack moved(std::move(source));
    EXPECT_EQ(moved.size(), 1U);
    EXPECT_TRUE(source.empty()); // NOLINT: состояние после перемещения определено

    Stack assigned;
    assigned.push("x");
    assigned = moved;
    EXPECT_EQ(assigned.peek(), "a");
    const Stack& alias = assigned;
    assigned = alias;
    EXPECT_EQ(assigned.size(), 1U);
}

TEST(StackTest, SerializationRestoresOrder)
{
    Stack stack;
    stack.push("bottom");
    stack.push("with space");
    stack.push("top\nline");

    for (Stack restored : {testutil::roundTripBinary(stack), testutil::roundTripText(stack)})
    {
        EXPECT_EQ(restored.size(), 3U);
        EXPECT_EQ(restored.pop(), "top\nline");
        EXPECT_EQ(restored.pop(), "with space");
        EXPECT_EQ(restored.pop(), "bottom");
    }
}

TEST(StackTest, LoadReplacesPreviousContent)
{
    Stack source;
    source.push("new");
    Stack target;
    target.push("old1");
    target.push("old2");
    std::stringstream buffer;
    source.saveText(buffer);
    target.loadText(buffer);
    EXPECT_EQ(target.toVector(), (Strings{"new"}));
}

TEST(StackTest, WrongTypeHeaderIsRejected)
{
    Stack stack;
    std::istringstream in("DSTXT 1\nTYPE queue\nCOUNT 0\n");
    EXPECT_THROW(stack.loadText(in), ds::SerializationError);
}
