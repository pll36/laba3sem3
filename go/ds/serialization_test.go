package ds_test

import (
	"bytes"
	"errors"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"lab3/ds"
)

var allTypes = []ds.StructureType{ds.TypeArray, ds.TypeSinglyList, ds.TypeDoublyList, ds.TypeStack, ds.TypeQueue, ds.TypeTree, ds.TypeHash}

func TestTypeNamesAreBidirectional(t *testing.T) {
	for _, typ := range allTypes {
		parsed, err := ds.ParseTypeName(typ.String())
		require.NoError(t, err)
		assert.Equal(t, typ, parsed)
	}
	_, err := ds.ParseTypeName("graph")
	assert.ErrorIs(t, err, ds.ErrFormat)
	assert.Equal(t, "unknown", ds.StructureType(99).String())
}

func TestNewCreatesEveryType(t *testing.T) {
	for _, typ := range allTypes {
		s := ds.New(typ)
		require.NotNil(t, s)
		assert.Equal(t, typ, s.Type())
		assert.Equal(t, 0, s.Size())
	}
	assert.Nil(t, ds.New(99))
}

func TestHeadersIdentifyType(t *testing.T) {
	s := &ds.Stack{}
	s.Push("x")
	typ, err := ds.ReadBinaryType(bytes.NewReader(binaryOf(t, s)))
	require.NoError(t, err)
	assert.Equal(t, ds.TypeStack, typ)
	typ, err = ds.ReadTextType(bytes.NewReader([]byte(textOf(t, s))))
	require.NoError(t, err)
	assert.Equal(t, ds.TypeStack, typ)

	for _, data := range []string{"DSB1\x63", "DS", "XXXX\x01", ""} {
		_, err = ds.ReadBinaryType(bytes.NewReader([]byte(data)))
		assert.ErrorIs(t, err, ds.ErrFormat, data)
	}
	for _, data := range []string{"HELLO\nTYPE stack\n", "DSTXT 1\nTYPE graph\n", "DSTXT 1\nKIND stack\n", "DSTXT 1", ""} {
		_, err = ds.ReadTextType(bytes.NewReader([]byte(data)))
		assert.ErrorIs(t, err, ds.ErrFormat, data)
	}
}

func TestEverythingRoundTripsPolymorphically(t *testing.T) {
	array := ds.NewDynamicArray()
	array.PushBack("1")
	slist := &ds.SinglyLinkedList{}
	slist.PushBack("2")
	dlist := &ds.DoublyLinkedList{}
	dlist.PushBack("3")
	stack := &ds.Stack{}
	stack.Push("4")
	queue := &ds.Queue{}
	queue.Push("5")
	tree := makeTree(6)
	hash := newTable(t, ds.ModeDouble, 7)
	hash.Insert(7)

	for _, item := range []ds.Structure{array, slist, dlist, stack, queue, tree, hash} {
		fromBin, fromText := ds.New(item.Type()), ds.New(item.Type())
		roundTrip(t, item, fromBin, fromText)
		assert.Equal(t, item.String(), fromBin.String())
		assert.Equal(t, item.String(), fromText.String())
	}
}

func TestLoadingIntoAnotherTypeFails(t *testing.T) {
	s := &ds.Stack{}
	s.Push("x")
	h := newTable(t, ds.ModeLinear, 11)
	assert.ErrorIs(t, loadBinary(h, binaryOf(t, s)), ds.ErrFormat)
	assert.ErrorIs(t, loadText(h, textOf(t, s)), ds.ErrFormat)
}

func TestMalformedTextLines(t *testing.T) {
	a := ds.NewDynamicArray()
	for _, text := range []string{
		"DSTXT 1\nTYPE array\nCOUNT abc\n",
		"DSTXT 1\nTYPE array\nCOUNT +1\nx\n",
		"DSTXT 1\nTYPE array\nCOUNT 1\nbad\\q\n",
		"DSTXT 1\nTYPE array\nCOUNT 1\nbad\\\n",
		"DSTXT 1\nTYPE array\nTOTAL 1\n",
		"DSTXT 1\nTYPE array\nCOUNTER 1\n",
	} {
		assert.ErrorIs(t, loadText(a, text), ds.ErrFormat, text)
	}
	// CRLF и последняя строка без перевода строки допустимы
	require.NoError(t, loadText(a, "DSTXT 1\r\nTYPE array\r\nCOUNT 2\r\nx\r\ny"))
	assert.Equal(t, []string{"x", "y"}, a.ToSlice())
}

func TestStringLengthLargerThanDataIsRejected(t *testing.T) {
	a := ds.NewDynamicArray()
	data := []byte("DSB1\x01\x01\x00\x00\x00\xff\xff\xff\xffabc")
	assert.ErrorIs(t, loadBinary(a, data), ds.ErrFormat)
	big := ds.NewDynamicArray()
	big.PushBack(string(bytes.Repeat([]byte("x"), 10000))) // больше внутреннего буфера чтения
	restored := ds.NewDynamicArray()
	require.NoError(t, loadBinary(restored, binaryOf(t, big)))
	assert.Equal(t, big.ToSlice(), restored.ToSlice())
}

type failingWriter struct{}

func (failingWriter) Write([]byte) (int, error) { return 0, errors.New("disk full") }

func TestWriteErrorsArePropagated(t *testing.T) {
	for _, typ := range allTypes {
		s := ds.New(typ)
		assert.Error(t, s.SaveBinary(failingWriter{}), typ.String())
		assert.Error(t, s.SaveText(failingWriter{}), typ.String())
	}
}
