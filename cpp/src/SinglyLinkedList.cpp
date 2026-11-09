#include "ds/SinglyLinkedList.h"

#include <stdexcept>
#include <utility>

#include "ds/Serialization.h"
#include "ds/Text.h"

namespace ds
{

SinglyLinkedList::SinglyLinkedList(const SinglyLinkedList& other)
{
    Node* tail = nullptr;
    try
    {
        for (const Node* node = other.head_; node != nullptr; node = node->next)
        {
            auto* fresh = new Node{node->value, nullptr};
            if (tail == nullptr)
            {
                head_ = fresh;
            }
            else
            {
                tail->next = fresh;
            }
            tail = fresh;
            ++size_;
        }
    }
    catch (...)
    {
        clear();
        throw;
    }
}

SinglyLinkedList::SinglyLinkedList(SinglyLinkedList&& other) noexcept
    : head_(std::exchange(other.head_, nullptr)), size_(std::exchange(other.size_, 0))
{
}

SinglyLinkedList& SinglyLinkedList::operator=(const SinglyLinkedList& other)
{
    SinglyLinkedList copy(other); // copy-and-swap: при исключении текущий объект не меняется
    swap(copy);
    return *this;
}

SinglyLinkedList& SinglyLinkedList::operator=(SinglyLinkedList&& other) noexcept
{
    SinglyLinkedList moved(std::move(other));
    swap(moved);
    return *this;
}

SinglyLinkedList::~SinglyLinkedList()
{
    clear();
}

void SinglyLinkedList::swap(SinglyLinkedList& other) noexcept
{
    std::swap(head_, other.head_);
    std::swap(size_, other.size_);
}

void SinglyLinkedList::pushFront(const std::string& value)
{
    // cppcheck-suppress publicAllocationError
    head_ = new Node{value, head_};
    ++size_;
}

void SinglyLinkedList::pushBack(const std::string& value)
{
    if (head_ == nullptr)
    {
        pushFront(value);
        return;
    }
    Node* last = head_;
    while (last->next != nullptr)
    {
        last = last->next;
    }
    last->next = new Node{value, nullptr};
    ++size_;
}

bool SinglyLinkedList::pushBefore(const std::string& target, const std::string& value)
{
    Node* previous = nullptr;
    for (Node* node = head_; node != nullptr; previous = node, node = node->next)
    {
        if (node->value != target)
        {
            continue;
        }
        if (previous == nullptr)
        {
            pushFront(value);
        }
        else
        {
            previous->next = new Node{value, node};
            ++size_;
        }
        return true;
    }
    return false;
}

bool SinglyLinkedList::pushAfter(const std::string& target, const std::string& value)
{
    for (Node* node = head_; node != nullptr; node = node->next)
    {
        if (node->value == target)
        {
            node->next = new Node{value, node->next};
            ++size_;
            return true;
        }
    }
    return false;
}

void SinglyLinkedList::popFront()
{
    if (head_ == nullptr)
    {
        throw std::out_of_range("list is empty");
    }
    const Node* node = head_;
    head_ = head_->next;
    delete node;
    --size_;
}

void SinglyLinkedList::popBack()
{
    if (head_ == nullptr)
    {
        throw std::out_of_range("list is empty");
    }
    if (head_->next == nullptr)
    {
        popFront();
        return;
    }
    Node* previous = head_;
    while (previous->next->next != nullptr)
    {
        previous = previous->next;
    }
    delete previous->next;
    previous->next = nullptr;
    --size_;
}

bool SinglyLinkedList::popBefore(const std::string& target)
{
    Node* beforePrevious = nullptr;
    Node* previous = nullptr;
    for (Node* node = head_; node != nullptr; beforePrevious = previous, previous = node, node = node->next)
    {
        if (node->value != target)
        {
            continue;
        }
        if (previous == nullptr)
        {
            return false; // перед головой ничего нет
        }
        if (beforePrevious == nullptr)
        {
            head_ = node;
        }
        else
        {
            beforePrevious->next = node;
        }
        delete previous;
        --size_;
        return true;
    }
    return false;
}

bool SinglyLinkedList::popAfter(const std::string& target)
{
    for (Node* node = head_; node != nullptr; node = node->next)
    {
        if (node->value != target)
        {
            continue;
        }
        if (node->next == nullptr)
        {
            return false; // после хвоста ничего нет
        }
        const Node* removed = node->next;
        node->next = removed->next;
        delete removed;
        --size_;
        return true;
    }
    return false;
}

bool SinglyLinkedList::removeByValue(const std::string& value)
{
    Node* previous = nullptr;
    for (Node* node = head_; node != nullptr; previous = node, node = node->next)
    {
        if (node->value != value)
        {
            continue;
        }
        if (previous == nullptr)
        {
            head_ = node->next;
        }
        else
        {
            previous->next = node->next;
        }
        delete node;
        --size_;
        return true;
    }
    return false;
}

bool SinglyLinkedList::contains(const std::string& value) const
{
    return indexOf(value) >= 0;
}

long SinglyLinkedList::indexOf(const std::string& value) const
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

std::vector<std::string> SinglyLinkedList::toVector() const
{
    std::vector<std::string> values;
    values.reserve(size_);
    for (const Node* node = head_; node != nullptr; node = node->next)
    {
        values.push_back(node->value);
    }
    return values;
}

std::string SinglyLinkedList::toStringFrom(const std::string& value) const
{
    std::vector<std::string> values;
    bool found = false;
    for (const Node* node = head_; node != nullptr; node = node->next)
    {
        found = found || node->value == value;
        if (found)
        {
            values.push_back(node->value);
        }
    }
    return join(values, " -> ");
}

StructureType SinglyLinkedList::type() const
{
    return StructureType::SinglyList;
}

std::size_t SinglyLinkedList::size() const
{
    return size_;
}

void SinglyLinkedList::clear()
{
    while (head_ != nullptr)
    {
        const Node* node = head_;
        head_ = head_->next;
        delete node;
    }
    size_ = 0;
}

std::string SinglyLinkedList::toString() const
{
    if (head_ == nullptr)
    {
        return emptyText();
    }
    return join(toVector(), " -> ");
}

void SinglyLinkedList::assign(const std::vector<std::string>& values)
{
    clear();
    Node* tail = nullptr;
    for (const std::string& value : values)
    {
        auto* fresh = new Node{value, nullptr};
        if (tail == nullptr)
        {
            head_ = fresh;
        }
        else
        {
            tail->next = fresh;
        }
        tail = fresh;
        ++size_;
    }
}

void SinglyLinkedList::doSaveBinary(std::ostream& out) const
{
    io::writeStrings(out, toVector());
}

void SinglyLinkedList::doLoadBinary(std::istream& in)
{
    assign(io::readStrings(in));
}

void SinglyLinkedList::doSaveText(std::ostream& out) const
{
    io::writeStringsText(out, toVector());
}

void SinglyLinkedList::doLoadText(std::istream& in)
{
    assign(io::readStringsText(in));
}

} // namespace ds
