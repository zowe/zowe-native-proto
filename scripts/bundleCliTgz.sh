#!/bin/bash
set -e

# Create dist dir to store artifacts
proj_dir="$(git rev-parse --show-toplevel)"
mkdir -p "$proj_dir/dist"

# Create temp dir to build the package
temp_dir=$(mktemp -d)
trap 'rm -rf "$temp_dir"' EXIT

# Copy cli files to temp dir
cp -r "$proj_dir/packages/cli/" "$temp_dir"
cp "$proj_dir/package-lock.json" "$temp_dir/npm-shrinkwrap.json"

# Install deps for cli package
cd "$temp_dir"
npm install --ignore-scripts

# Bundle sdk package with cli
rm -rf node_modules/zowe-native-proto-sdk
cp -r "$(realpath "$proj_dir/node_modules/zowe-native-proto-sdk")/" node_modules/zowe-native-proto-sdk

npm pack --pack-destination="$proj_dir/dist"
