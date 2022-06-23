/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090518  toastops@                 Created
    </table>

    Description

*/

#ifndef _IMS_LIB_H_
#define _IMS_LIB_H_

#include "IMSTypeDef.h"

#if defined(__IMS_LP64__)
#define IMS_LONG_MAX  (0x7FFFFFFFFFFFFFFFL)
#define IMS_LONG_MIN  (-IMS_LONG_MAX - 1L)
#define IMS_ULONG_MAX (0xFFFFFFFFFFFFFFFFUL)
#else
#define IMS_LONG_MAX  (0x7FFFFFFFL)
#define IMS_LONG_MIN  (-IMS_LONG_MAX - 1L)
#define IMS_ULONG_MAX (0xFFFFFFFFL)
#endif

#define IMS_INT_MAX    (0x7FFFFFFF)
#define IMS_INT_MIN    (-IMS_INT_MAX - 1)
#define IMS_UINT_MAX   (0xFFFFFFFF)

#define IMS_SHORT_MAX  (0x7FFF)
#define IMS_SHORT_MIN  (-IMS_SHORT_MAX - 1)
#define IMS_USHORT_MAX (0xFFFF)

template <typename T>
inline const T& IMS_MIN(IN CONST T& a, IN CONST T& b)
{
    return (a < b) ? a : b;
}

template <typename T>
inline const T& IMS_MAX(IN CONST T& a, IN CONST T& b)
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

#endif  // _IMS_LIB_H_
