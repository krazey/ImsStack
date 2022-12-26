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
#ifndef ANSI_STRING_H_
#define ANSI_STRING_H_

#include <stdarg.h>
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "RcObject.h"

class WCharPtr;

class AString
{
public:
    class CharRef
    {
    public:
        CharRef(IN AString& objStr, IN IMS_SINT32 nIndex);

    public:
        // lvalue operation
        CharRef& operator=(IN const CharRef& other);
        CharRef& operator=(IN IMS_CHAR c);
        // rvalue operation
        inline operator IMS_CHAR() const
        {
            return (m_nIndex < m_objStr.m_pData->nSize) ? m_objStr.m_pData->pValue[m_nIndex] : 0;
        }
        inline IMS_BOOL operator==(IN IMS_CHAR c) const
        {
            return (m_objStr.m_pData->pValue[m_nIndex] == c);
        }
        inline IMS_BOOL operator!=(IN IMS_CHAR c) const
        {
            return (m_objStr.m_pData->pValue[m_nIndex] != c);
        }
        inline IMS_BOOL operator>(IN IMS_CHAR c) const
        {
            return (m_objStr.m_pData->pValue[m_nIndex] > c);
        }
        inline IMS_BOOL operator>=(IN IMS_CHAR c) const
        {
            return (m_objStr.m_pData->pValue[m_nIndex] >= c);
        }
        inline IMS_BOOL operator<(IN IMS_CHAR c) const
        {
            return (m_objStr.m_pData->pValue[m_nIndex] < c);
        }
        inline IMS_BOOL operator<=(IN IMS_CHAR c) const
        {
            return (m_objStr.m_pData->pValue[m_nIndex] <= c);
        }

    private:
        AString& m_objStr;
        IMS_SINT32 m_nIndex;
    };

public:
    AString();
    // cppcheck-suppress noExplicitConstructor
    AString(IN const IMS_CHAR* pszValue);
    AString(IN const IMS_CHAR* pszValue, IN IMS_SINT32 nSize);
    AString(IN const IMS_CHAR* pszValue, IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);
    AString(IN IMS_SINT32 nSize, IN const IMS_CHAR c);
    AString(IN const AString& other);
    ~AString();

private:
    explicit AString(IN IMS_SINT32 nSize);

public:
    AString& operator=(IN const AString& other);
    AString& operator=(IN const IMS_CHAR c);
    AString& operator=(IN const IMS_CHAR* pszValue);
    AString& operator=(IN const IMS_WCHAR* pwszValue);
    inline AString& operator+=(IN const AString& objStr) { return Append(objStr); }
    inline AString& operator+=(IN const IMS_CHAR c) { return Append(c); }
    inline AString& operator+=(IN const IMS_CHAR* pszValue) { return Append(pszValue); }
    IMS_CHAR operator[](IN IMS_SINT32 nIndex) const;
    CharRef operator[](IN IMS_SINT32 nIndex);
    IMS_CHAR operator[](IN IMS_UINT32 nIndex) const;
    CharRef operator[](IN IMS_UINT32 nIndex);

    // For ImsMap class
    IMS_BOOL operator<(IN const AString& other);

public:
    inline IMS_SINT32 GetCapacity() const { return m_pData->nAlloc; }
    inline IMS_SINT32 GetLength() const { return m_pData->nSize; }

    inline IMS_BOOL IsEmpty() const
    {
        return ((m_pData == &SHARED_EMPTY) || (!IsNull() && (m_pData->nSize == 0)));
    }
    inline IMS_BOOL IsNull() const { return (m_pData == &SHARED_NULL); }
    inline IMS_BOOL IsNULL() const { return IsNull(); }

    inline IMS_CHAR* GetStr()
    {
        Detach();
        return m_pData->pValue;
    }
    inline const IMS_CHAR* GetStr() const { return m_pData->pValue; }
    AString GetSubStr(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount = -1) const;

    void Chop(IN IMS_SINT32 nCount);
    void Truncate(IN IMS_SINT32 nOffset);
    void Attach(IN const IMS_CHAR* pValue, IN IMS_SINT32 nSize);
    void Detach();
    void Resize(IN IMS_SINT32 nSize);

    IMS_SINT32 GetIndexOf(IN const IMS_CHAR c, IN IMS_SINT32 nOffset = 0) const;
    IMS_SINT32 GetIndexOf(IN const IMS_CHAR* pszValue, IN IMS_SINT32 nOffset = 0) const;
    IMS_SINT32 GetIndexOf(IN const AString& objStr, IN IMS_SINT32 nOffset = 0) const;

