package main

import (
  "encoding/base64"
  "bufio"
  "fmt"
  "os"
)

func main() {
  reader := bufio.NewReader(os.Stdin)
  fmt.Println("Started... ")
  text, _ := reader.ReadString('\n')
  fmt.Printf("Input was '%s'", text)

  // encodedStr := "MTIzNDU="
  decodedBytes, err := base64.StdEncoding.DecodeString(text)
  if err != nil {
    fmt.Println("Error:", err)
    return
  }
  fmt.Printf("Raw '%x'\n", decodedBytes)
  fmt.Println("String '%s'", string(decodedBytes))
  // reader := bufio.NewReader(os.Stdin)

  // for {
  //   fmt.Print("Enter text (type 'quit' to exit): ")
  //   input, _ := reader.ReadString('\n')
  //   input = strings.TrimSpace(input)

  //   if input == "quit" {
  //     break
  //   }

  //   fmt.Println("You entered:", input)
  // }

  fmt.Println("...ended!")

}
