# Pong Server Demo

Demonstrates z/OS SSH communication with EBCDIC to ASCII conversion.

## Server

- Upload: `zowe files upload ftu server.cpp <ussDir>/server.cpp --binary`
- Build on z/OS: `g++ -std=c++17 -o pong-server server.cpp`

## Client

- `cd examples/pong-server && npm install` (once)
- `npx tsx client.ts ibmuser@<zosHost> <serverCmd>`
  - `zosHost` - hostname of z/OS server
  - `serverCmd` - command to run the compiled binary
