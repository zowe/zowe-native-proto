# zowe-native-proto

## Setup

create your own `config.local.json` adjacent to `config.default.json` with something like:

```json
{
    "host": "my.mainframe.net",
    "username": "ibmuser",
    "password": "ibmpass",
    "deployDirectory": "/u/users/ibmuser/zowe-native-proto"
}
```

## Build Tool

`cd tools/build && npm install && npx tsc`

## Deploy & Build

`node tools/build/lib init` (once)
`node tools/build/lib/main.js deploy-build`
`node tools/build/lib/main.js deploy-build c/zds.cpp`  to deploy one file

## Test

On remote system, `cd` to deploy dir and run `test.sh` (or run `zowex`)
