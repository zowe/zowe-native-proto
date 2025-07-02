%module zjb_py

%{
#include "zjb_py.hpp"
%}

%include "std_string.i"
%include "std_vector.i"

%include "zjb_py.hpp"

%template(ZJobVector) std::vector<ZJob>;

struct ZJob {
	std::string jobname;
	std::string jobid;
	std::string owner;
	std::string status;
	std::string full_status;
	std::string retcode;
	std::string correlator;
};