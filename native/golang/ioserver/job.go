package main

import "encoding/json"
import "fmt"
import "os/exec"
import "log"
import "strings"

func HandleListJobRequest(jsonData []byte) {
	var listRequest ListJobsRequest
	err := json.Unmarshal(jsonData, &listRequest)
	if err != nil {
		// log.Println("Error decoding ListDatasetsRequest:", err)
		return
	}

	args := []string{"./zowex", "job", "list"}
	if len(listRequest.Owner) != 0 {
		args = append(args, "--owner", listRequest.Owner)
	}

	out, err := exec.Command(args[0], args[1:]...).Output()
	if err != nil {
		log.Println("Error executing command:", err)
		return
	}

	jobsResponse := ListJobsResponse{
		Items: []Job{},
	}

	items := strings.Split(string(out), "\n")

	for _, item := range items {
		vals := strings.Split(item, " ")
		jobsResponse.Items = append(jobsResponse.Items, Job{
			Id: vals[0],
			Retcode: vals[1],
			Name: vals[2],
			Status: vals[3],
		})
	}

	v, err := json.Marshal(jobsResponse)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		fmt.Println(string(v))
	}
}