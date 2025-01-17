package main

type Dataset struct {
	Name  string `json:"name"`
	Dsorg string `json:"dsorg"`
}

type DsMember struct {
	Name string `json:"name"`
}

type UssItem struct {
	Name string `json:"name"`
}

type Job struct {
	Id      string `json:"id"`
	Name    string `json:"name"`
	Status  string `json:"status"`
	Retcode string `json:"retcode"`
}
