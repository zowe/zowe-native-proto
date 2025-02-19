package utils

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	t "zowe-native-proto/ioserver/types/common"
)

var logFile *os.File

func LogError(format string, args ...any) {
	_, err := logFile.WriteString(fmt.Sprintf(format, args))
	_, err = logFile.WriteString("\n")
	if err != nil {
		log.Fatalln("Failed to write to log file:", err)
	}
}

func PrintErrorResponse(format string, args ...any) {
	LogError(format, args)
	errResponse := t.ErrorResponse{
		Msg: fmt.Sprintf(format, args),
	}
	out, _ := json.Marshal(errResponse)
	fmt.Fprintf(os.Stderr, string(out))
}

func InitLogger() {
	file, err := os.OpenFile("./ioserver.log", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Fatalln("Failed to initialize logger:", err)
		return
	}

	logFile = file
}
