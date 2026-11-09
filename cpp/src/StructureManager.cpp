#include "ds/StructureManager.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "ds/CompleteBinaryTree.h"
#include "ds/DoublyLinkedList.h"
#include "ds/DynamicArray.h"
#include "ds/HashTable.h"
#include "ds/Queue.h"
#include "ds/SinglyLinkedList.h"
#include "ds/Stack.h"
#include "ds/StructureFactory.h"

namespace ds
{

namespace
{

// Ошибка в тексте команды (неверные аргументы, структура не найдена и т.п.)
class CommandError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

std::string toUpper(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return text;
}

std::string readWord(std::istringstream& args, const char* what)
{
    std::string word;
    if (!(args >> word))
    {
        throw CommandError(std::string("Missing argument: ") + what);
    }
    return word;
}

// Остаток строки (значение может содержать пробелы)
std::string readRest(std::istringstream& args, const char* what)
{
    std::string rest;
    std::getline(args >> std::ws, rest);
    while (!rest.empty() && std::isspace(static_cast<unsigned char>(rest.back())) != 0)
    {
        rest.pop_back();
    }
    if (rest.empty())
    {
        throw CommandError(std::string("Missing argument: ") + what);
    }
    return rest;
}

template <typename T> T parseNumber(const std::string& word, const char* message)
{
    T value{};
    const auto result = std::from_chars(word.data(), word.data() + word.size(), value);
    if (result.ec != std::errc{} || result.ptr != word.data() + word.size())
    {
        throw CommandError(message);
    }
    return value;
}

int readInt(std::istringstream& args)
{
    return parseNumber<int>(readWord(args, "integer"), "Integer required");
}

std::size_t readIndex(std::istringstream& args)
{
    return parseNumber<std::size_t>(readWord(args, "index"), "Non-negative index required");
}

const char* boolText(bool value)
{
    return value ? "true" : "false";
}

// ---- Общие операции для списков (односвязного и двусвязного) ----

template <typename List> void listPush(List& list, std::istringstream& args)
{
    const std::string where = toUpper(readWord(args, "position"));
    if (where == "B")
    {
        list.pushBack(readRest(args, "value"));
    }
    else if (where == "F")
    {
        list.pushFront(readRest(args, "value"));
    }
    else if (where == "BEFORE" || where == "AFTER")
    {
        const std::string target = readWord(args, "target");
        const std::string value = readRest(args, "value");
        const bool done = where == "BEFORE" ? list.pushBefore(target, value) : list.pushAfter(target, value);
        if (!done)
        {
            throw CommandError("Target not found");
        }
    }
    else
    {
        throw CommandError("Expected B, F, BEFORE or AFTER");
    }
}

template <typename List> void listPop(List& list, std::istringstream& args)
{
    const std::string where = toUpper(readWord(args, "position"));
    if (where == "B")
    {
        list.popBack();
    }
    else if (where == "F")
    {
        list.popFront();
    }
    else if (where == "BEFORE" || where == "AFTER")
    {
        const std::string target = readWord(args, "target");
        const bool done = where == "BEFORE" ? list.popBefore(target) : list.popAfter(target);
        if (!done)
        {
            throw CommandError("Target not found or no neighbour to remove");
        }
    }
    else
    {
        throw CommandError("Expected B, F, BEFORE or AFTER");
    }
}

} // namespace

StructureManager::StructureManager()
{
    registerGeneral();
    registerArray();
    registerSinglyList();
    registerDoublyList();
    registerStack();
    registerQueue();
    registerTree();
    registerHash();
}

void StructureManager::add(const std::string& name, bool mutating, Handler handler)
{
    commands_[name] = Command{mutating, std::move(handler)};
}

template <typename T> void StructureManager::create(Args& args, std::ostream& out)
{
    const std::string name = readWord(args, "name");
    if (items_.count(name) != 0)
    {
        throw CommandError("Structure '" + name + "' already exists");
    }
    items_[name] = std::make_unique<T>();
    out << "OK\n";
}

IStructure& StructureManager::getAny(const std::string& name)
{
    const auto found = items_.find(name);
    if (found == items_.end())
    {
        throw CommandError("Structure '" + name + "' not found");
    }
    return *found->second;
}

template <typename T> T& StructureManager::get(const std::string& name)
{
    IStructure& item = getAny(name);
    auto* typed = dynamic_cast<T*>(&item);
    if (typed == nullptr)
    {
        throw CommandError("Structure '" + name + "' has another type (" + typeName(item.type()) + ")");
    }
    return *typed;
}

StructureManager::Status StructureManager::execute(const std::string& line, std::ostream& out)
{
    Args args(line);
    std::string name;
    if (!(args >> name))
    {
        return Status::Ok; // пустая строка
    }
    name = toUpper(name);
    if (name == "EXIT")
    {
        return Status::Exit;
    }
    const auto found = commands_.find(name);
    if (found == commands_.end())
    {
        out << "Unknown command: " << name << " (type HELP)\n";
        return Status::Error;
    }
    try
    {
        found->second.handler(args, out);
    }
    catch (const std::exception& error)
    {
        out << "Error: " << error.what() << '\n';
        return Status::Error;
    }
    return Status::Ok;
}

bool StructureManager::isMutating(const std::string& line) const
{
    Args args(line);
    std::string name;
    if (!(args >> name))
    {
        return false;
    }
    const auto found = commands_.find(toUpper(name));
    return found != commands_.end() && found->second.mutating;
}

std::size_t StructureManager::count() const noexcept
{
    return items_.size();
}

const IStructure* StructureManager::find(const std::string& name) const
{
    const auto found = items_.find(name);
    return found == items_.end() ? nullptr : found->second.get();
}

// ---------------------------------------------------------------- общие команды

void StructureManager::registerGeneral()
{
    add("HELP", false, [](Args&, std::ostream& out) { out << helpText(); });
    add("PRINT", false,
        [this](Args& args, std::ostream& out) { out << getAny(readWord(args, "name")).toString() << '\n'; });
    add("SIZE", false, [this](Args& args, std::ostream& out) { out << getAny(readWord(args, "name")).size() << '\n'; });
    add("LIST", false,
        [this](Args&, std::ostream& out)
        {
            if (items_.empty())
            {
                out << "(no structures)\n";
            }
            for (const auto& [name, item] : items_)
            {
                out << name << " (" << typeName(item->type()) << ", size " << item->size() << ")\n";
            }
        });
    add("DROP", true,
        [this](Args& args, std::ostream& out)
        {
            const std::string name = readWord(args, "name");
            getAny(name);
            items_.erase(name);
            out << "OK\n";
        });
    add("CLEAR", true,
        [this](Args&, std::ostream& out)
        {
            items_.clear();
            out << "OK\n";
        });
    add("SAVE", false, [this](Args& args, std::ostream& out) { save(args, out); });
    add("LOAD", true, [this](Args& args, std::ostream& out) { load(args, out); });
}

void StructureManager::save(Args& args, std::ostream& out)
{
    const IStructure& item = getAny(readWord(args, "name"));
    const std::string format = toUpper(readWord(args, "format"));
    const std::string path = readRest(args, "path");
    if (format != "BIN" && format != "TXT")
    {
        throw CommandError("Expected BIN or TXT");
    }
    std::ofstream file(path, std::ios::binary);
    if (!file)
    {
        throw CommandError("Cannot open file: " + path);
    }
    if (format == "BIN")
    {
        item.saveBinary(file);
    }
    else
    {
        item.saveText(file);
    }
    file.flush();
    if (!file)
    {
        throw CommandError("Write failed: " + path);
    }
    out << "OK\n";
}

void StructureManager::load(Args& args, std::ostream& out)
{
    const std::string name = readWord(args, "name");
    const std::string format = toUpper(readWord(args, "format"));
    const std::string path = readRest(args, "path");
    if (format != "BIN" && format != "TXT")
    {
        throw CommandError("Expected BIN or TXT");
    }
    const bool binary = format == "BIN";
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        throw CommandError("Cannot open file: " + path);
    }
    // Сначала определяем тип по заголовку, затем читаем файл с начала
    const StructureType type = binary ? readBinaryType(file) : readTextType(file);
    file.clear();
    file.seekg(0);
    std::unique_ptr<IStructure> fresh = makeStructure(type);
    if (binary)
    {
        fresh->loadBinary(file);
    }
    else
    {
        fresh->loadText(file);
    }
    items_[name] = std::move(fresh);
    out << "OK\n";
}

