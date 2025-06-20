#ifndef ZJB_PY_HPP
#define ZJB_PY_HPP

#include "../c/zjb.hpp"
#include "../c/ztype.h"
#include <string>
#include <vector>

std::vector<ZJob> list_jobs_by_owner(std::string owner_name);

#endif