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
#include <string>

namespace zstd
{

// Unexpected type for `expected`
template <typename E>
class unexpected
{
public:
  typedef E error_type;

  explicit unexpected(const E &e)
      : m_error(e)
  {
  }

  const E &error() const
  {
    return m_error;
  }
  E &error()
  {
    return m_error;
  }

private:
  E m_error;
};

template <typename E>
unexpected<E> make_unexpected(const E &e)
{
  return unexpected<E>(e);
}

// Backport of C++23 `std::expected`
template <typename T, typename E>
class expected
{
public:
  typedef T value_type;
  typedef E error_type;

  expected()
      : m_has_value(true)
  {
    construct_value();
  }

  expected(const expected &other)
      : m_has_value(other.m_has_value)
  {
    if (m_has_value)
    {
      construct_value(other.value());
    }
    else
    {
      construct_error(other.error());
    }
  }

  expected(const T &value)
      : m_has_value(true)
  {
    construct_value(value);
  }

  expected(const unexpected<E> &unexp)
      : m_has_value(false)
  {
    construct_error(unexp.error());
  }

  ~expected()
  {
    destroy();
  }

  expected &operator=(const expected &other)
  {
    if (this != &other)
    {
      if (m_has_value && other.m_has_value)
      {
        value() = other.value();
      }
      else if (!m_has_value && !other.m_has_value)
      {
        error() = other.error();
      }
      else
      {
        destroy();
        m_has_value = other.m_has_value;
        if (m_has_value)
        {
          construct_value(other.value());
        }
        else
        {
          construct_error(other.error());
        }
      }
    }
    return *this;
  }

  expected &operator=(const T &val)
  {
    if (m_has_value)
    {
      value() = val;
    }
    else
    {
      destroy();
      m_has_value = true;
      construct_value(val);
    }
    return *this;
  }

  expected &operator=(const unexpected<E> &unexp)
  {
    if (m_has_value)
    {
      destroy();
      m_has_value = false;
      construct_error(unexp.error());
    }
    else
    {
      error() = unexp.error();
    }
    return *this;
  }

  const T *operator->() const
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return get_value_ptr();
  }

  T *operator->()
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return get_value_ptr();
  }

  const T &operator*() const
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return *get_value_ptr();
  }

  T &operator*()
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return *get_value_ptr();
  }

  bool has_value() const
  {
    return m_has_value;
  }

  explicit operator bool() const
  {
    return m_has_value;
  }

  const T &value() const
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return *get_value_ptr();
  }

  T &value()
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return *get_value_ptr();
  }

  const E &error() const
  {
    if (has_value())
    {
      throw std::logic_error("expected contains value, not error");
    }
    return *get_error_ptr();
  }

  E &error()
  {
    if (has_value())
    {
      throw std::logic_error("expected contains value, not error");
    }
    return *get_error_ptr();
  }

  template <typename U>
  T value_or(const U &default_value) const
  {
    return has_value() ? value() : static_cast<T>(default_value);
  }

  void swap(expected &other)
  {
    if (has_value() && other.has_value())
    {
      using std::swap;
      swap(value(), other.value());
    }
    else if (!has_value() && !other.has_value())
    {
      using std::swap;
      swap(error(), other.error());
    }
    else if (has_value())
    {
      E temp_error = other.error();
      other.destroy();
      other.m_has_value = true;
      other.construct_value(value());
      destroy();
      m_has_value = false;
      construct_error(temp_error);
    }
    else
    {
      T temp_value = other.value();
      other.destroy();
      other.m_has_value = false;
      other.construct_error(error());
      destroy();
      m_has_value = true;
      construct_value(temp_value);
    }
  }

private:
  void construct_value()
  {
    new (m_storage) T();
  }

  void construct_value(const T &val)
  {
    new (m_storage) T(val);
  }

  void construct_error(const E &err)
  {
    new (m_storage) E(err);
  }

  void destroy()
  {
    if (m_has_value)
    {
      get_value_ptr()->~T();
    }
    else
    {
      get_error_ptr()->~E();
    }
  }

  const T *get_value_ptr() const
  {
    return reinterpret_cast<const T *>(m_storage);
  }

  T *get_value_ptr()
  {
    return reinterpret_cast<T *>(m_storage);
  }

  const E *get_error_ptr() const
  {
    return reinterpret_cast<const E *>(m_storage);
  }

  E *get_error_ptr()
  {
    return reinterpret_cast<E *>(m_storage);
  }

private:
  bool m_has_value;
  char m_storage[sizeof(T) > sizeof(E) ? sizeof(T) : sizeof(E)]
      __attribute__((aligned(__alignof__(T) > __alignof__(E) ? __alignof__(T) : __alignof__(E))));
};

template <typename T, typename E>
void swap(expected<T, E> &a, expected<T, E> &b)
{
  a.swap(b);
}

template <typename T, typename E>
bool operator==(const expected<T, E> &lhs, const expected<T, E> &rhs)
{
  if (lhs.has_value() != rhs.has_value())
    return false;
  return lhs.has_value() ? (lhs.value() == rhs.value()) : (lhs.error() == rhs.error());
}

template <typename T, typename E>
bool operator!=(const expected<T, E> &lhs, const expected<T, E> &rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename E>
bool operator==(const expected<T, E> &lhs, const T &rhs)
{
  return lhs.has_value() && (lhs.value() == rhs);
}

template <typename T, typename E>
bool operator==(const T &lhs, const expected<T, E> &rhs)
{
  return rhs.has_value() && (lhs == rhs.value());
}

template <typename T, typename E>
bool operator!=(const expected<T, E> &lhs, const T &rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename E>
bool operator!=(const T &lhs, const expected<T, E> &rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename E>
bool operator==(const expected<T, E> &lhs, const unexpected<E> &rhs)
{
  return !lhs.has_value() && (lhs.error() == rhs.error());
}

template <typename T, typename E>
bool operator==(const unexpected<E> &lhs, const expected<T, E> &rhs)
{
  return !rhs.has_value() && (lhs.error() == rhs.error());
}

template <typename T, typename E>
bool operator!=(const expected<T, E> &lhs, const unexpected<E> &rhs)
{
  return !(lhs == rhs);
}

template <typename T, typename E>
bool operator!=(const unexpected<E> &lhs, const expected<T, E> &rhs)
{
  return !(lhs == rhs);
}

} // namespace zstd

#endif // ZSTD_HPP
