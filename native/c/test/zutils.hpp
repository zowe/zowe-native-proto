#ifndef ZUTILS_HPP
#define ZUTILS_HPP
#include <string>
int execute_command_with_input(const std::string &command, const std::string &input);
int execute_command_with_output(const std::string &command, std::string &output);
std::string get_random_string(const int length, const bool allNumbers);
std::string get_random_uss(const std::string base_dir);
std::string parse_etag_from_output(const std::string &output);
std::string parse_hex_dump(const std::string &hex_dump);
#endif