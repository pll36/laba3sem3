package ds_test

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"lab3/ds"
)

func singly(values ...string) *ds.SinglyLinkedList {
	l := &ds.SinglyLinkedList{}
	for _, v := range values {
		l.PushBack(v)
	}
	return l
}

func doubly(values ...string) *ds.DoublyLinkedList {
	l := &ds.DoublyLinkedList{}
	for _, v := range values {
		l.PushBack(v)
	}
	return l
}

// ---------------------------------------------------------------- односвязный список

func TestSinglyBasics(t *testing.T) {
	l := &ds.SinglyLinkedList{}
	assert.Equal(t, "(empty)", l.String())
	assert.Equal(t, ds.TypeSinglyList, l.Type())
	l.PushBack("b")
	l.PushFront("a")
	l.PushBack("c")
	assert.Equal(t, []string{"a", "b", "c"}, l.ToSlice())
	assert.Equal(t, "a -> b -> c", l.String())
	assert.Equal(t, 3, l.Size())
}

func TestSinglyPushBeforeAfter(t *testing.T) {
	l := singly("a", "c")
	assert.True(t, l.PushBefore("c", "b"))
	assert.True(t, l.PushBefore("a", "start"))
	assert.False(t, l.PushBefore("zzz", "x"))
	assert.True(t, l.PushAfter("c", "end"))
	assert.False(t, l.PushAfter("zzz", "x"))
	assert.Equal(t, []string{"start", "a", "b", "c", "end"}, l.ToSlice())
	assert.Equal(t, 5, l.Size())
}

func TestSinglyPop(t *testing.T) {
	l := singly("a", "b", "c")
	require.NoError(t, l.PopFront())
	require.NoError(t, l.PopBack())
	assert.Equal(t, []string{"b"}, l.ToSlice())
	require.NoError(t, l.PopBack())
	assert.ErrorIs(t, l.PopFront(), ds.ErrListEmpty)
	assert.ErrorIs(t, l.PopBack(), ds.ErrListEmpty)
}

func TestSinglyPopBeforeAfter(t *testing.T) {
	l := singly("a", "b", "c", "d")
	assert.True(t, l.PopBefore("c"))
	assert.Equal(t, []string{"a", "c", "d"}, l.ToSlice())
	assert.True(t, l.PopBefore("c")) // удаляется голова
	assert.Equal(t, []string{"c", "d"}, l.ToSlice())
	assert.False(t, l.PopBefore("c"))
	assert.False(t, l.PopBefore("zzz"))
	assert.True(t, l.PopAfter("c"))
	assert.False(t, l.PopAfter("c"))
	assert.False(t, l.PopAfter("zzz"))
	assert.Equal(t, []string{"c"}, l.ToSlice())
	assert.Equal(t, 1, l.Size())
}

func TestSinglyRemoveAndSearch(t *testing.T) {
	l := singly("a", "b", "c", "b")
	assert.True(t, l.RemoveByValue("a"))
	assert.True(t, l.RemoveByValue("b"))
	assert.True(t, l.RemoveByValue("b"))
	assert.False(t, l.RemoveByValue("b"))
	assert.Equal(t, []string{"c"}, l.ToSlice())

	m := singly("a", "b", "c")
	assert.True(t, m.Contains("b"))
	assert.False(t, m.Contains("zzz"))
	assert.Equal(t, 2, m.IndexOf("c"))
	assert.Equal(t, -1, m.IndexOf("zzz"))
	assert.Equal(t, "b -> c", m.StringFrom("b"))
	assert.Equal(t, "", m.StringFrom("zzz"))
}

func TestSinglyCloneClearAndRoundTrip(t *testing.T) {
	l := singly("a b", "", "multi\nline")
	c := l.Clone()
	c.PushBack("z")
	assert.Equal(t, 3, l.Size())
	assert.Equal(t, 4, c.Size())

	fromBin, fromText := &ds.SinglyLinkedList{}, &ds.SinglyLinkedList{}
	roundTrip(t, l, fromBin, fromText)
	assert.Equal(t, l.ToSlice(), fromBin.ToSlice())
	assert.Equal(t, l.ToSlice(), fromText.ToSlice())

	l.Clear()
	assert.Equal(t, 0, l.Size())
	l.PushBack("x")
	assert.Equal(t, "x", l.String())
}

