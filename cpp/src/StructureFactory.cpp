#include "ds/StructureFactory.h"

#include "ds/CompleteBinaryTree.h"
#include "ds/DoublyLinkedList.h"
#include "ds/DynamicArray.h"
#include "ds/HashTable.h"
#include "ds/Queue.h"
#include "ds/SinglyLinkedList.h"
#include "ds/Stack.h"

namespace ds
{

std::unique_ptr<IStructure> makeStructure(StructureType type)
{
    switch (type)
    {
    case StructureType::Array:
        return std::make_unique<DynamicArray>();
    case StructureType::SinglyList:
        return std::make_unique<SinglyLinkedList>();
    case StructureType::DoublyList:
        return std::make_unique<DoublyLinkedList>();
    case StructureType::Stack:
        return std::make_unique<Stack>();
    case StructureType::Queue:
        return std::make_unique<Queue>();
    case StructureType::Tree:
        return std::make_unique<CompleteBinaryTree>();
    case StructureType::Hash:
        return std::make_unique<HashTable>();
    }
    return nullptr;
}

} // namespace ds
