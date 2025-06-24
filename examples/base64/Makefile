CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++98 -O2
TARGET = base64
BINARY_TEST = binary_test
SOURCE = base64.cpp
BINARY_SOURCE = binary_test.cpp

.PHONY: all clean test run-tests binary-test

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

$(BINARY_TEST): $(BINARY_SOURCE) base64.h
	$(CXX) $(CXXFLAGS) -o $(BINARY_TEST) $(BINARY_SOURCE)

test: $(TARGET)
	./$(TARGET) test

binary-test: $(BINARY_TEST)
	./$(BINARY_TEST)

run-tests: test binary-test

clean:
	rm -f $(TARGET) $(BINARY_TEST) test_binary.dat

# Usage examples
example: $(TARGET)
	@echo "=== Base64 Encoding Examples ==="
	@echo "Encoding 'Hello, World!':"
	@./$(TARGET) encode "Hello, World!"
	@echo ""
	@echo "Encoding 'foo':"
	@./$(TARGET) encode "foo"
	@echo ""
	@echo "=== Base64 Decoding Examples ==="
	@echo "Decoding 'SGVsbG8sIFdvcmxkIQ==':"
	@./$(TARGET) decode "SGVsbG8sIFdvcmxkIQ=="
	@echo ""
	@echo "Decoding 'Zm9v':"
	@./$(TARGET) decode "Zm9v"

help:
	@echo "Available targets:"
	@echo "  all      - Build the base64 program"
	@echo "  test     - Run all test cases"
	@echo "  example  - Run example encodings and decodings"
	@echo "  clean    - Remove built files"
	@echo "  help     - Show this help message"