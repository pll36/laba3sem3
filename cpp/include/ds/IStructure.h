#ifndef DS_ISTRUCTURE_H
#define DS_ISTRUCTURE_H

#include <cstddef>
#include <iosfwd>
#include <string>

namespace ds
{

// Тип структуры данных (записывается в заголовок сериализованных данных)
enum class StructureType : unsigned char
{
    Array = 1,
    SinglyList = 2,
    DoublyList = 3,
    Stack = 4,
    Queue = 5,
    Tree = 6,
    Hash = 7
};

// Короткое имя типа: array, slist, dlist, stack, queue, tree, hash
const char* typeName(StructureType type) noexcept;
// Обратное преобразование; бросает SerializationError для неизвестного имени
StructureType parseTypeName(const std::string& name);
// Тип из заголовка потока; позиция потока после вызова - сразу за заголовком
StructureType readBinaryType(std::istream& in);
StructureType readTextType(std::istream& in);

// Интерфейс (абстрактный класс) всех структур данных проекта.
// Сериализация построена по идиоме NVI (Non-Virtual Interface):
// публичные методы пишут/проверяют заголовок, а содержимое - виртуальные do*-методы.
class IStructure
{
public:
    IStructure() = default;
    IStructure(const IStructure&) = default;
    IStructure(IStructure&&) = default;
    IStructure& operator=(const IStructure&) = default;
    IStructure& operator=(IStructure&&) = default;
    virtual ~IStructure() = default;

    virtual StructureType type() const = 0;
    virtual std::size_t size() const = 0;
    virtual void clear() = 0;
    virtual std::string toString() const = 0;

    bool empty() const { return size() == 0; }

    void saveBinary(std::ostream& out) const;
    void loadBinary(std::istream& in);
    void saveText(std::ostream& out) const;
    void loadText(std::istream& in);

private:
    virtual void doSaveBinary(std::ostream& out) const = 0;
    virtual void doLoadBinary(std::istream& in) = 0;
    virtual void doSaveText(std::ostream& out) const = 0;
    virtual void doLoadText(std::istream& in) = 0;
};

} // namespace ds

#endif
