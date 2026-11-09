package ds

import "io"

// InitialCapacity - начальная ёмкость динамического массива.
const InitialCapacity = 10

// DynamicArray - динамический массив строк; ёмкость удваивается при заполнении.
type DynamicArray struct {
	data []string
	size int
}

// NewDynamicArray создаёт пустой массив.
func NewDynamicArray() *DynamicArray {
	return &DynamicArray{data: make([]string, InitialCapacity)}
}

// Clone возвращает глубокую копию.
func (a *DynamicArray) Clone() *DynamicArray {
	return &DynamicArray{data: append(make([]string, 0, len(a.data)), a.data...), size: a.size}
}

func (a *DynamicArray) grow() {
	newCap := len(a.data) * 2
	if newCap == 0 {
		newCap = InitialCapacity
	}
	fresh := make([]string, newCap)
	copy(fresh, a.data[:a.size])
	a.data = fresh
}

// PushBack добавляет элемент в конец.
func (a *DynamicArray) PushBack(value string) {
	if a.size >= len(a.data) {
		a.grow()
	}
	a.data[a.size] = value
	a.size++
}

// PushFront добавляет элемент в начало.
func (a *DynamicArray) PushFront(value string) {
	_ = a.InsertAt(0, value)
}

// InsertAt вставляет элемент по индексу (0..Size()).
func (a *DynamicArray) InsertAt(index int, value string) error {
	if index < 0 || index > a.size {
		return ErrIndexOutOfRange
	}
	if a.size >= len(a.data) {
		a.grow()
	}
	copy(a.data[index+1:a.size+1], a.data[index:a.size])
	a.data[index] = value
	a.size++
	return nil
}

// PopBack удаляет последний элемент.
func (a *DynamicArray) PopBack() error {
	if a.size == 0 {
		return ErrArrayEmpty
	}
	return a.RemoveAt(a.size - 1)
}

// PopFront удаляет первый элемент.
func (a *DynamicArray) PopFront() error {
	if a.size == 0 {
		return ErrArrayEmpty
	}
	return a.RemoveAt(0)
}

// RemoveAt удаляет элемент по индексу.
func (a *DynamicArray) RemoveAt(index int) error {
	if index < 0 || index >= a.size {
		return ErrIndexOutOfRange
	}
	copy(a.data[index:a.size-1], a.data[index+1:a.size])
	a.size--
	a.data[a.size] = ""
	return nil
}

// Get возвращает элемент по индексу.
func (a *DynamicArray) Get(index int) (string, error) {
	if index < 0 || index >= a.size {
		return "", ErrIndexOutOfRange
	}
	return a.data[index], nil
}

// Set заменяет элемент по индексу.
func (a *DynamicArray) Set(index int, value string) error {
	if index < 0 || index >= a.size {
		return ErrIndexOutOfRange
	}
	a.data[index] = value
	return nil
}

// Capacity возвращает текущую ёмкость.
func (a *DynamicArray) Capacity() int { return len(a.data) }

// ToSlice возвращает копию элементов.
func (a *DynamicArray) ToSlice() []string { return append([]string(nil), a.data[:a.size]...) }

// Type реализует Structure.
func (a *DynamicArray) Type() StructureType { return TypeArray }

// Size реализует Structure.
func (a *DynamicArray) Size() int { return a.size }

// Clear реализует Structure.
func (a *DynamicArray) Clear() {
	for i := 0; i < a.size; i++ {
		a.data[i] = ""
	}
	a.size = 0
}

// String реализует Structure: "[a, b, c]".
func (a *DynamicArray) String() string {
	if a.size == 0 {
		return emptyText
	}
	return "[" + join(a.ToSlice(), ", ") + "]"
}

func (a *DynamicArray) assign(values []string) {
	a.Clear()
	for _, v := range values {
		a.PushBack(v)
	}
}

// SaveBinary реализует Structure.
func (a *DynamicArray) SaveBinary(w io.Writer) error {
	return saveStringsBinary(w, TypeArray, a.ToSlice())
}

// LoadBinary реализует Structure; при ошибке содержимое не меняется.
func (a *DynamicArray) LoadBinary(r io.Reader) error {
	values, err := loadStringsBinary(r, TypeArray)
	if err == nil {
		a.assign(values)
	}
	return err
}

// SaveText реализует Structure.
func (a *DynamicArray) SaveText(w io.Writer) error {
	return saveStringsText(w, TypeArray, a.ToSlice())
}

// LoadText реализует Structure; при ошибке содержимое не меняется.
func (a *DynamicArray) LoadText(r io.Reader) error {
	values, err := loadStringsText(r, TypeArray)
	if err == nil {
		a.assign(values)
	}
	return err
}
