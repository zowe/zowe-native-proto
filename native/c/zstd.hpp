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
#include <cstring>
#include <new>

namespace zstd
{
template <bool B, class T = void>
struct enable_if
{
};

template <typename T>
struct enable_if<true, T>
{
  typedef T type;
};

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

// Unexpected type for expected
template <typename E>
class unexpected
{
public:
  typedef E error_type;

  explicit unexpected(const E &e) : m_error(e)
  {
  }
  explicit unexpected(E &&e) : m_error(std::move(e))
  {
  }

  const E &error() const &
  {
    return m_error;
  }
  E &error() &
  {
    return m_error;
  }
  const E &&error() const &&
  {
    return std::move(m_error);
  }
  E &&error() &&
  {
    return std::move(m_error);
  }

private:
  E m_error;
};

// Helper function to create unexpected values
template <typename E>
unexpected<E> make_unexpected(const E &e)
{
  return unexpected<E>(e);
}

template <typename E>
unexpected<E> make_unexpected(E &&e)
{
  return unexpected<E>(std::move(e));
}

// Expected type that mimics C++23 std::expected
template <typename T, typename E>
class expected
{
public:
  typedef T value_type;
  typedef E error_type;

  // Default constructor (constructs value)
  expected() : m_has_value(true)
  {
    construct_value();
  }

  // Copy constructor
  expected(const expected &other) : m_has_value(other.m_has_value)
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

  // Move constructor
  expected(expected &&other) : m_has_value(other.m_has_value)
  {
    if (m_has_value)
    {
      construct_value(std::move(other.value()));
    }
    else
    {
      construct_error(std::move(other.error()));
    }
  }

  // Value constructor
  expected(const T &value) : m_has_value(true)
  {
    construct_value(value);
  }

  // Value move constructor
  expected(T &&value) : m_has_value(true)
  {
    construct_value(std::move(value));
  }

  // Error constructor
  expected(const unexpected<E> &unexp) : m_has_value(false)
  {
    construct_error(unexp.error());
  }

  // Error move constructor
  expected(unexpected<E> &&unexp) : m_has_value(false)
  {
    construct_error(std::move(unexp.error()));
  }

  // Destructor
  ~expected()
  {
    destroy();
  }

  // Copy assignment
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

  // Move assignment
  expected &operator=(expected &&other)
  {
    if (this != &other)
    {
      if (m_has_value && other.m_has_value)
      {
        value() = std::move(other.value());
      }
      else if (!m_has_value && !other.m_has_value)
      {
        error() = std::move(other.error());
      }
      else
      {
        destroy();
        m_has_value = other.m_has_value;
        if (m_has_value)
        {
          construct_value(std::move(other.value()));
        }
        else
        {
          construct_error(std::move(other.error()));
        }
      }
    }
    return *this;
  }

  // Value assignment
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

  // Value move assignment
  expected &operator=(T &&val)
  {
    if (m_has_value)
    {
      value() = std::move(val);
    }
    else
    {
      destroy();
      m_has_value = true;
      construct_value(std::move(val));
    }
    return *this;
  }

  // Unexpected assignment
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

  // Unexpected move assignment
  expected &operator=(unexpected<E> &&unexp)
  {
    if (m_has_value)
    {
      destroy();
      m_has_value = false;
      construct_error(std::move(unexp.error()));
    }
    else
    {
      error() = std::move(unexp.error());
    }
    return *this;
  }

  // Access operators
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

  const T &operator*() const &
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return *get_value_ptr();
  }

  T &operator*() &
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return *get_value_ptr();
  }

  const T &&operator*() const &&
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return std::move(*get_value_ptr());
  }

  T &&operator*() &&
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return std::move(*get_value_ptr());
  }

  // Check if contains value
  bool has_value() const
  {
    return m_has_value;
  }

  explicit operator bool() const
  {
    return m_has_value;
  }

  // Value access
  const T &value() const &
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return *get_value_ptr();
  }

  T &value() &
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return *get_value_ptr();
  }

  const T &&value() const &&
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return std::move(*get_value_ptr());
  }

  T &&value() &&
  {
    if (!has_value())
    {
      throw std::logic_error("expected contains error, not value");
    }
    return std::move(*get_value_ptr());
  }

  // Error access
  const E &error() const &
  {
    if (has_value())
    {
      throw std::logic_error("expected contains value, not error");
    }
    return *get_error_ptr();
  }

  E &error() &
  {
    if (has_value())
    {
      throw std::logic_error("expected contains value, not error");
    }
    return *get_error_ptr();
  }

  const E &&error() const &&
  {
    if (has_value())
    {
      throw std::logic_error("expected contains value, not error");
    }
    return std::move(*get_error_ptr());
  }

  E &&error() &&
  {
    if (has_value())
    {
      throw std::logic_error("expected contains value, not error");
    }
    return std::move(*get_error_ptr());
  }

  // Value or default
  template <typename U>
  T value_or(U &&default_value) const &
  {
    return has_value() ? value() : static_cast<T>(std::forward<U>(default_value));
  }

  template <typename U>
  T value_or(U &&default_value) &&
  {
    return has_value() ? std::move(value()) : static_cast<T>(std::forward<U>(default_value));
  }

  // Swap
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
      E temp_error = std::move(other.error());
      other.destroy();
      other.m_has_value = true;
      other.construct_value(std::move(value()));
      destroy();
      m_has_value = false;
      construct_error(std::move(temp_error));
    }
    else
    {
      T temp_value = std::move(other.value());
      other.destroy();
      other.m_has_value = false;
      other.construct_error(std::move(error()));
      destroy();
      m_has_value = true;
      construct_value(std::move(temp_value));
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

  void construct_value(T &&val)
  {
    new (m_storage) T(std::move(val));
  }

  void construct_error(const E &err)
  {
    new (m_storage) E(err);
  }

  void construct_error(E &&err)
  {
    new (m_storage) E(std::move(err));
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

// Swap function for expected
template <typename T, typename E>
void swap(expected<T, E> &a, expected<T, E> &b)
{
  a.swap(b);
}

// Comparison operators
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
