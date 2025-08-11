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

/**
 * For a detailed explanation regarding each configuration property, visit:
 * https://vitest.dev/config/
 */

import { defineConfig } from "vitest/config";

export default defineConfig({
    test: {
        globals: true,
        environment: "node",
        coverage: {
            provider: "v8",
            reporter: ["text", "lcov", "html"],
            exclude: ["lib/**", "test/**", "**/*.d.ts", "**/*.test.ts", "**/node_modules/**"],
        },
        include: ["test/**/*.test.ts"],
        testTimeout: 10000,
        hookTimeout: 10000,
        clearMocks: true,
        restoreMocks: true,
    },
    esbuild: {
        target: "es2022",
    },
});
