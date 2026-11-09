#ifndef DS_SINGLYLINKEDLIST_H
#define DS_SINGLYLINKEDLIST_H

#include <cstddef>
#include <string>
#include <vector>

#include "ds/IStructure.h"

namespace ds
{

// Односвязный список строк (хранится только указатель на голову,
// поэтому операции в конце списка выполняются за O(N)).
class SinglyLinkedList final : public IStructure
{
public:
    SinglyLinkedList() = default;
    SinglyLinkedList(const SinglyLinkedList& other);
    SinglyLinkedList(SinglyLinkedList&& other) noexcept;
    SinglyLinkedList& operator=(const SinglyLinkedList& other);
    SinglyLinkedList& operator=(SinglyLinkedList&& other) noexcept;
    ~SinglyLinkedList() override;

    // Добавление: голова, хвост, до/после элемента (false, если элемент не найден)
    void pushFront(const std::string& value);
    void pushBack(const std::string& value);
    bool pushBefore(const std::string& target, const std::string& value);
    bool pushAfter(const std::string& target, const std::string& value);

    // Удаление: голова, хвост (при пустом списке бросают std::out_of_range),
    // до/после элемента и по значению (false, если удалять нечего)
    void popFront();
    void popBack();
    bool popBefore(const std::string& target);
    bool popAfter(const std::string& target);
    bool removeByValue(const std::string& value);

    // Поиск
    bool contains(const std::string& value) const;
    // Индекс первого вхождения или -1
    long indexOf(const std::string& value) const;

    // Чтение
    std::vector<std::string> toVector() const;
    // Содержимое, начиная с найденного элемента; пустая строка, если элемента нет
    std::string toStringFrom(const std::string& value) const;

    void swap(SinglyLinkedList& other) noexcept;

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

    Node* head_ = nullptr;
    std::size_t size_ = 0;
};

} // namespace ds

#endif