// ---------------------------------------------------------------- массив (M)

void StructureManager::registerArray()
{
    add("MCREATE", true, [this](Args& args, std::ostream& out) { create<DynamicArray>(args, out); });
    add("MPUSH", true,
        [this](Args& args, std::ostream& out)
        {
            DynamicArray& array = get<DynamicArray>(readWord(args, "name"));
            const std::string where = toUpper(readWord(args, "position"));
            if (where != "B" && where != "F")
            {
                throw CommandError("Expected B or F");
            }
            const std::string value = readRest(args, "value");
            where == "B" ? array.pushBack(value) : array.pushFront(value);
            out << "OK\n";
        });
    add("MPOP", true,
        [this](Args& args, std::ostream& out)
        {
            DynamicArray& array = get<DynamicArray>(readWord(args, "name"));
            const std::string where = toUpper(readWord(args, "position"));
            if (where != "B" && where != "F")
            {
                throw CommandError("Expected B or F");
            }
            where == "B" ? array.popBack() : array.popFront();
            out << "OK\n";
        });
    add("MINS", true,
        [this](Args& args, std::ostream& out)
        {
            DynamicArray& array = get<DynamicArray>(readWord(args, "name"));
            const std::size_t index = readIndex(args);
            array.insertAt(index, readRest(args, "value"));
            out << "OK\n";
        });
    add("MDEL", true,
        [this](Args& args, std::ostream& out)
        {
            DynamicArray& array = get<DynamicArray>(readWord(args, "name"));
            array.removeAt(readIndex(args));
            out << "OK\n";
        });
    add("MSET", true,
        [this](Args& args, std::ostream& out)
        {
            DynamicArray& array = get<DynamicArray>(readWord(args, "name"));
            const std::size_t index = readIndex(args);
            array.set(index, readRest(args, "value"));
            out << "OK\n";
        });
    add("MGET", false,
        [this](Args& args, std::ostream& out)
        {
            const DynamicArray& array = get<DynamicArray>(readWord(args, "name"));
            out << array.get(readIndex(args)) << '\n';
        });
    add("MSIZE", false,
        [this](Args& args, std::ostream& out) { out << get<DynamicArray>(readWord(args, "name")).size() << '\n'; });
    add("MPRINT", false,
        [this](Args& args, std::ostream& out) { out << get<DynamicArray>(readWord(args, "name")).toString() << '\n'; });
}

