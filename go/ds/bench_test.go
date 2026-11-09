package ds_test

import (
	"bytes"
	"fmt"
	"testing"

	"lab3/ds"
)

// Запуск: go test ./ds -bench . -benchmem

func BenchmarkArrayPushBack(b *testing.B) {
	for _, n := range []int{64, 1024, 16384} {
		b.Run(fmt.Sprint(n), func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				a := ds.NewDynamicArray()
				for j := 0; j < n; j++ {
					a.PushBack("value")
				}
			}
		})
	}
}

func BenchmarkArrayPushFront(b *testing.B) {
	for _, n := range []int{64, 1024, 4096} {
		b.Run(fmt.Sprint(n), func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				a := ds.NewDynamicArray()
				for j := 0; j < n; j++ {
					a.PushFront("value")
				}
			}
		})
	}
}

// Односвязный список: PushBack - O(N), PushFront - O(1); двусвязный PushBack - O(1).
func BenchmarkSinglyListPushBack(b *testing.B) {
	for _, n := range []int{64, 256, 1024, 4096} {
		b.Run(fmt.Sprint(n), func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				l := &ds.SinglyLinkedList{}
				for j := 0; j < n; j++ {
					l.PushBack("value")
				}
			}
		})
	}
}

func BenchmarkSinglyListPushFront(b *testing.B) {
	for _, n := range []int{64, 1024, 16384} {
		b.Run(fmt.Sprint(n), func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				l := &ds.SinglyLinkedList{}
				for j := 0; j < n; j++ {
					l.PushFront("value")
				}
			}
		})
	}
}

func BenchmarkDoublyListPushBack(b *testing.B) {
	for _, n := range []int{64, 1024, 16384} {
		b.Run(fmt.Sprint(n), func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				l := &ds.DoublyLinkedList{}
				for j := 0; j < n; j++ {
					l.PushBack("value")
				}
			}
		})
	}
}

func BenchmarkStackPushPop(b *testing.B) {
	for i := 0; i < b.N; i++ {
		s := &ds.Stack{}
		for j := 0; j < 1024; j++ {
			s.Push("value")
		}
		for s.Size() > 0 {
			_, _ = s.Pop()
		}
	}
}

func BenchmarkQueuePushPop(b *testing.B) {
	for i := 0; i < b.N; i++ {
		q := &ds.Queue{}
		for j := 0; j < 1024; j++ {
			q.Push("value")
		}
		for q.Size() > 0 {
			_, _ = q.Pop()
		}
	}
}

// Вставка и поиск в полном бинарном дереве - O(N) (обход в ширину).
func BenchmarkTreeInsert(b *testing.B) {
	for _, n := range []int{64, 256, 1024} {
		b.Run(fmt.Sprint(n), func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				t := &ds.CompleteBinaryTree{}
				for j := 0; j < n; j++ {
					t.Insert(int32(j))
				}
			}
		})
	}
}

func BenchmarkTreeContains(b *testing.B) {
	t := &ds.CompleteBinaryTree{}
	for j := 0; j < 4096; j++ {
		t.Insert(int32(j))
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		t.Contains(-1) // худший случай: полный обход
	}
}

// Хеш-таблица: линейное пробирование против двойного хеширования (ёмкость 10007).
func BenchmarkHashFill(b *testing.B) {
	for _, mode := range []ds.Mode{ds.ModeLinear, ds.ModeDouble} {
		for _, percent := range []int{30, 60, 90} {
			b.Run(fmt.Sprintf("%s/%d%%", mode, percent), func(b *testing.B) {
				keys := 10007 * percent / 100
				for i := 0; i < b.N; i++ {
					h, _ := ds.NewHashTable(mode, 10007)
					for k := 0; k < keys; k++ {
						h.Insert(int32(k*1000 + 7))
					}
				}
			})
		}
	}
}

// Сериализация: бинарный формат против текстового.
func benchArray(n int) *ds.DynamicArray {
	a := ds.NewDynamicArray()
	for i := 0; i < n; i++ {
		a.PushBack(fmt.Sprintf("element number %d", i))
	}
	return a
}

func BenchmarkArraySave(b *testing.B) {
	a := benchArray(10000)
	b.Run("binary", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = a.SaveBinary(&bytes.Buffer{})
		}
	})
	b.Run("text", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = a.SaveText(&bytes.Buffer{})
		}
	})
}

func BenchmarkArrayLoad(b *testing.B) {
	a := benchArray(10000)
	var bin, txt bytes.Buffer
	_ = a.SaveBinary(&bin)
	_ = a.SaveText(&txt)
	b.Run("binary", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = ds.NewDynamicArray().LoadBinary(bytes.NewReader(bin.Bytes()))
		}
	})
	b.Run("text", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = ds.NewDynamicArray().LoadText(bytes.NewReader(txt.Bytes()))
		}
	})
}
