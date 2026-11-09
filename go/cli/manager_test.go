package cli_test

import (
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"lab3/cli"
)

func run(m *cli.Manager, line string) string {
	var out bytes.Buffer
	m.Execute(line, &out)
	return out.String()
}

func runAll(t *testing.T, m *cli.Manager, lines ...string) {
	t.Helper()
	for _, line := range lines {
		var out bytes.Buffer
		require.Equal(t, cli.StatusOK, m.Execute(line, &out), "%s -> %s", line, out.String())
	}
}

func TestEmptyExitAndUnknown(t *testing.T) {
	m := cli.NewManager()
	assert.Equal(t, cli.StatusOK, m.Execute("", &bytes.Buffer{}))
	assert.Equal(t, cli.StatusOK, m.Execute("   ", &bytes.Buffer{}))
	assert.Equal(t, cli.StatusExit, m.Execute("EXIT", &bytes.Buffer{}))
	assert.Equal(t, cli.StatusExit, m.Execute("exit", &bytes.Buffer{}))
	assert.Equal(t, "Unknown command: FLY (type HELP)\n", run(m, "FLY away"))
	assert.Equal(t, cli.StatusError, m.Execute("FLY", &bytes.Buffer{}))
}

func TestCaseRules(t *testing.T) {
	m := cli.NewManager()
	assert.Equal(t, "OK\n", run(m, "mcreate Arr"))
	assert.Equal(t, "0\n", run(m, "MSIZE Arr"))
	assert.Equal(t, "Error: Structure 'arr' not found\n", run(m, "MSIZE arr"))
}

func TestHelp(t *testing.T) {
	help := run(cli.NewManager(), "HELP")
	for _, word := range []string{"ARRAY", "STACK", "QUEUE", "COMPLETE BINARY TREE", "HASH TABLE", "SAVE", "TINSERT"} {
		assert.Contains(t, help, word)
	}
}

func TestArrayCommands(t *testing.T) {
	m := cli.NewManager()
	assert.Equal(t, "OK\n", run(m, "MCREATE a"))
	assert.Equal(t, "Error: Structure 'a' already exists\n", run(m, "MCREATE a"))
	runAll(t, m, "MPUSH a B hello world", "MPUSH a F first", "MINS a 1 mid")
	assert.Equal(t, "[first, mid, hello world]\n", run(m, "MPRINT a"))
	assert.Equal(t, "hello world\n", run(m, "MGET a 2"))
	assert.Equal(t, "OK\n", run(m, "MSET a 0 FIRST"))
	assert.Equal(t, "3\n", run(m, "MSIZE a"))
	runAll(t, m, "MDEL a 1", "MPOP a B", "MPOP a F")
	assert.Equal(t, "(empty)\n", run(m, "MPRINT a"))
}

func TestArrayErrors(t *testing.T) {
	m := cli.NewManager()
	runAll(t, m, "MCREATE a")
	for line, want := range map[string]string{
		"MPUSH a X 1": "Error: Expected B or F\n",
		"MPUSH a B":   "Error: Missing argument: value\n",
		"MPOP a X":    "Error: Expected B or F\n",
		"MPOP a B":    "Error: array is empty\n",
		"MPOP a F":    "Error: array is empty\n",
		"MGET a 0":    "Error: index out of range\n",
		"MGET a -1":   "Error: Non-negative index required\n",
		"MGET a abc":  "Error: Non-negative index required\n",
		"MSET a 0 x":  "Error: index out of range\n",
		"MSET a 0":    "Error: Missing argument: value\n",
		"MDEL a 0":    "Error: index out of range\n",
		"MINS a 3 x":  "Error: index out of range\n",
		"MINS a 0":    "Error: Missing argument: value\n",
		"MGET":        "Error: Missing argument: name\n",
	} {
		assert.Equal(t, want, run(m, line), line)
	}
}

func TestSinglyListCommands(t *testing.T) {
	m := cli.NewManager()
	assert.Equal(t, "OK\n", run(m, "FCREATE f"))
	runAll(t, m, "FPUSH f B 1", "FPUSH f B 3", "FPUSH f AFTER 1 2", "FPUSH f BEFORE 1 0", "FPUSH f F start")
	assert.Equal(t, "start -> 0 -> 1 -> 2 -> 3\n", run(m, "FPRINT f"))
	assert.Equal(t, "3\n", run(m, "FFIND f 2"))
	assert.Equal(t, "2 -> 3\n", run(m, "FGET f 2"))
	runAll(t, m, "FPOP f F", "FPOP f B", "FPOP f BEFORE 2", "FPOP f AFTER 0")
	assert.Equal(t, "0\n", run(m, "FPRINT f"))
	runAll(t, m, "FDEL f 0")
	assert.Equal(t, "(empty)\n", run(m, "FPRINT f"))
}

