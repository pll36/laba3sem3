#ifndef DS_SERIALIZATION_H
#define DS_SERIALIZATION_H

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ds
{

// Ошибка чтения/записи сериализованных данных
class SerializationError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

namespace io
{

// ---- Бинарный формат (little-endian, не зависит от платформы) ----
void writeU8(std::ostream& out, std::uint8_t value);
void writeU32(std::ostream& out, std::uint32_t value);
void writeI32(std::ostream& out, std::int32_t value);
void writeString(std::ostream& out, const std::string& value);

std::uint8_t readU8(std::istream& in);
std::uint32_t readU32(std::istream& in);
std::int32_t readI32(std::istream& in);
std::string readString(std::istream& in);

// Последовательность строк: u32 количество, затем строки (u32 длина + байты)
void writeStrings(std::ostream& out, const std::vector<std::string>& values);
std::vector<std::string> readStrings(std::istream& in);

// ---- Текстовый формат (построчный) ----
std::string escapeLine(const std::string& value);
std::string unescapeLine(const std::string& value);
std::string readLine(std::istream& in);
// Читает строку вида "KEY значение" и проверяет ключ
std::string readField(std::istream& in, const std::string& key);
std::int32_t parseInt32(const std::string& text);
std::uint32_t parseUint32(const std::string& text);

// Последовательность строк: "COUNT n", затем n экранированных строк
void writeStringsText(std::ostream& out, const std::vector<std::string>& values);
std::vector<std::string> readStringsText(std::istream& in);

} // namespace io
} // namespace ds

#endif
