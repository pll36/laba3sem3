package ds

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"strconv"
	"strings"
)

// ErrFormat - ошибка формата сериализованных данных.
var ErrFormat = errors.New("serialization error")

func formatError(format string, args ...any) error {
	return fmt.Errorf("%w: %s", ErrFormat, fmt.Sprintf(format, args...))
}

// StructureType - тип структуры (записывается в заголовок сериализованных данных).
type StructureType byte

// Типы структур данных (значения совпадают с C++-версией проекта).
const (
	TypeArray StructureType = iota + 1
	TypeSinglyList
	TypeDoublyList
	TypeStack
	TypeQueue
	TypeTree
	TypeHash
)

var typeNames = map[StructureType]string{
	TypeArray:      "array",
	TypeSinglyList: "slist",
	TypeDoublyList: "dlist",
	TypeStack:      "stack",
	TypeQueue:      "queue",
	TypeTree:       "tree",
	TypeHash:       "hash",
}

// String возвращает короткое имя типа: array, slist, dlist, stack, queue, tree, hash.
func (t StructureType) String() string {
	if name, ok := typeNames[t]; ok {
		return name
	}
	return "unknown"
}

// ParseTypeName - обратное преобразование к String.
func ParseTypeName(name string) (StructureType, error) {
	for t, n := range typeNames {
		if n == name {
			return t, nil
		}
	}
	return 0, formatError("unknown structure type: %s", name)
}

var (
	binaryMagic = []byte("DSB1")
	textMagic   = "DSTXT 1"
)

// ---------------------------------------------------------------- бинарный формат

type binWriter struct{ buf bytes.Buffer }

func (w *binWriter) u8(v byte)    { w.buf.WriteByte(v) }
func (w *binWriter) u32(v uint32) { _ = binary.Write(&w.buf, binary.LittleEndian, v) }
func (w *binWriter) i32(v int32)  { w.u32(uint32(v)) }
func (w *binWriter) str(s string) {
	w.u32(uint32(len(s)))
	w.buf.WriteString(s)
}

func newBinWriter(t StructureType) *binWriter {
	w := &binWriter{}
	w.buf.Write(binaryMagic)
	w.u8(byte(t))
	return w
}

func (w *binWriter) flush(dst io.Writer) error {
	_, err := dst.Write(w.buf.Bytes())
	return err
}

type binReader struct{ r io.Reader }

func (b *binReader) u8() (byte, error) {
	var one [1]byte
	if _, err := io.ReadFull(b.r, one[:]); err != nil {
		return 0, formatError("unexpected end of data")
	}
	return one[0], nil
}

func (b *binReader) u32() (uint32, error) {
	var four [4]byte
	if _, err := io.ReadFull(b.r, four[:]); err != nil {
		return 0, formatError("unexpected end of data")
	}
	return binary.LittleEndian.Uint32(four[:]), nil
}

func (b *binReader) i32() (int32, error) {
	v, err := b.u32()
	return int32(v), err
}

func (b *binReader) str() (string, error) {
	remaining, err := b.u32()
	if err != nil {
		return "", err
	}
	// Читаем кусками: повреждённая длина не вызовет огромного выделения памяти
	const maxChunk = 4096
	var out strings.Builder
	chunk := make([]byte, min(int(remaining), maxChunk))
	for remaining > 0 {
		n := min(int(remaining), len(chunk))
		if _, err := io.ReadFull(b.r, chunk[:n]); err != nil {
			return "", formatError("unexpected end of data")
		}
		out.Write(chunk[:n])
		remaining -= uint32(n)
	}
	return out.String(), nil
}

// ReadBinaryType читает заголовок бинарных данных и возвращает тип структуры.
func ReadBinaryType(r io.Reader) (StructureType, error) {
	magic := make([]byte, len(binaryMagic))
	if _, err := io.ReadFull(r, magic); err != nil || !bytes.Equal(magic, binaryMagic) {
		return 0, formatError("bad binary signature")
	}
	raw, err := (&binReader{r}).u8()
	if err != nil {
		return 0, err
	}
	t := StructureType(raw)
	if _, ok := typeNames[t]; !ok {
		return 0, formatError("unknown structure type tag")
	}
	return t, nil
}

func openBinary(r io.Reader, want StructureType) (*binReader, error) {
	got, err := ReadBinaryType(r)
	if err != nil {
		return nil, err
	}
	if got != want {
		return nil, formatError("structure type mismatch: expected %s, got %s", want, got)
	}
	return &binReader{r}, nil
}

// ---------------------------------------------------------------- текстовый формат

type textReader struct{ r *bufio.Reader }

func newTextReader(r io.Reader) *textReader { return &textReader{bufio.NewReader(r)} }

