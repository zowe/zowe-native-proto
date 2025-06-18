%module zjb_py

%{
#include "zjb_py.hpp"
%}

%include "std_string.i"
%include "std_vector.i"

%include "zjb_py.hpp"

%template(ZJobVector) std::vector<ZJob>;
%template(ZJobDDVector) std::vector<ZJobDD>;

struct ZJob {
    std::string jobname;
    std::string jobid;
    std::string owner;
    std::string status;
    std::string full_status;
    std::string retcode;
    std::string job_correlator;
};

struct ZJobDD
{
  std::string jobid;
  std::string ddn;
  std::string dsn;
  std::string stepname;
  std::string procstep;
  int key;
};