package cli

import (
	"bufio"
	"fmt"
	"io"
	"os"
)

// CommandLog - файл команд: успешные команды, изменившие данные, дописываются в файл,
// при запуске файл воспроизводится и восстанавливает состояние структур.
type CommandLog struct{ Path string }

// Replay выполняет все команды из файла и возвращает их число.
func (l CommandLog) Replay(m *Manager) int {
	file, err := os.Open(l.Path)
	if err != nil {
		return 0
	}
	defer file.Close()
	executed := 0
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		if line := scanner.Text(); line != "" {
			m.Execute(line, io.Discard)
			executed++
		}
	}
	return executed
}

// Append дописывает команду в файл.
func (l CommandLog) Append(line string) error {
	file, err := os.OpenFile(l.Path, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0o644)
	if err != nil {
		return err
	}
	_, err = fmt.Fprintln(file, line)
	if closeErr := file.Close(); err == nil {
		err = closeErr
	}
	return err
}

// RunAndLog выполняет команду и, если она успешно изменила данные, записывает её в журнал.
func RunAndLog(m *Manager, log CommandLog, line string, out io.Writer) Status {
	status := m.Execute(line, out)
	if status == StatusOK && m.IsMutating(line) {
		if err := log.Append(line); err != nil {
			fmt.Fprintf(out, "Warning: cannot write to data file %s\n", log.Path)
		}
	}
	return status
}
