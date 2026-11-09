#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

#include "../common/TestUtils.h"
#include "ds/CompleteBinaryTree.h"
#include "ds/Serialization.h"

using ds::CompleteBinaryTree;
using Ints = std::vector<int>;

namespace
{

CompleteBinaryTree makeTree(const Ints& values)
{
    CompleteBinaryTree tree;
    for (const int value : values)
    {
        tree.insert(value);
    }
    return tree;
}

// Дерево произвольной формы из текстового представления (null - пропуск)
CompleteBinaryTree treeFromText(const std::vector<std::string>& slots)
{
    std::ostringstream text;
    text << "DSTXT 1\nTYPE tree\nCOUNT " << slots.size() << '\n';
    for (const std::string& slot : slots)
    {
        text << slot << '\n';
    }
    std::istringstream in(text.str());
    CompleteBinaryTree tree;
    tree.loadText(in);
    return tree;
}

} // namespace

TEST(CompleteTreeTest, NewTreeIsEmptyAndComplete)
{
    const CompleteBinaryTree tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_TRUE(tree.isComplete());
    EXPECT_FALSE(tree.contains(1));
    EXPECT_EQ(tree.height(), 0U);
    EXPECT_EQ(tree.toString(), "(empty)");
    EXPECT_EQ(tree.type(), ds::StructureType::Tree);
    EXPECT_TRUE(tree.levelOrder().empty());
}

TEST(CompleteTreeTest, InsertFillsLevelsLeftToRight)
{
    const CompleteBinaryTree tree = makeTree({1, 2, 3, 4, 5, 6});
    EXPECT_EQ(tree.levelOrder(), (Ints{1, 2, 3, 4, 5, 6}));
    EXPECT_EQ(tree.size(), 6U);
    EXPECT_EQ(tree.height(), 3U);
}

TEST(CompleteTreeTest, TreeBuiltByInsertIsAlwaysComplete)
{
    CompleteBinaryTree tree;
    for (int i = 1; i <= 40; ++i)
    {
        tree.insert(i);
        EXPECT_TRUE(tree.isComplete()) << "after inserting " << i;
    }
    EXPECT_EQ(tree.height(), 6U);
}

TEST(CompleteTreeTest, ContainsFindsValues)
{
    const CompleteBinaryTree tree = makeTree({10, 20, 30, 40});
    EXPECT_TRUE(tree.contains(10));
    EXPECT_TRUE(tree.contains(40));
    EXPECT_FALSE(tree.contains(99));
}

TEST(CompleteTreeTest, DuplicatesAreAllowed)
{
    const CompleteBinaryTree tree = makeTree({7, 7, 7});
    EXPECT_EQ(tree.size(), 3U);
    EXPECT_TRUE(tree.contains(7));
}

TEST(CompleteTreeTest, DrawingShowsStructure)
{
    EXPECT_EQ(makeTree({1}).toString(), "1");
    EXPECT_EQ(makeTree({1, 2, 3, 4}).toString(), "1\n"
                                                 "|-- 2\n"
                                                 "|   |-- 4\n"
                                                 "`-- 3");
}

TEST(CompleteTreeTest, ClearEmptiesTree)
{
    CompleteBinaryTree tree = makeTree({1, 2, 3});
    tree.clear();
    EXPECT_TRUE(tree.empty());
    tree.insert(5);
    EXPECT_EQ(tree.levelOrder(), (Ints{5}));
}

TEST(CompleteTreeTest, CopyIsDeep)
{
    CompleteBinaryTree original = makeTree({1, 2, 3});
    CompleteBinaryTree copy(original);
    copy.insert(4);
    EXPECT_EQ(original.size(), 3U);
    EXPECT_EQ(copy.levelOrder(), (Ints{1, 2, 3, 4}));
}

TEST(CompleteTreeTest, MoveAndAssignment)
{
    CompleteBinaryTree source = makeTree({1, 2});
    CompleteBinaryTree moved(std::move(source));
    EXPECT_EQ(moved.size(), 2U);
    EXPECT_TRUE(source.empty()); // NOLINT: состояние после перемещения определено

    CompleteBinaryTree assigned = makeTree({9});
    assigned = moved;
    EXPECT_EQ(assigned.levelOrder(), (Ints{1, 2}));
    const CompleteBinaryTree& alias = assigned;
    assigned = alias;
    EXPECT_EQ(assigned.size(), 2U);
}

