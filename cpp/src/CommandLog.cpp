#include "ds/CommandLog.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

namespace ds
{

CommandLog::CommandLog(std::string path) : path_(std::move(path)) {}

const std::string& CommandLog::path() const noexcept
{
    return path_;
}

std::size_t CommandLog::replay(StructureManager& manager) const
{
    std::ifstream file(path_);
    if (!file.is_open())
    {
        return 0;
    }
    std::ostringstream discarded;
    std::size_t executed = 0;
    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty())
        {
            manager.execute(line, discarded);
            ++executed;
        }
    }
    return executed;
}

bool CommandLog::append(const std::string& line) const
{
    std::ofstream file(path_, std::ios::app);
    if (!file.is_open())
    {
        return false;
    }
    file << line << '\n';
    return true;
}

StructureManager::Status runAndLog(StructureManager& manager, const CommandLog& log, const std::string& line,
                                   std::ostream& out)
{
    const StructureManager::Status status = manager.execute(line, out);
    if (status == StructureManager::Status::Ok && manager.isMutating(line) && !log.append(line))
    {
        out << "Warning: cannot write to data file " << log.path() << '\n';
    }
    return status;
}

} // namespace ds
