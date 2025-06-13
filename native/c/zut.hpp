/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#ifndef ZUT_HPP
#define ZUT_HPP

#include <iconv.h>
#include <iostream>
#include <vector>
#include <string>
#include "zcntype.h"

/**
 * @struct ZConvData
 * @brief Structure holding data for character set conversion
 */
typedef struct ZConvData
{
    char *input;            /**< Pointer to input buffer. */
    size_t input_size;      /**< Size of input buffer. */
    size_t max_output_size; /**< Maximum size of output buffer. */
    char *output_buffer;    /**< Pointer to output buffer. */
    char *output_iter;      /**< Pointer to current position in output buffer. */
} ZConvData;

/**
 * @brief Search for a specific string
 * @param input The string to search for
 * @return Return code (0 for success, non-zero for error)
 */
int zut_search(std::string input);

/**
 * @brief Run a specified command or operation
 * @param input The command string to execute
 * @return Return code (0 for success, non-zero for error)
 */
int zut_run(std::string input);

/**
 * @brief Substitute a symbol in a string
 * @param symbol The symbol to substitute
 * @param result Reference to a string where the result will be stored
 * @return Return code (0 for success, non-zero for error)
 */
int zut_substitute_sybmol(std::string symbol, std::string &result);

/**
 * @brief Invoke BPXWDYN service with the given parameters
 * @param command The command string
 * @param rc Pointer to return code (output)
 * @param result Reference to a string where the result will be stored
 * @return Return code (0 for success, non-zero for error)
 */
int zut_bpxwdyn(std::string command, unsigned int *rc, std::string &result);

/**
 * @brief Print a hello message
 * @param input Input string for the hello message
 * @return Return code (0 for success, non-zero for error)
 */
int zut_hello(std::string input);

/**
 * @brief Convert an integer to its hexadecimal character representation
 * @param value Integer value
 * @return Corresponding hexadecimal character
 */
char zut_get_hex_char(int value);

/**
 * @brief Get the current user name
 * @param user Reference to a string where the user name will be stored
 * @return Return code (0 for success, non-zero for error)
 */
int zut_get_current_user(std::string &user);

/**
 * @brief Convert a string to uppercase, pad or truncate as needed
 * @param dest Destination buffer
 * @param src Source string
 * @param length Desired length of the output
 */
void zut_uppercase_pad_truncate(char *dest, std::string src, int length);

/**
 * @brief Convert a DSECT
 * @return Return code (0 for success, non-zero for error)
 */
int zut_convert_dsect();

/**
 * @brief Prepare encoding options
 * @param encoding_value The encoding to use
 * @param opts Pointer to encoding options structure
 * @return True if preparation is successful, false otherwise
 */
bool zut_prepare_encoding(const std::string &encoding_value, ZEncode *opts);

/**
 * @brief Print a string as a sequence of bytes
 * @param input The string to print
 */
void zut_print_string_as_bytes(std::string &input);

/**
 * @brief Convert a hexadecimal string to a vector of bytes
 * @param hex_string The hexadecimal string
 * @return Vector containing the bytes
 */
std::vector<uint8_t> zut_get_contents_as_bytes(const std::string &hex_string);

/**
 * @brief Calculate the Adler-32 checksum of a string
 * @param input The input string
 * @return The Adler-32 checksum
 */
uint32_t zut_calc_adler32_checksum(const std::string &input);

/**
 * @brief Perform character set conversion using iconv
 * @param cd iconv conversion descriptor
 * @param data Reference to ZConvData containing buffers and sizes
 * @param diag Reference to diagnostic information structure
 * @return Number of bytes converted or error code
 */
size_t zut_iconv(iconv_t cd, ZConvData &data, ZDIAG &diag);

/**
 * @brief Build an ETag string from file modification time and size
 * @param mtime Modification time
 * @param byte_size File size in bytes
 * @return Generated ETag string
 */
std::string zut_build_etag(const size_t mtime, const size_t byte_size);

/**
 * @brief Encode a string from one character set to another
 * @param input_str The input string
 * @param from_encoding Source encoding name
 * @param to_encoding Target encoding name
 * @param diag Reference to diagnostic information structure
 * @return The encoded string
 */
std::string zut_encode(const std::string &input_str, const std::string &from_encoding, const std::string &to_encoding, ZDIAG &diag);

std::vector<char> zut_encode(const char *input_str, size_t input_size, const std::string &from_encoding, const std::string &to_encoding, ZDIAG &diag);

/**
 * @brief Format a vector of strings as a CSV line
 * @param fields Vector of fields
 * @return CSV-formatted string
 */
std::string zut_format_as_csv(std::vector<std::string> &fields);

/**
 * @brief Trim whitespace from the right end of a string
 * @param s String to trim
 * @param t Characters to trim (default is space)
 * @return Reference to the trimmed string
 */
std::string &zut_rtrim(std::string &s, const char *t = " ");

/**
 * @brief Trim whitespace from the left end of a string
 * @param s String to trim
 * @param t Characters to trim (default is space)
 * @return Reference to the trimmed string
 */
std::string &zut_ltrim(std::string &s, const char *t = " ");

/**
 * @brief Trim whitespace from both ends of a string
 * @param s String to trim
 * @param t Characters to trim (default is space)
 * @return Reference to the trimmed string
 */
std::string &zut_trim(std::string &s, const char *t = " ");

/**
 * @brief Function to dynamically allocate output debug DD
 *
 * @return int rc Return code (0 for success, non-zero for error)
 */
int zut_alloc_debug();

/**
 * @brief Default debug message function for zut_dump_storage
 *
 * @param message Message to be printed
 * @return int rc Return code (0 for success, non-zero for error)
 */
int zut_debug_message(const char *message);

#endif // ZUT_HPP
