#ifndef DS_STACK_H
#define DS_STACK_H

#include <cstddef>
#include <string>
#include <vector>

#include "ds/IStructure.h"

namespace ds
{

// Стек (LIFO) на односвязных узлах; все операции за O(1).
class Stack final : public IStructure
{
public:
    Stack() = default;
    Stack(const Stack& other);
    Stack(Stack&& other) noexcept;
    Stack& operator=(const Stack& other);
    Stack& operator=(Stack&& other) noexcept;
    ~Stack() override;

    void push(const std::string& value);
    // pop и peek бросают std::out_of_range, если стек пуст
    std::string pop();
    const std::string& peek() const;

    // Элементы от дна к вершине
    std::vector<std::string> toVector() const;
    void swap(Stack& other) noexcept;

    StructureType type() const override;
    std::size_t size() const override;
    void clear() override;
    std::string toString() const override;

private:
    struct Node
    {
        std::string value;
        Node* next;
    };

    void assign(const std::vector<std::string>& bottomToTop);
    void doSaveBinary(std::ostream& out) const override;
    void doLoadBinary(std::istream& in) override;
    void doSaveText(std::ostream& out) const override;
    void doLoadText(std::istream& in) override;

    Node* top_ = nullptr;
    std::size_t size_ = 0;
};

} // namespace ds

#endif
