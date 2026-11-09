#ifndef DS_DYNAMICARRAY_H
#define DS_DYNAMICARRAY_H

#include <cstddef>
#include <string>
#include <vector>

#include "ds/IStructure.h"

namespace ds
{

// Динамический массив строк. Память управляется вручную (new[]/delete[]),
// ёмкость удваивается при заполнении.
class DynamicArray final : public IStructure
{
public:
    static constexpr std::size_t kInitialCapacity = 10;

    DynamicArray();
    DynamicArray(const DynamicArray& other);
    DynamicArray(DynamicArray&& other) noexcept;
    DynamicArray& operator=(const DynamicArray& other);
    DynamicArray& operator=(DynamicArray&& other) noexcept;
    ~DynamicArray() override;

    // Добавление
    void pushBack(const std::string& value);
    void pushFront(const std::string& value);
    void insertAt(std::size_t index, const std::string& value);

    // Удаление (при пустом массиве / неверном индексе бросают std::out_of_range)
    void popBack();
    void popFront();
    void removeAt(std::size_t index);

    // Доступ
    const std::string& get(std::size_t index) const;
    void set(std::size_t index, const std::string& value);

    std::size_t capacity() const noexcept;
    std::vector<std::string> toVector() const;
    void swap(DynamicArray& other) noexcept;

    StructureType type() const override;
    std::size_t size() const override;
    void clear() override;
    std::string toString() const override;

private:
    void grow();
    void assign(const std::vector<std::string>& values);
    void doSaveBinary(std::ostream& out) const override;
    void doLoadBinary(std::istream& in) override;
    void doSaveText(std::ostream& out) const override;
    void doLoadText(std::istream& in) override;

    std::string* data_;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

} // namespace ds

#endif
