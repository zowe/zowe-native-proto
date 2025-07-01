#ifndef ZDS_PY_HPP
#define ZDS_PY_HPP

#include <string>
#include <vector>
#include "../c/zdstype.h"
#include "../c/zds.hpp"
#include "conversion.hpp"

void create_data_set(std::string dsn, DS_ATTRIBUTES attributes);

std::vector<ZDSEntry> list_data_sets(std::string dsn);

std::string read_data_set(std::string dsn, std::string codepage = "");

std::string write_data_set(std::string dsn, std::string data, std::string codepage = "", std::string etag = "");

void delete_data_set(std::string dsn);

void create_member(std::string dsn);

std::vector<ZDSMem> list_members(std::string dsn);

#endif