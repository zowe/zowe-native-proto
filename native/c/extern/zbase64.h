#ifndef BASE64_EBCDIC_FIXED_H
#define BASE64_EBCDIC_FIXED_H

#include <string>
#include <stdexcept>

namespace zbase64 {

// Standard Base64 alphabet (what we want to output)
static const char encode_table_ascii[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

// Create decode table by building it programmatically
static unsigned char create_ebcdic_decode_table() {
    static unsigned char table[256];
    static bool initialized = false;

    if (!initialized) {
        // Initialize all to invalid
        for (int i = 0; i < 256; ++i) {
            table[i] = 255;
        }

        // A-I: EBCDIC 193-201 -> Base64 0-8
        for (int i = 0; i < 9; ++i) {
            table[193 + i] = i;
        }

        // J-R: EBCDIC 209-217 -> Base64 9-17
        for (int i = 0; i < 9; ++i) {
            table[209 + i] = 9 + i;
        }

        // S-Z: EBCDIC 226-233 -> Base64 18-25
        for (int i = 0; i < 8; ++i) {
            table[226 + i] = 18 + i;
        }

        // a-i: EBCDIC 129-137 -> Base64 26-34
        for (int i = 0; i < 9; ++i) {
            table[129 + i] = 26 + i;
        }

        // j-r: EBCDIC 145-153 -> Base64 35-43
        for (int i = 0; i < 9; ++i) {
            table[145 + i] = 35 + i;
        }

        // s-z: EBCDIC 162-169 -> Base64 44-51
        for (int i = 0; i < 8; ++i) {
            table[162 + i] = 44 + i;
        }

        // 0-9: EBCDIC 240-249 -> Base64 52-61
        for (int i = 0; i < 10; ++i) {
            table[240 + i] = 52 + i;
        }

        // Special characters
        table[78] = 62;   // + -> 62
        table[97] = 63;   // / -> 63 (Note: this conflicts with 'a' in some EBCDIC variants)
        table[126] = 254; // = -> padding marker

        initialized = true;
    }

    return 0; // dummy return
}

// Get the decode table
static const unsigned char* get_ebcdic_decode_table() {
    static unsigned char table[256];
    static bool initialized = false;

    if (!initialized) {
        // Initialize all to invalid
        for (int i = 0; i < 256; ++i) {
            table[i] = 255;
        }

        // A-I: EBCDIC 193-201 -> Base64 0-8
        for (int i = 0; i < 9; ++i) {
            table[193 + i] = i;
        }

        // J-R: EBCDIC 209-217 -> Base64 9-17
        for (int i = 0; i < 9; ++i) {
            table[209 + i] = 9 + i;
        }

        // S-Z: EBCDIC 226-233 -> Base64 18-25
        for (int i = 0; i < 8; ++i) {
            table[226 + i] = 18 + i;
        }

        // a-i: EBCDIC 129-137 -> Base64 26-34
        for (int i = 0; i < 9; ++i) {
            table[129 + i] = 26 + i;
        }

        // j-r: EBCDIC 145-153 -> Base64 35-43
        for (int i = 0; i < 9; ++i) {
            table[145 + i] = 35 + i;
        }

        // s-z: EBCDIC 162-169 -> Base64 44-51
        for (int i = 0; i < 8; ++i) {
            table[162 + i] = 44 + i;
        }

        // 0-9: EBCDIC 240-249 -> Base64 52-61
        for (int i = 0; i < 10; ++i) {
            table[240 + i] = 52 + i;
        }

        // Special characters
        table[78] = 62;   // + -> 62
        table[97] = 63;   // / -> 63
        table[126] = 254; // = -> padding marker

        initialized = true;
    }

    return table;
}

// Fast inline function to calculate encoded size
inline size_t encoded_size(size_t input_size) {
    return ((input_size + 2) / 3) * 4;
}

// Fast inline function to calculate maximum decoded size
inline size_t max_decoded_size(size_t input_size) {
    return (input_size / 4) * 3;
}

// High-performance encode function (produces ASCII Base64 output)
inline std::string encode(const std::string& input) {
    if (input.empty()) {
        return std::string();
    }

    const size_t input_len = input.size();
    const size_t output_len = encoded_size(input_len);

    std::string output;
    output.reserve(output_len);

    const unsigned char* src = reinterpret_cast<const unsigned char*>(input.data());
    const unsigned char* const src_end = src + input_len;

    // Process 3 bytes at a time for optimal performance
    while (src + 2 < src_end) {
        const unsigned int b0 = src[0];
        const unsigned int b1 = src[1];
        const unsigned int b2 = src[2];

        // Pack 3 bytes into 24 bits, then extract 4 x 6-bit values
        const unsigned int combined = (b0 << 16) | (b1 << 8) | b2;

        output.push_back(encode_table_ascii[(combined >> 18) & 0x3F]);
        output.push_back(encode_table_ascii[(combined >> 12) & 0x3F]);
        output.push_back(encode_table_ascii[(combined >> 6) & 0x3F]);
        output.push_back(encode_table_ascii[combined & 0x3F]);

        src += 3;
    }

    // Handle remaining 1 or 2 bytes
    if (src < src_end) {
        const unsigned int b0 = src[0];
        const unsigned int b1 = (src + 1 < src_end) ? src[1] : 0;

        const unsigned int combined = (b0 << 16) | (b1 << 8);

        output.push_back(encode_table_ascii[(combined >> 18) & 0x3F]);
        output.push_back(encode_table_ascii[(combined >> 12) & 0x3F]);

        if (src + 1 < src_end) {
            output.push_back(encode_table_ascii[(combined >> 6) & 0x3F]);
            output.push_back('=');
        } else {
            output.push_back('=');
            output.push_back('=');
        }
    }

    return output;
}

// High-performance decode function (handles EBCDIC Base64 input)
inline std::string decode(const std::string& input) {
    if (input.empty()) {
        return std::string();
    }

    const size_t input_len = input.size();

    // Validate input length (must be multiple of 4)
    if (input_len % 4 != 0) {
        throw std::invalid_argument("Invalid base64 input length");
    }

    const unsigned char* decode_table = get_ebcdic_decode_table();

    // Count padding characters (EBCDIC '=' is 126)
    size_t padding = 0;
    if (input_len >= 2) {
        if (static_cast<unsigned char>(input[input_len - 1]) == 126) padding++;
        if (static_cast<unsigned char>(input[input_len - 2]) == 126) padding++;
    }

    const size_t output_len = max_decoded_size(input_len) - padding;
    std::string output;
    output.reserve(output_len);

    const unsigned char* src = reinterpret_cast<const unsigned char*>(input.data());
    const unsigned char* const src_end = src + input_len;

    // Process 4 characters at a time for optimal performance
    while (src + 4 <= src_end) {
        const unsigned char c0 = decode_table[src[0]];
        const unsigned char c1 = decode_table[src[1]];
        const unsigned char c2 = (src[2] == 126) ? 0 : decode_table[src[2]]; // 126 = EBCDIC '='
        const unsigned char c3 = (src[3] == 126) ? 0 : decode_table[src[3]]; // 126 = EBCDIC '='

        // Validate non-padding characters
        if ((c0 | c1) & 0x80) {
            throw std::invalid_argument("Invalid base64 character");
        }
        if (src[2] != 126 && (c2 & 0x80)) {
            throw std::invalid_argument("Invalid base64 character");
        }
        if (src[3] != 126 && (c3 & 0x80)) {
            throw std::invalid_argument("Invalid base64 character");
        }

        // Combine 4 x 6-bit values into 24 bits, then extract 3 bytes
        const unsigned int combined = (c0 << 18) | (c1 << 12) | (c2 << 6) | c3;

        output.push_back(static_cast<char>((combined >> 16) & 0xFF));

        if (src[2] != 126) {
            output.push_back(static_cast<char>((combined >> 8) & 0xFF));

            if (src[3] != 126) {
                output.push_back(static_cast<char>(combined & 0xFF));
            }
        }

        src += 4;
    }

    return output;
}

} // namespace base64_ebcdic_fixed

#endif // BASE64_EBCDIC_FIXED_H