#include "ds/Serialization.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <system_error>

namespace ds::io
{

namespace
{

constexpr std::size_t kChunkSize = 4096;

void checkWritten(const std::ostream& out)
{
    if (!out)
    {
        throw SerializationError("write failed");
    }
}

template <typename T> T parseNumber(const std::string& text)
{
    T value{};
    const char* first = text.data();
    const char* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, value);
    if (text.empty() || result.ec != std::errc{} || result.ptr != last)
    {
        throw SerializationError("invalid number: '" + text + "'");
    }
    return value;
}

} // namespace

void writeU8(std::ostream& out, std::uint8_t value)
{
    out.put(static_cast<char>(value));
    checkWritten(out);
}

void writeU32(std::ostream& out, std::uint32_t value)
{
    for (int shift = 0; shift < 32; shift += 8)
    {
        writeU8(out, static_cast<std::uint8_t>((value >> shift) & 0xFFU));
    }
}

void writeI32(std::ostream& out, std::int32_t value)
{
    writeU32(out, static_cast<std::uint32_t>(value));
}

void writeString(std::ostream& out, const std::string& value)
{
    writeU32(out, static_cast<std::uint32_t>(value.size()));
    out.write(value.data(), static_cast<std::streamsize>(value.size()));
    checkWritten(out);
}

std::uint8_t readU8(std::istream& in)
{
    char c = 0;
    if (!in.get(c))
    {
        throw SerializationError("unexpected end of data");
    }
    return static_cast<std::uint8_t>(c);
}

std::uint32_t readU32(std::istream& in)
{
    std::uint32_t value = 0;
    for (int shift = 0; shift < 32; shift += 8)
    {
        value |= static_cast<std::uint32_t>(readU8(in)) << shift;
    }
    return value;
}

std::int32_t readI32(std::istream& in)
{
    return static_cast<std::int32_t>(readU32(in));
}

std::string readString(std::istream& in)
{
    std::uint32_t remaining = readU32(in);
    std::string value;
    std::array<char, kChunkSize> buffer{};
    // Читаем кусками, чтобы повреждённая длина не вызвала огромное выделение памяти
    while (remaining > 0)
    {
        const auto chunk = static_cast<std::streamsize>(std::min<std::size_t>(remaining, kChunkSize));
        in.read(buffer.data(), chunk);
        if (in.gcount() != chunk)
        {
            throw SerializationError("unexpected end of data");
        }
        value.append(buffer.data(), static_cast<std::size_t>(chunk));
        remaining -= static_cast<std::uint32_t>(chunk);
    }
    return value;
}

void writeStrings(std::ostream& out, const std::vector<std::string>& values)
{
    writeU32(out, static_cast<std::uint32_t>(values.size()));
    for (const std::string& value : values)
    {
        writeString(out, value);
    }
}

std::vector<std::string> readStrings(std::istream& in)
{
    const std::uint32_t count = readU32(in);
    std::vector<std::string> values;
    for (std::uint32_t i = 0; i < count; ++i)
    {
        values.push_back(readString(in));
    }
    return values;
}

std::string escapeLine(const std::string& value)
{
    std::string result;
    for (const char c : value)
    {
        if (c == '\\')
        {
            result += "\\\\";
        }
        else if (c == '\n')
        {
            result += "\\n";
        }
        else if (c == '\r')
        {
            result += "\\r";
        }
        else
        {
            result += c;
        }
    }
    return result;
}

std::string unescapeLine(const std::string& value)
{
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        if (value[i] != '\\')
        {
            result += value[i];
            continue;
        }
        if (i + 1 >= value.size())
        {
            throw SerializationError("dangling escape character");
        }
        ++i;
        if (value[i] == '\\')
        {
            result += '\\';
        }
        else if (value[i] == 'n')
        {
            result += '\n';
        }
        else if (value[i] == 'r')
        {
            result += '\r';
        }
        else
        {
            throw SerializationError("unknown escape sequence");
        }
    }
    return result;
}

std::string readLine(std::istream& in)
{
    std::string line;
    if (!std::getline(in, line))
    {
        throw SerializationError("unexpected end of data");
    }
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }
    return line;
}

std::string readField(std::istream& in, const std::string& key)
{
    const std::string line = readLine(in);
    if (line == key)
    {
        return {};
    }
    if (line.size() > key.size() && line.compare(0, key.size(), key) == 0 && line[key.size()] == ' ')
    {
        return line.substr(key.size() + 1);
    }
    throw SerializationError("expected field " + key);
}

std::int32_t parseInt32(const std::string& text)
{
    return parseNumber<std::int32_t>(text);
}

std::uint32_t parseUint32(const std::string& text)
{
    return parseNumber<std::uint32_t>(text);
}

void writeStringsText(std::ostream& out, const std::vector<std::string>& values)
{
    out << "COUNT " << values.size() << '\n';
    for (const std::string& value : values)
    {
        out << escapeLine(value) << '\n';
    }
    checkWritten(out);
}

std::vector<std::string> readStringsText(std::istream& in)
{
    const std::uint32_t count = parseUint32(readField(in, "COUNT"));
    std::vector<std::string> values;
    for (std::uint32_t i = 0; i < count; ++i)
    {
        values.push_back(unescapeLine(readLine(in)));
    }
    return values;
}

} // namespace ds::io
