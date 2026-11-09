#include "ds/Stack.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "ds/Serialization.h"
#include "ds/Text.h"

namespace ds
{

Stack::Stack(const Stack& other)
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

Stack::Stack(Stack&& other) noexcept : top_(std::exchange(other.top_, nullptr)), size_(std::exchange(other.size_, 0)) {}

Stack& Stack::operator=(const Stack& other)
{
    Stack copy(other); // copy-and-swap: при исключении текущий объект не меняется
    swap(copy);
    return *this;
}

Stack& Stack::operator=(Stack&& other) noexcept
{
    Stack moved(std::move(other));
    swap(moved);
    return *this;
}

Stack::~Stack()
{
    clear();
}

void Stack::swap(Stack& other) noexcept
{
    std::swap(top_, other.top_);
    std::swap(size_, other.size_);
}

void Stack::push(const std::string& value)
{
    // cppcheck-suppress publicAllocationError
    top_ = new Node{value, top_};
    ++size_;
}

std::string Stack::pop()
{
    if (top_ == nullptr)
    {
        throw std::out_of_range("stack is empty");
    }
    const Node* node = top_;
    std::string value = node->value;
    top_ = node->next;
    delete node;
    --size_;
    return value;
}

const std::string& Stack::peek() const
{
    if (top_ == nullptr)
    {
        throw std::out_of_range("stack is empty");
    }
    return top_->value;
}

std::vector<std::string> Stack::toVector() const
{
    std::vector<std::string> values;
    values.reserve(size_);
    for (const Node* node = top_; node != nullptr; node = node->next)
    {
        values.push_back(node->value);
    }
    std::reverse(values.begin(), values.end());
    return values;
}

StructureType Stack::type() const
{
    return StructureType::Stack;
}

std::size_t Stack::size() const
{
    return size_;
}

void Stack::clear()
{
    while (top_ != nullptr)
    {
        const Node* node = top_;
        top_ = top_->next;
        delete node;
    }
    size_ = 0;
}

std::string Stack::toString() const
{
    if (top_ == nullptr)
    {
        return emptyText();
    }
    std::vector<std::string> values = toVector();
    std::reverse(values.begin(), values.end());
    return "Stack(top->bottom): " + join(values, ", ");
}

void Stack::assign(const std::vector<std::string>& bottomToTop)
{
    clear();
    for (const std::string& value : bottomToTop)
    {
        push(value);
    }
}

void Stack::doSaveBinary(std::ostream& out) const
{
    io::writeStrings(out, toVector());
}

void Stack::doLoadBinary(std::istream& in)
{
    assign(io::readStrings(in));
}

void Stack::doSaveText(std::ostream& out) const
{
    io::writeStringsText(out, toVector());
}

void Stack::doLoadText(std::istream& in)
{
    assign(io::readStringsText(in));
}

} // namespace ds
