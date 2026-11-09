#ifndef DS_DOUBLYLINKEDLIST_H
#define DS_DOUBLYLINKEDLIST_H

#include <cstddef>
#include <string>
#include <vector>

#include "ds/IStructure.h"

namespace ds
{

// Двусвязный список строк (указатели на голову и хвост:
// операции с обоих концов выполняются за O(1)).
class DoublyLinkedList final : public IStructure
{
public:
    DoublyLinkedList() = default;
    DoublyLinkedList(const DoublyLinkedList& other);
    DoublyLinkedList(DoublyLinkedList&& other) noexcept;
    DoublyLinkedList& operator=(const DoublyLinkedList& other);
    DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept;
    ~DoublyLinkedList() override;

    void pushFront(const std::string& value);
    void pushBack(const std::string& value);
    bool pushBefore(const std::string& target, const std::string& value);
    bool pushAfter(const std::string& target, const std::string& value);

    // popFront/popBack бросают std::out_of_range при пустом списке
    void popFront();
    void popBack();
    bool popBefore(const std::string& target);
    bool popAfter(const std::string& target);
    bool removeByValue(const std::string& value);

    bool contains(const std::string& value) const;
    long indexOf(const std::string& value) const;

    std::vector<std::string> toVector() const;
    std::vector<std::string> toVectorBackward() const;
    std::string toStringBackward() const;
    // Чтение от найденного элемента вперёд/назад; пустая строка, если элемента нет
    std::string toStringFrom(const std::string& value) const;
    std::string toStringFromBackward(const std::string& value) const;

    void swap(DoublyLinkedList& other) noexcept;

    StructureType type() const override;
    std::size_t size() const override;
    void clear() override;
    std::string toString() const override;

private:
    struct Node
    {
        std::string value;
        Node* next;
        Node* prev;
    };

    Node* findNode(const std::string& value) const;
    void unlink(Node* node);
    void assign(const std::vector<std::string>& values);
    void doSaveBinary(std::ostream& out) const override;
    void doLoadBinary(std::istream& in) override;
    void doSaveText(std::ostream& out) const override;
    void doLoadText(std::istream& in) override;

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    std::size_t size_ = 0;
};

} // namespace ds

#endif
