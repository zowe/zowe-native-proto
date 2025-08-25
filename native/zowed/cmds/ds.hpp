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

#include "../extern/picojson.h"

using picojson::value;

// Dataset command handlers
picojson::value HandleReadDatasetRequest(const picojson::value &params);
picojson::value HandleWriteDatasetRequest(const picojson::value &params);
picojson::value HandleListDatasetsRequest(const picojson::value &params);
picojson::value HandleListDsMembersRequest(const picojson::value &params);
