package ds_test

import (
	"bytes"
	"fmt"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"lab3/ds"
)

func makeTree(values ...int32) *ds.CompleteBinaryTree {
	t := &ds.CompleteBinaryTree{}
	for _, v := range values {
		t.Insert(v)
	}
	return t
}

// treeFromSlots строит дерево произвольной формы из текстового формата (null - пропуск).
func treeFromSlots(slots ...string) (*ds.CompleteBinaryTree, error) {
	text := fmt.Sprintf("DSTXT 1\nTYPE tree\nCOUNT %d\n%s\n", len(slots), strings.Join(slots, "\n"))
	t := &ds.CompleteBinaryTree{}
	return t, t.LoadText(bytes.NewReader([]byte(text)))
}

func TestTreeEmpty(t *testing.T) {
	tr := &ds.CompleteBinaryTree{}
	assert.True(t, tr.IsComplete())
	assert.False(t, tr.Contains(1))
	assert.Equal(t, 0, tr.Height())
	assert.Equal(t, "(empty)", tr.String())
	assert.Equal(t, ds.TypeTree, tr.Type())
	assert.Empty(t, tr.LevelOrder())
}

func TestTreeInsertFillsLevels(t *testing.T) {
	tr := makeTree(1, 2, 3, 4, 5, 6)
	assert.Equal(t, []int32{1, 2, 3, 4, 5, 6}, tr.LevelOrder())
	assert.Equal(t, 6, tr.Size())
	assert.Equal(t, 3, tr.Height())
}

func TestTreeBuiltByInsertIsComplete(t *testing.T) {
	tr := &ds.CompleteBinaryTree{}
	for i := int32(1); i <= 40; i++ {
		tr.Insert(i)
		require.True(t, tr.IsComplete(), "after inserting %d", i)
	}
	assert.Equal(t, 6, tr.Height())
}

func TestTreeContainsAndDuplicates(t *testing.T) {
	tr := makeTree(10, 20, 30, 40, 40)
	assert.True(t, tr.Contains(10))
	assert.True(t, tr.Contains(40))
	assert.False(t, tr.Contains(99))
	assert.Equal(t, 5, tr.Size())
}

func TestTreeDrawing(t *testing.T) {
	assert.Equal(t, "1", makeTree(1).String())
	assert.Equal(t, "1\n|-- 2\n|   |-- 4\n`-- 3", makeTree(1, 2, 3, 4).String())
}

func TestTreeCloneAndClear(t *testing.T) {
	tr := makeTree(1, 2, 3)
	c := tr.Clone()
	c.Insert(4)
	assert.Equal(t, 3, tr.Size())
	assert.Equal(t, []int32{1, 2, 3, 4}, c.LevelOrder())
	tr.Clear()
	assert.Equal(t, 0, tr.Size())
	tr.Insert(5)
	assert.Equal(t, []int32{5}, tr.LevelOrder())
}

func TestTreeDetectsNonCompleteShapes(t *testing.T) {
	for slots, want := range map[string]bool{
		"1,null,2":          false, // правый ребёнок без левого
		"1,2,3,null,null,4": false, // пропуск посередине уровня
		"1,2,3,4":           true,
		"1,2,3,4,5,6,7":     true,
		"1,2,3,4,null,6":    false,
		"1,2,null":          true,
	} {
		tr, err := treeFromSlots(strings.Split(slots, ",")...)
		require.NoError(t, err, slots)
		assert.Equal(t, want, tr.IsComplete(), slots)
	}
}

func TestTreeRoundTrip(t *testing.T) {
	tr := makeTree(5, -3, 0, 2147483647, -2147483648)
	fromBin, fromText := &ds.CompleteBinaryTree{}, &ds.CompleteBinaryTree{}
	roundTrip(t, tr, fromBin, fromText)
	assert.Equal(t, tr.LevelOrder(), fromBin.LevelOrder())
	assert.Equal(t, tr.String(), fromText.String())

	empty := &ds.CompleteBinaryTree{}
	roundTrip(t, empty, fromBin, fromText)
	assert.Equal(t, 0, fromBin.Size())
	assert.Equal(t, 0, fromText.Size())
}

func TestTreeRoundTripKeepsArbitraryShape(t *testing.T) {
	tr, err := treeFromSlots("1", "null", "2", "3")
	require.NoError(t, err)
	assert.False(t, tr.IsComplete())
	assert.Equal(t, 3, tr.Size())
	restored := &ds.CompleteBinaryTree{}
	require.NoError(t, loadBinary(restored, binaryOf(t, tr)))
	assert.Equal(t, tr.String(), restored.String())
	assert.False(t, restored.IsComplete())
}

func TestTreeTextFormat(t *testing.T) {
	assert.Equal(t, "DSTXT 1\nTYPE tree\nCOUNT 2\n1\n2\n", textOf(t, makeTree(1, 2)))
	tr, _ := treeFromSlots("1", "null", "2")
	assert.Equal(t, "DSTXT 1\nTYPE tree\nCOUNT 3\n1\nnull\n2\n", textOf(t, tr))
}

func TestTreeMalformedInput(t *testing.T) {
	_, err := treeFromSlots("null", "1")
	assert.ErrorIs(t, err, ds.ErrFormat)
	_, err = treeFromSlots("1", "null", "null", "5")
	assert.ErrorIs(t, err, ds.ErrFormat)
	_, err = treeFromSlots("1", "abc")
	assert.ErrorIs(t, err, ds.ErrFormat)

	tr := makeTree(42)
	assert.ErrorIs(t, loadText(tr, "DSTXT 1\nTYPE tree\nCOUNT 3\n1\n"), ds.ErrFormat)
	data := binaryOf(t, makeTree(1, 2, 3))
	assert.ErrorIs(t, loadBinary(tr, data[:len(data)-2]), ds.ErrFormat)
	assert.ErrorIs(t, loadBinary(tr, data[:len(data)-4]), ds.ErrFormat)
	badFlag := append([]byte("DSB1\x06\x01\x00\x00\x00"), 7)
	assert.ErrorIs(t, loadBinary(tr, badFlag), ds.ErrFormat)
	assert.Equal(t, []int32{42}, tr.LevelOrder()) // содержимое не изменилось
}
