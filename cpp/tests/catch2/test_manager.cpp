#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "ds/CommandLog.h"
#include "ds/StructureManager.h"

using ds::StructureManager;
using Status = StructureManager::Status;

namespace
{

// Выполняет команду и возвращает её вывод
std::string run(StructureManager& manager, const std::string& line)
{
    std::ostringstream out;
    manager.execute(line, out);
    return out.str();
}

Status statusOf(StructureManager& manager, const std::string& line)
{
    std::ostringstream out;
    return manager.execute(line, out);
}

// Временный файл, удаляется в деструкторе
class TempFile
{
public:
    explicit TempFile(const std::string& name)
        : path_((std::filesystem::temp_directory_path() / ("lab3_test_" + name)).string())
    {
        std::remove(path_.c_str());
    }
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    ~TempFile() { std::remove(path_.c_str()); }
    const std::string& path() const { return path_; }

private:
    std::string path_;
};

} // namespace

TEST_CASE("empty line, EXIT and unknown commands", "[manager]")
{
    StructureManager manager;
    CHECK(statusOf(manager, "") == Status::Ok);
    CHECK(statusOf(manager, "   ") == Status::Ok);
    CHECK(statusOf(manager, "EXIT") == Status::Exit);
    CHECK(statusOf(manager, "exit") == Status::Exit);
    CHECK(run(manager, "FLY away") == "Unknown command: FLY (type HELP)\n");
    CHECK(statusOf(manager, "FLY") == Status::Error);
}

TEST_CASE("commands are case-insensitive, names are case-sensitive", "[manager]")
{
    StructureManager manager;
    CHECK(run(manager, "mcreate Arr") == "OK\n");
    CHECK(run(manager, "MSIZE Arr") == "0\n");
    CHECK(run(manager, "MSIZE arr") == "Error: Structure 'arr' not found\n");
}

TEST_CASE("HELP lists the command groups", "[manager]")
{
    StructureManager manager;
    const std::string help = run(manager, "HELP");
    for (const char* word : {"ARRAY", "STACK", "QUEUE", "COMPLETE BINARY TREE", "HASH TABLE", "SAVE", "TINSERT"})
    {
        CHECK(help.find(word) != std::string::npos);
    }
}

TEST_CASE("array commands", "[manager][array]")
{
    StructureManager manager;
    CHECK(run(manager, "MCREATE a") == "OK\n");
    CHECK(run(manager, "MCREATE a") == "Error: Structure 'a' already exists\n");
    CHECK(run(manager, "MPUSH a B hello world") == "OK\n");
    CHECK(run(manager, "MPUSH a F first") == "OK\n");
    CHECK(run(manager, "MINS a 1 mid") == "OK\n");
    CHECK(run(manager, "MPRINT a") == "[first, mid, hello world]\n");
    CHECK(run(manager, "MGET a 2") == "hello world\n");
    CHECK(run(manager, "MSET a 0 FIRST") == "OK\n");
    CHECK(run(manager, "MSIZE a") == "3\n");
    CHECK(run(manager, "MDEL a 1") == "OK\n");
    CHECK(run(manager, "MPOP a B") == "OK\n");
    CHECK(run(manager, "MPOP a F") == "OK\n");
    CHECK(run(manager, "MPRINT a") == "(empty)\n");
}

TEST_CASE("array command errors", "[manager][array]")
{
    StructureManager manager;
    run(manager, "MCREATE a");
    CHECK(run(manager, "MPUSH a X 1") == "Error: Expected B or F\n");
    CHECK(run(manager, "MPUSH a B") == "Error: Missing argument: value\n");
    CHECK(run(manager, "MPOP a X") == "Error: Expected B or F\n");
    CHECK(run(manager, "MPOP a B") == "Error: array is empty\n");
    CHECK(run(manager, "MGET a 0") == "Error: index out of range\n");
    CHECK(run(manager, "MGET a -1") == "Error: Non-negative index required\n");
    CHECK(run(manager, "MGET a abc") == "Error: Non-negative index required\n");
    CHECK(run(manager, "MSET a 0 x") == "Error: index out of range\n");
    CHECK(run(manager, "MDEL a 0") == "Error: index out of range\n");
    CHECK(run(manager, "MINS a 3 x") == "Error: index out of range\n");
    CHECK(run(manager, "MGET") == "Error: Missing argument: name\n");
}