func TestSinglyListErrors(t *testing.T) {
	m := cli.NewManager()
	runAll(t, m, "FCREATE f", "FPUSH f B a")
	for line, want := range map[string]string{
		"FPUSH f X a":          "Error: Expected B, F, BEFORE or AFTER\n",
		"FPUSH f BEFORE zzz x": "Error: Target not found\n",
		"FPUSH f AFTER a":      "Error: Missing argument: value\n",
		"FPOP f X":             "Error: Expected B, F, BEFORE or AFTER\n",
		"FPOP f BEFORE a":      "Error: Target not found or no neighbour to remove\n",
		"FPOP f BEFORE":        "Error: Missing argument: target\n",
		"FDEL f zzz":           "Error: Value not found\n",
		"FFIND f zzz":          "Error: Value not found\n",
		"FGET f zzz":           "Error: Value not found\n",
	} {
		assert.Equal(t, want, run(m, line), line)
	}
	runAll(t, m, "FPOP f F")
	assert.Equal(t, "Error: list is empty\n", run(m, "FPOP f B"))
}

func TestDoublyListCommands(t *testing.T) {
	m := cli.NewManager()
	assert.Equal(t, "OK\n", run(m, "LCREATE l"))
	runAll(t, m, "LPUSH l B x", "LPUSH l B y", "LPUSH l B z")
	steps := [][2]string{
		{"LPRINT l", "x <-> y <-> z\n"},
		{"LPRINT l F", "x <-> y <-> z\n"},
		{"LPRINT l B", "z <-> y <-> x\n"},
		{"LPRINT l Q", "Error: Expected F or B\n"},
		{"LGET l y", "y <-> z\n"},
		{"LGETB l y", "y <-> x\n"},
		{"LGETB l nope", "Error: Value not found\n"},
		{"LFIND l z", "2\n"},
		{"LPOP l AFTER x", "OK\n"},
		{"LPRINT l", "x <-> z\n"},
		{"LDEL l x", "OK\n"},
	}
	for _, step := range steps {
		assert.Equal(t, step[1], run(m, step[0]), step[0])
	}
	assert.Equal(t, "z\n", run(m, "LPRINT l"))
}

func TestStackAndQueue(t *testing.T) {
	m := cli.NewManager()
	assert.Equal(t, "OK\n", run(m, "SCREATE s"))
	runAll(t, m, "SPUSH s 1", "SPUSH s two words")
	assert.Equal(t, "two words\n", run(m, "SPEEK s"))
	assert.Equal(t, "Stack(top->bottom): two words, 1\n", run(m, "SPRINT s"))
	assert.Equal(t, "two words\n", run(m, "SPOP s"))
	assert.Equal(t, "1\n", run(m, "SPOP s"))
	assert.Equal(t, "Error: stack is empty\n", run(m, "SPOP s"))
	assert.Equal(t, "Error: stack is empty\n", run(m, "SPEEK s"))

	assert.Equal(t, "OK\n", run(m, "QCREATE q"))
	runAll(t, m, "QPUSH q a", "QPUSH q b")
	assert.Equal(t, "a\n", run(m, "QFRONT q"))
	assert.Equal(t, "Queue(front->rear): a, b\n", run(m, "QPRINT q"))
	assert.Equal(t, "a\n", run(m, "QPOP q"))
	assert.Equal(t, "b\n", run(m, "QPOP q"))
	assert.Equal(t, "Error: queue is empty\n", run(m, "QPOP q"))
	assert.Equal(t, "Error: queue is empty\n", run(m, "QFRONT q"))
}

func TestTreeCommands(t *testing.T) {
	m := cli.NewManager()
	assert.Equal(t, "OK\n", run(m, "TCREATE t"))
	for i := 1; i <= 4; i++ {
		assert.Equal(t, "OK\n", run(m, fmt.Sprintf("TINSERT t %d", i)))
	}
	assert.Equal(t, "true\n", run(m, "TFIND t 3"))
	assert.Equal(t, "false\n", run(m, "TFIND t 9"))
	assert.Equal(t, "true\n", run(m, "TCOMPLETE t"))
	assert.Equal(t, "1\n|-- 2\n|   |-- 4\n`-- 3\n", run(m, "TPRINT t"))
	assert.Equal(t, "Error: Integer required\n", run(m, "TINSERT t abc"))
	assert.Equal(t, "Error: Missing argument: integer\n", run(m, "TINSERT t"))
}

