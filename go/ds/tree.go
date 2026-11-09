package ds

import (
	"fmt"
	"io"
	"strconv"
	"strings"
)

type treeNode struct {
	value       int32
	left, right *treeNode
}

// slot - ячейка обхода по уровням: пропуск (ok == false) или значение узла.
type slot struct {
	value int32
	ok    bool
}

// CompleteBinaryTree - полное бинарное дерево (Complete Binary Tree) из чисел.
// Узлы добавляются по уровням слева направо, поэтому дерево, построенное через
// Insert, всегда полное. Десериализация может восстановить произвольную форму,
// поэтому IsComplete - настоящая проверка.
type CompleteBinaryTree struct {
	root *treeNode
	size int
}

// Clone возвращает глубокую копию.
func (t *CompleteBinaryTree) Clone() *CompleteBinaryTree {
	c := &CompleteBinaryTree{}
	_ = c.buildFromSlots(t.toSlots())
	return c
}

// Insert добавляет значение в первое свободное место (обход в ширину), O(N).
func (t *CompleteBinaryTree) Insert(value int32) {
	fresh := &treeNode{value: value}
	t.size++
	if t.root == nil {
		t.root = fresh
		return
	}
	level := []*treeNode{t.root}
	for len(level) > 0 {
		cur := level[0]
		level = level[1:]
		if cur.left == nil {
			cur.left = fresh
			return
		}
		level = append(level, cur.left)
		if cur.right == nil {
			cur.right = fresh
			return
		}
		level = append(level, cur.right)
	}
}

// Contains ищет значение обходом в ширину, O(N).
func (t *CompleteBinaryTree) Contains(value int32) bool {
	for _, v := range t.LevelOrder() {
		if v == value {
			return true
		}
	}
	return false
}

// IsComplete проверяет свойство полноты, O(N).
func (t *CompleteBinaryTree) IsComplete() bool {
	level := []*treeNode{t.root}
	seenGap := false
	for len(level) > 0 {
		cur := level[0]
		level = level[1:]
		if cur == nil {
			seenGap = true
			continue
		}
		if seenGap {
			return false
		}
		level = append(level, cur.left, cur.right)
	}
	return true
}

func heightOf(n *treeNode) int {
	if n == nil {
		return 0
	}
	return 1 + max(heightOf(n.left), heightOf(n.right))
}

// Height - высота дерева (пустое дерево - 0).
func (t *CompleteBinaryTree) Height() int { return heightOf(t.root) }

// LevelOrder возвращает значения узлов в порядке обхода по уровням.
func (t *CompleteBinaryTree) LevelOrder() []int32 {
	values := make([]int32, 0, t.size)
	if t.root == nil {
		return values
	}
	level := []*treeNode{t.root}
	for len(level) > 0 {
		cur := level[0]
		level = level[1:]
		values = append(values, cur.value)
		if cur.left != nil {
			level = append(level, cur.left)
		}
		if cur.right != nil {
			level = append(level, cur.right)
		}
	}
	return values
}

// Type реализует Structure.
func (t *CompleteBinaryTree) Type() StructureType { return TypeTree }

// Size реализует Structure.
func (t *CompleteBinaryTree) Size() int { return t.size }

// Clear реализует Structure.
func (t *CompleteBinaryTree) Clear() { t.root, t.size = nil, 0 }

func drawNode(n *treeNode, prefix string, isLeft bool, sb *strings.Builder) {
	if n == nil {
		return
	}
	branch := "`-- "
	childPrefix := prefix + "    "
	if isLeft {
		branch = "|-- "
		childPrefix = prefix + "|   "
	}
	sb.WriteString(prefix + branch + strconv.Itoa(int(n.value)) + "\n")
	drawNode(n.left, childPrefix, true, sb)
	drawNode(n.right, childPrefix, false, sb)
}

// String реализует Structure: рисунок дерева.
func (t *CompleteBinaryTree) String() string {
	if t.root == nil {
		return emptyText
	}
	var sb strings.Builder
	sb.WriteString(strconv.Itoa(int(t.root.value)) + "\n")
	drawNode(t.root.left, "", true, &sb)
	drawNode(t.root.right, "", false, &sb)
	return strings.TrimSuffix(sb.String(), "\n")
}

