package main

import (
	"log"
	"strconv"
	"strings"
)

func collectContentsAsBytes(input string, isByteString bool) []byte {
	var data []byte

	if isByteString {
		data_split := strings.Split(string(input), " ")
		for _, b := range data_split[:len(data_split)-1] {
			byteNum, err := strconv.ParseUint(b, 16, 8)
			if err != nil {
				log.Println("Error parsing byte:", err)
				continue
			}
			data = append(data, byte(byteNum))
		}
	} else {
		data = []byte(input)
	}

	return data
}
