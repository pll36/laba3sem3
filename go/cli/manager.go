// Package cli содержит интерпретатор текстовых команд для работы со структурами данных.
package cli

import (
	"errors"
	"fmt"
	"io"
	"os"
	"sort"
	"strconv"
	"strings"
	"unicode"

	"lab3/ds"
)

// Status - результат выполнения команды.
type Status int

// Возможные результаты выполнения команды.
const (
	StatusOK Status = iota
	StatusError
	StatusExit
)

type command struct {
	mutating bool
	run      func(a *args, out io.Writer) error
}

// Manager хранит именованные структуры и выполняет команды (MPUSH, FPOP, TINSERT, HFIND, SAVE, ...).
type Manager struct {
	items    map[string]ds.Structure
	commands map[string]command
}

// NewManager создаёт менеджер со всеми зарегистрированными командами.
func NewManager() *Manager {
	m := &Manager{items: map[string]ds.Structure{}, commands: map[string]command{}}
	m.registerGeneral()
	m.registerArray()
	registerList(m, "F", func() list { return &ds.SinglyLinkedList{} })
	registerList(m, "L", func() list { return &ds.DoublyLinkedList{} })
	m.registerDoublyExtras()
	m.registerStack()
	m.registerQueue()
	m.registerTree()
	m.registerHash()
	return m
}

// ---------------------------------------------------------------- разбор аргументов

type args struct{ rest string }

func (a *args) optWord() (string, bool) {
	a.rest = strings.TrimLeftFunc(a.rest, unicode.IsSpace)
	if a.rest == "" {
		return "", false
	}
	end := strings.IndexFunc(a.rest, unicode.IsSpace)
	if end < 0 {
		end = len(a.rest)
	}
	word := a.rest[:end]
	a.rest = a.rest[end:]
	return word, true
}

func (a *args) word(what string) (string, error) {
	w, ok := a.optWord()
	if !ok {
		return "", fmt.Errorf("Missing argument: %s", what)
	}
	return w, nil
}

// text - остаток строки (значение может содержать пробелы).
func (a *args) text(what string) (string, error) {
	s := strings.TrimSpace(a.rest)
	a.rest = ""
	if s == "" {
		return "", fmt.Errorf("Missing argument: %s", what)
	}
	return s, nil
}

func (a *args) integer() (int32, error) {
	w, err := a.word("integer")
	if err != nil {
		return 0, err
	}
	v, err := strconv.ParseInt(w, 10, 32)
	if err != nil || strings.HasPrefix(w, "+") {
		return 0, errors.New("Integer required")
	}
	return int32(v), nil
}

func (a *args) index() (int, error) {
	w, err := a.word("index")
	if err != nil {
		return 0, err
	}
	v, err := strconv.ParseUint(w, 10, 31)
	if err != nil {
		return 0, errors.New("Non-negative index required")
	}
	return int(v), nil
}

func boolText(v bool) string { return strconv.FormatBool(v) }

// ---------------------------------------------------------------- выполнение

func (m *Manager) add(name string, mutating bool, run func(a *args, out io.Writer) error) {
	m.commands[name] = command{mutating, run}
}

// Execute выполняет одну команду, результат пишет в out.
func (m *Manager) Execute(line string, out io.Writer) Status {
	a := &args{line}
	name, ok := a.optWord()
	if !ok {
		return StatusOK
	}
	name = strings.ToUpper(name)
	if name == "EXIT" {
		return StatusExit
	}
	cmd, found := m.commands[name]
	if !found {
		fmt.Fprintf(out, "Unknown command: %s (type HELP)\n", name)
		return StatusError
	}
	if err := cmd.run(a, out); err != nil {
		fmt.Fprintf(out, "Error: %s\n", err)
		return StatusError
	}
	return StatusOK
}

// IsMutating сообщает, изменяет ли команда состав структур (такие команды сохраняются в файл).
func (m *Manager) IsMutating(line string) bool {
	name, ok := (&args{line}).optWord()
	if !ok {
		return false
	}
	cmd, found := m.commands[strings.ToUpper(name)]
	return found && cmd.mutating
}

// Count - число структур.
func (m *Manager) Count() int { return len(m.items) }

