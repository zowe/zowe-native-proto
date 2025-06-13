#include "zusf_py.h"
#include "../c/zusf.hpp"

std::string list_uss_dir(const std::string &path)
{
    ZUSF ctx = {0};
    std::string out;
    if (zusf_list_uss_file_path(&ctx, path.c_str(), out) != 0)
    {
        throw std::runtime_error(std::string(error_msg));
    }
    return out;
}