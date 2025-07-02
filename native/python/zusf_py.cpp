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

#include "zusf_py.h"

std::string list_uss_dir(const std::string &path)
{
  ZUSF ctx = {0};
  std::string out;
  if (zusf_list_uss_file_path(&ctx, path.c_str(), out) != 0)
  {
    std::string error_msg = ctx.diag.e_msg;
    throw std::runtime_error(error_msg);
  }
  return out;
}