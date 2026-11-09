#include <iostream>
#include <string>

#include "ds/CommandLog.h"
#include "ds/StructureManager.h"

// Запуск: lab3_app [--file <файл данных>] [--query "<команда>"]
int main(int argc, const char* const argv[])
{
    std::string dataFile = "file.data";
    std::string query;
    bool hasQuery = false;

    for (int i = 1; i < argc; ++i)
    {
        const std::string argument = argv[i];
        if (argument == "--file" && i + 1 < argc)
        {
            dataFile = argv[++i];
        }
        else if (argument == "--query" && i + 1 < argc)
        {
            query = argv[++i];
            hasQuery = true;
        }
    }

    ds::StructureManager manager;
    const ds::CommandLog log(dataFile);
    log.replay(manager);

    if (hasQuery)
    {
        const auto status = ds::runAndLog(manager, log, query, std::cout);
        return status == ds::StructureManager::Status::Error ? 1 : 0;
    }

    std::cout << "=== DATA STRUCTURES ===\nData file: " << dataFile << "\nType HELP to see commands.\n\n";
    std::string input;
    while (true)
    {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, input))
        {
            break;
        }
        if (ds::runAndLog(manager, log, input, std::cout) == ds::StructureManager::Status::Exit)
        {
            break;
        }
    }
    return 0;
}
