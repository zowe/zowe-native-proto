package cmds

// initializeDatasetHandlers initializes Data Set (MVS) command handlers
func initializeDatasetHandlers(disp *CmdDispatcher) {
	handlers := map[string]CommandHandler{
		"deleteDataset":  HandleDeleteDatasetRequest,
		"listDatasets":   HandleListDatasetsRequest,
		"listDsMembers":  HandleListDsMembersRequest,
		"readDataset":    HandleReadDatasetRequest,
		"restoreDataset": HandleRestoreDatasetRequest,
		"writeDataset":   HandleWriteDatasetRequest,
	}
	registerHandlers(disp, handlers)
}

// initializeUnixHandlers initializes Unix (USS) command handlers
func initializeUnixHandlers(disp *CmdDispatcher) {
	handlers := map[string]CommandHandler{
		"chownFile":  HandleChownFileRequest,
		"chmodFile":  HandleChmodFileRequest,
		"chtagFile":  HandleChtagFileRequest,
		"createFile": HandleCreateFileRequest,
		"deleteFile": HandleDeleteFileRequest,
		"listFiles":  HandleListFilesRequest,
		"readFile":   HandleReadFileRequest,
		"writeFile":  HandleWriteFileRequest,
	}
	registerHandlers(disp, handlers)
}

// initializeJobHandlers initializes job (JES) command handlers
func initializeJobHandlers(disp *CmdDispatcher) {
	handlers := map[string]CommandHandler{
		"getJcl":       HandleGetJclRequest,
		"getJobStatus": HandleGetStatusRequest,
		"listJobs":     HandleListJobsRequest,
		"listSpools":   HandleListSpoolsRequest,
		"readSpool":    HandleReadSpoolRequest,
		"submitJob":    HandleSubmitJobRequest,
		"submitJcl":    HandleSubmitJclRequest,
	}
	registerHandlers(disp, handlers)
}

// initializeConsoleHandlers initializes console command handlers
func initializeConsoleHandlers(disp *CmdDispatcher) {
	disp.MustRegister("consoleCommand", HandleConsoleCommandRequest)
}

// registerHandlers registers each handler in the given map with the dispatcher
func registerHandlers(disp *CmdDispatcher, handlers map[string]CommandHandler) {
	for cmd, handler := range handlers {
		disp.MustRegister(cmd, handler)
	}
}

// InitializeCoreHandlers initializes the built-in command handlers
func InitializeCoreHandlers(disp *CmdDispatcher) {
	initializeDatasetHandlers(disp)
	initializeUnixHandlers(disp)
	initializeJobHandlers(disp)
	initializeConsoleHandlers(disp)
}
