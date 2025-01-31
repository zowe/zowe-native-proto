# Binary SSH Demo

Demonstrates bidirectional transfer of raw bytes over a z/OS SSH connection.

## Server

- `zowe files upload dtu tools/binary-ssh <ussDir> --binary`
- `go build -o binary-ssh rawtty.go` (on z/OS)

## Client

- `cd tools/binary-ssh && npm install` (once)
- `npx tsx client.ts ibmuser@<zosHost> <serverCmd>`
  - `zosHost` - hostname of z/OS server
  - `serverCmd` - command to run Go binary