// ---------------------------------------------------------------- списки (F, L)

template <typename List> void StructureManager::registerList(const std::string& prefix)
{
    add(prefix + "CREATE", true, [this](Args& args, std::ostream& out) { create<List>(args, out); });
    add(prefix + "PUSH", true,
        [this](Args& args, std::ostream& out)
        {
            listPush(get<List>(readWord(args, "name")), args);
            out << "OK\n";
        });
    add(prefix + "POP", true,
        [this](Args& args, std::ostream& out)
        {
            listPop(get<List>(readWord(args, "name")), args);
            out << "OK\n";
        });
    add(prefix + "DEL", true,
        [this](Args& args, std::ostream& out)
        {
            List& list = get<List>(readWord(args, "name"));
            if (!list.removeByValue(readRest(args, "value")))
            {
                throw CommandError("Value not found");
            }
            out << "OK\n";
        });
    add(prefix + "FIND", false,
        [this](Args& args, std::ostream& out)
        {
            const List& list = get<List>(readWord(args, "name"));
            const long index = list.indexOf(readRest(args, "value"));
            if (index < 0)
            {
                throw CommandError("Value not found");
            }
            out << index << '\n';
        });
    add(prefix + "GET", false,
        [this](Args& args, std::ostream& out)
        {
            const List& list = get<List>(readWord(args, "name"));
            const std::string text = list.toStringFrom(readRest(args, "value"));
            if (text.empty())
            {
                throw CommandError("Value not found");
            }
            out << text << '\n';
        });
}

