// This file was generated from JSON Schema using quicktype, do not modify it directly.
// To parse and unparse this JSON data, add this code to your project and do:
//
//    request, err := UnmarshalRequest(bytes)
//    bytes, err = request.Marshal()

package doc

import "encoding/json"

func UnmarshalRequest(data []byte) (Request, error) {
	var r Request
	err := json.Unmarshal(data, &r)
	return r, err
}

func (r *Request) Marshal() ([]byte, error) {
	return json.Marshal(r)
}

// Request to list datasets matching a pattern
type Request struct {
	// Whether to include dataset attributes       
	Attributes                              *bool  `json:"attributes,omitempty"`
	// Dataset name pattern to match               
	Pattern                                 string `json:"pattern"`
}
