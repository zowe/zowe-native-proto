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
#include <utility>

namespace zstd
{

template <typename T>
class unique_ptr
{
public:
  typedef T element_type;

  explicit unique_ptr(element_type *ptr = nullptr)
      : m_ptr(ptr)
  {
  }

  unique_ptr(unique_ptr &&other)
      : m_ptr(other.release())
  {
  }

  ~unique_ptr()
  {
    delete m_ptr;
  }

  unique_ptr &operator=(unique_ptr &&other)
  {
    if (this != &other)
    {
      reset(other.release());
    }
    return *this;
  }

  element_type &operator*() const
  {
    return *m_ptr;
  }

  element_type *operator->() const
  {
    return m_ptr;
  }

  element_type *get() const
  {
    return m_ptr;
  }

  explicit operator bool() const
  {
    return m_ptr != nullptr;
  }

  element_type *release()
  {
    element_type *ptr = m_ptr;
    m_ptr = nullptr;
    return ptr;
  }

  void reset(element_type *ptr = nullptr)
  {
    if (ptr != m_ptr)
    {
      delete m_ptr;
      m_ptr = ptr;
    }
  }

  void swap(unique_ptr &other)
  {
    using std::swap;
    swap(m_ptr, other.m_ptr);
  }

private:
  unique_ptr(const unique_ptr &);
  unique_ptr &operator=(const unique_ptr &);

  element_type *m_ptr;
};

template <typename T>
void swap(unique_ptr<T> &a, unique_ptr<T> &b)
{
  a.swap(b);
}

template <typename T>
unique_ptr<T> make_unique()
{
  return unique_ptr<T>(new T());
}

template <typename T, typename A1>
unique_ptr<T> make_unique(const A1 &a1)
{
  return unique_ptr<T>(new T(a1));
}

template <typename T, typename A1, typename A2>
unique_ptr<T> make_unique(const A1 &a1, const A2 &a2)
{
  return unique_ptr<T>(new T(a1, a2));
}

template <typename T>
class optional
{
public:
  typedef T value_type;

  optional()
  {
    m_has_value = false;
    memset(m_storage, 0, sizeof(T));
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
  bool m_has_value;
  char m_storage[sizeof(T)] __attribute__((aligned(__alignof__(T))));
};

} // namespace zstd

#endif // ZSTD_HPP
