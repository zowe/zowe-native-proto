#ifndef ZUSF_PY_H
#define ZUSF_PY_H

#include <string>
#include <stdexcept>
#include "../../c/zusf.hpp"

void create_uss_file(const std::string &file, const std::string &mode);

void create_uss_dir(const std::string &file, const std::string &mode);

std::string list_uss_dir(const std::string &path);

std::string read_uss_file(const std::string &file, const std::string &codepage = "");

void read_uss_file_streamed(const std::string &file, const std::string &pipe, const std::string &codepage = "");

std::string write_uss_file(const std::string &file, const std::string &data, const std::string &codepage = "", const std::string &etag = "");

std::string write_uss_file_streamed(const std::string &file, const std::string &pipe, const std::string &codepage = "", const std::string &etag = "");

void chmod_uss_item(const std::string &file, const std::string &mode, bool recursive = false);

void delete_uss_item(const std::string &file, bool recursive = false);

void chown_uss_item(const std::string &file, const std::string &owner, bool recursive = false);

void chtag_uss_item(const std::string &file, const std::string &tag, bool recursive = false);

#endif