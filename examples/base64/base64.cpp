#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

class Base64 {
private:
  static const std::string chars;
  static const char pad_char = '=';

  static bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
  }

public:
  static std::string encode(const std::string& input) {
    std::string result;
    int val = 0;
    int valb = -6;

    for (unsigned char c : input) {
      val = (val << 8) + c;
      valb += 8;
      while (valb >= 0) {
        result.push_back(chars[(val >> valb) & 0x3F]);
        valb -= 6;
      }
    }

    if (valb > -6) {
      result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    while (result.size() % 4) {
      result.push_back(pad_char);
    }

    return result;
  }

  static std::string decode(const std::string& input) {
    if (input.length() % 4 != 0) {
      throw std::invalid_argument("Invalid base64 input length");
    }

    std::string result;
    int val = 0;
    int valb = -8;

    for (unsigned char c : input) {
      if (c == pad_char) break;

      if (!is_base64(c)) {
        throw std::invalid_argument("Invalid base64 character");
      }

      val = (val << 6) + chars.find(c);
      valb += 6;
      if (valb >= 0) {
        result.push_back(char((val >> valb) & 0xFF));
        valb -= 8;
      }
    }

    return result;
  }
};

// Base64 character set
const std::string Base64::chars =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";

void print_usage() {
  std::cout << "Usage:\n";
  std::cout << "  ./base64 encode <string>   - Encode a string to base64\n";
  std::cout << "  ./base64 decode <string>   - Decode a base64 string\n";
  std::cout << "  ./base64 test              - Run test cases\n";
}

void run_tests() {
  std::cout << "Running Base64 tests...\n\n";

  // Test cases
  std::vector<std::pair<std::string, std::string> > test_cases;
  test_cases.push_back(std::make_pair("", ""));
  test_cases.push_back(std::make_pair("f", "Zg=="));
  test_cases.push_back(std::make_pair("fo", "Zm8="));
  test_cases.push_back(std::make_pair("foo", "Zm9v"));
  test_cases.push_back(std::make_pair("foob", "Zm9vYg=="));
  test_cases.push_back(std::make_pair("fooba", "Zm9vYmE="));
  test_cases.push_back(std::make_pair("foobar", "Zm9vYmFy"));
  test_cases.push_back(std::make_pair("Hello, World!", "SGVsbG8sIFdvcmxkIQ=="));
  test_cases.push_back(std::make_pair("The quick brown fox jumps over the lazy dog", "VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZw=="));
  test_cases.push_back(std::make_pair("Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.", "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4="));

  bool all_passed = true;

  for (const auto& test : test_cases) {
    const std::string& original = test.first;
    const std::string& expected = test.second;

    try {
      // Test encoding
      std::string encoded = Base64::encode(original);
      if (encoded != expected) {
        std::cout << "❌ ENCODE FAILED:\n";
        std::cout << "  Input:    \"" << original << "\"\n";
        std::cout << "  Expected: \"" << expected << "\"\n";
        std::cout << "  Got:      \"" << encoded << "\"\n\n";
        all_passed = false;
        continue;
      }

      // Test decoding
      std::string decoded = Base64::decode(expected);
      if (decoded != original) {
        std::cout << "❌ DECODE FAILED:\n";
        std::cout << "  Input:    \"" << expected << "\"\n";
        std::cout << "  Expected: \"" << original << "\"\n";
        std::cout << "  Got:      \"" << decoded << "\"\n\n";
        all_passed = false;
        continue;
      }

      std::cout << "✅ PASSED: \"" << (original.empty() ? "(empty)" : original.substr(0, 20) + (original.length() > 20 ? "..." : "")) << "\"\n";

    } catch (const std::exception& e) {
      std::cout << "❌ EXCEPTION: " << e.what() << "\n";
      std::cout << "  Input: \"" << original << "\"\n\n";
      all_passed = false;
    }
  }

  std::cout << "\n" << (all_passed ? "🎉 All tests passed!" : "❌ Some tests failed!") << "\n";
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage();
    return 1;
  }

  std::string command = argv[1];

  if (command == "test") {
    run_tests();
    return 0;
  }

  if (argc < 3) {
    print_usage();
    return 1;
  }

  std::string input = argv[2];

  try {
    if (command == "encode") {
      std::string encoded = Base64::encode(input);
      std::cout << "Encoded: " << encoded << std::endl;
    } else if (command == "decode") {
      std::string decoded = Base64::decode(input);
      std::cout << "Decoded: " << decoded << std::endl;
    } else {
      std::cout << "Unknown command: " << command << std::endl;
      print_usage();
      return 1;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}