    IMS_SINT32 GetLastIndexOf(IN const IMS_CHAR c, IN IMS_SINT32 nOffset = -1) const;
    IMS_SINT32 GetLastIndexOf(IN const IMS_CHAR* pszValue, IN IMS_SINT32 nOffset = -1) const;
    IMS_SINT32 GetLastIndexOf(IN const AString& objStr, IN IMS_SINT32 nOffset = -1) const;

    IMS_BOOL Contains(IN const IMS_CHAR c) const;
    IMS_BOOL Contains(IN const IMS_CHAR* pszValue) const;
    IMS_BOOL Contains(IN const AString& objStr) const;
    IMS_BOOL EndsWith(IN const IMS_CHAR c) const;
    IMS_BOOL EndsWith(IN const IMS_CHAR* pszValue) const;
    IMS_BOOL EndsWith(IN const AString& objStr) const;
    IMS_BOOL StartsWith(IN const IMS_CHAR c) const;
    IMS_BOOL StartsWith(IN const IMS_CHAR* pszValue) const;
    IMS_BOOL StartsWith(IN const AString& objStr) const;

    IMS_BOOL Equals(IN const IMS_CHAR c) const;
    IMS_BOOL Equals(IN const IMS_CHAR* pszValue) const;
    IMS_BOOL Equals(IN const AString& objStr) const;
    IMS_BOOL EqualsIgnoreCase(IN const IMS_CHAR c) const;
    IMS_BOOL EqualsIgnoreCase(IN const IMS_CHAR* pszValue) const;
    IMS_BOOL EqualsIgnoreCase(IN const AString& objStr) const;