// ---------------------------------------------------------------- двусвязный список

func TestDoublyBasics(t *testing.T) {
	l := &ds.DoublyLinkedList{}
	assert.Equal(t, "(empty)", l.String())
	assert.Equal(t, "(empty)", l.StringBackward())
	assert.Equal(t, ds.TypeDoublyList, l.Type())
	l.PushBack("b")
	l.PushFront("a")
	l.PushBack("c")
	assert.Equal(t, "a <-> b <-> c", l.String())
	assert.Equal(t, "c <-> b <-> a", l.StringBackward())
	assert.Equal(t, []string{"c", "b", "a"}, l.ToSliceBackward())
}

func TestDoublyPushBeforeAfter(t *testing.T) {
	l := doubly("b", "d")
	assert.True(t, l.PushBefore("b", "a"))
	assert.True(t, l.PushBefore("d", "c"))
	assert.True(t, l.PushAfter("d", "e"))
	assert.True(t, l.PushAfter("a", "a2"))
	assert.False(t, l.PushBefore("zzz", "x"))
	assert.False(t, l.PushAfter("zzz", "x"))
	assert.Equal(t, []string{"a", "a2", "b", "c", "d", "e"}, l.ToSlice())
	assert.Equal(t, []string{"e", "d", "c", "b", "a2", "a"}, l.ToSliceBackward())
	assert.Equal(t, 6, l.Size())
}

func TestDoublyPop(t *testing.T) {
	l := doubly("a", "b", "c")
	require.NoError(t, l.PopFront())
	require.NoError(t, l.PopBack())
	assert.Equal(t, []string{"b"}, l.ToSlice())
	require.NoError(t, l.PopBack())
	assert.ErrorIs(t, l.PopFront(), ds.ErrListEmpty)
	assert.ErrorIs(t, l.PopBack(), ds.ErrListEmpty)
}

func TestDoublyPopBeforeAfterAndRemove(t *testing.T) {
	l := doubly("a", "b", "c", "d")
	assert.True(t, l.PopBefore("c"))
	assert.True(t, l.PopAfter("c"))
	assert.Equal(t, []string{"a", "c"}, l.ToSlice())
	assert.False(t, l.PopBefore("a"))
	assert.False(t, l.PopAfter("c"))
	assert.False(t, l.PopBefore("zzz"))
	assert.False(t, l.PopAfter("zzz"))
	assert.True(t, l.RemoveByValue("a"))
	assert.False(t, l.RemoveByValue("a"))
	assert.Equal(t, []string{"c"}, l.ToSliceBackward())
}

func TestDoublyReadFromElement(t *testing.T) {
	l := doubly("a", "b", "c")
	assert.True(t, l.Contains("b"))
	assert.False(t, l.Contains("zzz"))
	assert.Equal(t, 2, l.IndexOf("c"))
	assert.Equal(t, -1, l.IndexOf("zzz"))
	assert.Equal(t, "b <-> c", l.StringFrom("b"))
	assert.Equal(t, "b <-> a", l.StringFromBackward("b"))
	assert.Equal(t, "", l.StringFrom("zzz"))
	assert.Equal(t, "", l.StringFromBackward("zzz"))
}

func TestDoublyCloneClearAndRoundTrip(t *testing.T) {
	l := doubly("a b", "", "multi\nline")
	c := l.Clone()
	c.PushFront("z")
	assert.Equal(t, 3, l.Size())
	assert.Equal(t, []string{"multi\nline", "", "a b", "z"}, c.ToSliceBackward())

	fromBin, fromText := &ds.DoublyLinkedList{}, &ds.DoublyLinkedList{}
	roundTrip(t, l, fromBin, fromText)
	assert.Equal(t, l.ToSliceBackward(), fromBin.ToSliceBackward())
	assert.Equal(t, l.ToSlice(), fromText.ToSlice())

	l.Clear()
	l.PushFront("x")
	assert.Equal(t, "x", l.StringBackward())
}