TEST_CASE("singly linked list commands", "[manager][list]")
{
    StructureManager manager;
    CHECK(run(manager, "FCREATE f") == "OK\n");
    run(manager, "FPUSH f B 1");
    run(manager, "FPUSH f B 3");
    CHECK(run(manager, "FPUSH f AFTER 1 2") == "OK\n");
    CHECK(run(manager, "FPUSH f BEFORE 1 0") == "OK\n");
    CHECK(run(manager, "FPUSH f F start") == "OK\n");
    CHECK(run(manager, "FPRINT f") == "start -> 0 -> 1 -> 2 -> 3\n");
    CHECK(run(manager, "FFIND f 2") == "3\n");
    CHECK(run(manager, "FGET f 2") == "2 -> 3\n");
    CHECK(run(manager, "FPOP f F") == "OK\n");
    CHECK(run(manager, "FPOP f B") == "OK\n");
    CHECK(run(manager, "FPOP f BEFORE 2") == "OK\n");
    CHECK(run(manager, "FPOP f AFTER 0") == "OK\n");
    CHECK(run(manager, "FPRINT f") == "0\n");
    CHECK(run(manager, "FDEL f 0") == "OK\n");
    CHECK(run(manager, "FPRINT f") == "(empty)\n");
}

TEST_CASE("singly linked list command errors", "[manager][list]")
{
    StructureManager manager;
    run(manager, "FCREATE f");
    run(manager, "FPUSH f B a");
    CHECK(run(manager, "FPUSH f X a") == "Error: Expected B, F, BEFORE or AFTER\n");
    CHECK(run(manager, "FPUSH f BEFORE zzz x") == "Error: Target not found\n");
    CHECK(run(manager, "FPUSH f AFTER a") == "Error: Missing argument: value\n");
    CHECK(run(manager, "FPOP f X") == "Error: Expected B, F, BEFORE or AFTER\n");
    CHECK(run(manager, "FPOP f BEFORE a") == "Error: Target not found or no neighbour to remove\n");
    CHECK(run(manager, "FDEL f zzz") == "Error: Value not found\n");
    CHECK(run(manager, "FFIND f zzz") == "Error: Value not found\n");
    CHECK(run(manager, "FGET f zzz") == "Error: Value not found\n");
    run(manager, "FPOP f F");
    CHECK(run(manager, "FPOP f B") == "Error: list is empty\n");
}

TEST_CASE("doubly linked list commands", "[manager][list]")
{
    StructureManager manager;
    CHECK(run(manager, "LCREATE l") == "OK\n");
    run(manager, "LPUSH l B x");
    run(manager, "LPUSH l B y");
    run(manager, "LPUSH l B z");
    CHECK(run(manager, "LPRINT l") == "x <-> y <-> z\n");
    CHECK(run(manager, "LPRINT l F") == "x <-> y <-> z\n");
    CHECK(run(manager, "LPRINT l B") == "z <-> y <-> x\n");
    CHECK(run(manager, "LPRINT l Q") == "Error: Expected F or B\n");
    CHECK(run(manager, "LGET l y") == "y <-> z\n");
    CHECK(run(manager, "LGETB l y") == "y <-> x\n");
    CHECK(run(manager, "LGETB l nope") == "Error: Value not found\n");
    CHECK(run(manager, "LFIND l z") == "2\n");
    CHECK(run(manager, "LPOP l AFTER x") == "OK\n");
    CHECK(run(manager, "LPRINT l") == "x <-> z\n");
    CHECK(run(manager, "LDEL l x") == "OK\n");
    CHECK(run(manager, "LPRINT l") == "z\n");
}

TEST_CASE("stack and queue commands", "[manager][stack][queue]")
{
    StructureManager manager;
    CHECK(run(manager, "SCREATE s") == "OK\n");
    run(manager, "SPUSH s 1");
    run(manager, "SPUSH s two words");
    CHECK(run(manager, "SPEEK s") == "two words\n");
    CHECK(run(manager, "SPRINT s") == "Stack(top->bottom): two words, 1\n");
    CHECK(run(manager, "SPOP s") == "two words\n");
    CHECK(run(manager, "SPOP s") == "1\n");
    CHECK(run(manager, "SPOP s") == "Error: stack is empty\n");
    CHECK(run(manager, "SPEEK s") == "Error: stack is empty\n");

    CHECK(run(manager, "QCREATE q") == "OK\n");
    run(manager, "QPUSH q a");
    run(manager, "QPUSH q b");
    CHECK(run(manager, "QFRONT q") == "a\n");
    CHECK(run(manager, "QPRINT q") == "Queue(front->rear): a, b\n");
    CHECK(run(manager, "QPOP q") == "a\n");
    CHECK(run(manager, "QPOP q") == "b\n");
    CHECK(run(manager, "QPOP q") == "Error: queue is empty\n");
    CHECK(run(manager, "QFRONT q") == "Error: queue is empty\n");
}

