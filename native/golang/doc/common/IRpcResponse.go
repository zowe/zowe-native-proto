// This file was generated from JSON Schema using quicktype, do not modify it directly.
// To parse and unparse this JSON data, add this code to your project and do:
//
//    iRPCResponse, err := UnmarshalIRPCResponse(bytes)
//    bytes, err = iRPCResponse.Marshal()
//
//    iRPCRequest, err := UnmarshalIRPCRequest(bytes)
//    bytes, err = iRPCRequest.Marshal()

package doc

import "encoding/json"

func UnmarshalIRPCResponse(data []byte) (IRPCResponse, error) {
	var r IRPCResponse
	err := json.Unmarshal(data, &r)
	return r, err
}

func (r *IRPCResponse) Marshal() ([]byte, error) {
	return json.Marshal(r)
}

func UnmarshalIRPCRequest(data []byte) (IRPCRequest, error) {
	var r IRPCRequest
	err := json.Unmarshal(data, &r)
	return r, err
}

func (r *IRPCRequest) Marshal() ([]byte, error) {
	return json.Marshal(r)
}

// Interface for RPC response objects
type IRPCResponse struct {
	Success *bool `json:"success,omitempty"`
}

// Interface for RPC request objects
type IRPCRequest struct {
	Command string `json:"command"`
}