void StructureManager::registerSinglyList()
{
    registerList<SinglyLinkedList>("F");
    add("FPRINT", false, [this](Args& args, std::ostream& out)
        { out << get<SinglyLinkedList>(readWord(args, "name")).toString() << '\n'; });
}

void StructureManager::registerDoublyList()
{
    registerList<DoublyLinkedList>("L");
    add("LGETB", false,
        [this](Args& args, std::ostream& out)
        {
            const DoublyLinkedList& list = get<DoublyLinkedList>(readWord(args, "name"));
            const std::string text = list.toStringFromBackward(readRest(args, "value"));
            if (text.empty())
            {
                throw CommandError("Value not found");
            }
            out << text << '\n';
        });
    add("LPRINT", false,
        [this](Args& args, std::ostream& out)
        {
            const DoublyLinkedList& list = get<DoublyLinkedList>(readWord(args, "name"));
            std::string direction = "F";
            if (std::string word; args >> word)
            {
                direction = toUpper(word);
            }
            if (direction != "F" && direction != "B")
            {
                throw CommandError("Expected F or B");
            }
            out << (direction == "F" ? list.toString() : list.toStringBackward()) << '\n';
        });
}

// ---------------------------------------------------------------- стек (S), очередь (Q)

void StructureManager::registerStack()
{
    add("SCREATE", true, [this](Args& args, std::ostream& out) { create<Stack>(args, out); });
    add("SPUSH", true,
        [this](Args& args, std::ostream& out)
        {
            Stack& stack = get<Stack>(readWord(args, "name"));
            stack.push(readRest(args, "value"));
            out << "OK\n";
        });
    add("SPOP", true,
        [this](Args& args, std::ostream& out) { out << get<Stack>(readWord(args, "name")).pop() << '\n'; });
    add("SPEEK", false,
        [this](Args& args, std::ostream& out) { out << get<Stack>(readWord(args, "name")).peek() << '\n'; });
    add("SPRINT", false,
        [this](Args& args, std::ostream& out) { out << get<Stack>(readWord(args, "name")).toString() << '\n'; });
}

void StructureManager::registerQueue()
{
    add("QCREATE", true, [this](Args& args, std::ostream& out) { create<Queue>(args, out); });
    add("QPUSH", true,
        [this](Args& args, std::ostream& out)
        {
            Queue& queue = get<Queue>(readWord(args, "name"));
            queue.push(readRest(args, "value"));
            out << "OK\n";
        });
    add("QPOP", true,
        [this](Args& args, std::ostream& out) { out << get<Queue>(readWord(args, "name")).pop() << '\n'; });
    add("QFRONT", false,
        [this](Args& args, std::ostream& out) { out << get<Queue>(readWord(args, "name")).front() << '\n'; });
    add("QPRINT", false,
        [this](Args& args, std::ostream& out) { out << get<Queue>(readWord(args, "name")).toString() << '\n'; });
}

// ---------------------------------------------------------------- дерево (T)

void StructureManager::registerTree()
{
    add("TCREATE", true, [this](Args& args, std::ostream& out) { create<CompleteBinaryTree>(args, out); });
    add("TINSERT", true,
        [this](Args& args, std::ostream& out)
        {
            CompleteBinaryTree& tree = get<CompleteBinaryTree>(readWord(args, "name"));
            tree.insert(readInt(args));
            out << "OK\n";
        });
    add("TFIND", false,
        [this](Args& args, std::ostream& out)
        {
            const CompleteBinaryTree& tree = get<CompleteBinaryTree>(readWord(args, "name"));
            out << boolText(tree.contains(readInt(args))) << '\n';
        });
    add("TCOMPLETE", false, [this](Args& args, std::ostream& out)
        { out << boolText(get<CompleteBinaryTree>(readWord(args, "name")).isComplete()) << '\n'; });
    add("TPRINT", false, [this](Args& args, std::ostream& out)
        { out << get<CompleteBinaryTree>(readWord(args, "name")).toString() << '\n'; });
}

// ---------------------------------------------------------------- хеш-таблица (H)