TEST_CASE("tree commands", "[manager][tree]")
{
    StructureManager manager;
    CHECK(run(manager, "TCREATE t") == "OK\n");
    for (int i = 1; i <= 4; ++i)
    {
        CHECK(run(manager, "TINSERT t " + std::to_string(i)) == "OK\n");
    }
    CHECK(run(manager, "TFIND t 3") == "true\n");
    CHECK(run(manager, "TFIND t 9") == "false\n");
    CHECK(run(manager, "TCOMPLETE t") == "true\n");
    CHECK(run(manager, "TPRINT t") == "1\n|-- 2\n|   |-- 4\n`-- 3\n");
    CHECK(run(manager, "TINSERT t abc") == "Error: Integer required\n");
    CHECK(run(manager, "TINSERT t") == "Error: Missing argument: integer\n");
}

TEST_CASE("hash table commands", "[manager][hash]")
{
    StructureManager manager;
    CHECK(run(manager, "HCREATE h") == "OK\n");
    CHECK(run(manager, "HCREATE d double 7") == "OK\n");
    CHECK(run(manager, "HCREATE l LINEAR") == "OK\n");
    CHECK(run(manager, "HINSERT d 523456795") == "OK\n");
    CHECK(run(manager, "HFIND d 523456795") == "true\n");
    CHECK(run(manager, "HFIND d 1") == "false\n");
    CHECK(run(manager, "HPRINT d") == "Hash(double, capacity=7, size=1)\n[3] 523456795\n");
    CHECK(run(manager, "HDEL d 523456795") == "OK\n");
    CHECK(run(manager, "HDEL d 523456795") == "Error: Key not found\n");
    CHECK(run(manager, "HCREATE d") == "Error: Structure 'd' already exists\n");
    CHECK(run(manager, "HCREATE x triple") == "Error: Expected LINEAR or DOUBLE\n");
    CHECK(run(manager, "HCREATE x double 0") == "Error: Capacity must be a positive integer\n");
    CHECK(run(manager, "HCREATE x double abc") == "Error: Capacity must be a positive integer\n");

    run(manager, "HCREATE tiny linear 1");
    CHECK(run(manager, "HINSERT tiny 1") == "OK\n");
    CHECK(run(manager, "HINSERT tiny 2") == "Error: Table is full\n");
}

TEST_CASE("general commands: PRINT, SIZE, LIST, DROP, CLEAR", "[manager]")
{
    StructureManager manager;
    CHECK(run(manager, "LIST") == "(no structures)\n");
    run(manager, "SCREATE s");
    run(manager, "SPUSH s 1");
    run(manager, "QCREATE q");
    CHECK(run(manager, "PRINT s") == "Stack(top->bottom): 1\n");
    CHECK(run(manager, "SIZE s") == "1\n");
    CHECK(run(manager, "LIST") == "q (queue, size 0)\ns (stack, size 1)\n");
    CHECK(manager.count() == 2);
    CHECK(manager.find("s") != nullptr);
    CHECK(manager.find("none") == nullptr);
    CHECK(run(manager, "DROP s") == "OK\n");
    CHECK(run(manager, "DROP s") == "Error: Structure 's' not found\n");
    CHECK(run(manager, "CLEAR") == "OK\n");
    CHECK(manager.count() == 0);
}

TEST_CASE("using a structure through a command of another type is an error", "[manager]")
{
    StructureManager manager;
    run(manager, "SCREATE s");
    CHECK(run(manager, "QPUSH s x") == "Error: Structure 's' has another type (stack)\n");
    CHECK(run(manager, "PRINT nothing") == "Error: Structure 'nothing' not found\n");
}

TEST_CASE("mutating commands are recognised", "[manager]")
{
    StructureManager manager;
    for (const char* line : {"MCREATE a", "mpush a B 1", "FPOP f B", "SPUSH s 1", "QPOP q", "TINSERT t 1",
                             "HINSERT h 1", "DROP a", "CLEAR", "LOAD a BIN file"})
    {
        CHECK(manager.isMutating(line));
    }
    for (const char* line :
         {"MGET a 0", "FPRINT f", "TFIND t 1", "HFIND h 1", "PRINT a", "SAVE a BIN f", "LIST", "HELP", "UNKNOWN", ""})
    {
        CHECK_FALSE(manager.isMutating(line));
    }
}

