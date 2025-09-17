XLC_FLAGS=_CC_ACCEPTABLE_RC=0 _C89_ACCEPTABLE_RC=0 _CXX_ACCEPTABLE_RC=0
CXX=$(XLC_FLAGS) xlc++

sample_plugin.dll: sample_plugin.cpp
	$(CXX) $^ -qdll -Wl,dll -o $@