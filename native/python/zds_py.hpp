#ifndef ZDS_PY_HPP
#define ZDS_PY_HPP

#include <string>
#include <vector>
#include "../c/zdstype.h"
#include "../c/zds.hpp"

std::vector<ZDSEntry> list_data_sets(std::string dsn);

#endif