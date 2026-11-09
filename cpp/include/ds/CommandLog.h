#ifndef DS_COMMANDLOG_H
#define DS_COMMANDLOG_H

#include <cstddef>
#include <iosfwd>
#include <string>

#include "ds/StructureManager.h"

namespace ds
{

// Файл команд: каждая успешная команда, изменившая данные, дописывается в файл,
// при запуске файл воспроизводится и восстанавливает состояние структур.
class CommandLog
{
public:
    explicit CommandLog(std::string path);

    const std::string& path() const noexcept;
    // Выполняет все команды из файла; возвращает число выполненных команд
    std::size_t replay(StructureManager& manager) const;
    // false, если файл не удалось открыть
    bool append(const std::string& line) const;

private:
    std::string path_;
};

// Выполняет команду и, если она успешно изменила данные, записывает её в журнал
StructureManager::Status runAndLog(StructureManager& manager, const CommandLog& log, const std::string& line,
                                   std::ostream& out);

} // namespace ds

#endif
