#include "zusf_py.hpp"

void create_uss_file(const std::string &file, const std::string &mode)
{
    ZUSF ctx = {0};
    if (zusf_create_uss_file_or_dir(&ctx, file, mode, false) != 0)
    {
        std::string error_msg = (ctx.diag.e_msg);
        throw std::runtime_error(error_msg);
    }
}

void create_uss_dir(const std::string &file, const std::string &mode)
{
    ZUSF ctx = {0};
    if (zusf_create_uss_file_or_dir(&ctx, file, mode, true) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }
}

std::string list_uss_dir(const std::string &path)
{
    ZUSF ctx = {0};
    std::string out;
    if (zusf_list_uss_file_path(&ctx, path, out) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }
    return out;
}

std::string read_uss_file(const std::string &file, const std::string &codepage)
{
    ZUSF ctx = {0};

    if (!codepage.empty())
    {
        ctx.encoding_opts.data_type = codepage == "binary" ? eDataTypeBinary : eDataTypeText;
        strncpy(ctx.encoding_opts.codepage, codepage.c_str(), sizeof(ctx.encoding_opts.codepage) - 1);
    }

    std::string response;
    if (zusf_read_from_uss_file(&ctx, file, response) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }
    return response;
}

void read_uss_file_streamed(const std::string &file, const std::string &pipe, const std::string &codepage)
{
    ZUSF ctx = {0};

    if (!codepage.empty())
    {
        ctx.encoding_opts.data_type = codepage == "binary" ? eDataTypeBinary : eDataTypeText;
        strncpy(ctx.encoding_opts.codepage, codepage.c_str(), sizeof(ctx.encoding_opts.codepage) - 1);
    }

    if (zusf_read_from_uss_file_streamed(&ctx, file, pipe) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }
}

std::string write_uss_file(const std::string &file, const std::string &data, const std::string &codepage, const std::string &etag)
{
    ZUSF ctx = {0};

    if (!codepage.empty())
    {
        ctx.encoding_opts.data_type = codepage == "binary" ? eDataTypeBinary : eDataTypeText;
        strncpy(ctx.encoding_opts.codepage, codepage.c_str(), sizeof(ctx.encoding_opts.codepage) - 1);
    }

    if (!etag.empty())
    {
        strncpy(ctx.etag, etag.c_str(), sizeof(ctx.etag) - 1);
    }

    std::string data_copy = data;
    if (zusf_write_to_uss_file(&ctx, file, data_copy) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }

    return std::string(ctx.etag);
}

std::string write_uss_file_streamed(const std::string &file, const std::string &pipe, const std::string &codepage, const std::string &etag)
{
    ZUSF ctx = {0};

    if (!codepage.empty())
    {
        ctx.encoding_opts.data_type = codepage == "binary" ? eDataTypeBinary : eDataTypeText;
        strncpy(ctx.encoding_opts.codepage, codepage.c_str(), sizeof(ctx.encoding_opts.codepage) - 1);
    }

    if (!etag.empty())
    {
        strncpy(ctx.etag, etag.c_str(), sizeof(ctx.etag) - 1);
    }

    if (zusf_write_to_uss_file_streamed(&ctx, file, pipe) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }

    return std::string(ctx.etag);
}

void chmod_uss_item(const std::string &file, const std::string &mode, bool recursive)
{
    ZUSF ctx = {0};
    if (zusf_chmod_uss_file_or_dir(&ctx, file, mode, recursive) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }
}

void delete_uss_item(const std::string &file, bool recursive)
{
    ZUSF ctx = {0};
    if (zusf_delete_uss_item(&ctx, file, recursive) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }
}

void chown_uss_item(const std::string &file, const std::string &owner, bool recursive)
{
    ZUSF ctx = {0};
    if (zusf_chown_uss_file_or_dir(&ctx, file, owner, recursive) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }
}

void chtag_uss_item(const std::string &file, const std::string &tag, bool recursive)
{
    ZUSF ctx = {0};
    if (zusf_chtag_uss_file_or_dir(&ctx, file, tag, recursive) != 0)
    {
        std::string error_msg = ctx.diag.e_msg;
        throw std::runtime_error(error_msg);
    }
}