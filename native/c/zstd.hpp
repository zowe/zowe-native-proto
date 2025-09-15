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
#include <string>
#include <typeinfo>
#include <type_traits>

namespace zstd
{

// Backport of C++11 `std::remove_reference`
template <class T>
struct remove_reference
{
  typedef T type;
};
template <class T>
struct remove_reference<T &>
{
  typedef T type;
};
template <class T>
struct remove_reference<T &&>
{
  typedef T type;
};

// Backport of C++11 `std::is_same`
template <class T, class U>
struct is_same : std::false_type
{
};

template <class T>
struct is_same<T, T> : std::true_type
{
};

// Backport of C++11 `std::enable_if`
template <bool B, class T = void>
struct enable_if
{
};

template <typename T>
struct enable_if<true, T>
{
  typedef T type;
};

// Backport of C++11 `std::forward`
template <typename T>
T &&forward(typename remove_reference<T>::type &t)
{
  return static_cast<T &&>(t);
}

template <typename T>
T &&forward(typename remove_reference<T>::type &&t)
{
  return static_cast<T &&>(t);
}

// Backport of C++11 `std::unique_ptr`
template <typename T>
class unique_ptr
{
public:
  typedef T element_type;

  // Default constructor
  explicit unique_ptr(element_type *ptr = nullptr)
      : m_ptr(ptr)
  {
  }

  // Copy constructor
  unique_ptr(unique_ptr &&other)
      : m_ptr(other.release())
  {
  }

  ~unique_ptr()
  {
    delete m_ptr;
  }

  // Copy-assignment
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

// Backport of C++14 `std::make_unique`
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

// Backport of C++17 `std::optional`
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

// Unexpected type for `expected` backport
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

// Helper function to create unexpected values
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

  // Default constructor
  expected()
      : m_has_value(true)
  {
    construct_value();
  }

  // Copy constructor
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

  // Value constructor
  expected(const T &value)
      : m_has_value(true)
  {
    construct_value(value);
  }

