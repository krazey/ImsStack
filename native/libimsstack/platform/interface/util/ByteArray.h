/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _BYTE_ARRAY_H_
#define _BYTE_ARRAY_H_

#include "AString.h"

class ByteArray
{
public:
    class ByteRef
    {
    public:
        ByteRef(IN ByteArray& objBA_, IN IMS_SINT32 nIndex_);

    public:
        // lvalue operation
        ByteRef& operator=(IN CONST ByteRef& objRHS);
        ByteRef& operator=(IN IMS_BYTE byte);
        // rvalue operation
        inline operator IMS_BYTE() const
        {
            return (nIndex < objBA.pData->nSize) ? objBA.pData->pValue[nIndex] : 0;
        }

    private:
        ByteArray& objBA;
        IMS_SINT32 nIndex;
    };

public:
    ByteArray();
    ByteArray(IN IMS_BYTE byte);
    ByteArray(IN CONST IMS_CHAR* pValue_);
    ByteArray(IN CONST AString& strValue);
    ByteArray(IN CONST IMS_BYTE* pValue_, IN IMS_SINT32 nSize_);
    ByteArray(IN CONST ByteArray& objBA);
    ~ByteArray();

public:
    ByteArray& operator=(IN CONST ByteArray& objBA);
    ByteArray& operator=(IN IMS_BYTE byte);
    ByteArray& operator+=(IN CONST ByteArray& objBA);
    ByteArray& operator+=(IN IMS_BYTE byte);
    IMS_BYTE operator[](IN IMS_SINT32 nIndex) const;
    ByteRef operator[](IN IMS_SINT32 nIndex);
    IMS_BYTE operator[](IN IMS_UINT32 nIndex) const;
    ByteRef operator[](IN IMS_UINT32 nIndex);

public:
    ByteArray& Append(IN IMS_BYTE byte);
    ByteArray& Append(IN CONST IMS_BYTE* pValue_, IN IMS_SINT32 nSize_);
    ByteArray& Append(IN CONST ByteArray& objBA);
    ByteArray& Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);
    ByteArray& Prepend(IN IMS_BYTE byte);
    ByteArray& Prepend(IN CONST IMS_BYTE* pValue_, IN IMS_SINT32 nSize_);
    ByteArray& Prepend(IN CONST ByteArray& objBA);

    void Attach(IN CONST IMS_BYTE* pValue_, IN IMS_SINT32 nSize_);
    void Detach();
    void Resize(IN IMS_SINT32 nSize);

    inline const IMS_BYTE* GetData() const { return pData->pValue; }
    inline IMS_BYTE* GetData()
    {
        Detach();
        return pData->pValue;
    }
    inline IMS_SINT32 GetLength() const { return pData->nSize; }
    ByteArray GetSubData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount = -1) const;

    inline IMS_BOOL IsNULL() const { return (pData == &SHARED_NULL); }
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

    Data* pData;
};

inline const ByteArray operator+(IN CONST ByteArray& objBA1, IN CONST ByteArray& objBA2)
{
    return ByteArray(objBA1) += objBA2;
}

inline const ByteArray operator+(IN CONST ByteArray& objBA1, IN IMS_BYTE byte)
{
    return ByteArray(objBA1) += byte;
}

inline const ByteArray operator+(IN IMS_BYTE byte, IN CONST ByteArray& objBA1)
{
    return ByteArray(byte) += objBA1;
}

#endif  // _BYTE_ARRAY_H_
