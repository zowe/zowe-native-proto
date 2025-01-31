/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

package main

import (
	"fmt"
	"os"

	"golang.org/x/term"
)

func main() {
	// Set stdin to raw mode
	oldState, err := term.MakeRaw(int(os.Stdin.Fd()))
	if err != nil {
		fmt.Println("Error setting raw mode:", err)
		return
	}
	defer term.Restore(int(os.Stdin.Fd()), oldState)

	// Set stdin to raw mode
	oldState2, err := term.MakeRaw(int(os.Stdout.Fd()))
	if err != nil {
		fmt.Println("Error setting raw mode:", err)
		return
	}
	defer term.Restore(int(os.Stdout.Fd()), oldState2)

	// Read 5 bytes from stdin
	buffer := make([]byte, 256)
	for i := 0; i < 256; i++ {
		_, err := os.Stdin.Read(buffer[i : i+1])
		if err != nil {
			fmt.Println("Error reading from stdin:", err)
			return
		}
	}

	os.Stdout.Write(buffer)
	os.Stdout.Sync()
	// Write bytes to file
	err = os.WriteFile("output.txt", []byte(fmt.Sprintln(buffer)), 0644)
	if err != nil {
		fmt.Println("Error writing to file:", err)
		return
	}

	// fmt.Println("\nBytes written to output.txt")
}

// func main() {
// 	fd := int(os.Stdout.Fd())

// 	state, err := term.GetState(fd)
// 	if err != nil {
// 		fmt.Fprintln(os.Stderr, "Error getting terminal attributes:", err)
// 		return
// 	}

// 	file, err := os.OpenFile("/u/users/timothy/tty_flags.txt", os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0644)
// 	if err != nil {
// 		fmt.Fprintln(os.Stderr, "Error opening file:", err)
// 		return
// 	}
// 	defer file.Close()

// 	fmt.Fprintf(file, "%+v\n", state)
// 	fmt.Println("Hello World")
// }
