#include "ds/DoublyLinkedList.h"

#include <stdexcept>
#include <utility>

#include "ds/Serialization.h"
#include "ds/Text.h"

namespace ds
{

DoublyLinkedList::DoublyLinkedList(const DoublyLinkedList& other)
{
    try
    {
        for (const Node* node = other.head_; node != nullptr; node = node->next)
        {
            pushBack(node->value);
        }
    }
    catch (...)
    {
        clear();
        throw;
    }
}

DoublyLinkedList::DoublyLinkedList(DoublyLinkedList&& other) noexcept
    : head_(std::exchange(other.head_, nullptr)), tail_(std::exchange(other.tail_, nullptr)),
      size_(std::exchange(other.size_, 0))
{
}

DoublyLinkedList& DoublyLinkedList::operator=(const DoublyLinkedList& other)
{
    DoublyLinkedList copy(other); // copy-and-swap: при исключении текущий объект не меняется
    swap(copy);
    return *this;
}

DoublyLinkedList& DoublyLinkedList::operator=(DoublyLinkedList&& other) noexcept
{
    DoublyLinkedList moved(std::move(other));
    swap(moved);
    return *this;
}

DoublyLinkedList::~DoublyLinkedList()
{
    clear();
}

void DoublyLinkedList::swap(DoublyLinkedList& other) noexcept
{
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
}

void DoublyLinkedList::pushFront(const std::string& value)
{
    auto* fresh = new Node{value, head_, nullptr};
    if (head_ != nullptr)
    {
        head_->prev = fresh;
    }
    else
    {
        tail_ = fresh;
    }
    head_ = fresh;
    ++size_;
}

void DoublyLinkedList::pushBack(const std::string& value)
{
    auto* fresh = new Node{value, nullptr, tail_};
    if (tail_ != nullptr)
    {
        tail_->next = fresh;
    }
    else
    {
        head_ = fresh;
    }
    tail_ = fresh;
    ++size_;
}

bool DoublyLinkedList::pushBefore(const std::string& target, const std::string& value)
{
    Node* node = findNode(target);
    if (node == nullptr)
    {
        return false;
    }
    if (node == head_)
    {
        pushFront(value);
        return true;
    }
    auto* fresh = new Node{value, node, node->prev};
    node->prev->next = fresh;
    node->prev = fresh;
    ++size_;
    return true;
}

bool DoublyLinkedList::pushAfter(const std::string& target, const std::string& value)
{
    Node* node = findNode(target);
    if (node == nullptr)
    {
        return false;
    }
    if (node == tail_)
    {
        pushBack(value);
        return true;
    }
    auto* fresh = new Node{value, node->next, node};
    node->next->prev = fresh;
    node->next = fresh;
    ++size_;
    return true;
}

// Исключает узел из списка и освобождает его
void DoublyLinkedList::unlink(Node* node)
{
    if (node->prev != nullptr)
    {
        node->prev->next = node->next;
    }
    else
    {
        head_ = node->next;
    }
    if (node->next != nullptr)
    {
        node->next->prev = node->prev;
    }
    else
    {
        tail_ = node->prev;
    }
    delete node;
    --size_;
}

void DoublyLinkedList::popFront()
{
    if (head_ == nullptr)
    {
        throw std::out_of_range("list is empty");
    }
    unlink(head_);
}

void DoublyLinkedList::popBack()
{
    if (tail_ == nullptr)
    {
        throw std::out_of_range("list is empty");
    }
    unlink(tail_);
}

bool DoublyLinkedList::popBefore(const std::string& target)
{
    const Node* node = findNode(target);
    if (node == nullptr || node->prev == nullptr)
    {
        return false;
    }
    unlink(node->prev);
    return true;
}

bool DoublyLinkedList::popAfter(const std::string& target)
{
    const Node* node = findNode(target);
    if (node == nullptr || node->next == nullptr)
    {
        return false;
    }
    unlink(node->next);
    return true;
}

bool DoublyLinkedList::removeByValue(const std::string& value)
{
    Node* node = findNode(value);
    if (node == nullptr)
    {
        return false;
    }
    unlink(node);
    return true;
}

DoublyLinkedList::Node* DoublyLinkedList::findNode(const std::string& value) const
{
    for (Node* node = head_; node != nullptr; node = node->next)
    {
        if (node->value == value)
        {
            return node;
        }
    }
    return nullptr;
}

bool DoublyLinkedList::contains(const std::string& value) const
{
    return findNode(value) != nullptr;
}

long DoublyLinkedList::indexOf(const std::string& value) const
{
    long index = 0;
    for (const Node* node = head_; node != nullptr; node = node->next, ++index)
    {
        if (node->value == value)
        {
            return index;
        }
    }
    return -1;
}

std::vector<std::string> DoublyLinkedList::toVector() const
{
    std::vector<std::string> values;
    values.reserve(size_);
    for (const Node* node = head_; node != nullptr; node = node->next)
    {
        values.push_back(node->value);
    }
    return values;
}

std::vector<std::string> DoublyLinkedList::toVectorBackward() const
{
    std::vector<std::string> values;
    values.reserve(size_);
    for (const Node* node = tail_; node != nullptr; node = node->prev)
    {
        values.push_back(node->value);
    }
    return values;
}

std::string DoublyLinkedList::toStringBackward() const
{
    if (head_ == nullptr)
    {
        return emptyText();
    }
    return join(toVectorBackward(), " <-> ");
}

std::string DoublyLinkedList::toStringFrom(const std::string& value) const
{
    std::vector<std::string> values;
    for (const Node* node = findNode(value); node != nullptr; node = node->next)
    {
        values.push_back(node->value);
    }
    return join(values, " <-> ");
}

std::string DoublyLinkedList::toStringFromBackward(const std::string& value) const
{
    std::vector<std::string> values;
    for (const Node* node = findNode(value); node != nullptr; node = node->prev)
    {
        values.push_back(node->value);
    }
    return join(values, " <-> ");
}

StructureType DoublyLinkedList::type() const
{
    return StructureType::DoublyList;
}

std::size_t DoublyLinkedList::size() const
{
    return size_;
}

void DoublyLinkedList::clear()
{
    while (head_ != nullptr)
    {
        const Node* node = head_;
        head_ = head_->next;
        delete node;
    }
    tail_ = nullptr;
    size_ = 0;
}

std::string DoublyLinkedList::toString() const
{
    if (head_ == nullptr)
    {
        return emptyText();
    }
    return join(toVector(), " <-> ");
}

void DoublyLinkedList::assign(const std::vector<std::string>& values)
{
    clear();
    for (const std::string& value : values)
    {
        pushBack(value);
    }
}

void DoublyLinkedList::doSaveBinary(std::ostream& out) const
{
    io::writeStrings(out, toVector());
}

void DoublyLinkedList::doLoadBinary(std::istream& in)
{
    assign(io::readStrings(in));
}

void DoublyLinkedList::doSaveText(std::ostream& out) const
{
    io::writeStringsText(out, toVector());
}

void DoublyLinkedList::doLoadText(std::istream& in)
{
    assign(io::readStringsText(in));
}

} // namespace ds
