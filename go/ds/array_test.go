package ds_test

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"lab3/ds"
)

func TestArrayNewIsEmpty(t *testing.T) {
	a := ds.NewDynamicArray()
	assert.Equal(t, 0, a.Size())
	assert.Equal(t, ds.InitialCapacity, a.Capacity())
	assert.Equal(t, "(empty)", a.String())
	assert.Equal(t, ds.TypeArray, a.Type())
}

func TestArrayZeroValueIsUsable(t *testing.T) {
	var a ds.DynamicArray
	a.PushBack("x")
	assert.Equal(t, "[x]", a.String())
}

func TestArrayPushAndInsertKeepOrder(t *testing.T) {
	a := ds.NewDynamicArray()
	a.PushBack("b")
	a.PushFront("a")
	require.NoError(t, a.InsertAt(2, "d"))
	require.NoError(t, a.InsertAt(2, "c"))
	assert.Equal(t, []string{"a", "b", "c", "d"}, a.ToSlice())
	assert.Equal(t, "[a, b, c, d]", a.String())
}

func TestArrayInsertInvalidIndex(t *testing.T) {
	a := ds.NewDynamicArray()
	assert.ErrorIs(t, a.InsertAt(1, "x"), ds.ErrIndexOutOfRange)
	assert.ErrorIs(t, a.InsertAt(-1, "x"), ds.ErrIndexOutOfRange)
	require.NoError(t, a.InsertAt(0, "x"))
}

func TestArrayCapacityDoubles(t *testing.T) {
	a := ds.NewDynamicArray()
	for i := 0; i < 25; i++ {
		a.PushBack(fmt.Sprint(i))
	}
	assert.Equal(t, 40, a.Capacity())
	for i := 0; i < 25; i++ {
		v, err := a.Get(i)
		require.NoError(t, err)
		assert.Equal(t, fmt.Sprint(i), v)
	}
}

func TestArrayInsertIntoFullArrayGrows(t *testing.T) {
	a := ds.NewDynamicArray()
	for i := 0; i < ds.InitialCapacity; i++ {
		a.PushBack("v")
	}
	a.PushFront("first")
	assert.Equal(t, 2*ds.InitialCapacity, a.Capacity())
	v, _ := a.Get(0)
	assert.Equal(t, "first", v)
}

func TestArrayRemove(t *testing.T) {
	a := ds.NewDynamicArray()
	for _, v := range []string{"a", "b", "c", "d"} {
		a.PushBack(v)
	}
	require.NoError(t, a.RemoveAt(1))
	assert.Equal(t, []string{"a", "c", "d"}, a.ToSlice())
	require.NoError(t, a.PopFront())
	require.NoError(t, a.PopBack())
	assert.Equal(t, []string{"c"}, a.ToSlice())
}

func TestArrayInvalidAccess(t *testing.T) {
	a := ds.NewDynamicArray()
	assert.ErrorIs(t, a.PopBack(), ds.ErrArrayEmpty)
	assert.ErrorIs(t, a.PopFront(), ds.ErrArrayEmpty)
	assert.ErrorIs(t, a.RemoveAt(0), ds.ErrIndexOutOfRange)
	assert.ErrorIs(t, a.Set(0, "x"), ds.ErrIndexOutOfRange)
	_, err := a.Get(0)
	assert.ErrorIs(t, err, ds.ErrIndexOutOfRange)
	_, err = a.Get(-1)
	assert.ErrorIs(t, err, ds.ErrIndexOutOfRange)
}

func TestArraySetAndClear(t *testing.T) {
	a := ds.NewDynamicArray()
	a.PushBack("old")
	require.NoError(t, a.Set(0, "new"))
	v, _ := a.Get(0)
	assert.Equal(t, "new", v)
	a.Clear()
	assert.Equal(t, 0, a.Size())
	a.PushBack("again")
	assert.Equal(t, "[again]", a.String())
}

func TestArrayCloneIsDeep(t *testing.T) {
	a := ds.NewDynamicArray()
	a.PushBack("a")
	c := a.Clone()
	require.NoError(t, c.Set(0, "changed"))
	c.PushBack("b")
	assert.Equal(t, []string{"a"}, a.ToSlice())
	assert.Equal(t, 2, c.Size())
}

var specialStrings = []string{"plain", "", "with space", "line\nbreak\r\n", "back\\slash", "Привет", "null"}

func TestArrayRoundTrip(t *testing.T) {
	a := ds.NewDynamicArray()
	for _, s := range specialStrings {
		a.PushBack(s)
	}
	fromBin, fromText := ds.NewDynamicArray(), ds.NewDynamicArray()
	roundTrip(t, a, fromBin, fromText)
	assert.Equal(t, a.ToSlice(), fromBin.ToSlice())
	assert.Equal(t, a.ToSlice(), fromText.ToSlice())
}

func TestArrayFormats(t *testing.T) {
	a := ds.NewDynamicArray()
	a.PushBack("ab")
	assert.Equal(t, "DSB1\x01\x01\x00\x00\x00\x02\x00\x00\x00ab", string(binaryOf(t, a)))
	b := ds.NewDynamicArray()
	b.PushBack("a b")
	b.PushBack("x\ny")
	assert.Equal(t, "DSTXT 1\nTYPE array\nCOUNT 2\na b\nx\\ny\n", textOf(t, b))
}

func TestArrayLoadFailureKeepsContent(t *testing.T) {
	src := ds.NewDynamicArray()
	src.PushBack("hello")
	data := binaryOf(t, src)

	target := ds.NewDynamicArray()
	target.PushBack("keep")
	assert.ErrorIs(t, loadBinary(target, data[:len(data)-2]), ds.ErrFormat)
	assert.Equal(t, "[keep]", target.String())
	assert.ErrorIs(t, loadText(target, "DSTXT 1\nTYPE array\nCOUNT 3\nonly one\n"), ds.ErrFormat)
	assert.Equal(t, "[keep]", target.String())
}
