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

import { AbstractRpcClient } from "../src/AbstractRpcClient";

class DummyRpcClient extends AbstractRpcClient {
    public request = jest.fn();
}

describe("AbstractRpcClient", () => {
    it("defines ds methods", () => {
        const rpcClient = new DummyRpcClient();
        expect(Object.keys(rpcClient.ds)).toMatchSnapshot();
    });

    it("defines jobs methods", () => {
        const rpcClient = new DummyRpcClient();
        expect(Object.keys(rpcClient.jobs)).toMatchSnapshot();
    });

    it("defines uss methods", () => {
        const rpcClient = new DummyRpcClient();
        expect(Object.keys(rpcClient.uss)).toMatchSnapshot();
    });

    it("defines cmds methods", () => {
        const rpcClient = new DummyRpcClient();
        expect(Object.keys(rpcClient.cmds)).toMatchSnapshot();
    });
});
