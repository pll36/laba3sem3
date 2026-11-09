#ifndef LAB3_TESTUTILS_H
#define LAB3_TESTUTILS_H

#include <sstream>
#include <string>

#include "ds/IStructure.h"

namespace testutil
{

// Сериализует структуру и десериализует её в новый объект того же типа
template <typename T> T roundTripBinary(const T& source)
{
    std::stringstream buffer(std::ios::in | std::ios::out | std::ios::binary);
    source.saveBinary(buffer);
    T copy;
    copy.loadBinary(buffer);
    return copy;
}

template <typename T> T roundTripText(const T& source)
{
    std::stringstream buffer;
    source.saveText(buffer);
    T copy;
    copy.loadText(buffer);
    return copy;
}

inline std::string binaryOf(const ds::IStructure& source)
{
    std::ostringstream buffer(std::ios::binary);
    source.saveBinary(buffer);
    return buffer.str();
}

inline std::string textOf(const ds::IStructure& source)
{
    std::ostringstream buffer;
    source.saveText(buffer);
    return buffer.str();
}

} // namespace testutil

#endif
