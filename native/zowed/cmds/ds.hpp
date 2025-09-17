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

#pragma once

#include "../../c/zjson.hpp"

using zjson::Value;

// Dataset command handlers
zjson::Value HandleReadDatasetRequest(const zjson::Value &params);
zjson::Value HandleWriteDatasetRequest(const zjson::Value &params);
zjson::Value HandleListDatasetsRequest(const zjson::Value &params);
zjson::Value HandleListDsMembersRequest(const zjson::Value &params);
