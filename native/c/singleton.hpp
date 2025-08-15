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

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

template <typename T>
class Singleton
{
public:
  static T &get_instance()
  {
    static T instance;
    return instance;
  }

protected:
  Singleton()
  {
  }
  ~Singleton()
  {
  }

  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;

  Singleton(Singleton &&) = delete;
  Singleton &operator=(Singleton &&) = delete;
};

#endif