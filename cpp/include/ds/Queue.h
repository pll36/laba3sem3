#ifndef DS_QUEUE_H
#define DS_QUEUE_H

#include <cstddef>
#include <string>
#include <vector>

#include "ds/IStructure.h"

namespace ds
{

// Очередь (FIFO) на односвязных узлах с указателями на начало и конец; O(1).
class Queue final : public IStructure
{
public:
    Queue() = default;
    Queue(const Queue& other);
    Queue(Queue&& other) noexcept;
    Queue& operator=(const Queue& other);
    Queue& operator=(Queue&& other) noexcept;
    ~Queue() override;

    void push(const std::string& value);
    // pop и front бросают std::out_of_range, если очередь пуста
    std::string pop();
    const std::string& front() const;

    // Элементы от начала очереди к концу
    std::vector<std::string> toVector() const;
    void swap(Queue& other) noexcept;

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

    void assign(const std::vector<std::string>& values);
    void doSaveBinary(std::ostream& out) const override;
    void doLoadBinary(std::istream& in) override;
    void doSaveText(std::ostream& out) const override;
    void doLoadText(std::istream& in) override;

    Node* front_ = nullptr;
    Node* rear_ = nullptr;
    std::size_t size_ = 0;
};

} // namespace ds

#endif
