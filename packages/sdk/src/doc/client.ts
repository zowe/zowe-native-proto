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

export interface ClientOptions {
    /**
     * Number of seconds between keep-alive messages
     * (default: 30)
     */
    keepaliveInterval?: number;

    /**
     * Function called when the connection is closed
     */
    onClose?: () => void | Promise<void>;

    /**
     * Function called when there is a connection error
     */
    onError?: (error: Error) => void | Promise<void>;

    /**
     * Number of workers to spawn
     */
    numWorkers?: number;

    /**
     * Number of seconds to wait for a response
     * (default: 60)
     */
    responseTimeout?: number;

    /**
     * Remote path of the Zowe Native Proto server
     * (default: `~/.zowe-server`)
     */
    serverPath?: string;
}
