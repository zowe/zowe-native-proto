CXX=ibm-clang++64
CXX_FLAGS=-std=c++17 -fvisibility=default -fno-aligned-allocation -I../../
CXX_LINK_FLAGS=-shared

all: sample_plugin.so

sample_plugin.so: sample_plugin.cpp
    $(CXX) $(CXX_FLAGS) -c -o sample_plugin.o $^
    $(CXX) $(CXX_LINK_FLAGS) -o $@ sample_plugin.o