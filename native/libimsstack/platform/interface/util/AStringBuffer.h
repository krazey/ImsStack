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
#ifndef ANSI_STRING_BUFFER_H_
#define ANSI_STRING_BUFFER_H_

#include "AString.h"

class AStringBuffer
{
public:
    AStringBuffer();
    explicit AStringBuffer(IN IMS_SINT32 nSize);
    AStringBuffer(IN const AString& strValue);
    AStringBuffer(IN const AStringBuffer& other);
    inline ~AStringBuffer() {}

public:
    AStringBuffer& operator=(IN const AStringBuffer& other);
    AStringBuffer& operator=(IN const IMS_CHAR c);
    AStringBuffer& operator=(IN const IMS_CHAR* pszValue);
    AStringBuffer& operator=(IN const AString& strValue);
    AStringBuffer& operator+=(IN const AStringBuffer& other);
    inline AStringBuffer& operator+=(IN const IMS_CHAR c) { return Append(c); }
    inline AStringBuffer& operator+=(IN const IMS_CHAR* pszValue) { return Append(pszValue); }
    inline AStringBuffer& operator+=(IN const AString& strValue) { return Append(strValue); }
    inline AStringBuffer& operator+=(IN const IMS_SINT16 nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN const IMS_UINT16 nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN const IMS_SINT32 nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN const IMS_UINT32 nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN const IMS_SLONG nValue) { return Append(nValue); }
    inline AStringBuffer& operator+=(IN const IMS_ULONG nValue) { return Append(nValue); }
    inline IMS_CHAR operator[](IN IMS_SINT32 nIndex) const { return m_strValue[nIndex]; }
    inline AString::CharRef operator[](IN IMS_SINT32 nIndex) { return m_strValue[nIndex]; }
    inline IMS_CHAR operator[](IN IMS_UINT32 nIndex) const { return m_strValue[nIndex]; }
    inline AString::CharRef operator[](IN IMS_UINT32 nIndex) { return m_strValue[nIndex]; }

public:
    inline IMS_SINT32 GetCapacity() const { return m_strValue.GetCapacity(); }
    inline void SetCapacity(IN IMS_SINT32 nSize) { m_strValue.Realloc(nSize); }
    inline IMS_SINT32 GetLength() const { return m_strValue.GetLength(); }

    inline IMS_CHAR* GetCharString() { return m_strValue.GetStr(); }
    inline const IMS_CHAR* GetCharString() const
    {
        return static_cast<const AString&>(m_strValue).GetStr();
    }
    inline AString GetString() { return m_strValue; }
    inline const AString& GetString() const { return m_strValue; }

    inline IMS_BOOL IsEmpty() const { return m_strValue.IsEmpty(); }
    inline IMS_BOOL IsNull() const { return m_strValue.IsNull(); }
    inline IMS_BOOL IsNULL() const { return IsNull(); }

    inline void Chop(IN IMS_SINT32 nCount) { m_strValue.Chop(nCount); }
    inline void Truncate(IN IMS_SINT32 nOffset) { m_strValue.Truncate(nOffset); }

