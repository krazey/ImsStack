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
#ifndef BYTE_ARRAY_H_
#define BYTE_ARRAY_H_

#include "AString.h"

class ByteArray
{
public:
    class ByteRef
    {
    public:
        ByteRef(IN ByteArray& objBa, IN IMS_SINT32 nIndex);

    public:
        // lvalue operation
        ByteRef& operator=(IN const ByteRef& other);
        ByteRef& operator=(IN IMS_BYTE byte);
        // rvalue operation
        inline operator IMS_BYTE() const
        {
            return (m_nIndex < m_objBa.m_pData->nSize) ? m_objBa.m_pData->pValue[m_nIndex] : 0;
        }

    private:
        ByteArray& m_objBa;
        IMS_SINT32 m_nIndex;
    };

public:
    ByteArray();
    explicit ByteArray(IN IMS_BYTE byte);
    explicit ByteArray(IN const IMS_CHAR* pValue);
    // cppcheck-suppress noExplicitConstructor
    ByteArray(IN const AString& strValue);
    ByteArray(IN const IMS_BYTE* pValue, IN IMS_SINT32 nSize);
    ByteArray(IN const ByteArray& other);
    ~ByteArray();

public:
    ByteArray& operator=(IN const ByteArray& other);
    ByteArray& operator=(IN IMS_BYTE byte);
    inline ByteArray& operator+=(IN const ByteArray& objBa) { return Append(objBa); }
    inline ByteArray& operator+=(IN IMS_BYTE byte) { return Append(byte); }
    IMS_BYTE operator[](IN IMS_SINT32 nIndex) const;
    ByteRef operator[](IN IMS_SINT32 nIndex);
    IMS_BYTE operator[](IN IMS_UINT32 nIndex) const;
    ByteRef operator[](IN IMS_UINT32 nIndex);

public:
    ByteArray& Append(IN IMS_BYTE byte);
    ByteArray& Append(IN const IMS_BYTE* pValue, IN IMS_SINT32 nSize);
    ByteArray& Append(IN const ByteArray& objBa);
    ByteArray& Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);
    ByteArray& Prepend(IN IMS_BYTE byte);
    ByteArray& Prepend(IN const IMS_BYTE* pValue, IN IMS_SINT32 nSize);
    ByteArray& Prepend(IN const ByteArray& objBa);

    void Attach(IN const IMS_BYTE* pValue, IN IMS_SINT32 nSize);
    void Detach();
    void Resize(IN IMS_SINT32 nSize);

    inline const IMS_BYTE* GetData() const { return m_pData->pValue; }
    inline IMS_BYTE* GetData()
    {
        Detach();
        return m_pData->pValue;
    }
    inline IMS_SINT32 GetLength() const { return m_pData->nSize; }
    ByteArray GetSubData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount = -1) const;

    IMS_BOOL Equals(IN IMS_BYTE byte) const;
    IMS_BOOL Equals(IN const ByteArray& objBa) const;

    inline IMS_BOOL IsNull() const { return (m_pData == &SHARED_NULL); }
    inline IMS_BOOL IsNULL() const { return IsNull(); }
    AString ToString() const;
    AString ToHexString() const;

    static const ByteArray& ConstNull();

private:
    void Clear();
    void Realloc(IN IMS_SINT32 nAlloc);

private:
    friend class ByteRef;

    struct Data
    {
        IMS_SINT32 nAlloc;    // Size of allocated buffer
        IMS_SINT32 nSize;     // Size of real content
        IMS_BYTE* pValue;     // Pointer to the content (Points to byValue)
        IMS_BYTE byValue[1];  // Pointer to the content
    };

    static Data SHARED_NULL;

    Data* m_pData;
};

inline IMS_BOOL operator==(IN const ByteArray& objBa1, IN const ByteArray& objBa2)
{
    return objBa1.Equals(objBa2);
}
inline IMS_BOOL operator==(IN const ByteArray& objBa, IN IMS_BYTE byte)
{
    return objBa.Equals(byte);
}
inline IMS_BOOL operator==(IN IMS_BYTE byte, IN const ByteArray& objBa)
{
    return objBa.Equals(byte);
}

inline IMS_BOOL operator!=(IN const ByteArray& objBa1, IN const ByteArray& objBa2)
{
    return !objBa1.Equals(objBa2);
}
inline IMS_BOOL operator!=(IN const ByteArray& objBa, IN IMS_BYTE byte)
{
    return !objBa.Equals(byte);
}
inline IMS_BOOL operator!=(IN IMS_BYTE byte, IN const ByteArray& objBa)
{
    return !objBa.Equals(byte);
}

inline const ByteArray operator+(IN const ByteArray& objBa1, IN const ByteArray& objBa2)
{
    return ByteArray(objBa1) += objBa2;
}

inline const ByteArray operator+(IN const ByteArray& objBa, IN IMS_BYTE byte)
{
    return ByteArray(objBa) += byte;
}

inline const ByteArray operator+(IN IMS_BYTE byte, IN const ByteArray& objBa)
{
    return ByteArray(byte) += objBa;
}

#endif