func (t *textReader) line() (string, error) {
	s, err := t.r.ReadString('\n')
	if err != nil && (err != io.EOF || s == "") {
		return "", formatError("unexpected end of data")
	}
	s = strings.TrimSuffix(s, "\n")
	return strings.TrimSuffix(s, "\r"), nil
}

// field читает строку "KEY значение" и проверяет ключ.
func (t *textReader) field(key string) (string, error) {
	s, err := t.line()
	if err != nil {
		return "", err
	}
	if s == key {
		return "", nil
	}
	if strings.HasPrefix(s, key+" ") {
		return s[len(key)+1:], nil
	}
	return "", formatError("expected field %s", key)
}

func (t *textReader) uint32Field(key string) (uint32, error) {
	s, err := t.field(key)
	if err != nil {
		return 0, err
	}
	return parseUint32(s)
}

func parseInt32(s string) (int32, error) {
	v, err := strconv.ParseInt(s, 10, 32)
	if err != nil || strings.HasPrefix(s, "+") {
		return 0, formatError("invalid number: '%s'", s)
	}
	return int32(v), nil
}

func parseUint32(s string) (uint32, error) {
	v, err := strconv.ParseUint(s, 10, 32)
	if err != nil {
		return 0, formatError("invalid number: '%s'", s)
	}
	return uint32(v), nil
}

func escapeLine(s string) string {
	s = strings.ReplaceAll(s, "\\", "\\\\")
	s = strings.ReplaceAll(s, "\n", "\\n")
	return strings.ReplaceAll(s, "\r", "\\r")
}

func unescapeLine(s string) (string, error) {
	var out strings.Builder
	for i := 0; i < len(s); i++ {
		if s[i] != '\\' {
			out.WriteByte(s[i])
			continue
		}
		i++
		if i >= len(s) {
			return "", formatError("dangling escape character")
		}
		switch s[i] {
		case '\\':
			out.WriteByte('\\')
		case 'n':
			out.WriteByte('\n')
		case 'r':
			out.WriteByte('\r')
		default:
			return "", formatError("unknown escape sequence")
		}
	}
	return out.String(), nil
}

// ReadTextType читает заголовок текстовых данных и возвращает тип структуры.
func ReadTextType(r io.Reader) (StructureType, error) {
	return readTextHeader(newTextReader(r))
}

func readTextHeader(t *textReader) (StructureType, error) {
	first, err := t.line()
	if err != nil || first != textMagic {
		return 0, formatError("bad text signature")
	}
	name, err := t.field("TYPE")
	if err != nil {
		return 0, err
	}
	return ParseTypeName(name)
}

func openText(r io.Reader, want StructureType) (*textReader, error) {
	t := newTextReader(r)
	got, err := readTextHeader(t)
	if err != nil {
		return nil, err
	}
	if got != want {
		return nil, formatError("structure type mismatch: expected %s, got %s", want, got)
	}
	return t, nil
}

func textHeader(t StructureType) string {
	return textMagic + "\nTYPE " + t.String() + "\n"
}

// ---------------------------------------------------------------- последовательности строк

func saveStringsBinary(dst io.Writer, t StructureType, values []string) error {
	w := newBinWriter(t)
	w.u32(uint32(len(values)))
	for _, v := range values {
		w.str(v)
	}
	return w.flush(dst)
}

func loadStringsBinary(src io.Reader, t StructureType) ([]string, error) {
	r, err := openBinary(src, t)
	if err != nil {
		return nil, err
	}
	count, err := r.u32()
	if err != nil {
		return nil, err
	}
	var values []string
	for i := uint32(0); i < count; i++ {
		s, err := r.str()
		if err != nil {
			return nil, err
		}
		values = append(values, s)
	}
	return values, nil
}

func saveStringsText(dst io.Writer, t StructureType, values []string) error {
	var sb strings.Builder
	sb.WriteString(textHeader(t))
	fmt.Fprintf(&sb, "COUNT %d\n", len(values))
	for _, v := range values {
		sb.WriteString(escapeLine(v))
		sb.WriteByte('\n')
	}
	_, err := io.WriteString(dst, sb.String())
	return err
}

func loadStringsText(src io.Reader, t StructureType) ([]string, error) {
	r, err := openText(src, t)
	if err != nil {
		return nil, err
	}
	count, err := r.uint32Field("COUNT")
	if err != nil {
		return nil, err
	}
	var values []string
	for i := uint32(0); i < count; i++ {
		s, err := r.line()
		if err != nil {
			return nil, err
		}
		if s, err = unescapeLine(s); err != nil {
			return nil, err
		}
		values = append(values, s)
	}
	return values, nil
}
