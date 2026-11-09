# Go-часть ЛР3. Перед первым запуском: go mod tidy
.PHONY: test cover bench lint run

test:
	go test ./...

# HTML-отчёт о покрытии: coverage.html (порог 85%)
cover:
	go test ./... -coverpkg=./ds,./cli -coverprofile=coverage.out -covermode=atomic
	go tool cover -html=coverage.out -o coverage.html
	@go tool cover -func=coverage.out | tail -1
	@go tool cover -func=coverage.out | awk '/^total:/ {sub("%","",$$3); if ($$3+0 < 85) {print "Покрытие ниже 85%"; exit 1}}'

bench:
	go test ./ds -run '^$$' -bench . -benchmem

lint:
	gofmt -l .
	go vet ./...
	golangci-lint run

run:
	go run ./cmd/lab3
