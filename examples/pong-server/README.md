# Pong Server Demo

Demonstrates z/OS SSH communication with EBCDIC to ASCII conversion.

## Server

- `zowe files upload dtu tools/pong-server <ussDir> --binary`
- `go build -o pong-server pong.go` (on z/OS)

## Client

- `cd tools/pong-server && npm install` (once)
- `npx tsx client.ts ibmuser@<zosHost> <serverCmd>`
  - `zosHost` - hostname of z/OS server
  - `serverCmd` - command to run Go binary
