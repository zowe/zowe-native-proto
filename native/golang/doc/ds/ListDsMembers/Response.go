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

// Response containing list of dataset members
//
// Interface for RPC response objects
type Response struct {
	Items                        []Item `json:"items"`
	// Number of members returned       
	ReturnedRows                 int64  `json:"returnedRows"`
	Success                      *bool  `json:"success,omitempty"`
}

type Item struct {
	Name string `json:"name"`
}
