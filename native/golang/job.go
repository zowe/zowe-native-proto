package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os/exec"
	"strconv"
	"strings"
)

func HandleListJobsRequest(jsonData []byte) {
	var listRequest ListJobsRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListDatasetsRequest:", err)
		return
	}

	args := []string{"./zowex", "job", "list", "--rfc", "1"}
	if len(listRequest.Owner) != 0 {
		args = append(args, "--owner", listRequest.Owner)
	}

	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	jobs := strings.Split(strings.TrimSpace(string(out)), "\n")

	jobsResponse := ListJobsResponse{
		Items: make([]Job, len(jobs)),
	}

	for i, job := range jobs {
		vals := strings.Split(job, ",")
		if len(vals) < 4 {
			continue
		}
		jobsResponse.Items[i] = Job{
			Id:      vals[0],
			Retcode: vals[1],
			Name:    strings.TrimSpace(vals[2]),
			Status:  vals[3],
		}
	}

	v, err := json.Marshal(jobsResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleListSpoolsRequest(jsonData []byte) {
	var listRequest ListSpoolsRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListSpoolsRequest:", err)
		return
	}

	args := []string{"./zowex", "job", "list-files", listRequest.JobId, "--rfc", "1"}

	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	spools := strings.Split(strings.TrimSpace(string(out)), "\n")

	response := ListSpoolsResponse{
		Items: make([]Spool, len(spools)),
	}

	for i, spool := range spools {
		vals := strings.Split(spool, ",")
		if len(vals) < 4 {
			continue
		}
		response.Items[i] = Spool{
			Id:       strings.TrimSpace(vals[2]),
			DdName:   vals[0],
			StepName: vals[3],
		}
	}

	v, err := json.Marshal(response)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleReadSpoolRequest(jsonData []byte) {
	var request ReadSpoolRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil {
		// log.Println("Error decoding ReadSpoolRequest:", err)
		return
	}
	// log.Println("ReadSpoolRequest received:", ...)
	args := []string{"./zowex", "job", "view-file", request.JobId, strconv.Itoa(request.DsnKey)}
	hasEncoding := len(request.Encoding) != 0
	if hasEncoding {
		args = append(args, "--encoding", request.Encoding)
	}
	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	data := collectContentsAsBytes(string(out), hasEncoding)

	response := ReadSpoolResponse{
		Encoding: request.Encoding,
		DsnKey:   request.DsnKey,
		JobId:    request.JobId,
		Data:     data,
	}
	v, err := json.Marshal(response)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}

func HandleGetJclRequest(jsonData []byte) {
	var request GetJclRequest
	err := json.Unmarshal(jsonData, &request)
	if err != nil {
		// log.Println("Error decoding GetJclRequest:", err)
		return
	}
	// log.Println("GetJclRequest received:", ...)
	args := []string{"./zowex", "job", "view-jcl", request.JobId}
	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	response := GetJclResponse{
		JobId: request.JobId,
		Data:  string(out),
	}
	v, err := json.Marshal(response)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}
