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

#ifndef ZSTD_HPP
#define ZSTD_HPP

#include <stdexcept>
#include <algorithm>

namespace zstd
{

template <typename T>
class optional
{
public:
  typedef T value_type;

  optional()
  {
  }

  explicit optional(const T &value)
  {
    construct(value);
  }

  optional(const optional<T> &other)
      : m_has_value(false)
  {
    if (other.has_value())
    {
      construct(*other);
    }
  }

  ~optional()
  {
    destroy();
  }

  optional<T> &operator=(const optional<T> &other)
  {
    if (this != &other)
    {
      if (has_value() && other.has_value())
      {
        **this = *other;
      }
      else
      {
        destroy();
        if (other.has_value())
        {
          construct(*other);
        }
      }
    }
    return *this;
  }

  optional<T> &operator=(const T &value)
  {
    if (has_value())
    {
      **this = value;
    }
    else
    {
      construct(value);
    }
    return *this;
  }

  void swap(optional<T> &other)
  {
    if (has_value() && other.has_value())
    {
      using std::swap;
      swap(**this, *other);
    }
    else if (has_value())
    {
      other.construct(**this);
      destroy();
    }
    else if (other.has_value())
    {
      construct(*other);
      other.destroy();
    }
  }

  const T *operator->() const
  {
    if (!has_value())
    {
      throw std::logic_error("optional is not initialized");
    }
    return get_ptr();
  }

  T *operator->()
  {
    if (!has_value())
    {
      throw std::logic_error("optional is not initialized");
    }
    return get_ptr();
  }

  const T &operator*() const
  {
    if (!has_value())
    {
      throw std::logic_error("optional is not initialized");
    }
    return *get_ptr();
  }

  T &operator*()
  {
    if (!has_value())
    {
      throw std::logic_error("optional is not initialized");
    }
    return *get_ptr();
  }

  bool has_value() const
  {
    return m_has_value;
  }

  operator bool() const
  {
    return m_has_value;
  }

  T &value()
  {
    if (!m_has_value)
    {
      throw std::logic_error("optional does not contain a value");
    }
    return *get_ptr();
  }

  const T &value() const
  {
    if (!m_has_value)
    {
      throw std::logic_error("optional does not contain a value");
    }
    return *get_ptr();
  }

  void reset()
  {
    destroy();
  }

private:
  void construct(const T &val)
  {
    new (m_storage) T(val);
    m_has_value = true;
  }

  void destroy()
  {
    if (m_has_value)
    {
      get_ptr()->~T();
      m_has_value = false;
    }
  }

  const T *get_ptr() const
  {
    return reinterpret_cast<const T *>(m_storage);
  }

  T *get_ptr()
  {
    return reinterpret_cast<T *>(m_storage);
  }

private:
  bool m_has_value = false;
  char m_storage[sizeof(T)] __attribute__((aligned(__alignof__(T)))) = {};
};

} // namespace zstd

#endif // ZSTD_HPP
