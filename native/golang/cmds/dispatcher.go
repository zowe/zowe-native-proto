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

package cmds

import (
	"fmt"
	"sync"
	"zowe-native-proto/ioserver/utils"
)

// CommandHandler represents a function that handles a command request
type CommandHandler func([]byte)

// CmdDispatcher manages registration and lookup of command handlers
type CmdDispatcher struct {
	handlers sync.Map
}

// NewDispatcher creates a new CmdDispatcher instance
func NewDispatcher() *CmdDispatcher {
	return &CmdDispatcher{}
}

// Register adds a new command handler to the dispatcher
func (r *CmdDispatcher) Register(command string, handler CommandHandler) error {
	if _, loaded := r.handlers.LoadOrStore(command, handler); loaded {
		return fmt.Errorf("Handler already registered for command: %s", command)
	}
	return nil
}

// Get retrieves a command handler from the dispatcher
func (r *CmdDispatcher) Get(command string) (CommandHandler, bool) {
	handler, ok := r.handlers.Load(command)
	if !ok {
		return nil, false
	}
	return handler.(CommandHandler), true
}

// MustRegister registers a handler and prints to stderr if registration fails
func (r *CmdDispatcher) MustRegister(command string, handler CommandHandler) {
	if err := r.Register(command, handler); err != nil {
		utils.PrintErrorResponse("Failed to register command %s: %v", command, err)
	}
}
