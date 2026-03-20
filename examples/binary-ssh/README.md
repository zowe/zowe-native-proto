# Binary SSH Demo

Demonstrates bidirectional transfer of raw bytes over a z/OS SSH connection.

## Server

- Upload: `zowe files upload dtu examples/binary-ssh <ussDir> --binary`
- Build on z/OS: `make` or `g++ -std=c++17 -o server server.cpp`

## Client

- `cd examples/binary-ssh && npm install` (once)
- `npx tsx client.ts ibmuser@<zosHost> <serverCmd>`
  - `zosHost` - hostname of z/OS server
  - `serverCmd` - command to run the compiled binary

## Transfer Benchmarks

The `tests/` directory contains base64, base85, and raw binary transfer test binaries.
See `tests/setup.sh` to deploy and build them on z/OS.
