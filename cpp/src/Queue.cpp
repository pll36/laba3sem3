#include "ds/Queue.h"

#include <stdexcept>
#include <utility>

#include "ds/Serialization.h"
#include "ds/Text.h"

namespace ds
{

Queue::Queue(const Queue& other)
{
    try
    {
        assign(other.toVector());
    }
    catch (...)
    {
        clear();
        throw;
    }
}

Queue::Queue(Queue&& other) noexcept
    : front_(std::exchange(other.front_, nullptr)), rear_(std::exchange(other.rear_, nullptr)),
      size_(std::exchange(other.size_, 0))
{
}

Queue& Queue::operator=(const Queue& other)
{
    Queue copy(other); // copy-and-swap: при исключении текущий объект не меняется
    swap(copy);
    return *this;
}

Queue& Queue::operator=(Queue&& other) noexcept
{
    Queue moved(std::move(other));
    swap(moved);
    return *this;
}

Queue::~Queue()
{
    clear();
}

void Queue::swap(Queue& other) noexcept
{
    std::swap(front_, other.front_);
    std::swap(rear_, other.rear_);
    std::swap(size_, other.size_);
}

void Queue::push(const std::string& value)
{
    auto* fresh = new Node{value, nullptr};
    if (rear_ != nullptr)
    {
        rear_->next = fresh;
    }
    else
    {
        front_ = fresh;
    }
    rear_ = fresh;
    ++size_;
}

std::string Queue::pop()
{
    if (front_ == nullptr)
    {
        throw std::out_of_range("queue is empty");
    }
    const Node* node = front_;
    std::string value = node->value;
    front_ = node->next;
    if (front_ == nullptr)
    {
        rear_ = nullptr;
    }
    delete node;
    --size_;
    return value;
}

const std::string& Queue::front() const
{
    if (front_ == nullptr)
    {
        throw std::out_of_range("queue is empty");
    }
    return front_->value;
}

std::vector<std::string> Queue::toVector() const
{
    std::vector<std::string> values;
    values.reserve(size_);
    for (const Node* node = front_; node != nullptr; node = node->next)
    {
        values.push_back(node->value);
    }
    return values;
}

StructureType Queue::type() const
{
    return StructureType::Queue;
}

std::size_t Queue::size() const
{
    return size_;
}

void Queue::clear()
{
    while (front_ != nullptr)
    {
        const Node* node = front_;
        front_ = front_->next;
        delete node;
    }
    rear_ = nullptr;
    size_ = 0;
}

std::string Queue::toString() const
{
    if (front_ == nullptr)
    {
        return emptyText();
    }
    return "Queue(front->rear): " + join(toVector(), ", ");
}

void Queue::assign(const std::vector<std::string>& values)
{
    clear();
    for (const std::string& value : values)
    {
        push(value);
    }
}

void Queue::doSaveBinary(std::ostream& out) const
{
    io::writeStrings(out, toVector());
}

void Queue::doLoadBinary(std::istream& in)
{
    assign(io::readStrings(in));
}

void Queue::doSaveText(std::ostream& out) const
{
    io::writeStringsText(out, toVector());
}

void Queue::doLoadText(std::istream& in)
{
    assign(io::readStringsText(in));
}

} // namespace ds