TEST(CompleteTreeTest, DetectsNonCompleteShapes)
{
    // правый ребёнок есть, левого нет
    EXPECT_FALSE(treeFromText({"1", "null", "2"}).isComplete());
    // у узла 2 нет детей, а у узла 3 есть левый ребёнок: пропуск посередине уровня
    EXPECT_FALSE(treeFromText({"1", "2", "3", "null", "null", "4"}).isComplete());
    // корректное полное дерево
    EXPECT_TRUE(treeFromText({"1", "2", "3", "4"}).isComplete());
}

TEST(CompleteTreeTest, SerializationKeepsCompleteTree)
{
    const CompleteBinaryTree tree = makeTree({5, -3, 0, 2147483647, -2147483647});
    const CompleteBinaryTree fromBinary = testutil::roundTripBinary(tree);
    const CompleteBinaryTree fromText = testutil::roundTripText(tree);
    EXPECT_EQ(fromBinary.levelOrder(), tree.levelOrder());
    EXPECT_EQ(fromText.levelOrder(), tree.levelOrder());
    EXPECT_EQ(fromBinary.toString(), tree.toString());
}

TEST(CompleteTreeTest, SerializationKeepsArbitraryShape)
{
    const CompleteBinaryTree tree = treeFromText({"1", "null", "2", "3"});
    EXPECT_FALSE(tree.isComplete());
    EXPECT_EQ(tree.size(), 3U);
    const CompleteBinaryTree restored = testutil::roundTripBinary(tree);
    EXPECT_EQ(restored.toString(), tree.toString());
    EXPECT_FALSE(restored.isComplete());
}

TEST(CompleteTreeTest, EmptyTreeRoundTrip)
{
    const CompleteBinaryTree empty;
    EXPECT_TRUE(testutil::roundTripBinary(empty).empty());
    EXPECT_TRUE(testutil::roundTripText(empty).empty());
}

TEST(CompleteTreeTest, TextFormatLayout)
{
    EXPECT_EQ(testutil::textOf(makeTree({1, 2})), "DSTXT 1\nTYPE tree\nCOUNT 2\n1\n2\n");
    EXPECT_EQ(testutil::textOf(treeFromText({"1", "null", "2"})), "DSTXT 1\nTYPE tree\nCOUNT 3\n1\nnull\n2\n");
}

TEST(CompleteTreeTest, MalformedTextIsRejected)
{
    CompleteBinaryTree tree = makeTree({42});
    EXPECT_THROW(treeFromText({"null", "1"}), ds::SerializationError);              // нет корня
    EXPECT_THROW(treeFromText({"1", "null", "null", "5"}), ds::SerializationError); // узел без родителя
    EXPECT_THROW(treeFromText({"1", "abc"}), ds::SerializationError);
    std::istringstream truncated("DSTXT 1\nTYPE tree\nCOUNT 3\n1\n");
    EXPECT_THROW(tree.loadText(truncated), ds::SerializationError);
    EXPECT_EQ(tree.levelOrder(), (Ints{42})); // содержимое не изменилось
}

TEST(CompleteTreeTest, MalformedBinaryIsRejected)
{
    CompleteBinaryTree tree;
    std::string bytes = testutil::binaryOf(makeTree({1, 2, 3}));
    std::string truncated = bytes.substr(0, bytes.size() - 2);
    std::istringstream truncatedIn(truncated, std::ios::binary);
    EXPECT_THROW(tree.loadBinary(truncatedIn), ds::SerializationError);

    // флаг присутствия должен быть 0 или 1: 1 слот, флаг = 7
    const std::string badFlag = std::string("DSB1") + '\x06' + std::string("\x01\x00\x00\x00", 4) + '\x07';
    std::istringstream badFlagIn(badFlag, std::ios::binary);
    EXPECT_THROW(tree.loadBinary(badFlagIn), ds::SerializationError);
}
