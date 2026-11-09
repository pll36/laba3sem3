package ds_test

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/require"

	"lab3/ds"
)

func binaryOf(t *testing.T, s ds.Structure) []byte {
	t.Helper()
	var buf bytes.Buffer
	require.NoError(t, s.SaveBinary(&buf))
	return buf.Bytes()
}

func textOf(t *testing.T, s ds.Structure) string {
	t.Helper()
	var buf bytes.Buffer
	require.NoError(t, s.SaveText(&buf))
	return buf.String()
}

// roundTrip сохраняет src в обоих форматах и загружает результат в новые объекты dstBin и dstText.
func roundTrip(t *testing.T, src, dstBin, dstText ds.Structure) {
	t.Helper()
	require.NoError(t, dstBin.LoadBinary(bytes.NewReader(binaryOf(t, src))))
	require.NoError(t, dstText.LoadText(bytes.NewReader([]byte(textOf(t, src)))))
}

func loadText(s ds.Structure, text string) error { return s.LoadText(bytes.NewReader([]byte(text))) }

func loadBinary(s ds.Structure, data []byte) error { return s.LoadBinary(bytes.NewReader(data)) }
