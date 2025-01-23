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
	"context"
	"encoding/json"
	"fmt"
	"io"
	"os"

	"github.com/sourcegraph/jsonrpc2"
)

type Arith struct{}

type AddArgs struct {
	A int `json:"a"`
	B int `json:"b"`
}

type AddResult struct {
	Sum int `json:"sum"`
}

func (a *Arith) Add(args AddArgs, result *AddResult) error {
	result.Sum = args.A + args.B
	return nil
}

type Handler struct{}

func (h *Handler) Handle(ctx context.Context, conn *jsonrpc2.Conn, req *jsonrpc2.Request) {
	switch req.Method {
	case "Arith.Add":
		var params AddArgs
		if err := json.Unmarshal(*req.Params, &params); err != nil {
			conn.ReplyWithError(ctx, req.ID, &jsonrpc2.Error{
				Code:    jsonrpc2.CodeInvalidParams,
				Message: "Invalid parameters",
			})
			return
		}
		var result AddResult
		if err := (&Arith{}).Add(params, &result); err != nil {
			conn.ReplyWithError(ctx, req.ID, &jsonrpc2.Error{
				Code:    jsonrpc2.CodeInternalError,
				Message: err.Error(),
			})
			return
		}
		conn.Reply(ctx, req.ID, result)
	default:
		conn.ReplyWithError(ctx, req.ID, &jsonrpc2.Error{
			Code:    jsonrpc2.CodeMethodNotFound,
			Message: "Method not found",
		})
	}
}

type ReadWriteCloser struct {
	io.ReadCloser
	io.WriteCloser
}

func (rwc ReadWriteCloser) Close() error {
	var err error
	if rwc.ReadCloser != nil {
		err = rwc.ReadCloser.Close()
	}
	if rwc.WriteCloser != nil {
		err = rwc.WriteCloser.Close()
	}
	return err
}

func main() {
	stream := jsonrpc2.NewPlainObjectStream(ReadWriteCloser{
		ReadCloser:  os.Stdin,
		WriteCloser: os.Stdout,
	})
	conn := jsonrpc2.NewConn(context.Background(), stream, &Handler{})
	fmt.Fprintln(os.Stderr, "JSON-RPC server started...")
	<-conn.DisconnectNotify()
	fmt.Fprintln(os.Stderr, "JSON-RPC server stopped.")
}
