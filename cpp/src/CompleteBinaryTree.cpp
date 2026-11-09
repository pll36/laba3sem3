#include "ds/CompleteBinaryTree.h"

#include <algorithm>
#include <queue>
#include <utility>

#include "ds/Serialization.h"
#include "ds/Text.h"

namespace ds
{

CompleteBinaryTree::CompleteBinaryTree(const CompleteBinaryTree& other)
{
    try
    {
        buildFromSlots(other.toSlots());
    }
    catch (...)
    {
        clear();
        throw;
    }
}

CompleteBinaryTree::CompleteBinaryTree(CompleteBinaryTree&& other) noexcept
    : root_(std::exchange(other.root_, nullptr)), size_(std::exchange(other.size_, 0))
{
}

CompleteBinaryTree& CompleteBinaryTree::operator=(const CompleteBinaryTree& other)
{
    CompleteBinaryTree copy(other); // copy-and-swap: при исключении текущий объект не меняется
    swap(copy);
    return *this;
}

CompleteBinaryTree& CompleteBinaryTree::operator=(CompleteBinaryTree&& other) noexcept
{
    CompleteBinaryTree moved(std::move(other));
    swap(moved);
    return *this;
}

CompleteBinaryTree::~CompleteBinaryTree()
{
    clear();
}

void CompleteBinaryTree::swap(CompleteBinaryTree& other) noexcept
{
    std::swap(root_, other.root_);
    std::swap(size_, other.size_);
}

void CompleteBinaryTree::insert(int value)
{
    if (root_ == nullptr)
    {
        root_ = new Node{value, nullptr, nullptr};
        ++size_;
        return;
    }
    // Ищем первый узел уровневого обхода, у которого есть свободный ребёнок
    std::queue<Node*> level;
    level.push(root_);
    while (true)
    {
        Node* current = level.front();
        level.pop();
        if (current->left == nullptr)
        {
            current->left = new Node{value, nullptr, nullptr};
            break;
        }
        level.push(current->left);
        if (current->right == nullptr)
        {
            current->right = new Node{value, nullptr, nullptr};
            break;
        }
        level.push(current->right);
    }
    ++size_;
}

bool CompleteBinaryTree::contains(int value) const
{
    if (root_ == nullptr)
    {
        return false;
    }
    std::queue<const Node*> level;
    level.push(root_);
    while (!level.empty())
    {
        const Node* current = level.front();
        level.pop();
        if (current->value == value)
        {
            return true;
        }
        if (current->left != nullptr)
        {
            level.push(current->left);
        }
        if (current->right != nullptr)
        {
            level.push(current->right);
        }
    }
    return false;
}

bool CompleteBinaryTree::isComplete() const
{
    std::queue<const Node*> level;
    level.push(root_);
    bool seenGap = false;
    while (!level.empty())
    {
        const Node* current = level.front();
        level.pop();
        if (current == nullptr)
        {
            seenGap = true;
            continue;
        }
        if (seenGap)
        {
            return false; // после пропуска встретился узел
        }
        level.push(current->left);
        level.push(current->right);
    }
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
std::size_t CompleteBinaryTree::heightOf(const Node* node)
{
    if (node == nullptr)
    {
        return 0;
    }
    return 1 + std::max(heightOf(node->left), heightOf(node->right));
}

std::size_t CompleteBinaryTree::height() const
{
    return heightOf(root_);
}

std::vector<int> CompleteBinaryTree::levelOrder() const
{
    std::vector<int> values;
    values.reserve(size_);
    if (root_ == nullptr)
    {
        return values;
    }
    std::queue<const Node*> level;
    level.push(root_);
    while (!level.empty())
    {
        const Node* current = level.front();
        level.pop();
        values.push_back(current->value);
        if (current->left != nullptr)
        {
            level.push(current->left);
        }
        if (current->right != nullptr)
        {
            level.push(current->right);
        }
    }
    return values;
}

StructureType CompleteBinaryTree::type() const
{
    return StructureType::Tree;
}

std::size_t CompleteBinaryTree::size() const
{
    return size_;
}

void CompleteBinaryTree::destroy(Node* node)
{
    // Итеративное удаление: глубокое дерево не переполнит стек
    std::vector<Node*> pending;
    if (node != nullptr)
    {
        pending.push_back(node);
    }
    while (!pending.empty())
    {
        Node* current = pending.back();
        pending.pop_back();
        if (current->left != nullptr)
        {
            pending.push_back(current->left);
        }
        if (current->right != nullptr)
        {
            pending.push_back(current->right);
        }
        delete current;
    }
}

void CompleteBinaryTree::clear()
{
    destroy(root_);
    root_ = nullptr;
    size_ = 0;
}

// NOLINTNEXTLINE(misc-no-recursion)
void CompleteBinaryTree::appendDrawing(const Node* node, const std::string& prefix, bool isLeft, std::string& out)
{
    if (node == nullptr)
    {
        return;
    }
    out += prefix + (isLeft ? "|-- " : "`-- ") + std::to_string(node->value) + '\n';
    const std::string childPrefix = prefix + (isLeft ? "|   " : "    ");
    appendDrawing(node->left, childPrefix, true, out);
    appendDrawing(node->right, childPrefix, false, out);
}

std::string CompleteBinaryTree::toString() const
{
    if (root_ == nullptr)
    {
        return emptyText();
    }
    std::string out = std::to_string(root_->value) + '\n';
    appendDrawing(root_->left, "", true, out);
    appendDrawing(root_->right, "", false, out);
    out.pop_back(); // убираем последний перевод строки
    return out;
}

CompleteBinaryTree::Slots CompleteBinaryTree::toSlots() const
{
    Slots slots;
    std::queue<const Node*> level;
    level.push(root_);
    while (!level.empty())
    {
        const Node* current = level.front();
        level.pop();
        if (current == nullptr)
        {
            slots.emplace_back(std::nullopt);
            continue;
        }
        slots.emplace_back(current->value);
        level.push(current->left);
        level.push(current->right);
    }
    while (!slots.empty() && !slots.back().has_value())
    {
        slots.pop_back(); // хвостовые пропуски не нужны
    }
    return slots;
}

void CompleteBinaryTree::buildFromSlots(const Slots& slots)
{
    CompleteBinaryTree built; // при исключении его деструктор освободит узлы
    if (!slots.empty())
    {
        const std::optional<int>& rootSlot = slots.front();
        if (!rootSlot.has_value())
        {
            throw SerializationError("tree root is missing");
        }
        built.root_ = new Node{rootSlot.value(), nullptr, nullptr};
        built.size_ = 1;
        std::queue<Node*> parents;
        parents.push(built.root_);
        std::size_t next = 1;
        while (next < slots.size())
        {
            if (parents.empty())
            {
                throw SerializationError("tree has nodes without a parent");
            }
            Node* parent = parents.front();
            parents.pop();
            for (Node** child : {&parent->left, &parent->right})
            {
                if (next >= slots.size())
                {
                    break;
                }
                const std::optional<int>& slot = slots[next++];
                if (slot.has_value())
                {
                    *child = new Node{*slot, nullptr, nullptr};
                    ++built.size_;
                    parents.push(*child);
                }
            }
        }
    }
    swap(built);
}

void CompleteBinaryTree::doSaveBinary(std::ostream& out) const
{
    const Slots slots = toSlots();
    io::writeU32(out, static_cast<std::uint32_t>(slots.size()));
    for (const std::optional<int>& slot : slots)
    {
        io::writeU8(out, slot.has_value() ? 1 : 0);
        if (slot.has_value())
        {
            io::writeI32(out, *slot);
        }
    }
}

void CompleteBinaryTree::doLoadBinary(std::istream& in)
{
    const std::uint32_t count = io::readU32(in);
    Slots slots;
    for (std::uint32_t i = 0; i < count; ++i)
    {
        const std::uint8_t present = io::readU8(in);
        if (present > 1)
        {
            throw SerializationError("invalid presence flag");
        }
        if (present == 1)
        {
            slots.emplace_back(io::readI32(in));
        }
        else
        {
            slots.emplace_back(std::nullopt);
        }
    }
    buildFromSlots(slots);
}

void CompleteBinaryTree::doSaveText(std::ostream& out) const
{
    const Slots slots = toSlots();
    out << "COUNT " << slots.size() << '\n';
    for (const std::optional<int>& slot : slots)
    {
        if (slot.has_value())
        {
            out << *slot << '\n';
        }
        else
        {
            out << "null\n";
        }
    }
}

void CompleteBinaryTree::doLoadText(std::istream& in)
{
    const std::uint32_t count = io::parseUint32(io::readField(in, "COUNT"));
    Slots slots;
    for (std::uint32_t i = 0; i < count; ++i)
    {
        const std::string line = io::readLine(in);
        if (line == "null")
        {
            slots.emplace_back(std::nullopt);
        }
        else
        {
            slots.emplace_back(io::parseInt32(line));
        }
    }
    buildFromSlots(slots);
}

} // namespace ds