void StructureManager::registerHash()
{
    add("HCREATE", true,
        [this](Args& args, std::ostream& out)
        {
            const std::string name = readWord(args, "name");
            if (items_.count(name) != 0)
            {
                throw CommandError("Structure '" + name + "' already exists");
            }
            HashTable::Mode mode = HashTable::Mode::Linear;
            std::size_t capacity = HashTable::kDefaultCapacity;
            if (std::string word; args >> word)
            {
                word = toUpper(word);
                if (word == "DOUBLE")
                {
                    mode = HashTable::Mode::Double;
                }
                else if (word != "LINEAR")
                {
                    throw CommandError("Expected LINEAR or DOUBLE");
                }
                if (std::string capacityWord; args >> capacityWord)
                {
                    capacity = parseNumber<std::size_t>(capacityWord, "Capacity must be a positive integer");
                    if (capacity == 0)
                    {
                        throw CommandError("Capacity must be a positive integer");
                    }
                }
            }
            items_[name] = std::make_unique<HashTable>(mode, capacity);
            out << "OK\n";
        });
    add("HINSERT", true,
        [this](Args& args, std::ostream& out)
        {
            HashTable& table = get<HashTable>(readWord(args, "name"));
            if (!table.insert(readInt(args)))
            {
                throw CommandError("Table is full");
            }
            out << "OK\n";
        });
    add("HFIND", false,
        [this](Args& args, std::ostream& out)
        {
            const HashTable& table = get<HashTable>(readWord(args, "name"));
            out << boolText(table.contains(readInt(args))) << '\n';
        });
    add("HDEL", true,
        [this](Args& args, std::ostream& out)
        {
            HashTable& table = get<HashTable>(readWord(args, "name"));
            if (!table.erase(readInt(args)))
            {
                throw CommandError("Key not found");
            }
            out << "OK\n";
        });
    add("HPRINT", false,
        [this](Args& args, std::ostream& out) { out << get<HashTable>(readWord(args, "name")).toString() << '\n'; });
}

std::string StructureManager::helpText()
{
    return "=== DATA STRUCTURES ===\n"
           "ARRAY:\n"
           "  MCREATE <name>\n"
           "  MPUSH <name> B|F <value>      MPOP <name> B|F\n"
           "  MINS <name> <index> <value>   MDEL <name> <index>\n"
           "  MGET <name> <index>           MSET <name> <index> <value>\n"
           "  MSIZE <name>                  MPRINT <name>\n"
           "SINGLY LINKED LIST (F) / DOUBLY LINKED LIST (L):\n"
           "  FCREATE <name>\n"
           "  FPUSH <name> B|F <value>\n"
           "  FPUSH <name> BEFORE|AFTER <target> <value>\n"
           "  FPOP <name> B|F\n"
           "  FPOP <name> BEFORE|AFTER <target>\n"
           "  FDEL <name> <value>           FFIND <name> <value>\n"
           "  FGET <name> <value>           FPRINT <name>\n"
           "  (for the doubly linked list use the prefix L; extra: LGETB <name> <value>, LPRINT <name> [F|B])\n"
           "STACK:\n"
           "  SCREATE <name>   SPUSH <name> <value>   SPOP <name>   SPEEK <name>   SPRINT <name>\n"
           "QUEUE:\n"
           "  QCREATE <name>   QPUSH <name> <value>   QPOP <name>   QFRONT <name>   QPRINT <name>\n"
           "COMPLETE BINARY TREE:\n"
           "  TCREATE <name>   TINSERT <name> <int>   TFIND <name> <int>   TCOMPLETE <name>   TPRINT <name>\n"
           "HASH TABLE:\n"
           "  HCREATE <name> [LINEAR|DOUBLE] [capacity]\n"
           "  HINSERT <name> <int>   HFIND <name> <int>   HDEL <name> <int>   HPRINT <name>\n"
           "SERIALIZATION:\n"
           "  SAVE <name> BIN|TXT <path>    LOAD <name> BIN|TXT <path>\n"
           "GENERAL:\n"
           "  PRINT <name>   SIZE <name>   LIST   DROP <name>   CLEAR   HELP   EXIT\n";
}

} // namespace ds
