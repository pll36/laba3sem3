#ifndef DS_STRUCTUREMANAGER_H
#define DS_STRUCTUREMANAGER_H

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "ds/IStructure.h"

namespace ds
{

// Интерпретатор команд: хранит именованные структуры данных
// и выполняет текстовые команды (MPUSH, FPOP, TINSERT, HFIND, SAVE, ...).
class StructureManager
{
public:
    enum class Status : unsigned char
    {
        Ok,
        Error,
        Exit
    };

    StructureManager();
    StructureManager(const StructureManager&) = delete;
    StructureManager& operator=(const StructureManager&) = delete;
    StructureManager(StructureManager&&) = delete;
    StructureManager& operator=(StructureManager&&) = delete;
    ~StructureManager() = default;

    // Выполняет одну команду, результат пишет в out
    Status execute(const std::string& line, std::ostream& out);
    // Изменяет ли команда состав структур (такие команды сохраняются в файл)
    bool isMutating(const std::string& line) const;

    std::size_t count() const noexcept;
    const IStructure* find(const std::string& name) const;
    static std::string helpText();

private:
    using Args = std::istringstream;
    using Handler = std::function<void(Args&, std::ostream&)>;

    struct Command
    {
        bool mutating;
        Handler handler;
    };

    void add(const std::string& name, bool mutating, Handler handler);
    void registerGeneral();
    void registerArray();
    template <typename List> void registerList(const std::string& prefix);
    void registerSinglyList();
    void registerDoublyList();
    void registerStack();
    void registerQueue();
    void registerTree();
    void registerHash();

    template <typename T> void create(Args& args, std::ostream& out);
    template <typename T> T& get(const std::string& name);
    IStructure& getAny(const std::string& name);
    void save(Args& args, std::ostream& out);
    void load(Args& args, std::ostream& out);

    std::map<std::string, std::unique_ptr<IStructure>> items_;
    std::unordered_map<std::string, Command> commands_;
};

} // namespace ds

#endif
