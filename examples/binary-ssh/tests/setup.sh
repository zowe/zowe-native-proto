#!/bin/bash
set -e

# Generate resource files with random data
if [ ! -d "resources" ]; then
  mkdir -p resources
  dd if=/dev/urandom of=resources/test10k.bin bs=1K count=10
  dd if=/dev/urandom of=resources/test100k.bin bs=1K count=100
  dd if=/dev/urandom of=resources/test1m.bin bs=1M count=1
  dd if=/dev/urandom of=resources/test10m.bin bs=1M count=10
  dd if=/dev/urandom of=resources/test100m.bin bs=1M count=100
  dd if=/dev/urandom of=resources/test1g.bin bs=1M count=1000
fi

# Create config file if it doesn't exist
if [ ! -f "config.json" ]; then
  cp config.example.json config.json
  echo "Update the properties defined in config.json. Then re-run this script to deploy and build the test binaries."
  exit
fi

# Deploy Golang test scripts and build them
ussDir=$(jq -r .ussDir config.json)
zosmfProfile=$(jq -r .zosmfProfile config.json)
npx zowe files upload ftu ../go.mod ${ussDir}/go.mod --binary --zosmf-p $zosmfProfile
npx zowe files upload ftu ../go.sum ${ussDir}/go.sum --binary --zosmf-p $zosmfProfile
npx zowe files upload ftu testb64.go ${ussDir}/testb64.go --binary --zosmf-p $zosmfProfile
npx zowe files upload ftu testb85.go ${ussDir}/testb85.go --binary --zosmf-p $zosmfProfile
npx zowe files upload ftu testraw.go ${ussDir}/testraw.go --binary --zosmf-p $zosmfProfile
sshCmd="cd ${ussDir}; for f in testb64 testb85 testraw; do go build -o \$f \$f.go; done"
sshProfile=$(jq -r .sshProfile config.json)
if [ "$sshProfile" != "null" ]; then
  npx zowe uss issue ssh "cd ${ussDir}; for f in testb64 testb85 testraw; do go build -v -o \$f \$f.go; done" --ssh-p $sshProfile
else
  echo
  echo "Connect to the ${zosmfProfile} system and run this command:"
  echo "  $sshCmd"
fi
# NOTE: you may need to export env vars: GOINSECURE="*" GOPROXY=direct
