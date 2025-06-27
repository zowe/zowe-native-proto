#include "zds_py.hpp"
#include <unistd.h>
#include <string>

static void e2a_inplace(std::string &s)
{
    if (s.empty())
        return;
    s.push_back('\0');
    __e2a_s(&s[0]);
    s.pop_back();
}

std::vector<ZDSEntry> list_data_sets(std::string dsn)
{
    std::vector<ZDSEntry> entries;
    ZDS zds = {0};

    __a2e_s(&dsn[0]);
    int rc = zds_list_data_sets(&zds, dsn, entries);

    if (rc != 0)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        __e2a_s(&diag[0]);
        diag.pop_back();
        std::cerr
            << "ZDS call failed, rc=" << rc
            << ", diag=\"" << diag << "\"\n";
    }

    for (auto &e : entries)
    {
        e2a_inplace(e.name);
        e2a_inplace(e.dsorg);
        e2a_inplace(e.volser);
        e2a_inplace(e.recfm);
    }
    return entries;
}