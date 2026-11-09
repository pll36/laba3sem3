#include "ds/HashTable.h"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <utility>

#include "ds/Serialization.h"
#include "ds/Text.h"

namespace ds
{

namespace
{

// Модуль числа без UB для INT_MIN
std::uint32_t magnitudeOf(int key)
{
    const auto raw = static_cast<std::uint32_t>(key);
    return key < 0 ? 0U - raw : raw;
}

const char* modeName(HashTable::Mode mode)
{
    return mode == HashTable::Mode::Linear ? "linear" : "double";
}

} // namespace

HashTable::HashTable(Mode mode, std::size_t capacity) : mode_(mode), slots_(capacity)
{
    if (capacity == 0)
    {
        throw std::invalid_argument("capacity must be positive");
    }
}

int HashTable::foldingHash(int key, std::size_t capacity)
{
    if (capacity == 0)
    {
        return 0;
    }
    // Суммируем группы по три цифры справа налево
    std::uint64_t sum = 0;
    std::uint32_t rest = magnitudeOf(key);
    while (rest > 0)
    {
        sum += rest % 1000U;
        rest /= 1000U;
    }
    return static_cast<int>(sum % capacity);
}

// Шаг для двойного хеширования; взаимно прост с размером таблицы
std::size_t HashTable::stepHash(int key, std::size_t capacity)
{
    if (capacity <= 2)
    {
        return 1;
    }
    std::size_t step = 1 + magnitudeOf(key) % (capacity - 1);
    while (std::gcd(step, capacity) != 1)
    {
        ++step;
        if (step >= capacity)
        {
            step = 1;
        }
    }
    return step;
}

HashTable::Probe HashTable::makeProbe(int key) const
{
    Probe probe{};
    probe.position = static_cast<std::size_t>(foldingHash(key, slots_.size()));
    probe.step = mode_ == Mode::Double ? stepHash(key, slots_.size()) : 1;
    return probe;
}

std::size_t HashTable::nextPosition(const Probe& probe) const
{
    return (probe.position + probe.step) % slots_.size();
}

bool HashTable::insert(int key)
{
    Probe probe = makeProbe(key);
    bool hasDeleted = false;
    std::size_t firstDeleted = 0;
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        Slot& slot = slots_[probe.position];
        if (slot.state == State::Occupied)
        {
            if (slot.key == key)
            {
                return true; // ключ уже есть
            }
        }
        else if (slot.state == State::Deleted)
        {
            if (!hasDeleted)
            {
                hasDeleted = true;
                firstDeleted = probe.position;
            }
        }
        else
        {
            // Пустая ячейка: дальше ключа точно нет
            Slot& target = hasDeleted ? slots_[firstDeleted] : slot;
            target = Slot{key, State::Occupied};
            ++count_;
            return true;
        }
        probe.position = nextPosition(probe);
    }
    if (hasDeleted)
    {
        slots_[firstDeleted] = Slot{key, State::Occupied};
        ++count_;
        return true;
    }
    return false; // таблица полна
}

bool HashTable::contains(int key) const
{
    Probe probe = makeProbe(key);
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        const Slot& slot = slots_[probe.position];
        if (slot.state == State::Empty)
        {
            return false;
        }
        if (slot.state == State::Occupied && slot.key == key)
        {
            return true;
        }
        probe.position = nextPosition(probe);
    }
    return false;
}

bool HashTable::erase(int key)
{
    Probe probe = makeProbe(key);
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        Slot& slot = slots_[probe.position];
        if (slot.state == State::Empty)
        {
            return false;
        }
        if (slot.state == State::Occupied && slot.key == key)
        {
            slot.state = State::Deleted;
            --count_;
            return true;
        }
        probe.position = nextPosition(probe);
    }
    return false;
}

HashTable::Mode HashTable::mode() const noexcept
{
    return mode_;
}

std::size_t HashTable::capacity() const noexcept
{
    return slots_.size();
}

double HashTable::loadFactor() const noexcept
{
    return static_cast<double>(count_) / static_cast<double>(slots_.size());
}

