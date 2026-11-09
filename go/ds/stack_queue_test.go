package ds_test

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"lab3/ds"
)

func TestStack(t *testing.T) {
	s := &ds.Stack{}
	assert.Equal(t, "(empty)", s.String())
	assert.Equal(t, ds.TypeStack, s.Type())
	_, err := s.Pop()
	assert.ErrorIs(t, err, ds.ErrStackEmpty)
	_, err = s.Peek()
	assert.ErrorIs(t, err, ds.ErrStackEmpty)

	s.Push("1")
	s.Push("2")
	s.Push("3")
	top, _ := s.Peek()
	assert.Equal(t, "3", top)
	assert.Equal(t, "Stack(top->bottom): 3, 2, 1", s.String())
	assert.Equal(t, []string{"1", "2", "3"}, s.ToSlice())
	for _, want := range []string{"3", "2", "1"} {
		got, err := s.Pop()
		require.NoError(t, err)
		assert.Equal(t, want, got)
	}
	assert.Equal(t, 0, s.Size())
}

func TestStackCloneClearAndRoundTrip(t *testing.T) {
	s := &ds.Stack{}
	s.Push("bottom")
	s.Push("with space")
	s.Push("top\nline")
	c := s.Clone()
	c.Push("extra")
	assert.Equal(t, 3, s.Size())

	fromBin, fromText := &ds.Stack{}, &ds.Stack{}
	roundTrip(t, s, fromBin, fromText)
	for _, restored := range []*ds.Stack{fromBin, fromText} {
		assert.Equal(t, s.ToSlice(), restored.ToSlice())
		top, _ := restored.Pop()
		assert.Equal(t, "top\nline", top)
	}
	s.Clear()
	assert.Equal(t, 0, s.Size())
}

func TestQueue(t *testing.T) {
	q := &ds.Queue{}
	assert.Equal(t, "(empty)", q.String())
	assert.Equal(t, ds.TypeQueue, q.Type())
	_, err := q.Pop()
	assert.ErrorIs(t, err, ds.ErrQueueEmpty)
	_, err = q.Front()
	assert.ErrorIs(t, err, ds.ErrQueueEmpty)

	q.Push("1")
	q.Push("2")
	q.Push("3")
	front, _ := q.Front()
	assert.Equal(t, "1", front)
	assert.Equal(t, "Queue(front->rear): 1, 2, 3", q.String())
	for _, want := range []string{"1", "2", "3"} {
		got, err := q.Pop()
		require.NoError(t, err)
		assert.Equal(t, want, got)
	}
	assert.Equal(t, 0, q.Size())
}

func TestQueueUsableAfterBecomingEmpty(t *testing.T) {
	q := &ds.Queue{}
	q.Push("a")
	_, _ = q.Pop()
	q.Push("b") // rear должен быть сброшен при опустошении
	assert.Equal(t, []string{"b"}, q.ToSlice())
}

func TestQueueCloneClearAndRoundTrip(t *testing.T) {
	q := &ds.Queue{}
	q.Push("first")
	q.Push("sec ond")
	q.Push("")
	c := q.Clone()
	c.Push("x")
	assert.Equal(t, 3, q.Size())

	fromBin, fromText := &ds.Queue{}, &ds.Queue{}
	roundTrip(t, q, fromBin, fromText)
	assert.Equal(t, q.ToSlice(), fromBin.ToSlice())
	assert.Equal(t, q.ToSlice(), fromText.ToSlice())
	q.Clear()
	assert.Equal(t, 0, q.Size())
}
