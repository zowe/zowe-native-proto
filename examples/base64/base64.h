#ifndef BASE64_H
#define BASE64_H

#include <string>
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

    for (size_t i = 0; i < input.length(); ++i) {
      unsigned char c = input[i];
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

    for (size_t i = 0; i < input.length(); ++i) {
      unsigned char c = input[i];
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

// Base64 character set definition
const std::string Base64::chars =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";

#endif // BASE64_H