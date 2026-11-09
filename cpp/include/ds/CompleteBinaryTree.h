#ifndef DS_COMPLETEBINARYTREE_H
#define DS_COMPLETEBINARYTREE_H

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "ds/IStructure.h"

namespace ds
{

// Complete Binary Tree (полное бинарное дерево) из целых чисел.
// Это не дерево поиска: узлы добавляются по уровням слева направо
// в первое свободное место, поэтому дерево, построенное через insert(),
// всегда полное. Десериализация может восстановить и произвольную форму
// дерева, поэтому isComplete() - настоящая проверка.
class CompleteBinaryTree final : public IStructure
{
public:
    CompleteBinaryTree() = default;
    CompleteBinaryTree(const CompleteBinaryTree& other);
    CompleteBinaryTree(CompleteBinaryTree&& other) noexcept;
    CompleteBinaryTree& operator=(const CompleteBinaryTree& other);
    CompleteBinaryTree& operator=(CompleteBinaryTree&& other) noexcept;
    ~CompleteBinaryTree() override;

    // Добавление в первое свободное место (обход в ширину), O(N)
    void insert(int value);
    // Поиск значения (обход в ширину), O(N)
    bool contains(int value) const;
    // Проверка свойства полноты, O(N)
    bool isComplete() const;
    // Высота дерева (пустое дерево - 0)
    std::size_t height() const;
    // Значения узлов в порядке обхода по уровням
    std::vector<int> levelOrder() const;

    void swap(CompleteBinaryTree& other) noexcept;

    StructureType type() const override;
    std::size_t size() const override;
    void clear() override;
    std::string toString() const override;

private:
    struct Node
    {
        int value;
        Node* left;
        Node* right;
    };

    using Slots = std::vector<std::optional<int>>;

    // Представление формы дерева обходом по уровням с пропусками (nullopt)
    Slots toSlots() const;
    void buildFromSlots(const Slots& slots);
    static void destroy(Node* node);
    static std::size_t heightOf(const Node* node);
    static void appendDrawing(const Node* node, const std::string& prefix, bool isLeft, std::string& out);

    void doSaveBinary(std::ostream& out) const override;
    void doLoadBinary(std::istream& in) override;
    void doSaveText(std::ostream& out) const override;
    void doLoadText(std::istream& in) override;

    Node* root_ = nullptr;
    std::size_t size_ = 0;
};

} // namespace ds

#endif