// Find возвращает структуру по имени (nil, если её нет).
func (m *Manager) Find(name string) ds.Structure { return m.items[name] }

func (m *Manager) any(name string) (ds.Structure, error) {
	item, ok := m.items[name]
	if !ok {
		return nil, fmt.Errorf("Structure '%s' not found", name)
	}
	return item, nil
}

func get[T ds.Structure](m *Manager, a *args) (T, error) {
	var zero T
	name, err := a.word("name")
	if err != nil {
		return zero, err
	}
	item, err := m.any(name)
	if err != nil {
		return zero, err
	}
	typed, ok := item.(T)
	if !ok {
		return zero, fmt.Errorf("Structure '%s' has another type (%s)", name, item.Type())
	}
	return typed, nil
}

func (m *Manager) create(a *args, out io.Writer, factory func() ds.Structure) error {
	name, err := a.word("name")
	if err != nil {
		return err
	}
	if _, exists := m.items[name]; exists {
		return fmt.Errorf("Structure '%s' already exists", name)
	}
	m.items[name] = factory()
	fmt.Fprintln(out, "OK")
	return nil
}

// ---------------------------------------------------------------- общие команды

func (m *Manager) registerGeneral() {
	m.add("HELP", false, func(_ *args, out io.Writer) error {
		_, err := io.WriteString(out, HelpText())
		return err
	})

	m.add("PRINT", false, func(a *args, out io.Writer) error {
		name, err := a.word("name")
		if err != nil {
			return err
		}
		item, err := m.any(name)
		if err == nil {
			fmt.Fprintln(out, item)
		}
		return err
	})
	m.add("SIZE", false, func(a *args, out io.Writer) error {
		name, err := a.word("name")
		if err != nil {
			return err
		}
		item, err := m.any(name)
		if err == nil {
			fmt.Fprintln(out, item.Size())
		}
		return err
	})
	m.add("LIST", false, func(_ *args, out io.Writer) error {
		if len(m.items) == 0 {
			fmt.Fprintln(out, "(no structures)")
		}
		names := make([]string, 0, len(m.items))
		for name := range m.items {
			names = append(names, name)
		}
		sort.Strings(names)
		for _, name := range names {
			item := m.items[name]
			fmt.Fprintf(out, "%s (%s, size %d)\n", name, item.Type(), item.Size())
		}
		return nil
	})
	m.add("DROP", true, func(a *args, out io.Writer) error {
		name, err := a.word("name")
		if err != nil {
			return err
		}
		if _, err := m.any(name); err != nil {
			return err
		}
		delete(m.items, name)
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add("CLEAR", true, func(_ *args, out io.Writer) error {
		m.items = map[string]ds.Structure{}
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add("SAVE", false, m.save)
	m.add("LOAD", true, m.load)
}

func parseFormat(a *args) (binary bool, path string, err error) {
	format, err := a.word("format")
	if err != nil {
		return false, "", err
	}
	path, err = a.text("path")
	if err != nil {
		return false, "", err
	}
	switch strings.ToUpper(format) {
	case "BIN":
		return true, path, nil
	case "TXT":
		return false, path, nil
	}
	return false, "", errors.New("Expected BIN or TXT")
}

func (m *Manager) save(a *args, out io.Writer) error {
	name, err := a.word("name")
	if err != nil {
		return err
	}
	item, err := m.any(name)
	if err != nil {
		return err
	}
	binary, path, err := parseFormat(a)
	if err != nil {
		return err
	}
	file, err := os.Create(path)
	if err != nil {
		return fmt.Errorf("Cannot open file: %s", path)
	}
	if binary {
		err = item.SaveBinary(file)
	} else {
		err = item.SaveText(file)
	}
	if closeErr := file.Close(); err == nil {
		err = closeErr
	}
	if err != nil {
		return err
	}
	fmt.Fprintln(out, "OK")
	return nil
}

func (m *Manager) load(a *args, out io.Writer) error {
	name, err := a.word("name")
	if err != nil {
		return err
	}
	binary, path, err := parseFormat(a)
	if err != nil {
		return err
	}
	file, err := os.Open(path)
	if err != nil {
		return fmt.Errorf("Cannot open file: %s", path)
	}
	defer file.Close()
	// Сначала определяем тип по заголовку, затем читаем файл с начала
	var kind ds.StructureType
	if binary {
		kind, err = ds.ReadBinaryType(file)
	} else {
		kind, err = ds.ReadTextType(file)
	}
	if err != nil {
		return err
	}
	if _, err = file.Seek(0, io.SeekStart); err != nil {
		return err
	}
	fresh := ds.New(kind)
	if binary {
		err = fresh.LoadBinary(file)
	} else {
		err = fresh.LoadText(file)
	}
	if err != nil {
		return err
	}
	m.items[name] = fresh
	fmt.Fprintln(out, "OK")
	return nil
}

// ---------------------------------------------------------------- массив (M)

func (m *Manager) registerArray() {
	m.add("MCREATE", true, func(a *args, out io.Writer) error {
		return m.create(a, out, func() ds.Structure { return ds.NewDynamicArray() })
	})
	endOp := func(push bool) func(a *args, out io.Writer) error {
		return func(a *args, out io.Writer) error {
			arr, err := get[*ds.DynamicArray](m, a)
			if err != nil {
				return err
			}
			where, err := a.word("position")
			if err != nil {
				return err
			}
			where = strings.ToUpper(where)
			if where != "B" && where != "F" {
				return errors.New("Expected B or F")
			}
			if push {
				value, err := a.text("value")
				if err != nil {
					return err
				}
				if where == "B" {
					arr.PushBack(value)
				} else {
					arr.PushFront(value)
				}
			} else if where == "B" {
				err = arr.PopBack()
			} else {
				err = arr.PopFront()
			}
			if err == nil {
				fmt.Fprintln(out, "OK")
			}
			return err
		}
	}
	m.add("MPUSH", true, endOp(true))
	m.add("MPOP", true, endOp(false))
	m.add("MINS", true, func(a *args, out io.Writer) error {
		arr, err := get[*ds.DynamicArray](m, a)
		if err != nil {
			return err
		}
		index, err := a.index()
		if err != nil {
			return err
		}
		value, err := a.text("value")
		if err != nil {
			return err
		}
		if err = arr.InsertAt(index, value); err == nil {
			fmt.Fprintln(out, "OK")
		}
		return err
	})
	m.add("MDEL", true, func(a *args, out io.Writer) error {
		arr, err := get[*ds.DynamicArray](m, a)
		if err != nil {
			return err
		}
		index, err := a.index()
		if err != nil {
			return err
		}
		if err = arr.RemoveAt(index); err == nil {
			fmt.Fprintln(out, "OK")
		}
		return err
	})
	m.add("MSET", true, func(a *args, out io.Writer) error {
		arr, err := get[*ds.DynamicArray](m, a)
		if err != nil {
			return err
		}
		index, err := a.index()
		if err != nil {
			return err
		}
		value, err := a.text("value")
		if err != nil {
			return err
		}
		if err = arr.Set(index, value); err == nil {
			fmt.Fprintln(out, "OK")
		}
		return err
	})
	m.add("MGET", false, func(a *args, out io.Writer) error {
		arr, err := get[*ds.DynamicArray](m, a)
		if err != nil {
			return err
		}
		index, err := a.index()
		if err != nil {
			return err
		}
		value, err := arr.Get(index)
		if err == nil {
			fmt.Fprintln(out, value)
		}
		return err
	})
	m.add("MSIZE", false, func(a *args, out io.Writer) error {
		arr, err := get[*ds.DynamicArray](m, a)
		if err == nil {
			fmt.Fprintln(out, arr.Size())
		}
		return err
	})
	m.add("MPRINT", false, func(a *args, out io.Writer) error {
		arr, err := get[*ds.DynamicArray](m, a)
		if err == nil {
			fmt.Fprintln(out, arr)
		}
		return err
	})
}

// ---------------------------------------------------------------- списки (F, L)

// list - общий набор операций односвязного и двусвязного списков.
type list interface {
	ds.Structure
	PushFront(string)
	PushBack(string)
	PushBefore(target, value string) bool
	PushAfter(target, value string) bool
	PopFront() error
	PopBack() error
	PopBefore(target string) bool
	PopAfter(target string) bool
	RemoveByValue(string) bool
	IndexOf(string) int
	StringFrom(string) string
}

func registerList(m *Manager, prefix string, factory func() list) {
	m.add(prefix+"CREATE", true, func(a *args, out io.Writer) error {
		return m.create(a, out, func() ds.Structure { return factory() })
	})
	m.add(prefix+"PUSH", true, func(a *args, out io.Writer) error {
		l, err := get[list](m, a)
		if err != nil {
			return err
		}
		where, err := a.word("position")
		if err != nil {
			return err
		}
		switch w := strings.ToUpper(where); w {
		case "B", "F":
			value, err := a.text("value")
			if err != nil {
				return err
			}
			if w == "B" {
				l.PushBack(value)
			} else {
				l.PushFront(value)
			}
		case "BEFORE", "AFTER":
			target, err := a.word("target")
			if err != nil {
				return err
			}
			value, err := a.text("value")
			if err != nil {
				return err
			}
			var done bool
			if w == "BEFORE" {
				done = l.PushBefore(target, value)
			} else {
				done = l.PushAfter(target, value)
			}
			if !done {
				return errors.New("Target not found")
			}
		default:
			return errors.New("Expected B, F, BEFORE or AFTER")
		}
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add(prefix+"POP", true, func(a *args, out io.Writer) error {
		l, err := get[list](m, a)
		if err != nil {
			return err
		}
		where, err := a.word("position")
		if err != nil {
			return err
		}
		switch w := strings.ToUpper(where); w {
		case "B":
			err = l.PopBack()
		case "F":
			err = l.PopFront()
		case "BEFORE", "AFTER":
			target, targetErr := a.word("target")
			if targetErr != nil {
				return targetErr
			}
			var done bool
			if w == "BEFORE" {
				done = l.PopBefore(target)
			} else {
				done = l.PopAfter(target)
			}
			if !done {
				err = errors.New("Target not found or no neighbour to remove")
			}
		default:
			err = errors.New("Expected B, F, BEFORE or AFTER")
		}
		if err == nil {
			fmt.Fprintln(out, "OK")
		}
		return err
	})
	m.add(prefix+"DEL", true, func(a *args, out io.Writer) error {
		l, err := get[list](m, a)
		if err != nil {
			return err
		}
		value, err := a.text("value")
		if err != nil {
			return err
		}
		if !l.RemoveByValue(value) {
			return errors.New("Value not found")
		}
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add(prefix+"FIND", false, func(a *args, out io.Writer) error {
		l, err := get[list](m, a)
		if err != nil {
			return err
		}
		value, err := a.text("value")
		if err != nil {
			return err
		}
		index := l.IndexOf(value)
		if index < 0 {
			return errors.New("Value not found")
		}
		fmt.Fprintln(out, index)
		return nil
	})
	m.add(prefix+"GET", false, func(a *args, out io.Writer) error {
		l, err := get[list](m, a)
		if err != nil {
			return err
		}
		value, err := a.text("value")
		if err != nil {
			return err
		}
		text := l.StringFrom(value)
		if text == "" {
			return errors.New("Value not found")
		}
		fmt.Fprintln(out, text)
		return nil
	})
	if prefix == "F" {
		m.add("FPRINT", false, func(a *args, out io.Writer) error {
			l, err := get[*ds.SinglyLinkedList](m, a)
			if err == nil {
				fmt.Fprintln(out, l)
			}
			return err
		})
	}
}

func (m *Manager) registerDoublyExtras() {
	m.add("LGETB", false, func(a *args, out io.Writer) error {
		l, err := get[*ds.DoublyLinkedList](m, a)
		if err != nil {
			return err
		}
		value, err := a.text("value")
		if err != nil {
			return err
		}
		text := l.StringFromBackward(value)
		if text == "" {
			return errors.New("Value not found")
		}
		fmt.Fprintln(out, text)
		return nil
	})
	m.add("LPRINT", false, func(a *args, out io.Writer) error {
		l, err := get[*ds.DoublyLinkedList](m, a)
		if err != nil {
			return err
		}
		direction := "F"
		if w, ok := a.optWord(); ok {
			direction = strings.ToUpper(w)
		}
		switch direction {
		case "F":
			fmt.Fprintln(out, l)
		case "B":
			fmt.Fprintln(out, l.StringBackward())
		default:
			return errors.New("Expected F or B")
		}
		return nil
	})
}

// ---------------------------------------------------------------- стек (S), очередь (Q)

func (m *Manager) registerStack() {
	m.add("SCREATE", true, func(a *args, out io.Writer) error {
		return m.create(a, out, func() ds.Structure { return &ds.Stack{} })
	})
	m.add("SPUSH", true, func(a *args, out io.Writer) error {
		s, err := get[*ds.Stack](m, a)
		if err != nil {
			return err
		}
		value, err := a.text("value")
		if err != nil {
			return err
		}
		s.Push(value)
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add("SPOP", true, func(a *args, out io.Writer) error {
		s, err := get[*ds.Stack](m, a)
		if err != nil {
			return err
		}
		value, err := s.Pop()
		if err == nil {
			fmt.Fprintln(out, value)
		}
		return err
	})
	m.add("SPEEK", false, func(a *args, out io.Writer) error {
		s, err := get[*ds.Stack](m, a)
		if err != nil {
			return err
		}
		value, err := s.Peek()
		if err == nil {
			fmt.Fprintln(out, value)
		}
		return err
	})
	m.add("SPRINT", false, func(a *args, out io.Writer) error {
		s, err := get[*ds.Stack](m, a)
		if err == nil {
			fmt.Fprintln(out, s)
		}
		return err
	})
}

func (m *Manager) registerQueue() {
	m.add("QCREATE", true, func(a *args, out io.Writer) error {
		return m.create(a, out, func() ds.Structure { return &ds.Queue{} })
	})
	m.add("QPUSH", true, func(a *args, out io.Writer) error {
		q, err := get[*ds.Queue](m, a)
		if err != nil {
			return err
		}
		value, err := a.text("value")
		if err != nil {
			return err
		}
		q.Push(value)
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add("QPOP", true, func(a *args, out io.Writer) error {
		q, err := get[*ds.Queue](m, a)
		if err != nil {
			return err
		}
		value, err := q.Pop()
		if err == nil {
			fmt.Fprintln(out, value)
		}
		return err
	})
	m.add("QFRONT", false, func(a *args, out io.Writer) error {
		q, err := get[*ds.Queue](m, a)
		if err != nil {
			return err
		}
		value, err := q.Front()
		if err == nil {
			fmt.Fprintln(out, value)
		}
		return err
	})
	m.add("QPRINT", false, func(a *args, out io.Writer) error {
		q, err := get[*ds.Queue](m, a)
		if err == nil {
			fmt.Fprintln(out, q)
		}
		return err
	})
}

// ---------------------------------------------------------------- дерево (T)

func (m *Manager) registerTree() {
	m.add("TCREATE", true, func(a *args, out io.Writer) error {
		return m.create(a, out, func() ds.Structure { return &ds.CompleteBinaryTree{} })
	})
	m.add("TINSERT", true, func(a *args, out io.Writer) error {
		t, err := get[*ds.CompleteBinaryTree](m, a)
		if err != nil {
			return err
		}
		value, err := a.integer()
		if err != nil {
			return err
		}
		t.Insert(value)
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add("TFIND", false, func(a *args, out io.Writer) error {
		t, err := get[*ds.CompleteBinaryTree](m, a)
		if err != nil {
			return err
		}
		value, err := a.integer()
		if err != nil {
			return err
		}
		fmt.Fprintln(out, boolText(t.Contains(value)))
		return nil
	})
	m.add("TCOMPLETE", false, func(a *args, out io.Writer) error {
		t, err := get[*ds.CompleteBinaryTree](m, a)
		if err == nil {
			fmt.Fprintln(out, boolText(t.IsComplete()))
		}
		return err
	})
	m.add("TPRINT", false, func(a *args, out io.Writer) error {
		t, err := get[*ds.CompleteBinaryTree](m, a)
		if err == nil {
			fmt.Fprintln(out, t)
		}
		return err
	})
}

// ---------------------------------------------------------------- хеш-таблица (H)

func (m *Manager) registerHash() {
	m.add("HCREATE", true, func(a *args, out io.Writer) error {
		name, err := a.word("name")
		if err != nil {
			return err
		}
		if _, exists := m.items[name]; exists {
			return fmt.Errorf("Structure '%s' already exists", name)
		}
		mode, capacity := ds.ModeLinear, ds.DefaultCapacity
		if word, ok := a.optWord(); ok {
			switch strings.ToUpper(word) {
			case "DOUBLE":
				mode = ds.ModeDouble
			case "LINEAR":
			default:
				return errors.New("Expected LINEAR or DOUBLE")
			}
			if capWord, ok := a.optWord(); ok {
				v, convErr := strconv.ParseUint(capWord, 10, 31)
				if convErr != nil || v == 0 {
					return errors.New("Capacity must be a positive integer")
				}
				capacity = int(v)
			}
		}
		table, err := ds.NewHashTable(mode, capacity)
		if err != nil {
			return err
		}
		m.items[name] = table
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add("HINSERT", true, func(a *args, out io.Writer) error {
		h, err := get[*ds.HashTable](m, a)
		if err != nil {
			return err
		}
		key, err := a.integer()
		if err != nil {
			return err
		}
		if !h.Insert(key) {
			return errors.New("Table is full")
		}
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add("HFIND", false, func(a *args, out io.Writer) error {
		h, err := get[*ds.HashTable](m, a)
		if err != nil {
			return err
		}
		key, err := a.integer()
		if err != nil {
			return err
		}
		fmt.Fprintln(out, boolText(h.Contains(key)))
		return nil
	})
	m.add("HDEL", true, func(a *args, out io.Writer) error {
		h, err := get[*ds.HashTable](m, a)
		if err != nil {
			return err
		}
		key, err := a.integer()
		if err != nil {
			return err
		}
		if !h.Erase(key) {
			return errors.New("Key not found")
		}
		fmt.Fprintln(out, "OK")
		return nil
	})
	m.add("HPRINT", false, func(a *args, out io.Writer) error {
		h, err := get[*ds.HashTable](m, a)
		if err == nil {
			fmt.Fprintln(out, h)
		}
		return err
	})
}

// HelpText - справка по командам.
func HelpText() string {
	return `=== DATA STRUCTURES ===
ARRAY:
  MCREATE <name>
  MPUSH <name> B|F <value>      MPOP <name> B|F
  MINS <name> <index> <value>   MDEL <name> <index>
  MGET <name> <index>           MSET <name> <index> <value>
  MSIZE <name>                  MPRINT <name>
SINGLY LINKED LIST (F) / DOUBLY LINKED LIST (L):
  FCREATE <name>
  FPUSH <name> B|F <value>
  FPUSH <name> BEFORE|AFTER <target> <value>
  FPOP <name> B|F
  FPOP <name> BEFORE|AFTER <target>
  FDEL <name> <value>           FFIND <name> <value>
  FGET <name> <value>           FPRINT <name>
  (for the doubly linked list use the prefix L; extra: LGETB <name> <value>, LPRINT <name> [F|B])
STACK:
  SCREATE <name>   SPUSH <name> <value>   SPOP <name>   SPEEK <name>   SPRINT <name>
QUEUE:
  QCREATE <name>   QPUSH <name> <value>   QPOP <name>   QFRONT <name>   QPRINT <name>
COMPLETE BINARY TREE:
  TCREATE <name>   TINSERT <name> <int>   TFIND <name> <int>   TCOMPLETE <name>   TPRINT <name>
HASH TABLE:
  HCREATE <name> [LINEAR|DOUBLE] [capacity]
  HINSERT <name> <int>   HFIND <name> <int>   HDEL <name> <int>   HPRINT <name>
SERIALIZATION:
  SAVE <name> BIN|TXT <path>    LOAD <name> BIN|TXT <path>
GENERAL:
  PRINT <name>   SIZE <name>   LIST   DROP <name>   CLEAR   HELP   EXIT
`
}
