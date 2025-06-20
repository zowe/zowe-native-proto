%module zds_py

%{
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

struct ZDSMem {
    std::string name;
};

struct DS_ATTRIBUTES {
    std::string alcunit;
    int blksize;
    int dirblk;
    std::string dsorg;
    int primary;
    std::string recfm;
    int lrecl;
    std::string dataclass;
    std::string unit;
    std::string dsntype;
    std::string mgntclass;
    std::string dsname;
    int avgblk;
    int secondary;
    int size;
    std::string storclass;
    std::string vol;
};