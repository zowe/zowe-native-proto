#include "zds_py.hpp"
#include <unistd.h>
#include <string>
#include <stdexcept>

static void e2a_inplace(std::string &s)
{
    if (s.empty())
        return;
    s.push_back('\0');
    __e2a_s(&s[0]);
    s.pop_back();
}

static void a2e_inplace(std::string &s)
{
    if (s.empty())
        return;
    s.push_back('\0');
    __a2e_s(&s[0]);
    s.pop_back();
}

/**
 * Dataset Functions
 */

void create_dataset(std::string dsn, DS_ATTRIBUTES attributes)
{
    ZDS zds = {0};
    std::string response;

    DS_ATTRIBUTES attrs_copy = attributes;

    a2e_inplace(dsn);

    a2e_inplace(attrs_copy.alcunit);
    a2e_inplace(attrs_copy.dsorg);
    a2e_inplace(attrs_copy.recfm);
    a2e_inplace(attrs_copy.dataclass);
    a2e_inplace(attrs_copy.unit);
    a2e_inplace(attrs_copy.dsntype);
    a2e_inplace(attrs_copy.mgntclass);
    a2e_inplace(attrs_copy.dsname);
    a2e_inplace(attrs_copy.storclass);
    a2e_inplace(attrs_copy.vol);

    int rc = zds_create_dsn(&zds, dsn, attrs_copy, response);

    if (rc != 0)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
    }
}

std::vector<ZDSEntry> list_datasets(std::string dsn)
{
    std::vector<ZDSEntry> entries;
    ZDS zds = {0};

    a2e_inplace(dsn);
    int rc = zds_list_data_sets(&zds, dsn, entries);

    if (rc != 0 && rc != RTNCD_WARNING)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
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

std::string read_dataset(std::string dsn, std::string codepage)
{
    ZDS zds = {0};

    if (!codepage.empty())
    {
        zds.encoding_opts.data_type = eDataTypeText;
        strncpy(zds.encoding_opts.codepage, codepage.c_str(), sizeof(zds.encoding_opts.codepage) - 1);
    }

    a2e_inplace(dsn);
    std::string response;
    int rc = zds_read_from_dsn(&zds, dsn, response);

    if (rc != 0)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
    }

    e2a_inplace(response);
    return response;
}

std::string write_dataset(std::string dsn, std::string data, std::string codepage, std::string etag)
{
    ZDS zds = {0};

    if (!codepage.empty())
    {
        zds.encoding_opts.data_type = eDataTypeText;
        strncpy(zds.encoding_opts.codepage, codepage.c_str(), sizeof(zds.encoding_opts.codepage) - 1);
    }

    if (!etag.empty())
    {
        strncpy(zds.etag, etag.c_str(), sizeof(zds.etag) - 1);
    }

    a2e_inplace(dsn);
    a2e_inplace(data);
    int rc = zds_write_to_dsn(&zds, dsn, data);

    if (rc != 0)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
    }

    std::string new_etag(zds.etag);
    e2a_inplace(new_etag);
    return new_etag;
}

void delete_dataset(std::string dsn)
{
    ZDS zds = {0};

    a2e_inplace(dsn);
    int rc = zds_delete_dsn(&zds, dsn);

    if (rc != 0)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
    }
}

/**
 * Dataset Member Functions
 */

void create_member(std::string dsn)
{
    ZDS zds = {0};

    a2e_inplace(dsn);

    std::string empty_data = "";
    a2e_inplace(empty_data);
    int rc = zds_write_to_dsn(&zds, dsn, empty_data);

    if (rc != 0)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
    }
}

std::vector<ZDSMem> list_members(std::string dsn)
{
    std::vector<ZDSMem> members;
    ZDS zds = {0};

    a2e_inplace(dsn);
    int rc = zds_list_members(&zds, dsn, members);

    if (rc != 0 && rc != RTNCD_WARNING)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
    }

    for (auto &m : members)
    {
        e2a_inplace(m.name);
    }
    return members;
}

std::string read_member(std::string dsn, std::string member, std::string codepage)
{
    ZDS zds = {0};

    // Set encoding options if codepage provided
    if (!codepage.empty())
    {
        zds.encoding_opts.data_type = eDataTypeText;
        strncpy(zds.encoding_opts.codepage, codepage.c_str(), sizeof(zds.encoding_opts.codepage) - 1);
    }

    std::string member_dsn = dsn + "(" + member + ")";
    a2e_inplace(member_dsn);

    std::string response;
    int rc = zds_read_from_dsn(&zds, member_dsn, response);

    if (rc != 0)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
    }
    e2a_inplace(response);
    return response;
}

std::string write_member(std::string dsn, std::string member, std::string data, std::string codepage, std::string etag)
{
    ZDS zds = {0};

    if (!codepage.empty())
    {
        zds.encoding_opts.data_type = eDataTypeText;
        strncpy(zds.encoding_opts.codepage, codepage.c_str(), sizeof(zds.encoding_opts.codepage) - 1);
    }

    // Set etag if provided
    if (!etag.empty())
    {
        strncpy(zds.etag, etag.c_str(), sizeof(zds.etag) - 1);
    }

    std::string member_dsn = dsn + "(" + member + ")";
    a2e_inplace(member_dsn);
    a2e_inplace(data);

    int rc = zds_write_to_dsn(&zds, member_dsn, data);

    if (rc != 0)
    {
        std::string diag(zds.diag.e_msg, zds.diag.e_msg_len);
        diag.push_back('\0');
        e2a_inplace(diag);
        diag.pop_back();
        throw std::runtime_error(diag);
    }

    std::string new_etag(zds.etag);
    e2a_inplace(new_etag);
    return new_etag;
}