package ds

import "io"

type slNode struct {
	value string
	next  *slNode
}

// SinglyLinkedList - односвязный список строк (только указатель на голову,
// поэтому операции в конце списка выполняются за O(N)).
type SinglyLinkedList struct {
	head *slNode
	size int
}

// Clone возвращает глубокую копию.
func (l *SinglyLinkedList) Clone() *SinglyLinkedList {
	c := &SinglyLinkedList{}
	c.assign(l.ToSlice())
	return c
}

// PushFront добавляет элемент в голову.
func (l *SinglyLinkedList) PushFront(value string) {
	l.head = &slNode{value, l.head}
	l.size++
}

// PushBack добавляет элемент в хвост (O(N)).
func (l *SinglyLinkedList) PushBack(value string) {
	if l.head == nil {
		l.PushFront(value)
		return
	}
	last := l.head
	for last.next != nil {
		last = last.next
	}
	last.next = &slNode{value, nil}
	l.size++
}

// PushBefore вставляет value перед первым элементом target.
func (l *SinglyLinkedList) PushBefore(target, value string) bool {
	var prev *slNode
	for n := l.head; n != nil; prev, n = n, n.next {
		if n.value != target {
			continue
		}
		if prev == nil {
			l.PushFront(value)
		} else {
			prev.next = &slNode{value, n}
			l.size++
		}
		return true
	}
	return false
}

// PushAfter вставляет value после первого элемента target.
func (l *SinglyLinkedList) PushAfter(target, value string) bool {
	for n := l.head; n != nil; n = n.next {
		if n.value == target {
			n.next = &slNode{value, n.next}
			l.size++
			return true
		}
	}
	return false
}

// PopFront удаляет голову.
func (l *SinglyLinkedList) PopFront() error {
	if l.head == nil {
		return ErrListEmpty
	}
	l.head = l.head.next
	l.size--
	return nil
}

// PopBack удаляет хвост (O(N)).
func (l *SinglyLinkedList) PopBack() error {
	if l.head == nil {
		return ErrListEmpty
	}
	if l.head.next == nil {
		return l.PopFront()
	}
	prev := l.head
	for prev.next.next != nil {
		prev = prev.next
	}
	prev.next = nil
	l.size--
	return nil
}

// PopBefore удаляет элемент перед target; false, если удалять нечего.
func (l *SinglyLinkedList) PopBefore(target string) bool {
	var beforePrev, prev *slNode
	for n := l.head; n != nil; beforePrev, prev, n = prev, n, n.next {
		if n.value != target {
			continue
		}
		if prev == nil {
			return false
		}
		if beforePrev == nil {
			l.head = n
		} else {
			beforePrev.next = n
		}
		l.size--
		return true
	}
	return false
}

// PopAfter удаляет элемент после target; false, если удалять нечего.
func (l *SinglyLinkedList) PopAfter(target string) bool {
	for n := l.head; n != nil; n = n.next {
		if n.value != target {
			continue
		}
		if n.next == nil {
			return false
		}
		n.next = n.next.next
		l.size--
		return true
	}
	return false
}

// RemoveByValue удаляет первое вхождение значения.
func (l *SinglyLinkedList) RemoveByValue(value string) bool {
	var prev *slNode
	for n := l.head; n != nil; prev, n = n, n.next {
		if n.value != value {
			continue
		}
		if prev == nil {
			l.head = n.next
		} else {
			prev.next = n.next
		}
		l.size--
		return true
	}
	return false
}

// IndexOf возвращает индекс первого вхождения или -1.
func (l *SinglyLinkedList) IndexOf(value string) int {
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
func (l *SinglyLinkedList) Contains(value string) bool { return l.IndexOf(value) >= 0 }

// ToSlice возвращает элементы от головы к хвосту.
func (l *SinglyLinkedList) ToSlice() []string {
	values := make([]string, 0, l.size)
	for n := l.head; n != nil; n = n.next {
		values = append(values, n.value)
	}
	return values
}

// StringFrom возвращает содержимое начиная с найденного элемента ("" если элемента нет).
func (l *SinglyLinkedList) StringFrom(value string) string {
	var values []string
	found := false
	for n := l.head; n != nil; n = n.next {
		found = found || n.value == value
		if found {
			values = append(values, n.value)
		}
	}
	return join(values, " -> ")
}

// Type реализует Structure.
func (l *SinglyLinkedList) Type() StructureType { return TypeSinglyList }

// Size реализует Structure.
func (l *SinglyLinkedList) Size() int { return l.size }

// Clear реализует Structure.
func (l *SinglyLinkedList) Clear() { l.head, l.size = nil, 0 }

// String реализует Structure: "a -> b -> c".
func (l *SinglyLinkedList) String() string {
	if l.head == nil {
		return emptyText
	}
	return join(l.ToSlice(), " -> ")
}

func (l *SinglyLinkedList) assign(values []string) {
	l.Clear()
	var tail *slNode
	for _, v := range values {
		node := &slNode{value: v}
		if tail == nil {
			l.head = node
		} else {
			tail.next = node
		}
		tail = node
		l.size++
	}
}

// SaveBinary реализует Structure.
func (l *SinglyLinkedList) SaveBinary(w io.Writer) error {
	return saveStringsBinary(w, TypeSinglyList, l.ToSlice())
}

// LoadBinary реализует Structure.
func (l *SinglyLinkedList) LoadBinary(r io.Reader) error {
	values, err := loadStringsBinary(r, TypeSinglyList)
	if err == nil {
		l.assign(values)
	}
	return err
}

// SaveText реализует Structure.
func (l *SinglyLinkedList) SaveText(w io.Writer) error {
	return saveStringsText(w, TypeSinglyList, l.ToSlice())
}

// LoadText реализует Structure.
func (l *SinglyLinkedList) LoadText(r io.Reader) error {
	values, err := loadStringsText(r, TypeSinglyList)
	if err == nil {
		l.assign(values)
	}
	return err
}
