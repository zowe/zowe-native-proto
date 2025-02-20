package cmds

// InitializeCoreHandlers initializes the built-in command handlers
func InitializeCoreHandlers(disp *CmdDispatcher) {
	handlers := map[string]CommandHandler{
		"chownFile":      HandleChownFileRequest,
		"chmodFile":      HandleChmodFileRequest,
		"chtagFile":      HandleChtagFileRequest,
		"consoleCommand": HandleConsoleCommandRequest,
		"createFile":     HandleCreateFileRequest,
		"deleteDataset":  HandleDeleteDatasetRequest,
		"deleteFile":     HandleDeleteFileRequest,
		"getJcl":         HandleGetJclRequest,
		"getJobStatus":   HandleGetStatusRequest,
		"listDatasets":   HandleListDatasetsRequest,
		"listDsMembers":  HandleListDsMembersRequest,
		"listFiles":      HandleListFilesRequest,
		"listJobs":       HandleListJobsRequest,
		"listSpools":     HandleListSpoolsRequest,
		"readDataset":    HandleReadDatasetRequest,
		"readFile":       HandleReadFileRequest,
		"readSpool":      HandleReadSpoolRequest,
		"restoreDataset": HandleRestoreDatasetRequest,
		"submitJob":      HandleSubmitJobRequest,
		"submitJcl":      HandleSubmitJclRequest,
		"writeDataset":   HandleWriteDatasetRequest,
		"writeFile":      HandleWriteFileRequest,
	}

	for cmd, handler := range handlers {
		disp.MustRegister(cmd, handler)
	}
}
