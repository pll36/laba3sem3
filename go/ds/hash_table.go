package ds

import (
	"errors"
	"fmt"
	"io"
	"strconv"
	"strings"
)

// Mode - способ разрешения коллизий.
type Mode byte

// Режимы хеш-таблицы.
const (
	ModeLinear Mode = iota // линейное пробирование
	ModeDouble             // двойное хеширование
)

func (m Mode) String() string {
	if m == ModeDouble {
		return "double"
	}
	return "linear"
}

// DefaultCapacity - ёмкость хеш-таблицы по умолчанию.
const DefaultCapacity = 11

type slotState byte

const (
	stateEmpty slotState = iota
	stateOccupied
	stateDeleted
)

type hashSlot struct {
	key   int32
	state slotState
}

// HashTable - хеш-таблица целых чисел с открытой адресацией.
// Хеш-функция - метод свёртки (сумма групп по три цифры по модулю размера).
type HashTable struct {
	mode  Mode
	slots []hashSlot
	count int
}

// NewHashTable создаёт таблицу; capacity должна быть положительной.
func NewHashTable(mode Mode, capacity int) (*HashTable, error) {
	if capacity <= 0 {
		return nil, errors.New("capacity must be positive")
	}
	return &HashTable{mode: mode, slots: make([]hashSlot, capacity)}, nil
}

// Clone возвращает глубокую копию.
func (h *HashTable) Clone() *HashTable {
	return &HashTable{mode: h.mode, slots: append([]hashSlot(nil), h.slots...), count: h.count}
}

func magnitude(key int32) uint32 {
	if key < 0 {
		return 0 - uint32(key)
	}
	return uint32(key)
}

// FoldingHash - метод свёртки: 523456795 -> 523 + 456 + 795 = 1774, затем по модулю capacity.
func FoldingHash(key int32, capacity int) int {
	if capacity <= 0 {
		return 0
	}
	var sum uint64
	for rest := magnitude(key); rest > 0; rest /= 1000 {
		sum += uint64(rest % 1000)
	}
	return int(sum % uint64(capacity))
}

func gcd(a, b int) int {
	for b != 0 {
		a, b = b, a%b
	}
	return a
}

// stepHash - шаг двойного хеширования, взаимно простой с размером таблицы.
func stepHash(key int32, capacity int) int {
	if capacity <= 2 {
		return 1
	}
	step := 1 + int(magnitude(key)%uint32(capacity-1))
	for gcd(step, capacity) != 1 {
		step++
		if step >= capacity {
			step = 1
		}
	}
	return step
}

func (h *HashTable) start(key int32) (position, step int) {
	position = FoldingHash(key, len(h.slots))
	step = 1
	if h.mode == ModeDouble {
		step = stepHash(key, len(h.slots))
	}
	return position, step
}

// Insert добавляет ключ; false, если свободных ячеек нет.
func (h *HashTable) Insert(key int32) bool {
	pos, step := h.start(key)
	firstDeleted := -1
	for i := 0; i < len(h.slots); i++ {
		s := h.slots[pos]
		switch s.state {
		case stateOccupied:
			if s.key == key {
				return true
			}
		case stateDeleted:
			if firstDeleted < 0 {
				firstDeleted = pos
			}
		default: // пустая ячейка: дальше ключа точно нет
			target := pos
			if firstDeleted >= 0 {
				target = firstDeleted
			}
			h.slots[target] = hashSlot{key, stateOccupied}
			h.count++
			return true
		}
		pos = (pos + step) % len(h.slots)
	}
	if firstDeleted >= 0 {
		h.slots[firstDeleted] = hashSlot{key, stateOccupied}
		h.count++
		return true
	}
	return false
}

// Contains проверяет наличие ключа.
func (h *HashTable) Contains(key int32) bool {
	pos, step := h.start(key)
	for i := 0; i < len(h.slots); i++ {
		s := h.slots[pos]
		if s.state == stateEmpty {
			return false
		}
		if s.state == stateOccupied && s.key == key {
			return true
		}
		pos = (pos + step) % len(h.slots)
	}
	return false
}

// Erase удаляет ключ; false, если ключа нет.
func (h *HashTable) Erase(key int32) bool {
	pos, step := h.start(key)
	for i := 0; i < len(h.slots); i++ {
		s := &h.slots[pos]
		if s.state == stateEmpty {
			return false
		}
		if s.state == stateOccupied && s.key == key {
			s.state = stateDeleted
			h.count--
			return true
		}
		pos = (pos + step) % len(h.slots)
	}
	return false
}

