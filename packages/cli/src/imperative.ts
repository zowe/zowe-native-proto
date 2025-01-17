/*
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

import { IImperativeConfig } from "@zowe/imperative";

const config: IImperativeConfig = {
  commandModuleGlobs: ["*/*.definition!(.d).*s"],
  rootCommandDescription: "Hello world",
  productDisplayName: "z/OS SSH Plug-in",
  name: "zowe-native-proto",
  pluginAliases: ["zssh"],
  pluginSummary: "z/OS Files and jobs via SSH"
};

export = config;
