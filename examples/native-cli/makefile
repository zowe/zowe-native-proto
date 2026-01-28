CXX=g++
HEADER=../../native/c
INCLUDE=-I$(HEADER)

all: touch docli chmod

docli: docli.cpp
	$(CXX) $(INCLUDE) $^ -o $@

touch:
	touch docli.cpp

chmod:
	chmod +x docli