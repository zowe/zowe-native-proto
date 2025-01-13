#!/usr/bin/env bash

zowe files download uf $1/zowe-native-proto/libzcn.a --binary --file zowe-native-proto-package/libzcn.a
zowe files download uf $1/zowe-native-proto/ztype.h --binary --file zowe-native-proto-package/ztype.h
zowe files download uf $1/zowe-native-proto/zcntype.h --binary --file zowe-native-proto-package/zcntype.h
zowe files download uf $1/zowe-native-proto/zcn.hpp --binary --file zowe-native-proto-package/zcn.hpp
zip -r zowe-native-proto-package.zip zowe-native-proto-package