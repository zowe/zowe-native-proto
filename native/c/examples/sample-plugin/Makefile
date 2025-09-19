XLC_FLAGS=_CC_ACCEPTABLE_RC=0 _C89_ACCEPTABLE_RC=0 _CXX_ACCEPTABLE_RC=0
CXX=$(XLC_FLAGS) xlc++
CXX_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink,exportall" -D__IBMCPP_TR1__=1 -I../../

all: sample_plugin.so

sample_plugin.so: sample_plugin.cpp
	$(CXX) $(CXX_FLAGS) -qdll -Wl,dll -o $@ $^