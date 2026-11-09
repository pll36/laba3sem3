package ds

import "io"

type snode struct {
	value string
	next  *snode
}

// Stack - стек (LIFO) на односвязных узлах.
type Stack struct {
	top  *snode
	size int
}

// Clone возвращает глубокую копию.
func (s *Stack) Clone() *Stack {
	c := &Stack{}
	c.assign(s.ToSlice())
	return c
}

// Push кладёт элемент на вершину.
func (s *Stack) Push(value string) {
	s.top = &snode{value, s.top}
	s.size++
}

// Pop снимает и возвращает вершину.
func (s *Stack) Pop() (string, error) {
	if s.top == nil {
		return "", ErrStackEmpty
	}
	value := s.top.value
	s.top = s.top.next
	s.size--
	return value, nil
}

// Peek возвращает вершину, не снимая её.
func (s *Stack) Peek() (string, error) {
	if s.top == nil {
		return "", ErrStackEmpty
	}
	return s.top.value, nil
}

// ToSlice возвращает элементы от дна к вершине.
func (s *Stack) ToSlice() []string {
	values := make([]string, s.size)
	i := s.size - 1
	for n := s.top; n != nil; n = n.next {
		values[i] = n.value
		i--
	}
	return values
}

// Type реализует Structure.
func (s *Stack) Type() StructureType { return TypeStack }

// Size реализует Structure.
func (s *Stack) Size() int { return s.size }

// Clear реализует Structure.
func (s *Stack) Clear() { s.top, s.size = nil, 0 }

// String реализует Structure.
func (s *Stack) String() string {
	if s.top == nil {
		return emptyText
	}
	values := s.ToSlice()
	for i, j := 0, len(values)-1; i < j; i, j = i+1, j-1 {
		values[i], values[j] = values[j], values[i]
	}
	return "Stack(top->bottom): " + join(values, ", ")
}

func (s *Stack) assign(values []string) {
	s.Clear()
	for _, v := range values {
		s.Push(v)
	}
}

// SaveBinary реализует Structure.
func (s *Stack) SaveBinary(w io.Writer) error { return saveStringsBinary(w, TypeStack, s.ToSlice()) }

// LoadBinary реализует Structure.
func (s *Stack) LoadBinary(r io.Reader) error {
	values, err := loadStringsBinary(r, TypeStack)
	if err == nil {
		s.assign(values)
	}
	return err
}

// SaveText реализует Structure.
func (s *Stack) SaveText(w io.Writer) error { return saveStringsText(w, TypeStack, s.ToSlice()) }

// LoadText реализует Structure.
func (s *Stack) LoadText(r io.Reader) error {
	values, err := loadStringsText(r, TypeStack)
	if err == nil {
		s.assign(values)
	}
	return err
}

// Queue - очередь (FIFO) на односвязных узлах с указателями на начало и конец.
type Queue struct {
	front, rear *snode
	size        int
}

// Clone возвращает глубокую копию.
func (q *Queue) Clone() *Queue {
	c := &Queue{}
	c.assign(q.ToSlice())
	return c
}

// Push добавляет элемент в конец очереди.
func (q *Queue) Push(value string) {
	n := &snode{value: value}
	if q.rear != nil {
		q.rear.next = n
	} else {
		q.front = n
	}
	q.rear = n
	q.size++
}

// Pop извлекает элемент из начала очереди.
func (q *Queue) Pop() (string, error) {
	if q.front == nil {
		return "", ErrQueueEmpty
	}
	value := q.front.value
	q.front = q.front.next
	if q.front == nil {
		q.rear = nil
	}
	q.size--
	return value, nil
}

// Front возвращает первый элемент, не извлекая его.
func (q *Queue) Front() (string, error) {
	if q.front == nil {
		return "", ErrQueueEmpty
	}
	return q.front.value, nil
}

// ToSlice возвращает элементы от начала очереди к концу.
func (q *Queue) ToSlice() []string {
	values := make([]string, 0, q.size)
	for n := q.front; n != nil; n = n.next {
		values = append(values, n.value)
	}
	return values
}

// Type реализует Structure.
func (q *Queue) Type() StructureType { return TypeQueue }

// Size реализует Structure.
func (q *Queue) Size() int { return q.size }

// Clear реализует Structure.
func (q *Queue) Clear() { q.front, q.rear, q.size = nil, nil, 0 }

// String реализует Structure.
func (q *Queue) String() string {
	if q.front == nil {
		return emptyText
	}
	return "Queue(front->rear): " + join(q.ToSlice(), ", ")
}

func (q *Queue) assign(values []string) {
	q.Clear()
	for _, v := range values {
		q.Push(v)
	}
}

// SaveBinary реализует Structure.
func (q *Queue) SaveBinary(w io.Writer) error { return saveStringsBinary(w, TypeQueue, q.ToSlice()) }

// LoadBinary реализует Structure.
func (q *Queue) LoadBinary(r io.Reader) error {
	values, err := loadStringsBinary(r, TypeQueue)
	if err == nil {
		q.assign(values)
	}
	return err
}

// SaveText реализует Structure.
func (q *Queue) SaveText(w io.Writer) error { return saveStringsText(w, TypeQueue, q.ToSlice()) }

// LoadText реализует Structure.
func (q *Queue) LoadText(r io.Reader) error {
	values, err := loadStringsText(r, TypeQueue)
	if err == nil {
		q.assign(values)
	}
	return err
}
