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
#ifndef IMS_TYPE_DEF_H_
#define IMS_TYPE_DEF_H_

#if defined(__LINUX__)

#if defined(__LP64__)
#define __IMS_LP64__
#endif

#if defined(__clang__)
#define __IMS_CLANG__
#endif

#endif

// C++17
#if (__cplusplus >= 201703L)
#define __IMS_FALLTHROUGH__ [[fallthrough]];
#else
#define __IMS_FALLTHROUGH__
#endif

#if defined(__clang__) && (__cplusplus >= 201103L)
#define __IMS_DEPRECATED__(message) __attribute__((deprecated(message)))
#endif

#ifndef __IMS_DEPRECATED__
#define __IMS_DEPRECATED__(message)
#endif

#ifndef IMS_NULL
#define IMS_NULL (nullptr)
#endif

#ifndef IMS_TRUE
#define IMS_TRUE ((1) == (1))
#endif

#ifndef IMS_FALSE
#define IMS_FALSE ((1) == (0))
#endif

#ifndef IMS_SUCCESS
#define IMS_SUCCESS (0)
#endif

#ifndef IMS_FAILURE
#define IMS_FAILURE (-1)
#endif

#ifndef null
#define null (0)
#endif

#ifndef GLOBAL
#define GLOBAL
#endif

#ifndef LOCAL
#define LOCAL static
#endif

#ifndef PROTECTED
#define PROTECTED
#endif

#ifndef PUBLIC
#define PUBLIC
#endif

#ifndef PRIVATE
#define PRIVATE
#endif

#ifndef ABSTRACT
#define ABSTRACT
#endif

#ifndef VIRTUAL
#define VIRTUAL
#endif

/* Input argument in function declaration */
#ifndef IN
#define IN
#endif

/* Output argument in function declaration */
#ifndef OUT
#define OUT
#endif

/* Input & Output argument in function declaration */
#ifndef IN_OUT
#define IN_OUT
#endif

/* Constant argument in function declaration */
#ifndef CONST
#define CONST const
#endif

#ifndef IMS_UINT8
typedef unsigned char IMS_UINT8;
#endif

#ifndef IMS_UINT16
typedef unsigned short IMS_UINT16;
#endif

#ifndef IMS_UINT32
typedef unsigned int IMS_UINT32;
#endif

#ifndef IMS_ULONG
typedef unsigned long IMS_ULONG;
#endif

#ifndef IMS_SINT8
typedef signed char IMS_SINT8;
#endif

#ifndef IMS_SINT16
typedef signed short IMS_SINT16;
#endif

#ifndef IMS_SINT32
typedef signed int IMS_SINT32;
#endif

#ifndef IMS_SLONG
typedef signed long IMS_SLONG;
#endif

#ifndef IMS_PVOID
typedef void* IMS_PVOID;
#endif

#ifndef IMS_BOOL
#ifdef __cplusplus
typedef bool IMS_BOOL;
#else
typedef int IMS_BOOL;
#endif  // __cplusplus
#endif

#ifndef IMS_CHAR
typedef char IMS_CHAR;
#endif

#ifndef IMS_UCHAR
typedef unsigned char IMS_UCHAR;
#endif

#ifndef IMS_WCHAR
typedef unsigned short IMS_WCHAR;
#endif

#ifndef IMS_BYTE
typedef unsigned char IMS_BYTE;
#endif

#ifndef IMS_FLOAT
typedef float IMS_FLOAT;
#endif

#ifndef IMS_DOUBLE
typedef double IMS_DOUBLE;
#endif

#ifndef IMS_RESULT
typedef IMS_SINT32 IMS_RESULT;
#endif

// START :: definitions for 64-bit platform
#ifdef __IMS_LP64__

#ifndef IMS_SINT64
typedef signed long IMS_SINT64;
#endif

#ifndef IMS_UINT64
typedef unsigned long IMS_UINT64;
#endif

#ifndef IMS_SIZE_T
typedef unsigned long IMS_SIZE_T;
#endif

#ifndef IMS_SINTP
typedef signed long IMS_SINTP;
#endif

#ifndef IMS_UINTP
typedef unsigned long IMS_UINTP;
#endif

#define LONG_TO_INT(l)      ((l)&0xFFFFFFFF)