  // Error constructor
  expected(const unexpected<E> &unexp)
      : m_has_value(false)
  {
    construct_error(unexp.error());
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

  // Error access
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

  // Value or default
  template <typename U>
  T value_or(const U &default_value) const
  {
    return has_value() ? value() : static_cast<T>(default_value);
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

// Backport of C++17 class `std::string_view`
class string_view
{
public:
  typedef char value_type;
  typedef const char *pointer;
  typedef const char *const_pointer;
  typedef const char &reference;
  typedef const char &const_reference;
  typedef const char *const_iterator;
  typedef const_iterator iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  static const size_type npos = static_cast<size_type>(-1);

  // Default constructor
  string_view()
      : m_data(nullptr), m_size(0)
  {
  }

  // C-string constructor
  string_view(const char *str)
      : m_data(str), m_size(str ? strlen(str) : 0)
  {
  }

  // Data and size constructor
  string_view(const char *data, size_type size)
      : m_data(data), m_size(size)
  {
  }

  // std::string constructor
  string_view(const std::string &str)
      : m_data(str.c_str()), m_size(str.size())
  {
  }

  // Copy constructor
  string_view(const string_view &other)
      : m_data(other.m_data), m_size(other.m_size)
  {
  }

  // Assignment operator
  string_view &operator=(const string_view &other)
  {
    if (this != &other)
    {
      m_data = other.m_data;
      m_size = other.m_size;
    }
    return *this;
  }

  // Iterators
  const_iterator begin() const
  {
    return m_data;
  }

  const_iterator end() const
  {
    return m_data + m_size;
  }

  const_iterator cbegin() const
  {
    return begin();
  }

  const_iterator cend() const
  {
    return end();
  }

  // Element access
  const_reference operator[](size_type pos) const
  {
    return m_data[pos];
  }

  const_reference at(size_type pos) const
  {
    if (pos >= m_size)
    {
      throw std::out_of_range("string_view::at: position out of range");
    }
    return m_data[pos];
  }

  const_reference front() const
  {
    return m_data[0];
  }

  const_reference back() const
  {
    return m_data[m_size - 1];
  }

  const_pointer data() const
  {
    return m_data;
  }

  // Capacity
  size_type size() const
  {
    return m_size;
  }

  size_type length() const
  {
    return m_size;
  }

  size_type max_size() const
  {
    return static_cast<size_type>(-1);
  }

  bool empty() const
  {
    return m_size == 0;
  }

  // Modifiers
  void remove_prefix(size_type n)
  {
    m_data += n;
    m_size -= n;
  }

  void remove_suffix(size_type n)
  {
    m_size -= n;
  }

  void swap(string_view &other)
  {
    using std::swap;
    swap(m_data, other.m_data);
    swap(m_size, other.m_size);
  }

  // String operations
  size_type copy(char *dest, size_type count, size_type pos = 0) const
  {
    if (pos > m_size)
    {
      throw std::out_of_range("string_view::copy: position out of range");
    }

    size_type rcount = std::min(count, m_size - pos);
    std::copy(m_data + pos, m_data + pos + rcount, dest);
    return rcount;
  }

  string_view substr(size_type pos = 0, size_type count = npos) const
  {
    if (pos > m_size)
    {
      throw std::out_of_range("string_view::substr: position out of range");
    }

    size_type rcount = std::min(count, m_size - pos);
    return string_view(m_data + pos, rcount);
  }

  int compare(string_view other) const
  {
    size_type rlen = std::min(m_size, other.m_size);
    int result = rlen ? std::memcmp(m_data, other.m_data, rlen) : 0;
    if (result == 0)
    {
      result = (m_size < other.m_size) ? -1 : (m_size > other.m_size) ? 1
                                                                      : 0;
    }
    return result;
  }

  int compare(size_type pos1, size_type count1, string_view other) const
  {
    return substr(pos1, count1).compare(other);
  }

  int compare(size_type pos1, size_type count1, string_view other, size_type pos2, size_type count2) const
  {
    return substr(pos1, count1).compare(other.substr(pos2, count2));
  }

  int compare(const char *s) const
  {
    return compare(string_view(s));
  }

  int compare(size_type pos1, size_type count1, const char *s) const
  {
    return substr(pos1, count1).compare(string_view(s));
  }

  int compare(size_type pos1, size_type count1, const char *s, size_type count2) const
  {
    return substr(pos1, count1).compare(string_view(s, count2));
  }

  bool starts_with(string_view prefix) const
  {
    return m_size >= prefix.m_size && substr(0, prefix.m_size).compare(prefix) == 0;
  }

  bool starts_with(char c) const
  {
    return !empty() && front() == c;
  }

  bool starts_with(const char *s) const
  {
    return starts_with(string_view(s));
  }

  bool ends_with(string_view suffix) const
  {
    return m_size >= suffix.m_size && substr(m_size - suffix.m_size).compare(suffix) == 0;
  }

  bool ends_with(char c) const
  {
    return !empty() && back() == c;
  }

  bool ends_with(const char *s) const
  {
    return ends_with(string_view(s));
  }

  // Find operations
  size_type find(string_view str, size_type pos = 0) const
  {
    if (pos > m_size || str.m_size > m_size - pos)
      return npos;

    if (str.empty())
      return pos;

    const char *result = std::search(m_data + pos, m_data + m_size, str.m_data, str.m_data + str.m_size);
    return result == m_data + m_size ? npos : result - m_data;
  }

  size_type find(char c, size_type pos = 0) const
  {
    if (pos >= m_size)
      return npos;

    const char *result = static_cast<const char *>(std::memchr(m_data + pos, c, m_size - pos));
    return result ? result - m_data : npos;
  }

  size_type find(const char *s, size_type pos, size_type count) const
  {
    return find(string_view(s, count), pos);
  }

  size_type find(const char *s, size_type pos = 0) const
  {
    return find(string_view(s), pos);
  }

  size_type rfind(string_view str, size_type pos = npos) const
  {
    if (str.m_size > m_size)
      return npos;

    if (str.empty())
      return std::min(pos, m_size);

    size_type start_pos = std::min(pos, m_size - str.m_size);
    for (size_type i = start_pos + 1; i > 0; --i)
    {
      if (substr(i - 1, str.m_size).compare(str) == 0)
        return i - 1;
    }
    return npos;
  }

  size_type rfind(char c, size_type pos = npos) const
  {
    if (empty())
      return npos;

    size_type start_pos = std::min(pos, m_size - 1);
    for (size_type i = start_pos + 1; i > 0; --i)
    {
      if (m_data[i - 1] == c)
        return i - 1;
    }
    return npos;
  }

  size_type rfind(const char *s, size_type pos, size_type count) const
  {
    return rfind(string_view(s, count), pos);
  }

  size_type rfind(const char *s, size_type pos = npos) const
  {
    return rfind(string_view(s), pos);
  }

  size_type find_first_of(string_view str, size_type pos = 0) const
  {
    for (size_type i = pos; i < m_size; ++i)
    {
      if (str.find(m_data[i]) != npos)
        return i;
    }
    return npos;
  }

  size_type find_first_of(char c, size_type pos = 0) const
  {
    return find(c, pos);
  }

  size_type find_first_of(const char *s, size_type pos, size_type count) const
  {
    return find_first_of(string_view(s, count), pos);
  }

  size_type find_first_of(const char *s, size_type pos = 0) const
  {
    return find_first_of(string_view(s), pos);
  }

  size_type find_last_of(string_view str, size_type pos = npos) const
  {
    if (empty() || str.empty())
      return npos;

    size_type start_pos = std::min(pos, m_size - 1);
    for (size_type i = start_pos + 1; i > 0; --i)
    {
      if (str.find(m_data[i - 1]) != npos)
        return i - 1;
    }
    return npos;
  }

  size_type find_last_of(char c, size_type pos = npos) const
  {
    return rfind(c, pos);
  }

  size_type find_last_of(const char *s, size_type pos, size_type count) const
  {
    return find_last_of(string_view(s, count), pos);
  }

  size_type find_last_of(const char *s, size_type pos = npos) const
  {
    return find_last_of(string_view(s), pos);
  }

  size_type find_first_not_of(string_view str, size_type pos = 0) const
  {
    for (size_type i = pos; i < m_size; ++i)
    {
      if (str.find(m_data[i]) == npos)
        return i;
    }
    return npos;
  }

  size_type find_first_not_of(char c, size_type pos = 0) const
  {
    for (size_type i = pos; i < m_size; ++i)
    {
      if (m_data[i] != c)
        return i;
    }
    return npos;
  }

  size_type find_first_not_of(const char *s, size_type pos, size_type count) const
  {
    return find_first_not_of(string_view(s, count), pos);
  }

  size_type find_first_not_of(const char *s, size_type pos = 0) const
  {
    return find_first_not_of(string_view(s), pos);
  }

  size_type find_last_not_of(string_view str, size_type pos = npos) const
  {
    if (empty())
      return npos;

    size_type start_pos = std::min(pos, m_size - 1);
    for (size_type i = start_pos + 1; i > 0; --i)
    {
      if (str.find(m_data[i - 1]) == npos)
        return i - 1;
    }
    return npos;
  }

  size_type find_last_not_of(char c, size_type pos = npos) const
  {
    if (empty())
      return npos;

    size_type start_pos = std::min(pos, m_size - 1);
    for (size_type i = start_pos + 1; i > 0; --i)
    {
      if (m_data[i - 1] != c)
        return i - 1;
    }
    return npos;
  }

  size_type find_last_not_of(const char *s, size_type pos, size_type count) const
  {
    return find_last_not_of(string_view(s, count), pos);
  }

  size_type find_last_not_of(const char *s, size_type pos = npos) const
  {
    return find_last_not_of(string_view(s), pos);
  }

private:
  const char *m_data;
  size_type m_size;
};

// Non-member functions for string_view
inline void swap(string_view &a, string_view &b)
{
  a.swap(b);
}

// Comparison operators
inline bool operator==(const string_view &lhs, const string_view &rhs)
{
  return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}

inline bool operator!=(const string_view &lhs, const string_view &rhs)
{
  return !(lhs == rhs);
}

inline bool operator<(const string_view &lhs, const string_view &rhs)
{
  return lhs.compare(rhs) < 0;
}

inline bool operator<=(const string_view &lhs, const string_view &rhs)
{
  return lhs.compare(rhs) <= 0;
}

inline bool operator>(const string_view &lhs, const string_view &rhs)
{
  return lhs.compare(rhs) > 0;
}

inline bool operator>=(const string_view &lhs, const string_view &rhs)
{
  return lhs.compare(rhs) >= 0;
}

// Backport of C++17 `std::monostate`
struct monostate
{
};

// Comparison operators for monostate
inline bool operator==(const monostate&, const monostate&)
{
  return true;
}

inline bool operator!=(const monostate&, const monostate&)
{
  return false;
}

inline bool operator<(const monostate&, const monostate&)
{
  return false;
}

inline bool operator<=(const monostate&, const monostate&)
{
  return true;
}

inline bool operator>(const monostate&, const monostate&)
{
  return false;
}

inline bool operator>=(const monostate&, const monostate&)
{
  return true;
}

// Variant type that mimics C++17 std::variant
template <typename... Types>
class variant;

namespace internal
{
template <typename T, typename... Types>
struct is_one_of;

template <typename T, typename Head, typename... Tail>
struct is_one_of<T, Head, Tail...>
{
  static const bool value = is_same<T, Head>::value || is_one_of<T, Tail...>::value;
};

template <typename T>
struct is_one_of<T>
{
  static const bool value = false;
};

template <typename T, typename... Types>
struct IndexOf;

template <typename T, typename Head, typename... Tail>
struct IndexOf<T, Head, Tail...>
{
  static const size_t value = IndexOf<T, Tail...>::value + 1;
};

template <typename T, typename... Tail>
struct IndexOf<T, T, Tail...>
{
  static const size_t value = 0;
};

template <typename T>
struct IndexOf<T>
{
  static const size_t value = (size_t)-1; // Not found
};

template <size_t N, typename... Types>
struct AtIndex;

template <size_t N, typename Head, typename... Tail>
struct AtIndex<N, Head, Tail...>
{
  typedef typename AtIndex<N - 1, Tail...>::type type;
};

template <typename Head, typename... Tail>
struct AtIndex<0, Head, Tail...>
{
  typedef Head type;
};

template <typename... Types>
struct MaxSizeOf;

template <typename Head, typename... Tail>
struct MaxSizeOf<Head, Tail...>
{
  static const size_t tail_size = MaxSizeOf<Tail...>::value;
  static const size_t value = sizeof(Head) > tail_size ? sizeof(Head) : tail_size;
};

template <>
struct MaxSizeOf<>
{
  static const size_t value = 0;
};

template <typename... Types>
struct MaxAlignOf;

template <typename Head, typename... Tail>
struct MaxAlignOf<Head, Tail...>
{
  static const size_t tail_align = MaxAlignOf<Tail...>::value;
  static const size_t value = __alignof__(Head) > tail_align ? __alignof__(Head) : tail_align;
};

template <>
struct MaxAlignOf<>
{
  static const size_t value = 0;
};

template <typename Visitor, typename Variant, size_t I, size_t Max>
struct visitor_caller
{
  static typename Visitor::result_type apply(Variant &v, Visitor &visitor)
  {
    if (v.index() == I)
    {
      return visitor(v.template get<I>());
    }
    return visitor_caller<Visitor, Variant, I + 1, Max>::apply(v, visitor);
  }
};

template <typename Visitor, typename Variant, size_t Max>
struct visitor_caller<Visitor, Variant, Max, Max>
{
  static typename Visitor::result_type apply(Variant &v, Visitor &visitor)
  {
    throw std::bad_cast(); // Should not happen
  }
};

template <size_t I, typename... Types>
struct destroy_visitor
{
  static void apply(size_t index, void *data)
  {
    if (index == I)
    {
      typedef typename AtIndex<I, Types...>::type T;
      reinterpret_cast<T *>(data)->~T();
    }
    else
    {
      destroy_visitor<I + 1, Types...>::apply(index, data);
    }
  }
};

template <typename... Types>
struct destroy_visitor<sizeof...(Types), Types...>
{
  static void apply(size_t index, void *data)
  {
  }
};

template <size_t I, typename... Types>
struct copy_visitor
{
  static void apply(size_t index, void *dest, const void *src)
  {
    if (index == I)
    {
      typedef typename AtIndex<I, Types...>::type T;
      new (dest) T(*reinterpret_cast<const T *>(src));
    }
    else
    {
      copy_visitor<I + 1, Types...>::apply(index, dest, src);
    }
  }
};

template <typename... Types>
struct copy_visitor<sizeof...(Types), Types...>
{
  static void apply(size_t index, void *dest, const void *src)
  {
  }
};
} // namespace internal

struct bad_variant_access : public std::logic_error
{
  bad_variant_access()
      : std::logic_error("bad_variant_access")
  {
  }
};

// Backport of C++17 utility `std::variant`
template <typename... Types>
class variant
{
public:
  variant()
      : m_index(0)
  {
    construct<0>();
  }

  template <typename T>
  variant(const T &value)
  {
    m_index = internal::IndexOf<T, Types...>::value;
    construct_value(value);
  }

  variant(const variant &other)
      : m_index(other.m_index)
  {
    copy_construct(other);
  }

  ~variant()
  {
    destroy();
  }

  variant &operator=(const variant &other)
  {
    if (this != &other)
    {
      destroy();
      m_index = other.m_index;
      copy_construct(other);
    }
    return *this;
  }

  template <typename T>
  variant &operator=(const T &value)
  {
    destroy();
    m_index = internal::IndexOf<T, Types...>::value;
    construct_value(value);
    return *this;
  }

  size_t index() const
  {
    return m_index;
  }

  template <typename T>
  T &get()
  {
    if (m_index != internal::IndexOf<T, Types...>::value)
    {
      throw bad_variant_access();
    }
    return *reinterpret_cast<T *>(m_storage);
  }

  template <typename T>
  T *get_ptr() const
  {
    if (m_index != internal::IndexOf<T, Types...>::value)
    {
      throw bad_variant_access();
    }
    return reinterpret_cast<T *>(m_storage);
  }

  template <typename T>
  const T &get() const
  {
    if (m_index != internal::IndexOf<T, Types...>::value)
    {
      throw bad_variant_access();
    }
    return *reinterpret_cast<const T *>(m_storage);
  }

  template <size_t I>
  typename internal::AtIndex<I, Types...>::type &get()
  {
    if (m_index != I)
    {
      throw bad_variant_access();
    }
    typedef typename internal::AtIndex<I, Types...>::type T;
    return *reinterpret_cast<T *>(m_storage);
  }

  template <size_t I>
  const typename internal::AtIndex<I, Types...>::type &get() const
  {
    if (m_index != I)
    {
      throw bad_variant_access();
    }
    typedef typename internal::AtIndex<I, Types...>::type T;
    return *reinterpret_cast<const T *>(m_storage);
  }

  void reset()
  {
    destroy();
  }

private:
  template <typename Visitor, typename Variant, size_t I, size_t Max>
  friend struct internal::visitor_caller;

  template <size_t I, typename... Args>
  void construct(Args &&...args)
  {
    typedef typename internal::AtIndex<I, Types...>::type T;
    new (m_storage) T(forward<Args>(args)...);
  }

  template <typename T>
  void construct_value(const T &value)
  {
    new (m_storage) T(value);
  }

  void destroy()
  {
    internal::destroy_visitor<0, Types...>::apply(m_index, m_storage);
  }

  void copy_construct(const variant &other)
  {
    internal::copy_visitor<0, Types...>::apply(m_index, m_storage, other.m_storage);
  }

  size_t m_index;
  char m_storage[internal::MaxSizeOf<Types...>::value] __attribute__((aligned(internal::MaxAlignOf<Types...>::value)));
};

template <typename Visitor, typename... Types>
typename Visitor::result_type apply_visitor(Visitor &visitor, variant<Types...> &v)
{
  return internal::visitor_caller<Visitor, variant<Types...>, 0, sizeof...(Types)>::apply(v, visitor);
}

} // namespace zstd

#endif // ZSTD_HPP
