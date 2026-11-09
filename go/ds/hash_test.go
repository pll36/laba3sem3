package ds_test

import (
	"math"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"lab3/ds"
)

func newTable(t *testing.T, mode ds.Mode, capacity int) *ds.HashTable {
	t.Helper()
	h, err := ds.NewHashTable(mode, capacity)
	require.NoError(t, err)
	return h
}

func TestFoldingHash(t *testing.T) {
	assert.Equal(t, 1774, ds.FoldingHash(523456795, 100000)) // 523 + 456 + 795
	assert.Equal(t, 774, ds.FoldingHash(523456795, 1000))
	assert.Equal(t, 0, ds.FoldingHash(0, 11))
	assert.Equal(t, 5, ds.FoldingHash(5, 11))
	assert.Equal(t, 235, ds.FoldingHash(1234, 1000)) // 1 + 234
	assert.Equal(t, 235, ds.FoldingHash(-1234, 1000))
	assert.Equal(t, (2+147+483+648)%1000, ds.FoldingHash(math.MinInt32, 1000))
	assert.Equal(t, 0, ds.FoldingHash(42, 0))
}

func TestHashNewTable(t *testing.T) {
	h := newTable(t, ds.ModeLinear, ds.DefaultCapacity)
	assert.Equal(t, 0, h.Size())
	assert.Equal(t, ds.DefaultCapacity, h.Capacity())
	assert.Equal(t, ds.ModeLinear, h.Mode())
	assert.Equal(t, 0.0, h.LoadFactor())
	assert.Equal(t, ds.TypeHash, h.Type())
	_, err := ds.NewHashTable(ds.ModeLinear, 0)
	assert.Error(t, err)
	assert.Equal(t, "linear", ds.ModeLinear.String())
	assert.Equal(t, "double", ds.ModeDouble.String())
}

func TestHashInsertContainsErase(t *testing.T) {
	for _, mode := range []ds.Mode{ds.ModeLinear, ds.ModeDouble} {
		h := newTable(t, mode, 13)
		assert.True(t, h.Insert(15))
		assert.True(t, h.Insert(28)) // 28 % 13 == 2 == 15 % 13 -> коллизия
		assert.True(t, h.Contains(15))
		assert.True(t, h.Contains(28))
		assert.False(t, h.Contains(41))
		assert.True(t, h.Erase(15))
		assert.False(t, h.Contains(15))
		assert.True(t, h.Contains(28)) // поиск проходит через удалённую ячейку
		assert.False(t, h.Erase(15))
		assert.Equal(t, 1, h.Size())
	}
}

func TestHashDuplicateKeyDoesNotChangeSize(t *testing.T) {
	h := newTable(t, ds.ModeLinear, 11)
	assert.True(t, h.Insert(5))
	assert.True(t, h.Insert(5))
	assert.Equal(t, 1, h.Size())
}

func TestHashDeletedSlotsAreReused(t *testing.T) {
	h := newTable(t, ds.ModeLinear, 5)
	for _, k := range []int32{1, 2, 3, 4, 0} {
		require.True(t, h.Insert(k))
	}
	require.True(t, h.Erase(3))
	assert.True(t, h.Insert(8)) // свободна только удалённая ячейка (8 % 5 == 3)
	assert.True(t, h.Contains(8))
	assert.Equal(t, 5, h.Size())

	g := newTable(t, ds.ModeLinear, 7)
	g.Insert(1)
	g.Insert(8)
	g.Insert(15)
	g.Erase(8)
	assert.True(t, g.Insert(22)) // проход через удалённую ячейку 2 до пустой 4: занимаем 2
	assert.True(t, g.Contains(22) && g.Contains(15) && g.Contains(1))
}

func TestHashFullTable(t *testing.T) {
	for _, mode := range []ds.Mode{ds.ModeLinear, ds.ModeDouble} {
		h := newTable(t, mode, 4)
		for k := int32(0); k < 4; k++ {
			require.True(t, h.Insert(k))
		}
		assert.Equal(t, 1.0, h.LoadFactor())
		assert.False(t, h.Insert(100))
		assert.False(t, h.Contains(100))
		assert.False(t, h.Erase(100))
		assert.True(t, h.Contains(3))
	}
}

func TestHashTinyTables(t *testing.T) {
	for _, capacity := range []int{1, 2} {
		h := newTable(t, ds.ModeDouble, capacity)
		assert.True(t, h.Insert(10))
		assert.True(t, h.Contains(10))
		assert.True(t, h.Erase(10))
	}
}

