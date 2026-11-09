// Команда lab3 - консольное приложение для работы со структурами данных.
//
//	lab3 [--file <файл данных>] [--query "<команда>"]
package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"

	"lab3/cli"
)

func main() {
	dataFile := flag.String("file", "file.data", "файл данных (журнал команд)")
	query := flag.String("query", "", "выполнить одну команду и выйти")
	flag.Parse()

	manager := cli.NewManager()
	log := cli.CommandLog{Path: *dataFile}
	log.Replay(manager)

	if *query != "" {
		if cli.RunAndLog(manager, log, *query, os.Stdout) == cli.StatusError {
			os.Exit(1)
		}
		return
	}

	fmt.Printf("=== DATA STRUCTURES ===\nData file: %s\nType HELP to see commands.\n\n", *dataFile)
	scanner := bufio.NewScanner(os.Stdin)
	for {
		fmt.Print("> ")
		if !scanner.Scan() {
			break
		}
		if cli.RunAndLog(manager, log, scanner.Text(), os.Stdout) == cli.StatusExit {
			break
		}
	}
}
