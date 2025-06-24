# Base64 Encoder/Decoder

A complete C++ implementation of Base64 encoding and decoding functionality.

## Features

- **Efficient Base64 encoding** - Convert any string to Base64 format
- **Robust Base64 decoding** - Convert Base64 strings back to original text
- **Comprehensive testing** - Built-in test suite with multiple test cases
- **Error handling** - Proper validation and error reporting
- **Cross-platform** - Standard C++98 compatible code

## Building

### Using Make
```bash
make all          # Build the program
make test         # Build and run all tests
make example      # Build and run example usage
make clean        # Remove built files
make help         # Show available targets
```

### Manual Compilation
```bash
g++ -Wall -Wextra -std=c++98 -O2 -o base64 base64.cpp
```

## Usage

### Command Line Interface

```bash
# Encode a string to Base64
./base64 encode "Hello, World!"
# Output: Encoded: SGVsbG8sIFdvcmxkIQ==

# Decode a Base64 string
./base64 decode "SGVsbG8sIFdvcmxkIQ=="
# Output: Decoded: Hello, World!

# Run comprehensive tests
./base64 test
# Output: Test results with pass/fail status
```

### Programmatic Usage

The `Base64` class can be used directly in your C++ code:

```cpp
#include "base64.cpp"  // or extract the class to a header file

int main() {
    // Encoding
    std::string original = "Hello, World!";
    std::string encoded = Base64::encode(original);
    std::cout << "Encoded: " << encoded << std::endl;

    // Decoding
    std::string decoded = Base64::decode(encoded);
    std::cout << "Decoded: " << decoded << std::endl;

    return 0;
}
```

## Algorithm Details

### Base64 Encoding Process

1. **Input Processing**: Take groups of 3 bytes (24 bits) from input
2. **Bit Manipulation**: Split 24 bits into 4 groups of 6 bits each
3. **Character Mapping**: Map each 6-bit value to Base64 character set
4. **Padding**: Add '=' padding characters to make output length multiple of 4

### Base64 Decoding Process

1. **Validation**: Ensure input length is multiple of 4 and contains valid characters
2. **Character Lookup**: Convert Base64 characters back to 6-bit values
3. **Bit Reconstruction**: Combine 4 groups of 6 bits into 3 bytes
4. **Padding Handling**: Strip padding and reconstruct original data

### Character Set

```
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
```

## Test Cases

The program includes comprehensive test cases covering:

- Empty strings
- Single characters
- Short strings
- Long strings
- Special characters
- Binary data compatibility

Run `./base64 test` to see all test results.

## Error Handling

The program handles various error conditions:

- **Invalid Base64 characters**: Non-standard characters in input
- **Invalid length**: Input not properly padded (length not multiple of 4)
- **Malformed data**: Corrupted Base64 strings

## Performance Characteristics

- **Time Complexity**: O(n) where n is input length
- **Space Complexity**: O(n) for output storage
- **Memory Efficient**: Minimal temporary storage, single-pass processing
- **No External Dependencies**: Pure C++ standard library implementation

## Technical Implementation

### Key Features

- **Bit manipulation optimization** for efficient encoding/decoding
- **Exception safety** with proper error handling
- **Standard compliance** following RFC 4648 Base64 specification
- **Clean class design** with static methods for easy usage

### Compatibility

- **C++ Standard**: C++98 and later
- **Compilers**: GCC, Clang, MSVC
- **Platforms**: Linux, macOS, Windows
- **Dependencies**: Standard C++ library only

## Examples

### Basic Usage Examples

```bash
# Encode simple text
./base64 encode "foo"
# Output: Encoded: Zm9v

# Encode with special characters
./base64 encode "Hello, 世界!"
# Output: Encoded: SGVsbG8sIOS4lueVjCE=

# Decode standard Base64
./base64 decode "Zm9vYmFy"
# Output: Decoded: foobar
```

### Testing Binary Data

The encoder handles binary data correctly and can encode/decode any byte sequence.

## License

This implementation is provided as an example and can be freely used and modified.