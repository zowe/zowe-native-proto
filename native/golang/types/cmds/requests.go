package cmds

type IssueConsoleRequest struct {
	Command     string `json:"command" tstype:"\"consoleCommand\""`
	CommandText string `json:"commandText"`
	ConsoleName string `json:"consoleName"`
}
