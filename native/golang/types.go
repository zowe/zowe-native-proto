package main

type Dataset struct {
	Name   string `json:"name"`
	Dsorg  string `json:"dsorg"`
	Volser string `json:"volser"`
}

type DsMember struct {
	Name string `json:"name"`
}

type UssItem struct {
	Name  string `json:"name"`
	IsDir bool   `json:"isDir"`
}

type Job struct {
	Id      string `json:"id"`
	Name    string `json:"name"`
	Status  string `json:"status"`
	Retcode string `json:"retcode"`
}

type Spool struct {
	Id       int    `json:"id"`
	DdName   string `json:"ddname"`
	StepName string `json:"stepname"`
}