// ProbeCount - число проверенных ячеек при поиске ключа.
func (h *HashTable) ProbeCount(key int32) int {
	pos, step := h.start(key)
	probes := 0
	for i := 0; i < len(h.slots); i++ {
		s := h.slots[pos]
		probes++
		if s.state == stateEmpty || (s.state == stateOccupied && s.key == key) {
			break
		}
		pos = (pos + step) % len(h.slots)
	}
	return probes
}

// Mode возвращает режим разрешения коллизий.
func (h *HashTable) Mode() Mode { return h.mode }

// Capacity возвращает число ячеек.
func (h *HashTable) Capacity() int { return len(h.slots) }

// LoadFactor - коэффициент заполнения.
func (h *HashTable) LoadFactor() float64 { return float64(h.count) / float64(len(h.slots)) }

// Type реализует Structure.
func (h *HashTable) Type() StructureType { return TypeHash }

// Size реализует Structure.
func (h *HashTable) Size() int { return h.count }

// Clear реализует Structure (ёмкость и режим сохраняются).
func (h *HashTable) Clear() {
	for i := range h.slots {
		h.slots[i] = hashSlot{}
	}
	h.count = 0
}

// String реализует Structure.
func (h *HashTable) String() string {
	var sb strings.Builder
	fmt.Fprintf(&sb, "Hash(%s, capacity=%d, size=%d)", h.mode, len(h.slots), h.count)
	for i, s := range h.slots {
		if s.state == stateOccupied {
			fmt.Fprintf(&sb, "\n[%d] %d", i, s.key)
		}
	}
	return sb.String()
}

func (h *HashTable) apply(mode Mode, slots []hashSlot) error {
	if len(slots) == 0 {
		return formatError("hash table capacity must be positive")
	}
	count := 0
	for _, s := range slots {
		if s.state == stateOccupied {
			count++
		}
	}
	h.mode, h.slots, h.count = mode, slots, count
	return nil
}

// SaveBinary реализует Structure.
func (h *HashTable) SaveBinary(w io.Writer) error {
	bw := newBinWriter(TypeHash)
	bw.u8(byte(h.mode))
	bw.u32(uint32(len(h.slots)))
	for _, s := range h.slots {
		bw.u8(byte(s.state))
		bw.i32(s.key)
	}
	return bw.flush(w)
}

// LoadBinary реализует Structure.
func (h *HashTable) LoadBinary(r io.Reader) error {
	br, err := openBinary(r, TypeHash)
	if err != nil {
		return err
	}
	rawMode, err := br.u8()
	if err != nil {
		return err
	}
	if rawMode > 1 {
		return formatError("invalid hash mode")
	}
	count, err := br.u32()
	if err != nil {
		return err
	}
	var slots []hashSlot
	for i := uint32(0); i < count; i++ {
		state, err := br.u8()
		if err != nil {
			return err
		}
		if state > 2 {
			return formatError("invalid slot state")
		}
		key, err := br.i32()
		if err != nil {
			return err
		}
		slots = append(slots, hashSlot{key, slotState(state)})
	}
	return h.apply(Mode(rawMode), slots)
}

// SaveText реализует Structure.
func (h *HashTable) SaveText(w io.Writer) error {
	var sb strings.Builder
	sb.WriteString(textHeader(TypeHash))
	fmt.Fprintf(&sb, "MODE %s\nCAPACITY %d\n", h.mode, len(h.slots))
	letters := map[slotState]string{stateEmpty: "E", stateOccupied: "O", stateDeleted: "D"}
	for _, s := range h.slots {
		sb.WriteString(letters[s.state] + " " + strconv.Itoa(int(s.key)) + "\n")
	}
	_, err := io.WriteString(w, sb.String())
	return err
}

// LoadText реализует Structure.
func (h *HashTable) LoadText(r io.Reader) error {
	tr, err := openText(r, TypeHash)
	if err != nil {
		return err
	}
	modeText, err := tr.field("MODE")
	if err != nil {
		return err
	}
	mode := ModeLinear
	switch modeText {
	case "double":
		mode = ModeDouble
	case "linear":
	default:
		return formatError("invalid hash mode: %s", modeText)
	}
	count, err := tr.uint32Field("CAPACITY")
	if err != nil {
		return err
	}
	var slots []hashSlot
	for i := uint32(0); i < count; i++ {
		line, err := tr.line()
		if err != nil {
			return err
		}
		if len(line) < 3 || line[1] != ' ' {
			return formatError("invalid slot line: '%s'", line)
		}
		var state slotState
		switch line[0] {
		case 'O':
			state = stateOccupied
		case 'D':
			state = stateDeleted
		case 'E':
		default:
			return formatError("invalid slot state: '%s'", line)
		}
		key, err := parseInt32(line[2:])
		if err != nil {
			return err
		}
		slots = append(slots, hashSlot{key, state})
	}
	return h.apply(mode, slots)
}