std::size_t HashTable::probeCount(int key) const
{
    Probe probe = makeProbe(key);
    std::size_t probes = 0;
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        const Slot& slot = slots_[probe.position];
        ++probes;
        if (slot.state == State::Empty || (slot.state == State::Occupied && slot.key == key))
        {
            break;
        }
        probe.position = nextPosition(probe);
    }
    return probes;
}

StructureType HashTable::type() const
{
    return StructureType::Hash;
}

std::size_t HashTable::size() const
{
    return count_;
}

void HashTable::clear()
{
    std::fill(slots_.begin(), slots_.end(), Slot{});
    count_ = 0;
}

std::string HashTable::toString() const
{
    std::string out = std::string("Hash(") + modeName(mode_) + ", capacity=" + std::to_string(slots_.size()) +
                      ", size=" + std::to_string(count_) + ")";
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        if (slots_[i].state == State::Occupied)
        {
            out += "\n[" + std::to_string(i) + "] " + std::to_string(slots_[i].key);
        }
    }
    return out;
}

void HashTable::applyLoaded(Mode mode, std::vector<Slot> slots)
{
    if (slots.empty())
    {
        throw SerializationError("hash table capacity must be positive");
    }
    const auto occupied =
        std::count_if(slots.begin(), slots.end(), [](const Slot& slot) { return slot.state == State::Occupied; });
    mode_ = mode;
    slots_ = std::move(slots);
    count_ = static_cast<std::size_t>(occupied);
}

void HashTable::doSaveBinary(std::ostream& out) const
{
    io::writeU8(out, static_cast<std::uint8_t>(mode_));
    io::writeU32(out, static_cast<std::uint32_t>(slots_.size()));
    for (const Slot& slot : slots_)
    {
        io::writeU8(out, static_cast<std::uint8_t>(slot.state));
        io::writeI32(out, slot.key);
    }
}

void HashTable::doLoadBinary(std::istream& in)
{
    const std::uint8_t rawMode = io::readU8(in);
    if (rawMode > 1)
    {
        throw SerializationError("invalid hash mode");
    }
    const std::uint32_t slotCount = io::readU32(in);
    std::vector<Slot> slots;
    for (std::uint32_t i = 0; i < slotCount; ++i)
    {
        const std::uint8_t rawState = io::readU8(in);
        if (rawState > 2)
        {
            throw SerializationError("invalid slot state");
        }
        const std::int32_t key = io::readI32(in);
        slots.push_back(Slot{key, static_cast<State>(rawState)});
    }
    applyLoaded(static_cast<Mode>(rawMode), std::move(slots));
}

void HashTable::doSaveText(std::ostream& out) const
{
    out << "MODE " << modeName(mode_) << '\n';
    out << "CAPACITY " << slots_.size() << '\n';
    for (const Slot& slot : slots_)
    {
        const char letter = slot.state == State::Occupied ? 'O' : (slot.state == State::Deleted ? 'D' : 'E');
        out << letter << ' ' << slot.key << '\n';
    }
}

void HashTable::doLoadText(std::istream& in)
{
    const std::string modeText = io::readField(in, "MODE");
    Mode parsedMode = Mode::Linear;
    if (modeText == "double")
    {
        parsedMode = Mode::Double;
    }
    else if (modeText != "linear")
    {
        throw SerializationError("invalid hash mode: " + modeText);
    }
    const std::uint32_t slotCount = io::parseUint32(io::readField(in, "CAPACITY"));
    std::vector<Slot> slots;
    for (std::uint32_t i = 0; i < slotCount; ++i)
    {
        const std::string line = io::readLine(in);
        if (line.size() < 3 || line[1] != ' ')
        {
            throw SerializationError("invalid slot line: '" + line + "'");
        }
        State state = State::Empty;
        if (line[0] == 'O')
        {
            state = State::Occupied;
        }
        else if (line[0] == 'D')
        {
            state = State::Deleted;
        }
        else if (line[0] != 'E')
        {
            throw SerializationError("invalid slot state: '" + line + "'");
        }
        slots.push_back(Slot{io::parseInt32(line.substr(2)), state});
    }
    applyLoaded(parsedMode, std::move(slots));
}

} // namespace ds
