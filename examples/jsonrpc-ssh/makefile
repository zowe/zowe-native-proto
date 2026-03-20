CXX=ibm-clang++64
CXXFLAGS=-std=c++17
HEADER=../../native/c
INCLUDE=-I$(HEADER)

all: server

server: server.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

clean:
	rm -f server