#define INT64_TO_SINTP(i64) (i64)
#define INT64_TO_UINTP(i64) (static_cast<IMS_UINTP>(i64))

// Printf Format : Length-Specifier
#define PFLS_d              "ld"
#define PFLS_u              "lu"
#define PFLS_o              "lo"
#define PFLS_x              "lx"
#define PFLS_X              "lX"

#else  // __IMS_LP64__

#ifndef IMS_SINT64
typedef signed long long IMS_SINT64;
#endif

#ifndef IMS_UINT64
typedef unsigned long long IMS_UINT64;
#endif

#ifndef IMS_SIZE_T
typedef unsigned int IMS_SIZE_T;
#endif

#ifndef IMS_SINTP
typedef signed int IMS_SINTP;
#endif

#ifndef IMS_UINTP
typedef unsigned int IMS_UINTP;
#endif

#define LONG_TO_INT(l)      (l)

#define INT64_TO_SINTP(i64) ((i64)&0xFFFFFFFF)
#define INT64_TO_UINTP(i64) (static_cast<IMS_UINTP>((i64)&0xFFFFFFFF))

// Printf Format : Length-Specifier
#define PFLS_d              "d"
#define PFLS_u              "u"
#define PFLS_o              "o"
#define PFLS_x              "x"
#define PFLS_X              "X"

#endif  // __IMS_LP64__

#define LONG_TO_SINT(l) (static_cast<IMS_SINT32>(LONG_TO_INT(l)))
#define LONG_TO_UINT(l) (static_cast<IMS_UINT32>(LONG_TO_INT(l)))

#define PTR_TO_UINTP(p) (reinterpret_cast<IMS_UINTP>(p))
#define PTR_TO_SINTP(p) (reinterpret_cast<IMS_SINTP>(p))
// END :: definitions for 64-bit platform

// Slot definition for Multi-IMS architecture
// Slot 0 is a default if not specified
#define IMS_SLOT_ANY    (-1)
#define IMS_SLOT_0      0
#define IMS_SLOT_1      1
// FIXME: for dual SIM only
#define IMS_SLOT_MAX    2

#ifndef NULL
#define NULL (0)
#endif

#ifndef true
#define true (1 == 1)
#endif

#ifndef false
#define false (1 == 0)
#endif

#if defined(__arm)

#if defined(__clang__)
#define __IMS_FILE__ __FILE__
#else
#define __IMS_FILE__ __MODULE__
#endif
#define __IMS_LINE__ __LINE__
#define __IMS_FUNC__ __func__

#elif defined(__LINUX__)

#define __IMS_FILE__ __FILE__
#define __IMS_LINE__ __LINE__
#define __IMS_FUNC__ __func__

#else

#define __IMS_FILE__ "N/A"
#define __IMS_LINE__ 0
#define __IMS_FUNC__ "N/A"

#endif

#ifdef IMS_RTTI_ENABLED
#define DYNAMIC_CAST(TYPE, VALUE)     (dynamic_cast<TYPE>(VALUE))
#define REINTERPRET_CAST(TYPE, VALUE) (reinterpret_cast<TYPE>(VALUE))
#else
// C-style type casting
#define DYNAMIC_CAST(TYPE, VALUE)     ((TYPE)(VALUE))
#define REINTERPRET_CAST(TYPE, VALUE) (reinterpret_cast<TYPE>(VALUE))
#endif  // IMS_RTTI_ENABLED

// IMS ASSERT (ServiceTrace.cpp implements this function)
extern void TraceService_Assert(IN const IMS_CHAR*, IN const IMS_CHAR*, IN IMS_UINT32);

#define IMS_ASSERT(CONDITION)                                            \
    do                                                                   \
    {                                                                    \
        if (!(CONDITION))                                                \
            TraceService_Assert(#CONDITION, __IMS_FUNC__, __IMS_LINE__); \
    } while (0)

// This template class is used to pass the reference with a default value
// as an input argument of a method.
template <typename T>
class ByRef final
{
public:
    inline ByRef() = default;
    inline ByRef(IN const T value) :
            mValue(value)
    {
    }
    inline ByRef(IN const ByRef&) = delete;
    inline ByRef& operator=(IN const ByRef&) = delete;
    inline operator T&() const { return const_cast<T&>(mValue); }

private:
    T mValue;
};

#endif
