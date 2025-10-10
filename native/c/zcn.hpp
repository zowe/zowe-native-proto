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

#ifndef ZCN_HPP
#define ZCN_HPP

#include <cstring>
#include <string>

#include "zcntype.h"

/**
 * @brief Activate extended console
 * @note Prefer `ZcnSession` for RAII-managed usage. When using this API directly you MUST call `zcn_deactivate` to prevent resource leaks.
 * @param zcn extended console returned attributes and error information
 * @param name console name, max of 8 characters e.g. MYCONSOL
 * @return int 0 for success; non zero otherwise
 */
int zcn_activate(ZCN *zcn, std::string name);

/**
 * @brief Deactivate extended console
 *
 * @param zcn extended console returned attributes and error information
 * @return int 0 for success; non zero otherwise
 */
int zcn_deactivate(ZCN *zcn);

/**
 * @brief Write command to extended console
 *
 * @param zcn extended console returned attributes and error information
 * @param command command to run, e.g. D IPLINFO
 * @return int
 */
int zcn_put(ZCN *zcn, std::string command);

/**
 * @brief Obtain data from extended console
 *
 * @param zcn extended console returned attributes and error information
 * @param response command response
 * @return int
 */
int zcn_get(ZCN *zcn, std::string &response);

// Lightweight RAII helper that ensures `zcn_deactivate` is invoked.
class ZcnSession
{
public:
  ZcnSession()
      : active_(false), zcn_()
  {
  }

  ~ZcnSession()
  {
    deactivate();
  }

  ZcnSession(const ZcnSession &) = delete;
  ZcnSession &operator=(const ZcnSession &) = delete;
  ZcnSession(ZcnSession &&) = delete;
  ZcnSession &operator=(ZcnSession &&) = delete;

  int activate(const std::string &name)
  {
    if (active_)
    {
      int rc = deactivate();
      if (0 != rc)
      {
        return rc;
      }
    }

    int rc = zcn_activate(&zcn_, name);
    active_ = (0 == rc);
    return rc;
  }

  int deactivate()
  {
    if (!active_)
    {
      return 0;
    }

    int rc = zcn_deactivate(&zcn_);
    active_ = false;
    return rc;
  }

  int put(const std::string &command)
  {
    return zcn_put(&zcn_, command);
  }

  int get(std::string &response)
  {
    return zcn_get(&zcn_, response);
  }

  bool is_active() const
  {
    return active_;
  }

  ZCN &control_block()
  {
    return zcn_;
  }

  const ZCN &control_block() const
  {
    return zcn_;
  }

private:
  ZCN zcn_;
  bool active_;
};

#endif
