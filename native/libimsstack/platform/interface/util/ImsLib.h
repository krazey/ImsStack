/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef IMS_LIB_H_
#define IMS_LIB_H_

#include "ImsTypeDef.h"

#if defined(__IMS_LP64__)
#define IMS_LONG_MAX  (0x7FFFFFFFFFFFFFFFL)
#define IMS_LONG_MIN  (-IMS_LONG_MAX - 1L)
#define IMS_ULONG_MAX (0xFFFFFFFFFFFFFFFFUL)
#else
#define IMS_LONG_MAX  (0x7FFFFFFFL)
#define IMS_LONG_MIN  (-IMS_LONG_MAX - 1L)
#define IMS_ULONG_MAX (0xFFFFFFFFUL)
#endif

#define IMS_INT_MAX    (0x7FFFFFFF)
#define IMS_INT_MIN    (-IMS_INT_MAX - 1)
#define IMS_UINT_MAX   (0xFFFFFFFFU)

#define IMS_SHORT_MAX  (0x7FFF)
#define IMS_SHORT_MIN  (-IMS_SHORT_MAX - 1)
#define IMS_USHORT_MAX (0xFFFFU)

template <typename T>
inline const T& IMS_MIN(IN const T& a, IN const T& b)
{
    return (a < b) ? a : b;
}

template <typename T>
inline const T& IMS_MAX(IN const T& a, IN const T& b)
{
    return (a < b) ? b : a;
}

inline IMS_CHAR IMS_TOLOWER(IN IMS_CHAR c)
{
    return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : (c);
}

inline IMS_CHAR IMS_TOUPPER(IN IMS_CHAR c)
{
    return ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : (c);
}

inline IMS_WCHAR IMS_TOLOWER(IN IMS_WCHAR c)
{
    return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : (c);
}

inline IMS_WCHAR IMS_TOUPPER(IN IMS_WCHAR c)
{
    return ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : (c);
}

inline IMS_BOOL IMS_ISSPACE(IN IMS_CHAR c)
{
    return ((c == '\n') || (c == '\t') || (c == '\v') || (c == ' ') || (c == '\r') || (c == '\f'))
            ? IMS_TRUE
            : IMS_FALSE;
}

inline IMS_BOOL IMS_ISDIGIT(IN IMS_CHAR c)
{
    return ((c >= '0') && (c <= '9')) ? IMS_TRUE : IMS_FALSE;
}

inline IMS_BOOL IMS_ISASCII(IN IMS_CHAR c)
{
    return (((unsigned)c) < 128);
}

inline IMS_BOOL IMS_ISALPHA(IN IMS_CHAR c)
{
    return (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) ? IMS_TRUE : IMS_FALSE;
}

inline IMS_BOOL IMS_ISUPPER(IN IMS_CHAR c)
{
    return ((c >= 'A') && (c <= 'Z')) ? IMS_TRUE : IMS_FALSE;
}

#endif
