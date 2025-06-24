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

#ifndef COMPAT_HPP
#define COMPAT_HPP

// Compatibility for shared_ptr and enable_shared_from_this for compilers w/
// separate TR1 folder (C++11 Tech Report 1 standard)
#if defined(__has_include)
#if __has_include(<memory>)
#include <memory>
#elif __has_include(<tr1/memory>)
#include <tr1/memory>
namespace std
{
    typedef tr1::shared_ptr shared_ptr;
    typedef tr1::enable_shared_from_this enable_shared_from_this;
} // namespace std
#endif
#else
// Fallback for specialized compilers w/ standard headers but separation via tr1
// namespace
#if defined(__IBMCPP_TR1__)
#include <memory>
#else
#include <tr1/memory>
namespace std
{
    using tr1::enable_shared_from_this;
    using tr1::shared_ptr;
} // namespace std
#endif
#endif

// Detect whether shared_ptr is in std::tr1 or std for parser-specific typedefs
#if defined(_MSC_VER) && (_MSC_VER < 1700)
// MSVC before VS2012: shared_ptr is in std::tr1
#define COMPAT_USE_TR1_SHARED_PTR
#elif defined(__GNUC__) && !defined(__clang__)
// GCC: check libstdc++ version
#if defined(__GLIBCXX__)
#if __GLIBCXX__ < 20110325
// Before GCC 4.3, shared_ptr is in std::tr1
#define COMPAT_USE_TR1_SHARED_PTR
#endif
#endif
#elif defined(__has_include)
#if __has_include(<tr1/memory>)
#define COMPAT_USE_TR1_SHARED_PTR
#endif
#elif defined(__IBMCPP_TR1__)
#define COMPAT_USE_TR1_SHARED_PTR
#endif

#endif // COMPAT_HPP