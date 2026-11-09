#include "ds/DynamicArray.h"

#include <stdexcept>
#include <utility>

#include "ds/Serialization.h"
#include "ds/Text.h"

namespace ds
{

DynamicArray::DynamicArray() : data_(new std::string[kInitialCapacity]), capacity_(kInitialCapacity) {}

DynamicArray::DynamicArray(const DynamicArray& other)
    : data_(new std::string[other.capacity_]), size_(other.size_), capacity_(other.capacity_)
{
    try
    {
        for (std::size_t i = 0; i < size_; ++i)
        {
            data_[i] = other.data_[i];
        }
    }
    catch (...)
    {
        delete[] data_;
        throw;
    }
}

DynamicArray::DynamicArray(DynamicArray&& other) noexcept
    : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)),
      capacity_(std::exchange(other.capacity_, 0))
{
}

DynamicArray& DynamicArray::operator=(const DynamicArray& other)
{
    DynamicArray copy(other); // copy-and-swap: при исключении текущий объект не меняется
    swap(copy);
    return *this;
}

DynamicArray& DynamicArray::operator=(DynamicArray&& other) noexcept
{
    DynamicArray moved(std::move(other));
    swap(moved);
    return *this;
}

DynamicArray::~DynamicArray()
{
    delete[] data_;
}

void DynamicArray::swap(DynamicArray& other) noexcept
{
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

void DynamicArray::grow()
{
    const std::size_t newCapacity = capacity_ == 0 ? kInitialCapacity : capacity_ * 2;
    auto* fresh = new std::string[newCapacity];
    for (std::size_t i = 0; i < size_; ++i)
    {
        fresh[i] = std::move(data_[i]);
    }
    delete[] data_;
    data_ = fresh;
    capacity_ = newCapacity;
}

void DynamicArray::pushBack(const std::string& value)
{
    if (size_ >= capacity_)
    {
        grow();
    }
    data_[size_] = value;
    ++size_;
}

void DynamicArray::pushFront(const std::string& value)
{
    insertAt(0, value);
}

void DynamicArray::insertAt(std::size_t index, const std::string& value)
{
    if (index > size_)
    {
        throw std::out_of_range("index out of range");
    }
    if (size_ >= capacity_)
    {
        grow();
    }
    for (std::size_t i = size_; i > index; --i)
    {
        data_[i] = std::move(data_[i - 1]);
    }
    data_[index] = value;
    ++size_;
}

void DynamicArray::popBack()
{
    if (size_ == 0)
    {
        throw std::out_of_range("array is empty");
    }
    removeAt(size_ - 1);
}

void DynamicArray::popFront()
{
    if (size_ == 0)
    {
        throw std::out_of_range("array is empty");
    }
    removeAt(0);
}

void DynamicArray::removeAt(std::size_t index)
{
    if (index >= size_)
    {
        throw std::out_of_range("index out of range");
    }
    for (std::size_t i = index; i + 1 < size_; ++i)
    {
        data_[i] = std::move(data_[i + 1]);
    }
    --size_;
    data_[size_].clear();
}

const std::string& DynamicArray::get(std::size_t index) const
{
    if (index >= size_)
    {
        throw std::out_of_range("index out of range");
    }
    return data_[index];
}

void DynamicArray::set(std::size_t index, const std::string& value)
{
    if (index >= size_)
    {
        throw std::out_of_range("index out of range");
    }
    data_[index] = value;
}

std::size_t DynamicArray::capacity() const noexcept
{
    return capacity_;
}

std::vector<std::string> DynamicArray::toVector() const
{
    return {data_, data_ + size_};
}

StructureType DynamicArray::type() const
{
    return StructureType::Array;
}

std::size_t DynamicArray::size() const
{
    return size_;
}

void DynamicArray::clear()
{
    for (std::size_t i = 0; i < size_; ++i)
    {
        data_[i].clear();
    }
    size_ = 0;
}

std::string DynamicArray::toString() const
{
    if (size_ == 0)
    {
        return emptyText();
    }
    return "[" + join(toVector(), ", ") + "]";
}

void DynamicArray::assign(const std::vector<std::string>& values)
{
    clear();
    for (const std::string& value : values)
    {
        pushBack(value);
    }
}

void DynamicArray::doSaveBinary(std::ostream& out) const
{
    io::writeStrings(out, toVector());
}

void DynamicArray::doLoadBinary(std::istream& in)
{
    assign(io::readStrings(in));
}

void DynamicArray::doSaveText(std::ostream& out) const
{
    io::writeStringsText(out, toVector());
}

void DynamicArray::doLoadText(std::istream& in)
{
    assign(io::readStringsText(in));
}

} // namespace ds
