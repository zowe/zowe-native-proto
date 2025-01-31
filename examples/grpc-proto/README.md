# gRPC Protobuf Demo

## Server

* `zowe files upload dtu tools/grpc-proto <ussDir> --binary`
* `go run .` (on z/OS)

## Client

* `cd tools/grpc-proto && npm install` (once)
* `ssh -L 3000:/tmp/zowe-native-proto.sock ibmuser@<zosHost>`
* `npx tsx client.ts zowe --target localhost:3000`

## Alternatives

* FlatBuffers - painful to serialize/deserialize, tradeoff between less CPU usage and more I/O
* Cap'n Proto - no actively maintained package for Node.js