func TestHashCommands(t *testing.T) {
	m := cli.NewManager()
	runAll(t, m, "HCREATE h", "HCREATE d double 7", "HCREATE l LINEAR")
	runAll(t, m, "HINSERT d 523456795")
	assert.Equal(t, "true\n", run(m, "HFIND d 523456795"))
	assert.Equal(t, "false\n", run(m, "HFIND d 1"))
	assert.Equal(t, "Hash(double, capacity=7, size=1)\n[3] 523456795\n", run(m, "HPRINT d"))
	assert.Equal(t, "OK\n", run(m, "HDEL d 523456795"))
	assert.Equal(t, "Error: Key not found\n", run(m, "HDEL d 523456795"))
	assert.Equal(t, "Error: Structure 'd' already exists\n", run(m, "HCREATE d"))
	assert.Equal(t, "Error: Expected LINEAR or DOUBLE\n", run(m, "HCREATE x triple"))
	assert.Equal(t, "Error: Capacity must be a positive integer\n", run(m, "HCREATE x double 0"))
	assert.Equal(t, "Error: Capacity must be a positive integer\n", run(m, "HCREATE x double abc"))
	runAll(t, m, "HCREATE tiny linear 1", "HINSERT tiny 1")
	assert.Equal(t, "Error: Table is full\n", run(m, "HINSERT tiny 2"))
}

func TestGeneralCommands(t *testing.T) {
	m := cli.NewManager()
	assert.Equal(t, "(no structures)\n", run(m, "LIST"))
	runAll(t, m, "SCREATE s", "SPUSH s 1", "QCREATE q")
	assert.Equal(t, "Stack(top->bottom): 1\n", run(m, "PRINT s"))
	assert.Equal(t, "1\n", run(m, "SIZE s"))
	assert.Equal(t, "q (queue, size 0)\ns (stack, size 1)\n", run(m, "LIST"))
	assert.Equal(t, 2, m.Count())
	assert.NotNil(t, m.Find("s"))
	assert.Nil(t, m.Find("none"))
	assert.Equal(t, "OK\n", run(m, "DROP s"))
	assert.Equal(t, "Error: Structure 's' not found\n", run(m, "DROP s"))
	assert.Equal(t, "OK\n", run(m, "CLEAR"))
	assert.Equal(t, 0, m.Count())
}

func TestWrongTypeAndMissingStructure(t *testing.T) {
	m := cli.NewManager()
	runAll(t, m, "SCREATE s")
	assert.Equal(t, "Error: Structure 's' has another type (stack)\n", run(m, "QPUSH s x"))
	assert.Equal(t, "Error: Structure 'nothing' not found\n", run(m, "PRINT nothing"))
	assert.Equal(t, "Error: Structure 'nothing' not found\n", run(m, "SIZE nothing"))
	assert.Equal(t, "Error: Missing argument: name\n", run(m, "PRINT"))
}

func TestMutatingCommands(t *testing.T) {
	m := cli.NewManager()
	for _, line := range []string{"MCREATE a", "mpush a B 1", "FPOP f B", "SPUSH s 1", "QPOP q", "TINSERT t 1", "HINSERT h 1", "DROP a", "CLEAR", "LOAD a BIN file"} {
		assert.True(t, m.IsMutating(line), line)
	}
	for _, line := range []string{"MGET a 0", "FPRINT f", "TFIND t 1", "HFIND h 1", "PRINT a", "SAVE a BIN f", "LIST", "HELP", "UNKNOWN", ""} {
		assert.False(t, m.IsMutating(line), line)
	}
}