func TestHashManyCollisions(t *testing.T) {
	for _, mode := range []ds.Mode{ds.ModeLinear, ds.ModeDouble} {
		h := newTable(t, mode, 101)
		for i := int32(0); i < 90; i++ {
			require.True(t, h.Insert(i*1000+7))
		}
		for i := int32(0); i < 90; i++ {
			require.True(t, h.Contains(i*1000+7))
		}
		assert.Equal(t, 90, h.Size())
	}
}

func TestHashProbeCount(t *testing.T) {
	linear := newTable(t, ds.ModeLinear, 101)
	double := newTable(t, ds.ModeDouble, 101)
	// ключи a*1000 + (500-a) сворачиваются в одно значение 500 -> все коллизируют в одной ячейке
	key := func(a int32) int32 { return a*1000 + (500 - a) }
	for a := int32(0); a < 60; a++ {
		require.Equal(t, ds.FoldingHash(key(0), 101), ds.FoldingHash(key(a), 101))
		linear.Insert(key(a))
		double.Insert(key(a))
	}
	var lp, dp int
	for a := int32(0); a < 60; a++ {
		lp += linear.ProbeCount(key(a))
		dp += double.ProbeCount(key(a))
	}
	assert.Less(t, dp, lp, "двойное хеширование должно давать меньше проб на кластерных ключах")
	assert.GreaterOrEqual(t, linear.ProbeCount(-12345), 1)
}

func TestHashClearCloneAndString(t *testing.T) {
	h := newTable(t, ds.ModeLinear, 11)
	for _, k := range []int32{3, 5, 9} {
		h.Insert(k)
	}
	h.Erase(5)
	assert.Equal(t, "Hash(linear, capacity=11, size=2)\n[3] 3\n[9] 9", h.String())
	c := h.Clone()
	c.Insert(99)
	assert.Equal(t, 2, h.Size())
	assert.Equal(t, 3, c.Size())
	h.Clear()
	assert.Equal(t, 0, h.Size())
	assert.Equal(t, 11, h.Capacity())
	assert.Equal(t, "Hash(double, capacity=7, size=0)", newTable(t, ds.ModeDouble, 7).String())
}

func TestHashRoundTripKeepsDeletedSlots(t *testing.T) {
	h := newTable(t, ds.ModeDouble, 11)
	for _, k := range []int32{1, 12, 23, -7, 523456795} {
		h.Insert(k)
	}
	h.Erase(12)
	fromBin, fromText := newTable(t, ds.ModeLinear, 3), newTable(t, ds.ModeLinear, 3)
	roundTrip(t, h, fromBin, fromText)
	for _, restored := range []*ds.HashTable{fromBin, fromText} {
		assert.Equal(t, h.String(), restored.String())
		assert.Equal(t, ds.ModeDouble, restored.Mode())
		assert.Equal(t, h.Size(), restored.Size())
		assert.True(t, restored.Contains(23))
		assert.False(t, restored.Contains(12))
	}
}

func TestHashTextFormat(t *testing.T) {
	h := newTable(t, ds.ModeLinear, 3)
	h.Insert(1)
	h.Insert(2)
	h.Erase(2)
	assert.Equal(t, "DSTXT 1\nTYPE hash\nMODE linear\nCAPACITY 3\nE 0\nO 1\nD 2\n", textOf(t, h))
}

func TestHashCorruptedData(t *testing.T) {
	h := newTable(t, ds.ModeLinear, 11)
	h.Insert(77)
	before := h.String()
	for _, body := range []string{
		"MODE triple\nCAPACITY 1\nE 0\n",
		"MODE linear\nCAPACITY 0\n",
		"MODE linear\nCAPACITY 2\nE 0\n",
		"MODE linear\nCAPACITY 1\nX 0\n",
		"MODE linear\nCAPACITY 1\nO\n",
		"MODE linear\nCAPACITY 1\nO x\n",
	} {
		assert.ErrorIs(t, loadText(h, "DSTXT 1\nTYPE hash\n"+body), ds.ErrFormat, body)
	}
	for name, bytes := range map[string]string{
		"mode 5":     "\x05\x01\x00\x00\x00",
		"capacity 0": "\x00\x00\x00\x00\x00",
		"state 9":    "\x00\x01\x00\x00\x00\x09\x00\x00\x00\x00",
		"truncated":  "\x00\x02\x00\x00\x00\x01",
	} {
		assert.ErrorIs(t, loadBinary(h, []byte("DSB1\x07"+bytes)), ds.ErrFormat, name)
	}
	assert.Equal(t, before, h.String())
}
