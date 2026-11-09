#ifndef DS_HASHTABLE_H
#define DS_HASHTABLE_H

#include <cstddef>
#include <string>
#include <vector>

#include "ds/IStructure.h"

namespace ds
{

// Хеш-таблица целых чисел с открытой адресацией.
// Хеш-функция - метод свёртки (сумма групп по три цифры ключа по модулю размера).
// Разрешение коллизий: линейное пробирование или двойное хеширование.
class HashTable final : public IStructure
{
public:
    enum class Mode : unsigned char
    {
        Linear = 0,
        Double = 1
    };

    static constexpr std::size_t kDefaultCapacity = 11;

    // capacity == 0 -> std::invalid_argument
    explicit HashTable(Mode mode = Mode::Linear, std::size_t capacity = kDefaultCapacity);

    // true - ключ добавлен или уже был; false - свободных ячеек нет
    bool insert(int key);
    bool contains(int key) const;
    // true - ключ был найден и удалён
    bool erase(int key);

    Mode mode() const noexcept;
    std::size_t capacity() const noexcept;
    double loadFactor() const noexcept;
    // Число проверенных ячеек при поиске ключа (для анализа коллизий)
    std::size_t probeCount(int key) const;

    // Метод свёртки: 523456795 -> 523 + 456 + 795 = 1774 -> 1774 % capacity
    static int foldingHash(int key, std::size_t capacity);

    StructureType type() const override;
    std::size_t size() const override;
    void clear() override;
    std::string toString() const override;

private:
    enum class State : unsigned char
    {
        Empty = 0,
        Occupied = 1,
        Deleted = 2
    };

    struct Slot
    {
        int key = 0;
        State state = State::Empty;
    };

    struct Probe
    {
        std::size_t position;
        std::size_t step;
    };

    Probe makeProbe(int key) const;
    std::size_t nextPosition(const Probe& probe) const;
    static std::size_t stepHash(int key, std::size_t capacity);

    void doSaveBinary(std::ostream& out) const override;
    void doLoadBinary(std::istream& in) override;
    void doSaveText(std::ostream& out) const override;
    void doLoadText(std::istream& in) override;
    void applyLoaded(Mode mode, std::vector<Slot> slots);

    Mode mode_;
    std::vector<Slot> slots_;
    std::size_t count_ = 0;
};

} // namespace ds

#endif
