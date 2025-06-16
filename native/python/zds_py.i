%module zds_py

%{
#ifndef SWIGEXPORT
#define SWIGEXPORT
#endif
#undef SWIGEXPORT
#define SWIGEXPORT
#undef SWIGSTDCALL
#define SWIGSTDCALL

#include "zds_py.hpp"
%}

%include "std_string.i"
%include "std_vector.i"

%include "zds_py.hpp"

%template(ZDSMemVector) std::vector<ZDSMem>;
%template(ZDSEntryVector) std::vector<ZDSEntry>;

struct ZDSEntry {
    std::string name;
    std::string dsorg;
    std::string volser;
    std::string recfm;
    bool migr;
};