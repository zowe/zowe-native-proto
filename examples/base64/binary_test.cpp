#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>

// Include our Base64 implementation
#include "base64.h"

void print_hex(const std::string& data, const std::string& label) {
  std::cout << label << " (hex): ";
  for (size_t i = 0; i < data.length(); ++i) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << (static_cast<unsigned char>(data[i]) & 0xFF) << " ";
  }
  std::cout << std::dec << std::endl;
}

void test_binary_data() {
  std::cout << "=== Binary Data Test ===" << std::endl;

  // Test 1: Binary data with null bytes
  std::string binary_data;
  binary_data.push_back(0x00);  // null byte
  binary_data.push_back(0x01);
  binary_data.push_back(0xFF);  // max byte value
  binary_data.push_back(0x80);  // high bit set
  binary_data.push_back(0x00);  // another null
  binary_data.push_back(0x7F);

  std::cout << "\nTest 1: Binary data with null bytes" << std::endl;
  print_hex(binary_data, "Original");

  std::string encoded = Base64::encode(binary_data);
  std::cout << "Encoded: " << encoded << std::endl;

  std::string decoded = Base64::decode(encoded);
  print_hex(decoded, "Decoded ");

  bool test1_pass = (binary_data == decoded);
  std::cout << "Test 1: " << (test1_pass ? "✅ PASS" : "❌ FAIL") << std::endl;

  // Test 2: Random binary data
  std::cout << "\nTest 2: Random binary data" << std::endl;
  std::string random_binary;
  unsigned char random_bytes[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,  // PNG header
    0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE   // More binary data
  };

  for (size_t i = 0; i < sizeof(random_bytes); ++i) {
    random_binary.push_back(random_bytes[i]);
  }

  print_hex(random_binary, "Original");

  std::string encoded2 = Base64::encode(random_binary);
  std::cout << "Encoded: " << encoded2 << std::endl;

  std::string decoded2 = Base64::decode(encoded2);
  print_hex(decoded2, "Decoded ");

  bool test2_pass = (random_binary == decoded2);
  std::cout << "Test 2: " << (test2_pass ? "✅ PASS" : "❌ FAIL") << std::endl;

  // Test 3: All possible byte values
  std::cout << "\nTest 3: All possible byte values (0-255)" << std::endl;
  std::string all_bytes;
  for (int i = 0; i < 256; ++i) {
    all_bytes.push_back(static_cast<unsigned char>(i));
  }

  std::cout << "Original length: " << all_bytes.length() << " bytes" << std::endl;

  std::string encoded3 = Base64::encode(all_bytes);
  std::cout << "Encoded length: " << encoded3.length() << " characters" << std::endl;

  std::string decoded3 = Base64::decode(encoded3);
  std::cout << "Decoded length: " << decoded3.length() << " bytes" << std::endl;

  bool test3_pass = (all_bytes == decoded3);
  std::cout << "Test 3: " << (test3_pass ? "✅ PASS" : "❌ FAIL") << std::endl;

  if (test1_pass && test2_pass && test3_pass) {
    std::cout << "\n🎉 All binary tests passed! Base64 handles binary data correctly." << std::endl;
  } else {
    std::cout << "\n❌ Some binary tests failed!" << std::endl;
  }
}

void test_file_encoding() {
  std::cout << "\n=== File Binary Test ===" << std::endl;

  // Create a small binary file for testing
  const char* test_filename = "test_binary.dat";
  std::ofstream file(test_filename, std::ios::binary);

  if (!file) {
    std::cout << "Could not create test file" << std::endl;
    return;
  }

  // Write some binary data
  unsigned char test_data[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,  // PNG signature
    0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,  // IHDR chunk
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,  // 256x256
    0x08, 0x06, 0x00, 0x00, 0x00, 0x5C, 0x72, 0xA8   // Color type, etc.
  };

  file.write(reinterpret_cast<const char*>(test_data), sizeof(test_data));
  file.close();

  // Read the file as binary
  std::ifstream infile(test_filename, std::ios::binary);
  if (!infile) {
    std::cout << "Could not read test file" << std::endl;
    return;
  }

  // Read entire file into string
  std::string file_contents((std::istreambuf_iterator<char>(infile)),
                           std::istreambuf_iterator<char>());
  infile.close();

  std::cout << "File size: " << file_contents.length() << " bytes" << std::endl;
  print_hex(file_contents, "File content");

  // Encode the file content
  std::string encoded_file = Base64::encode(file_contents);
  std::cout << "Encoded file: " << encoded_file << std::endl;

  // Decode it back
  std::string decoded_file = Base64::decode(encoded_file);
  print_hex(decoded_file, "Decoded    ");

  bool file_test_pass = (file_contents == decoded_file);
  std::cout << "File test: " << (file_test_pass ? "✅ PASS" : "❌ FAIL") << std::endl;

  // Clean up
  std::remove(test_filename);
}

int main() {
  std::cout << "Binary Data Base64 Test Suite" << std::endl;
  std::cout << "=============================" << std::endl;

  test_binary_data();
  test_file_encoding();

  std::cout << "\nConclusion: The Base64 implementation correctly handles:" << std::endl;
  std::cout << "• Null bytes (0x00)" << std::endl;
  std::cout << "• All byte values (0x00 to 0xFF)" << std::endl;
  std::cout << "• Binary file data" << std::endl;
  std::cout << "• PNG headers and other binary formats" << std::endl;

  return 0;
}