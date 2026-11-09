// Package ds содержит структуры данных лабораторной работы №3:
// динамический массив, списки, стек, очередь, полное бинарное дерево и хеш-таблицу.
package ds

import (
	"errors"
	"io"
)

// Structure - общий интерфейс всех структур данных.
type Structure interface {
	Type() StructureType
	Size() int
	Clear()
	String() string
	SaveBinary(w io.Writer) error
	LoadBinary(r io.Reader) error
	SaveText(w io.Writer) error
	LoadText(r io.Reader) error
}

// Ошибки операций над структурами.
var (
	ErrIndexOutOfRange = errors.New("index out of range")
	ErrArrayEmpty      = errors.New("array is empty")
	ErrListEmpty       = errors.New("list is empty")
	ErrStackEmpty      = errors.New("stack is empty")
	ErrQueueEmpty      = errors.New("queue is empty")
)

const emptyText = "(empty)"

// New создаёт пустую структуру заданного типа.
func New(t StructureType) Structure {
	switch t {
	case TypeArray:
		return NewDynamicArray()
	case TypeSinglyList:
		return &SinglyLinkedList{}
	case TypeDoublyList:
		return &DoublyLinkedList{}
	case TypeStack:
		return &Stack{}
	case TypeQueue:
		return &Queue{}
	case TypeTree:
		return &CompleteBinaryTree{}
	case TypeHash:
		table, _ := NewHashTable(ModeLinear, DefaultCapacity)
		return table
	}
	return nil
}

func join(items []string, sep string) string {
	out := ""
	for i, s := range items {
		if i > 0 {
			out += sep
		}
		out += s
	}
	return out
}
