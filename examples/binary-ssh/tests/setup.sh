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