TEST_CASE("SAVE and LOAD round trip all structures in both formats", "[manager][io]")
{
    StructureManager manager;
    const char* setup[] = {"MCREATE m",   "MPUSH m B a b",      "FCREATE f",    "FPUSH f B 1", "FPUSH f B 2",
                           "LCREATE l",   "LPUSH l B x",        "SCREATE s",    "SPUSH s 5",   "SPUSH s 6",
                           "QCREATE q",   "QPUSH q 7",          "TCREATE t",    "TINSERT t 1", "TINSERT t 2",
                           "TINSERT t 3", "HCREATE h double 7", "HINSERT h 10", "HINSERT h 20"};
    for (const char* line : setup)
    {
        REQUIRE(statusOf(manager, line) == Status::Ok);
    }

    for (const char* name : {"m", "f", "l", "s", "q", "t", "h"})
    {
        for (const std::string format : {"BIN", "TXT"})
        {
            const TempFile file(std::string(name) + "." + format);
            const std::string copy = std::string(name) + "_copy";
            CHECK(run(manager, std::string("SAVE ") + name + " " + format + " " + file.path()) == "OK\n");
            CHECK(run(manager, "LOAD " + copy + " " + format + " " + file.path()) == "OK\n");
            CHECK(run(manager, std::string("PRINT ") + name) == run(manager, "PRINT " + copy));
        }
    }
}

TEST_CASE("LOAD replaces an existing structure", "[manager][io]")
{
    StructureManager manager;
    run(manager, "SCREATE s");
    run(manager, "SPUSH s old");
    const TempFile file("replace.bin");
    run(manager, "SCREATE other");
    run(manager, "SPUSH other new");
    REQUIRE(run(manager, "SAVE other BIN " + file.path()) == "OK\n");
    CHECK(run(manager, "LOAD s BIN " + file.path()) == "OK\n");
    CHECK(run(manager, "PRINT s") == "Stack(top->bottom): new\n");
}

TEST_CASE("SAVE and LOAD report problems", "[manager][io]")
{
    StructureManager manager;
    run(manager, "SCREATE s");
    const TempFile file("errors.txt");
    CHECK(run(manager, "SAVE s ZIP x") == "Error: Expected BIN or TXT\n");
    CHECK(run(manager, "SAVE nothing BIN x") == "Error: Structure 'nothing' not found\n");
    CHECK(run(manager, "SAVE s BIN /nonexistent_dir_lab3/file.bin") ==
          "Error: Cannot open file: /nonexistent_dir_lab3/file.bin\n");
    CHECK(run(manager, "LOAD s ZIP x") == "Error: Expected BIN or TXT\n");
    CHECK(run(manager, "LOAD s BIN " + file.path()) == "Error: Cannot open file: " + file.path() + "\n");

    {
        std::ofstream garbage(file.path());
        garbage << "this is not a serialized structure\n";
    }
    CHECK(run(manager, "LOAD s TXT " + file.path()) == "Error: bad text signature\n");
    CHECK(run(manager, "LOAD s BIN " + file.path()) == "Error: bad binary signature\n");
}

TEST_CASE("command log restores state in a new session", "[manager][log]")
{
    const TempFile file("log.data");
    const ds::CommandLog log(file.path());
    CHECK(log.path() == file.path());
    {
        StructureManager manager;
        std::ostringstream out;
        CHECK(ds::runAndLog(manager, log, "SCREATE s", out) == Status::Ok);
        CHECK(ds::runAndLog(manager, log, "SPUSH s 1", out) == Status::Ok);
        CHECK(ds::runAndLog(manager, log, "SPUSH s 2", out) == Status::Ok);
        CHECK(ds::runAndLog(manager, log, "SPOP s", out) == Status::Ok);
        CHECK(ds::runAndLog(manager, log, "SPEEK s", out) == Status::Ok); // чтение не сохраняется
        CHECK(ds::runAndLog(manager, log, "SPUSH nothing 1", out) == Status::Error); // ошибки не сохраняются
        CHECK(ds::runAndLog(manager, log, "EXIT", out) == Status::Exit);
    }
    {
        std::ifstream saved(file.path());
        std::stringstream content;
        content << saved.rdbuf();
        CHECK(content.str() == "SCREATE s\nSPUSH s 1\nSPUSH s 2\nSPOP s\n");
    }
    StructureManager restored;
    CHECK(log.replay(restored) == 4);
    CHECK(run(restored, "SPRINT s") == "Stack(top->bottom): 1\n");
}

TEST_CASE("command log handles a missing or unwritable file", "[manager][log]")
{
    const TempFile missing("missing.data");
    const ds::CommandLog log(missing.path());
    StructureManager manager;
    CHECK(log.replay(manager) == 0);

    const ds::CommandLog unwritable("/nonexistent_dir_lab3/file.data");
    CHECK_FALSE(unwritable.append("SCREATE s"));
    std::ostringstream out;
    CHECK(ds::runAndLog(manager, unwritable, "SCREATE s", out) == Status::Ok);
    CHECK(out.str().find("Warning: cannot write to data file") != std::string::npos);
}
