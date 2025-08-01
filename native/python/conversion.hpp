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

#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include <string>

// Define a type for the conversion function pointers
typedef size_t (*conversion_func_t)(char *);

/**
 * A generic function to perform in-place string character set conversion.
 * @param s The string to convert.
 * @param func The conversion function to apply (e.g., __a2e_s or __e2a_s).
 */
inline void convert_inplace(std::string &s, conversion_func_t func)
{
    if (s.empty())
    {
        return;
    }
    s.push_back('\0');
    // Call the conversion function on the underlying C-style string.
    func(&s[0]);
    // Remove the null terminator.
    s.pop_back();
}

/**
 * Convert string from EBCDIC to ASCII in-place.
 * @param s The string to convert.
 */
inline void e2a_inplace(std::string &s)
{
    convert_inplace(s, __e2a_s);
}

/**
 * Convert string from ASCII to EBCDIC in-place.
 * @param s The string to convert.
 */
inline void a2e_inplace(std::string &s)
{
    convert_inplace(s, __a2e_s);
}

#endif