    inline AStringBuffer& Append(IN const IMS_CHAR c)
    {
        m_strValue.Append(c);
        return (*this);
    }
    inline AStringBuffer& Append(IN const IMS_CHAR* pszValue)
    {
        m_strValue.Append(pszValue);
        return (*this);
    }
    inline AStringBuffer& Append(IN const AString& strValue)
    {
        m_strValue.Append(strValue);
        return (*this);
    }
    AStringBuffer& Append(IN const IMS_SINT16 nValue);
    AStringBuffer& Append(IN const IMS_UINT16 nValue);
    AStringBuffer& Append(IN const IMS_SINT32 nValue);
    AStringBuffer& Append(IN const IMS_UINT32 nValue);
    AStringBuffer& Append(IN const IMS_SLONG nValue);
    AStringBuffer& Append(IN const IMS_ULONG nValue);
    inline AStringBuffer& Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
    {
        m_strValue.Erase(nOffset, nCount);
        return (*this);
    }
    inline AStringBuffer& Fill(IN IMS_CHAR c, IN IMS_SINT32 nSize = -1)
    {
        m_strValue.Fill(c, nSize);
        return (*this);
    }
    inline AStringBuffer& Insert(IN IMS_SINT32 i, IN const IMS_CHAR c)
    {
        m_strValue.Insert(i, c);
        return (*this);
    }
    inline AStringBuffer& Insert(IN IMS_SINT32 i, IN const IMS_CHAR* pszValue)
    {
        m_strValue.Insert(i, pszValue);
        return (*this);
    }
    inline AStringBuffer& Insert(IN IMS_SINT32 i, IN const AString& strValue)
    {
        m_strValue.Insert(i, strValue);
        return (*this);
    }
    AStringBuffer& Insert(IN IMS_SINT32 i, IN const IMS_SINT16 nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN const IMS_UINT16 nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN const IMS_SINT32 nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN const IMS_UINT32 nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN const IMS_SLONG nValue);
    AStringBuffer& Insert(IN IMS_SINT32 i, IN const IMS_ULONG nValue);
    AStringBuffer& Prepend(IN const IMS_CHAR c)
    {
        m_strValue.Prepend(c);
        return (*this);
    }
    inline AStringBuffer& Prepend(IN const IMS_CHAR* pszValue)
    {
        m_strValue.Prepend(pszValue);
        return (*this);
    }
    inline AStringBuffer& Prepend(IN const AString& strValue)
    {
        m_strValue.Prepend(strValue);
        return (*this);
    }
    inline AStringBuffer& Replace(
            IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN const IMS_CHAR* pszNew)
    {
        m_strValue.Replace(nOffset, nCount, pszNew);
        return (*this);
    }
    inline AStringBuffer& Replace(
            IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN const AString& strNew)
    {
        m_strValue.Replace(nOffset, nCount, strNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN const IMS_CHAR cOld, IN const IMS_CHAR cNew)
    {
        m_strValue.Replace(cOld, cNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN const IMS_CHAR cOld, IN const IMS_CHAR* pszNew)
    {
        m_strValue.Replace(cOld, pszNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN const IMS_CHAR cOld, IN const AString& strNew)
    {
        m_strValue.Replace(cOld, strNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN const IMS_CHAR* pszOld, IN const IMS_CHAR* pszNew)
    {
        m_strValue.Replace(pszOld, pszNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN const IMS_CHAR* pszOld, IN const AString& strNew)
    {
        m_strValue.Replace(pszOld, strNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN const AString& strOld, IN const IMS_CHAR* pszNew)
    {
        m_strValue.Replace(strOld, pszNew);
        return (*this);
    }
    inline AStringBuffer& Replace(IN const AString& strOld, IN const AString& strNew)
    {
        m_strValue.Replace(strOld, strNew);
        return (*this);
    }

    AStringBuffer& Sprintf(IN const IMS_CHAR* pszFormat, ...);

private:
    AString m_strValue;
};

inline const AStringBuffer operator+(IN const AStringBuffer& objA1, IN const AStringBuffer& objA2)
{
    return AStringBuffer(objA1) += objA2;
}

inline const AStringBuffer operator+(IN const AStringBuffer& objA1, IN const IMS_CHAR* pszA2)
{
    return AStringBuffer(objA1) += pszA2;
}

inline const AStringBuffer operator+(IN const AStringBuffer& objA1, IN const IMS_CHAR cA2)
{
    return AStringBuffer(objA1) += cA2;
}

inline const AStringBuffer operator+(IN const IMS_CHAR* pszA1, IN const AStringBuffer& objA2)
{
    return AStringBuffer(AString(pszA1)) += objA2;
}

inline const AStringBuffer operator+(IN const IMS_CHAR cA1, IN const AStringBuffer& objA1)
{
    return AStringBuffer(AString(&cA1, 1)) += objA1;
}

#endif
