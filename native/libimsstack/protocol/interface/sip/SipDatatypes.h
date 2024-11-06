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
#ifndef __SIP_DATATYPES_H__
#define __SIP_DATATYPES_H__

#define SIP_NULL             (0)
#define SIP_INVALID          (-1)
#define SIP_EQUALS           0
#define SIP_MATCHES          0
#define SIP_NOT_MATCH        1
#define SIP_YES              1 /* To Know Request/Response Headaer */
#define SIP_NO               0
#define SIP_INDEX_ZERO       0
#define SIP_START_INDEX      0
#define SIP_ENABLE           1
#define SIP_DISABLE          0
#define SIP_NOT_EXISTS       0
#define SIP_EXISTS           1
#define SIP_NULL_CHAR        '\0'
/* Numerals while used as INDEX or SIZE */
#define SIP_ZERO             0
#define SIP_ONE              1
#define SIP_TWO              2
#define SIP_THREE            3
#define SIP_FOUR             4
#define SIP_FIVE             5
#define SIP_SIX              6
#define SIP_SEVEN            7
#define SIP_EIGHT            8
#define SIP_NINE             9
#define SIP_TEN              10
#define SIP_11               11
#define SIP_12               12
#define SIP_13               13
#define SIP_14               14
#define SIP_15               15
#define SIP_16               16
#define SIP_17               17
#define SIP_18               18
#define SIP_19               19
#define SIP_20               20
#define SIP_21               21
#define SIP_22               22
#define SIP_23               23
#define SIP_24               24
#define SIP_25               25
#define SIP_26               26
#define SIP_27               27
#define SIP_28               28
#define SIP_29               29
#define SIP_30               30
#define SIP_31               31
#define SIP_32               32

#define SIP_MAX_HOP          70
#define SIP_UNSPECIFIED_PORT 0xFFFF

#define SIP_MAX_INT          11  // including null-terminated string

/* Function Parameter Notation */
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef IN_OUT
#define IN_OUT
#endif

#ifndef PUBLIC
#define PUBLIC
#endif

#ifndef PRIVATE
#define PRIVATE
#endif

// #define SIP_BOOL bool
// #define SIP_TRUE true
// #define SIP_FALSE false

/****************************************************************************
  Enum Declaration
 *****************************************************************************/
typedef enum _SIP_BOOL
{
    SIP_FALSE = 0,
    SIP_TRUE = 1
} SIP_BOOL;

// Logger I,D,E identifier for Android Trace Service
enum
{
    CAT_D = 0x44,  // 'D' : debug
    CAT_E = 0x45,  // 'E' : error
    CAT_I = 0x49,  // 'I' : information
    CAT_E_BASE = 0xFF
};

/* Basic Data Types */
typedef void SIP_VOID;
typedef unsigned char SIP_UCHAR;
typedef char SIP_CHAR;
typedef signed char SIP_SCHAR;
typedef unsigned short SIP_UINT16;
typedef short SIP_INT16;
typedef unsigned int SIP_UINT32;
typedef int SIP_INT32;

typedef SIP_UINT32 SIP_STACK_HANDLE;
typedef SIP_UINT32 SIP_TXN_HANDLE_KEY;
typedef SIP_UINT32 SIP_MSG_HANDLE;
typedef SIP_UINT32 SIP_TIMER_HANDLE;

#if defined(__LP64__)

#ifndef SIP_SIZE_T
typedef unsigned long SIP_SIZE_T;
#endif

#else  // defined(__LP64__)

#ifndef SIP_SIZE_T
typedef unsigned int SIP_SIZE_T;
#endif

#endif  // defined(__LP64__)

template <typename T>
inline const T& SIP_MIN(const T& a, const T& b)
{
    return (a < b) ? a : b;
}

template <typename T>
inline const T& SIP_MAX(const T& a, const T& b)
{
    return (a < b) ? b : a;
}
#endif  //__SIP_DATATYPES_H__