// toSlots - обход по уровням с пропусками, хвостовые пропуски отброшены.
func (t *CompleteBinaryTree) toSlots() []slot {
	var slots []slot
	level := []*treeNode{t.root}
	for len(level) > 0 {
		cur := level[0]
		level = level[1:]
		if cur == nil {
			slots = append(slots, slot{})
			continue
		}
		slots = append(slots, slot{cur.value, true})
		level = append(level, cur.left, cur.right)
	}
	for len(slots) > 0 && !slots[len(slots)-1].ok {
		slots = slots[:len(slots)-1]
	}
	return slots
}

// buildFromSlots строит дерево заново; при ошибке текущее содержимое не меняется.
func (t *CompleteBinaryTree) buildFromSlots(slots []slot) error {
	built := &CompleteBinaryTree{}
	if len(slots) > 0 {
		if !slots[0].ok {
			return formatError("tree root is missing")
		}
		built.root = &treeNode{value: slots[0].value}
		built.size = 1
		parents := []*treeNode{built.root}
		next := 1
		for next < len(slots) {
			if len(parents) == 0 {
				return formatError("tree has nodes without a parent")
			}
			parent := parents[0]
			parents = parents[1:]
			for _, child := range []**treeNode{&parent.left, &parent.right} {
				if next >= len(slots) {
					break
				}
				s := slots[next]
				next++
				if s.ok {
					*child = &treeNode{value: s.value}
					built.size++
					parents = append(parents, *child)
				}
			}
		}
	}
	*t = *built
	return nil
}

// SaveBinary реализует Structure.
func (t *CompleteBinaryTree) SaveBinary(w io.Writer) error {
	slots := t.toSlots()
	bw := newBinWriter(TypeTree)
	bw.u32(uint32(len(slots)))
	for _, s := range slots {
		if s.ok {
			bw.u8(1)
			bw.i32(s.value)
		} else {
			bw.u8(0)
		}
	}
	return bw.flush(w)
}

// LoadBinary реализует Structure.
func (t *CompleteBinaryTree) LoadBinary(r io.Reader) error {
	br, err := openBinary(r, TypeTree)
	if err != nil {
		return err
	}
	count, err := br.u32()
	if err != nil {
		return err
	}
	var slots []slot
	for i := uint32(0); i < count; i++ {
		flag, err := br.u8()
		if err != nil {
			return err
		}
		switch flag {
		case 0:
			slots = append(slots, slot{})
		case 1:
			v, err := br.i32()
			if err != nil {
				return err
			}
			slots = append(slots, slot{v, true})
		default:
			return formatError("invalid presence flag")
		}
	}
	return t.buildFromSlots(slots)
}

// SaveText реализует Structure.
func (t *CompleteBinaryTree) SaveText(w io.Writer) error {
	slots := t.toSlots()
	var sb strings.Builder
	sb.WriteString(textHeader(TypeTree))
	fmt.Fprintf(&sb, "COUNT %d\n", len(slots))
	for _, s := range slots {
		if s.ok {
			sb.WriteString(strconv.Itoa(int(s.value)) + "\n")
		} else {
			sb.WriteString("null\n")
		}
	}
	_, err := io.WriteString(w, sb.String())
	return err
}

// LoadText реализует Structure.
func (t *CompleteBinaryTree) LoadText(r io.Reader) error {
	tr, err := openText(r, TypeTree)
	if err != nil {
		return err
	}
	count, err := tr.uint32Field("COUNT")
	if err != nil {
		return err
	}
	var slots []slot
	for i := uint32(0); i < count; i++ {
		line, err := tr.line()
		if err != nil {
			return err
		}
		if line == "null" {
			slots = append(slots, slot{})
			continue
		}
		v, err := parseInt32(line)
		if err != nil {
			return err
		}
		slots = append(slots, slot{v, true})
	}
	return t.buildFromSlots(slots)
}
