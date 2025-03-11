# I/O Server

ioserver (`zowed`) acts as the middleware between the client and server. It is written in Go and leverages an input channel to support asynchronous request processing.

## Architectural overview

ioserver mediates all requests and dispatches requests to appropriate command handlers. These command handlers are defined in a map and are accessed by the command name. Those command handlers can perform actions in Go, or execute the backend layer (`zowex`) to access data. The response data is composed and serialized as JSON before being returned to the caller through stdout.

## Request and response processing

The ioserver process is instantiated by the client through SSH, which opens a communication channel over stdio. When a request is received from the client over stdin, ioserver attempts to parse the input as JSON. If the JSON response is valid, the server looks for the `command` property of the JSON object and attempts to identify a matching command handler. If a command handler is found for the given command, the handler is executed and given the JSON object for further processing.

The command handlers can expect a stronger request type than what is expected during initial command processing. Appropriate request and response types can be exposed for use with these handlers. In the event of a JSON deserialization error, the command handler stops execution and returns early, returning an error response with any additional context. Once the JSON is successfully deserialized into the desired type, command processing continues and the handler can perform any actions necessary to create, receive, update or delete data.

Once the command handler has the required data for a response, the handler "marshals" (serializes) the response type as JSON and prints it to stdout. The output is redirected to the SSH client, where it is interpreted as a response and processed according to the corresponding response type.

See the example JSON request below for listing data sets:

```json
{
  "command": "listDatasets",
  "pattern": "DS.PAT.*"
}
```

This request is deserialized by ioserver and `zowex` is invoked to get the list of data sets matching the given pattern. Then the response is composed, serialized as JSON and returned to the caller, for example:

```json
{
  "items": [
    {
      "dsname": "DS.PAT.AB",
      "dsorg": "PO",
      "volser": "MIGRAT"
    },
    {
      "dsname": "DS.PAT.ABC",
      "dsorg": "PO-E",
      "volser": "WRKD01"
    },
    {
      "dsname": "DS.PAT.ABCD",
      "dsorg": "PS",
      "volser": "WRKD03"
    }
  ],
  "returnedRows": 3
}
```

## Handling encoding for resource contents

Modern text editors expect a standardized encoding format such as UTF-8. `ioserver` implements processing for reading/writing data sets, USS files and job spools (read-only) with a given encoding.

### Read

When an encoding is not provided, we use the system default codepage as the source encoding for resource contents (`IBM-1047`, in most cases). We convert the contents from the source encoding to UTF-8 so the contents can be rendered and edited within any modern text editor.

```json
{
  "command": "readFile",
  "path": "/u/users/you/file.txt",
  "encoding": "ISO8859-1"
}
```

Response:

```jsonc
{
  "encoding": "ISO8859-1",
  "path": "/u/users/you/file.txt",
  "data": [104, 101, 108, 108, 111] // "hello" in ASCII
}
```

### Write

The contents of write requests are interpreted as UTF-8 data. We convert the UTF-8 bytes to the given encoding so the data can be read by z/OSMF and other mainframe utilities. If no encoding is provided, we convert the contents from `UTF-8` to the system default codepage.

```jsonc
{
  "command": "writeFile",
  "path": "/u/users/you/file.txt",
  "encoding": "IBM-939",
  "contents": [
    165, 131, 191, 165, 132, 192, 165, 131, 171, 165, 131, 161, 165, 131, 175,
    33
  ] // "Hello!" in Japanese, encoded as UTF-8
}
```

Response:

```json
{
  "success": true
}
```

### Data transmission

To send and receive converted contents between `ioserver` and `zowex`, we pipe the bytes of a write request to `zowex` and interpret the stdout from `zowex` during a read request. The environment variable `_BPXK_AUTOCVT` is temporarily disabled within the command handlers during write operations to prevent additional conversions of piped data.
