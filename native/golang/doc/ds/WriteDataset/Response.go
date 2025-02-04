// This file was generated from JSON Schema using quicktype, do not modify it directly.
// To parse and unparse this JSON data, add this code to your project and do:
//
//    response, err := UnmarshalResponse(bytes)
//    bytes, err = response.Marshal()

package doc

import "encoding/json"

func UnmarshalResponse(data []byte) (Response, error) {
	var r Response
	err := json.Unmarshal(data, &r)
	return r, err
}

func (r *Response) Marshal() ([]byte, error) {
	return json.Marshal(r)
}

// Response from writing to a dataset
//
// Interface for RPC response objects
type Response struct {
	// Dataset name that was written to                 
	Dsname                                       string `json:"dsname"`
	// Whether the write operation was successful       
	Success                                      bool   `json:"success"`
}
