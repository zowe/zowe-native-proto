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

#ifndef ZJSON_HPP
#define ZJSON_HPP

#include <iostream>
#include <string>
#include <sstream>
#include "zjsonm.h"

class ZJson
{

private:
  JSON_INSTANCE instance;

public:
  ZJson()
  {
    std::cout << "ZJson constructor" << std::endl;
    ZJSMINIT(&instance);
  }
  ~ZJson()
  {
    std::cout << "ZJson destructor" << std::endl;
    ZJSMTERM(&instance);
  }
  void parse(const std::string &json)
  {
    int rc = 0;
    if (instance.json != nullptr)
    {
      ZJSMTERM(&instance);
      ZJSMINIT(&instance);
    }
    rc = ZJSMPARS(&instance, json.c_str());
    if (0 != rc)
    {
      std::stringstream ss;
      ss << std::hex << rc;
      throw std::runtime_error("Error parsing JSON rc was x'" + ss.str() + "'");
    }
  }
  // void init();
  // void parse(const std::string &json);
  // void serialize(const std::string &json);
  // void print();
};

#endif
