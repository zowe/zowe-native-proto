#ifndef ZDS_PY_HPP
#define ZDS_PY_HPP

#include <string>
#include <vector>
#include "../c/zdstype.h"
#include "../c/zds.hpp"

void create_dataset(std::string dsn, DS_ATTRIBUTES attributes);

std::vector<ZDSEntry> list_datasets(std::string dsn);

std::string read_dataset(std::string dsn, std::string codepage = "");

std::string write_dataset(std::string dsn, std::string data, std::string codepage = "", std::string etag = "");

void delete_dataset(std::string dsn);

void create_member(std::string dsn);

std::vector<ZDSMem> list_members(std::string dsn);

std::string read_member(std::string dsn, std::string member, std::string codepage = "");

std::string write_member(std::string dsn, std::string member, std::string data, std::string codepage = "", std::string etag = "");

#endif