    AString& Append(IN const IMS_CHAR c);
    AString& Append(IN const IMS_CHAR* pszValue);
    AString& Append(IN const AString& objStr);
    AString& Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);
    AString& Fill(IN IMS_CHAR c, IN IMS_SINT32 nSize = -1);
    AString& Insert(IN IMS_SINT32 i, IN const IMS_CHAR c);
    AString& Insert(IN IMS_SINT32 i, IN const IMS_CHAR* pszValue);
    AString& Insert(IN IMS_SINT32 i, IN const AString& objStr);
    AString& Prepend(IN const IMS_CHAR c);
    AString& Prepend(IN const IMS_CHAR* pszValue);
    AString& Prepend(IN const AString& objStr);
    AString& Replace(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN const IMS_CHAR* pszNew);
    AString& Replace(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN const AString& strNew);
    AString& Replace(IN const IMS_CHAR cOld, IN const IMS_CHAR cNew);
    AString& Replace(IN const IMS_CHAR cOld, IN const IMS_CHAR* pszNew);
    AString& Replace(IN const IMS_CHAR cOld, IN const AString& strNew);
    AString& Replace(IN const IMS_CHAR* pszOld, IN const IMS_CHAR* pszNew);
    AString& Replace(IN const IMS_CHAR* pszOld, IN const AString& strNew);
    AString& Replace(IN const AString& strOld, IN const IMS_CHAR* pszNew);
    AString& Replace(IN const AString& strOld, IN const AString& strNew);

    AString MakeLower() const;
    AString MakeUpper() const;
    AString SimplifyWsp() const;
    AString Trim() const;
    AString TrimLeft() const;
    AString TrimRight() const;
    AString Left(IN IMS_SINT32 nCount) const;
    AString Mid(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount = -1) const;
    AString Right(IN IMS_SINT32 nCount) const;
    AString AlignLeft(
            IN IMS_SINT32 nWidth, IN IMS_CHAR cFill = ' ', IN IMS_BOOL bTruncate = IMS_FALSE) const;
    AString AlignRight(
            IN IMS_SINT32 nWidth, IN IMS_CHAR cFill = ' ', IN IMS_BOOL bTruncate = IMS_FALSE) const;

    AString& SetNumber(IN IMS_SINT16 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_UINT16 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_SINT32 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_UINT32 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_SINT64 n, IN IMS_SINT32 nBase = 10);
    AString& SetNumber(IN IMS_UINT64 n, IN IMS_SINT32 nBase = 10);
    IMS_SINT16 ToInt16(OUT IMS_BOOL* pbOk = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_UINT16 ToUInt16(OUT IMS_BOOL* pbOk = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_SINT32 ToInt32(OUT IMS_BOOL* pbOk = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_UINT32 ToUInt32(OUT IMS_BOOL* pbOk = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_SINT64 ToInt64(OUT IMS_BOOL* pbOk = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    IMS_UINT64 ToUInt64(OUT IMS_BOOL* pbOk = IMS_NULL, IN IMS_SINT32 nBase = 10) const;
    AString ToBase64() const;
    AString ToHexString() const;
    AString ToUtf8() const;

    AString& Sprintf(IN const IMS_CHAR* pszFormat, ...);
    AString& Vsprintf(IN const IMS_CHAR* pszFormat, va_list ap);

    IMS_CHAR* Duplicate() const;
    ImsList<AString> Split(IN IMS_CHAR cSep, IN IMS_BOOL bTrim = IMS_TRUE) const;
    IMS_SINT32 SplitF(IN IMS_CHAR cSep, OUT AString& strLhs, OUT AString& strRhs,
            IN IMS_BOOL bTrim = IMS_TRUE) const;
    IMS_SINT32 SplitB(IN IMS_CHAR cSep, OUT AString& strLhs, OUT AString& strRhs,
            IN IMS_BOOL bTrim = IMS_TRUE) const;

    IMS_UINT32 GetHashCode() const;

    // FIXME: it's for a temporary usage
    RcPtr<WCharPtr> GetWCharPtr() const;

    static const AString& ConstEmpty();
    static const AString& ConstNull();

    static IMS_SINT32 Compare(IN const IMS_CHAR* pszStr1, IN const IMS_CHAR* pszStr2);
    static IMS_SINT32 CompareIgnoreCase(IN const IMS_CHAR* pszStr1, IN const IMS_CHAR* pszStr2);
    static AString FromBase64(IN const AString& strBase64);
    static AString FromRawData(IN const IMS_CHAR* pszValue, IN IMS_SINT32 nSize);
    static IMS_UINT32 GetHashCode(IN const IMS_CHAR* pszValue);

private:
    void Clear();
    void Realloc(IN IMS_SINT32 nAlloc);

public:
    enum
    {
        NPOS = (-1)
    };

private:
    friend class CharRef;
    friend class AStringBuffer;

    struct Data
    {
        IMS_SINT32 nAlloc;    // Size of allocated buffer
        IMS_SINT32 nSize;     // Size of null-terminated string (excludes NULL character)
        IMS_CHAR* pValue;     // Pointer to the null-terminated string (Points to acValue)
        IMS_CHAR acValue[1];  // Pointer to the null-terminated string
    };

    explicit AString(IN Data* pData);

    static Data SHARED_NULL;
    static Data SHARED_EMPTY;

    Data* m_pData;
};

inline IMS_BOOL operator==(IN const AString& objA1, IN const AString& objA2)
{
    return objA1.Equals(objA2);
}
inline IMS_BOOL operator==(IN const AString& objA1, IN const IMS_CHAR* pszA2)
{
    return objA1.Equals(pszA2);
}
inline IMS_BOOL operator==(IN const IMS_CHAR* pszA1, IN const AString& objA2)
{
    return objA2.Equals(pszA1);
}

inline IMS_BOOL operator!=(IN const AString& objA1, IN const AString& objA2)
{
    return !objA1.Equals(objA2);
}
inline IMS_BOOL operator!=(IN const AString& objA1, IN const IMS_CHAR* pszA2)
{
    return !objA1.Equals(pszA2);
}
inline IMS_BOOL operator!=(IN const IMS_CHAR* pszA1, IN const AString& objA2)
{
    return !objA2.Equals(pszA1);
}

inline const AString operator+(IN const AString& objA1, IN const AString& objA2)
{
    return AString(objA1) += objA2;
}
inline const AString operator+(IN const AString& objA1, IN const IMS_CHAR* pszA2)
{
    return AString(objA1) += pszA2;
}
inline const AString operator+(IN const AString& objA1, IN const IMS_CHAR cA2)
{
    return AString(objA1) += cA2;
}
inline const AString operator+(IN const IMS_CHAR* pszA1, IN const AString& objA2)
{
    return AString(pszA1) += objA2;
}
inline const AString operator+(IN const IMS_CHAR cA1, IN const AString& objA1)
{
    return AString(&cA1, 1) += objA1;
}

// For ImsMap class
inline IMS_BOOL operator<(IN const AString& objA1, IN const AString& objA2)
{
    return (AString::Compare(objA1.GetStr(), objA2.GetStr()) < 0);
}

class WCharPtr : public RcObject
{
public:
    explicit WCharPtr(IN const AString& str);
    WCharPtr(IN const WCharPtr& other);
    virtual ~WCharPtr();

public:
    WCharPtr& operator=(IN const WCharPtr& other);

public:
    inline IMS_WCHAR* GetStr() const { return m_pwszData; }
    inline IMS_SINT32 GetLength() const { return m_nSize; }

private:
    IMS_SINT32 m_nSize;
    IMS_WCHAR* m_pwszData;
};

#endif