func TestSaveLoadAllStructures(t *testing.T) {
	m := cli.NewManager()
	runAll(t, m, "MCREATE m", "MPUSH m B a b", "FCREATE f", "FPUSH f B 1", "FPUSH f B 2", "LCREATE l", "LPUSH l B x",
		"SCREATE s", "SPUSH s 5", "SPUSH s 6", "QCREATE q", "QPUSH q 7", "TCREATE t", "TINSERT t 1", "TINSERT t 2",
		"TINSERT t 3", "HCREATE h double 7", "HINSERT h 10", "HINSERT h 20")
	dir := t.TempDir()
	for _, name := range []string{"m", "f", "l", "s", "q", "t", "h"} {
		for _, format := range []string{"BIN", "TXT"} {
			path := filepath.Join(dir, name+"."+format)
			require.Equal(t, "OK\n", run(m, fmt.Sprintf("SAVE %s %s %s", name, format, path)))
			require.Equal(t, "OK\n", run(m, fmt.Sprintf("LOAD %s_copy %s %s", name, format, path)))
			assert.Equal(t, run(m, "PRINT "+name), run(m, "PRINT "+name+"_copy"), name+" "+format)
		}
	}
}

func TestLoadReplacesExisting(t *testing.T) {
	m := cli.NewManager()
	runAll(t, m, "SCREATE s", "SPUSH s old", "SCREATE other", "SPUSH other new")
	path := filepath.Join(t.TempDir(), "x.bin")
	runAll(t, m, "SAVE other BIN "+path, "LOAD s BIN "+path)
	assert.Equal(t, "Stack(top->bottom): new\n", run(m, "PRINT s"))
}

func TestSaveLoadErrors(t *testing.T) {
	m := cli.NewManager()
	runAll(t, m, "SCREATE s")
	dir := t.TempDir()
	missing := filepath.Join(dir, "no_such_dir", "file.bin")
	existing := filepath.Join(dir, "garbage.txt")
	require.NoError(t, os.WriteFile(existing, []byte("this is not a serialized structure\n"), 0o600))
	for line, want := range map[string]string{
		"SAVE s ZIP x":           "Error: Expected BIN or TXT\n",
		"SAVE nothing BIN x":     "Error: Structure 'nothing' not found\n",
		"SAVE s BIN " + missing:  "Error: Cannot open file: " + missing + "\n",
		"SAVE s BIN":             "Error: Missing argument: path\n",
		"LOAD s ZIP x":           "Error: Expected BIN or TXT\n",
		"LOAD s BIN " + missing:  "Error: Cannot open file: " + missing + "\n",
		"LOAD s TXT " + existing: "Error: serialization error: bad text signature\n",
		"LOAD s BIN " + existing: "Error: serialization error: bad binary signature\n",
	} {
		assert.Equal(t, want, run(m, line), line)
	}
}

func TestCommandLogRestoresState(t *testing.T) {
	path := filepath.Join(t.TempDir(), "log.data")
	log := cli.CommandLog{Path: path}
	m := cli.NewManager()
	var out bytes.Buffer
	assert.Equal(t, cli.StatusOK, cli.RunAndLog(m, log, "SCREATE s", &out))
	assert.Equal(t, cli.StatusOK, cli.RunAndLog(m, log, "SPUSH s 1", &out))
	assert.Equal(t, cli.StatusOK, cli.RunAndLog(m, log, "SPUSH s 2", &out))
	assert.Equal(t, cli.StatusOK, cli.RunAndLog(m, log, "SPOP s", &out))
	assert.Equal(t, cli.StatusOK, cli.RunAndLog(m, log, "SPEEK s", &out))            // чтение не сохраняется
	assert.Equal(t, cli.StatusError, cli.RunAndLog(m, log, "SPUSH nothing 1", &out)) // ошибки не сохраняются
	assert.Equal(t, cli.StatusExit, cli.RunAndLog(m, log, "EXIT", &out))

	saved, err := os.ReadFile(path)
	require.NoError(t, err)
	assert.Equal(t, "SCREATE s\nSPUSH s 1\nSPUSH s 2\nSPOP s\n", string(saved))

	restored := cli.NewManager()
	assert.Equal(t, 4, log.Replay(restored))
	assert.Equal(t, "Stack(top->bottom): 1\n", run(restored, "SPRINT s"))
}

func TestCommandLogMissingAndUnwritableFile(t *testing.T) {
	dir := t.TempDir()
	assert.Equal(t, 0, cli.CommandLog{Path: filepath.Join(dir, "missing.data")}.Replay(cli.NewManager()))

	unwritable := cli.CommandLog{Path: filepath.Join(dir, "no_such_dir", "file.data")}
	assert.Error(t, unwritable.Append("SCREATE s"))
	var out bytes.Buffer
	assert.Equal(t, cli.StatusOK, cli.RunAndLog(cli.NewManager(), unwritable, "SCREATE s", &out))
	assert.True(t, strings.Contains(out.String(), "Warning: cannot write to data file"))
}
