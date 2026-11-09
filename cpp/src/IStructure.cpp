#include "ds/IStructure.h"

#include <algorithm>
#include <array>
#include <istream>
#include <ostream>

#include "ds/Serialization.h"

namespace ds
{

namespace
{

constexpr std::array<char, 4> kBinaryMagic = {'D', 'S', 'B', '1'};
const char* const kTextMagic = "DSTXT 1";

struct TypeInfo
{
    StructureType type;
    const char* name;
};

constexpr std::array<TypeInfo, 7> kTypes = {{{StructureType::Array, "array"},
                                             {StructureType::SinglyList, "slist"},
                                             {StructureType::DoublyList, "dlist"},
                                             {StructureType::Stack, "stack"},
                                             {StructureType::Queue, "queue"},
                                             {StructureType::Tree, "tree"},
                                             {StructureType::Hash, "hash"}}};

void checkSameType(StructureType expected, StructureType stored)
{
    if (stored != expected)
    {
        throw SerializationError(std::string("structure type mismatch: expected ") + typeName(expected) + ", got " +
                                 typeName(stored));
    }
}

} // namespace

const char* typeName(StructureType type) noexcept
{
    const auto found =
        std::find_if(kTypes.begin(), kTypes.end(), [type](const TypeInfo& info) { return info.type == type; });
    return found == kTypes.end() ? "unknown" : found->name;
}

StructureType parseTypeName(const std::string& name)
{
    const auto found =
        std::find_if(kTypes.begin(), kTypes.end(), [&name](const TypeInfo& info) { return name == info.name; });
    if (found == kTypes.end())
    {
        throw SerializationError("unknown structure type: " + name);
    }
    return found->type;
}

StructureType readBinaryType(std::istream& in)
{
    std::array<char, 4> magic{};
    in.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (in.gcount() != static_cast<std::streamsize>(magic.size()) || magic != kBinaryMagic)
    {
        throw SerializationError("bad binary signature");
    }
    const std::uint8_t raw = io::readU8(in);
    const auto found = std::find_if(kTypes.begin(), kTypes.end(), [raw](const TypeInfo& info)
                                    { return static_cast<std::uint8_t>(info.type) == raw; });
    if (found == kTypes.end())
    {
        throw SerializationError("unknown structure type tag");
    }
    return found->type;
}

StructureType readTextType(std::istream& in)
{
    if (io::readLine(in) != kTextMagic)
    {
        throw SerializationError("bad text signature");
    }
    return parseTypeName(io::readField(in, "TYPE"));
}

void IStructure::saveBinary(std::ostream& out) const
{
    out.write(kBinaryMagic.data(), static_cast<std::streamsize>(kBinaryMagic.size()));
    io::writeU8(out, static_cast<std::uint8_t>(type()));
    doSaveBinary(out);
}

void IStructure::loadBinary(std::istream& in)
{
    checkSameType(type(), readBinaryType(in));
    doLoadBinary(in);
}

void IStructure::saveText(std::ostream& out) const
{
    out << kTextMagic << '\n' << "TYPE " << typeName(type()) << '\n';
    doSaveText(out);
}

void IStructure::loadText(std::istream& in)
{
    checkSameType(type(), readTextType(in));
    doLoadText(in);
}

} // namespace ds
