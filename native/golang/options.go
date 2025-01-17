package main

type ListOptions struct {
	MaxItems        int `json:"maxItems,omitempty"`
	ResponseTimeout int `json:"responseTimeout,omitempty"`
}

type ListDatasetOptions struct {
	Start string `json:"start"`
}
