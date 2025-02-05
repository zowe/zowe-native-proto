package cmds

type IssueConsoleRequest struct {
	Command     string `json:"command" tstype:"\"consoleCommand\""`
	CommandText string `json:"commandText"`
	ConsoleName string `json:"consoleName"`
}

type IssueTsoRequest struct {
	Command     string `json:"command" tstype:"\"tsoCommand\""`
	CommandText string `json:"commandText"`
}
type IssueUnixRequest struct {
	Command     string `json:"command" tstype:"\"unixCommand\""`
	CommandText string `json:"commandText"`
}
