#ifndef DS_STRUCTUREFACTORY_H
#define DS_STRUCTUREFACTORY_H

#include <memory>

#include "ds/IStructure.h"

namespace ds
{

// Фабрика: создаёт пустую структуру нужного типа (полиморфный std::unique_ptr)
std::unique_ptr<IStructure> makeStructure(StructureType type);

} // namespace ds

#endif
