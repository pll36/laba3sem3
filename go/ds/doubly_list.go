package ds

import "io"

type dlNode struct {
	value      string
	next, prev *dlNode
}

// DoublyLinkedList - двусвязный список строк (указатели на голову и хвост: O(1) с обоих концов).
type DoublyLinkedList struct {
	head, tail *dlNode
	size       int
}

// Clone возвращает глубокую копию.
func (l *DoublyLinkedList) Clone() *DoublyLinkedList {
	c := &DoublyLinkedList{}
	c.assign(l.ToSlice())
	return c
}

// PushFront добавляет элемент в голову.
func (l *DoublyLinkedList) PushFront(value string) {
	n := &dlNode{value: value, next: l.head}
	if l.head != nil {
		l.head.prev = n
	} else {
		l.tail = n
	}
	l.head = n
	l.size++
}

// PushBack добавляет элемент в хвост.
func (l *DoublyLinkedList) PushBack(value string) {
	n := &dlNode{value: value, prev: l.tail}
	if l.tail != nil {
		l.tail.next = n
	} else {
		l.head = n
	}
	l.tail = n
	l.size++
}

func (l *DoublyLinkedList) find(value string) *dlNode {
	for n := l.head; n != nil; n = n.next {
		if n.value == value {
			return n
		}
	}
	return nil
}

// PushBefore вставляет value перед первым элементом target.
func (l *DoublyLinkedList) PushBefore(target, value string) bool {
	n := l.find(target)
	if n == nil {
		return false
	}
	if n == l.head {
		l.PushFront(value)
		return true
	}
	fresh := &dlNode{value: value, next: n, prev: n.prev}
	n.prev.next = fresh
	n.prev = fresh
	l.size++
	return true
}

// PushAfter вставляет value после первого элемента target.
func (l *DoublyLinkedList) PushAfter(target, value string) bool {
	n := l.find(target)
	if n == nil {
		return false
	}
	if n == l.tail {
		l.PushBack(value)
		return true
	}
	fresh := &dlNode{value: value, next: n.next, prev: n}
	n.next.prev = fresh
	n.next = fresh
	l.size++
	return true
}

func (l *DoublyLinkedList) unlink(n *dlNode) {
	if n.prev != nil {
		n.prev.next = n.next
	} else {
		l.head = n.next
	}
	if n.next != nil {
		n.next.prev = n.prev
	} else {
		l.tail = n.prev
	}
	l.size--
}

// PopFront удаляет голову.
func (l *DoublyLinkedList) PopFront() error {
	if l.head == nil {
		return ErrListEmpty
	}
	l.unlink(l.head)
	return nil
}

// PopBack удаляет хвост.
func (l *DoublyLinkedList) PopBack() error {
	if l.tail == nil {
		return ErrListEmpty
	}
	l.unlink(l.tail)
	return nil
}

// PopBefore удаляет элемент перед target.
func (l *DoublyLinkedList) PopBefore(target string) bool {
	n := l.find(target)
	if n == nil || n.prev == nil {
		return false
	}
	l.unlink(n.prev)
	return true
}

// PopAfter удаляет элемент после target.
func (l *DoublyLinkedList) PopAfter(target string) bool {
	n := l.find(target)
	if n == nil || n.next == nil {
		return false
	}
	l.unlink(n.next)
	return true
}

// RemoveByValue удаляет первое вхождение значения.
func (l *DoublyLinkedList) RemoveByValue(value string) bool {
	n := l.find(value)
	if n == nil {
		return false
	}
	l.unlink(n)
	return true
}

// IndexOf возвращает индекс первого вхождения или -1.
func (l *DoublyLinkedList) IndexOf(value string) int {
	i := 0
	for n := l.head; n != nil; n = n.next {
		if n.value == value {
			return i
		}
		i++
	}
	return -1
}

// Contains проверяет наличие значения.
func (l *DoublyLinkedList) Contains(value string) bool { return l.find(value) != nil }

// ToSlice возвращает элементы от головы к хвосту.
func (l *DoublyLinkedList) ToSlice() []string {
	values := make([]string, 0, l.size)
	for n := l.head; n != nil; n = n.next {
		values = append(values, n.value)
	}
	return values
}

// ToSliceBackward возвращает элементы от хвоста к голове.
func (l *DoublyLinkedList) ToSliceBackward() []string {
	values := make([]string, 0, l.size)
	for n := l.tail; n != nil; n = n.prev {
		values = append(values, n.value)
	}
	return values
}

// StringBackward - содержимое от хвоста к голове.
func (l *DoublyLinkedList) StringBackward() string {
	if l.head == nil {
		return emptyText
	}
	return join(l.ToSliceBackward(), " <-> ")
}

// StringFrom - содержимое от найденного элемента вперёд ("" если элемента нет).
func (l *DoublyLinkedList) StringFrom(value string) string {
	var values []string
	for n := l.find(value); n != nil; n = n.next {
		values = append(values, n.value)
	}
	return join(values, " <-> ")
}

// StringFromBackward - содержимое от найденного элемента назад ("" если элемента нет).
func (l *DoublyLinkedList) StringFromBackward(value string) string {
	var values []string
	for n := l.find(value); n != nil; n = n.prev {
		values = append(values, n.value)
	}
	return join(values, " <-> ")
}

// Type реализует Structure.
func (l *DoublyLinkedList) Type() StructureType { return TypeDoublyList }

// Size реализует Structure.
func (l *DoublyLinkedList) Size() int { return l.size }

// Clear реализует Structure.
func (l *DoublyLinkedList) Clear() { l.head, l.tail, l.size = nil, nil, 0 }

// String реализует Structure: "a <-> b <-> c".
func (l *DoublyLinkedList) String() string {
	if l.head == nil {
		return emptyText
	}
	return join(l.ToSlice(), " <-> ")
}

func (l *DoublyLinkedList) assign(values []string) {
	l.Clear()
	for _, v := range values {
		l.PushBack(v)
	}
}

// SaveBinary реализует Structure.
func (l *DoublyLinkedList) SaveBinary(w io.Writer) error {
	return saveStringsBinary(w, TypeDoublyList, l.ToSlice())
}

// LoadBinary реализует Structure.
func (l *DoublyLinkedList) LoadBinary(r io.Reader) error {
	values, err := loadStringsBinary(r, TypeDoublyList)
	if err == nil {
		l.assign(values)
	}
	return err
}

// SaveText реализует Structure.
func (l *DoublyLinkedList) SaveText(w io.Writer) error {
	return saveStringsText(w, TypeDoublyList, l.ToSlice())
}

// LoadText реализует Structure.
func (l *DoublyLinkedList) LoadText(r io.Reader) error {
	values, err := loadStringsText(r, TypeDoublyList)
	if err == nil {
		l.assign(values)
	}
	return err
}
