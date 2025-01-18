CXX=g++
HEADER=../../../native/c/
INCLUDE=-I$(HEADER)

all: touch testcli chmod

testcli: testcli.cpp
	$(CXX) $(INCLUDE) $^ -o $@

touch:
	touch testcli.cpp

chmod:
	chmod +x testcli