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

#include "zcli.hpp"

ZCLIOption ZCLIOptionProvider::option_not_found = ZCLIOption("not found");
ZCLIPositional ZCLIPositionalProvider::positional_not_found = ZCLIPositional("not found");
ZCLIVerb ZCLIGroup::verb_not_found = ZCLIVerb("not found");
ZCLIGroup ZCLI::group_not_found = ZCLIGroup("not found");