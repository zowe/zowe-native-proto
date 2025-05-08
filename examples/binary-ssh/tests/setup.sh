#!/bin/bash

# Generate resource files with random data
mkdir -p resources
dd if=/dev/urandom of=resources/test10k.bin bs=1K count=10
dd if=/dev/urandom of=resources/test100k.bin bs=1K count=100
dd if=/dev/urandom of=resources/test1m.bin bs=1M count=1
dd if=/dev/urandom of=resources/test10m.bin bs=1M count=10
dd if=/dev/urandom of=resources/test100m.bin bs=1M count=100
dd if=/dev/urandom of=resources/test1g.bin bs=1M count=1000

# Create config file if it doesn't exist
if [ ! -f "config.json" ]; then
    cp config.example.json config.json
fi

# Upload Golang test scripts
ussDir=$(jq -r .ussDir config.json)
zosmfProfile=$(jq -r .zosmfProfile config.json)
zowe files upload ftu ../go.mod ${ussDir}/go.mod --binary --zosmf-p $zosmfProfile
zowe files upload ftu ../go.sum ${ussDir}/go.sum --binary --zosmf-p $zosmfProfile
zowe files upload ftu testb64.go ${ussDir}/testb64.go --binary --zosmf-p $zosmfProfile
zowe files upload ftu testraw.go ${ussDir}/testraw.go --binary --zosmf-p $zosmfProfile
echo
echo "Connect to the ${zosmfProfile} system and run this command:"
echo "  cd ${ussDir} && go build -o testb64 testb64.go && go build -o testraw testraw.go"
# NOTE: you may need to export env vars: GOINSECURE="*" GOPROXY=direct
