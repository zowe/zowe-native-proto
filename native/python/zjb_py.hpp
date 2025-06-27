#ifndef ZJB_PY_HPP
#define ZJB_PY_HPP

#include "../c/zjb.hpp"
#include "../c/ztype.h"
#include <string>
#include <vector>

std::vector<ZJob> list_jobs_by_owner(std::string owner_name);

ZJob get_job_status(std::string jobid);

std::vector<ZJobDD> list_spool_files(std::string jobid);

std::string read_spool_file(std::string jobid, int key);

std::string get_job_jcl(std::string jobid);

std::string submit_job(std::string jcl_content);

bool delete_job(std::string jobid);

#endif