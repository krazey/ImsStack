/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "IMSLib.h"
#include "IMSStrLib.h"
#include "ByteArray.h"

PRIVATE GLOBAL ByteArray::Data ByteArray::SHARED_NULL =
{
    0, 0, ByteArray::SHARED_NULL.byValue,
    {
        0
    }
};

LOCAL
IMS_SINT32 ByteArray_AllocateMore(IN IMS_SIZE_T nAlloc, IN IMS_SIZE_T nExtra)
{
    const IMS_SIZE_T nPage = 1 << 12;  // 4096 bytes
    IMS_SIZE_T nNumAlloc;

    nAlloc += nExtra;

    if (nAlloc < (1 << 5))  // 32 bytes
    {
        nNumAlloc = (1 << 3) + ((nAlloc >> 3) << 3);
    }
    else
    {
        nNumAlloc = (nAlloc < nPage) ? (1 << 3) : nPage;

        while (nNumAlloc < nAlloc)
        {
            nNumAlloc *= 2;
        }
    }

    return LONG_TO_INT(nNumAlloc - nExtra);
}

PUBLIC
ByteArray::ByteRef::ByteRef(IN ByteArray& objBA_, IN IMS_SINT32 nIndex_) :
        objBA(objBA_),
        nIndex(nIndex_)
{
}

PUBLIC
ByteArray::ByteRef& ByteArray::ByteRef::operator=(IN CONST ByteRef& objRHS)
{
    if (this != &objRHS)
    {
        objBA.pData->pValue[nIndex] = objRHS.objBA.pData->pValue[objRHS.nIndex];
    }

    return (*this);
}

PUBLIC
ByteArray::ByteRef& ByteArray::ByteRef::operator=(IN IMS_BYTE byte)
{
    objBA.pData->pValue[nIndex] = byte;
    return (*this);
}

PUBLIC
ByteArray::ByteArray() :
        pData(&SHARED_NULL)
{
}

PUBLIC
ByteArray::ByteArray(IN IMS_BYTE byte)
{
    pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + 1));

    if (pData == IMS_NULL)
    {
        pData = &SHARED_NULL;
    }
    else
    {
        pData->nAlloc = pData->nSize = 1;
        pData->pValue = pData->byValue;
        pData->byValue[0] = byte;
        pData->byValue[1] = '\0';
    }
}

PUBLIC
ByteArray::ByteArray(IN CONST IMS_CHAR* pValue_)
{
    IMS_UINT32 nSize = IMS_StrLen(pValue_);

    if ((pValue_ == IMS_NULL) || (nSize == 0))
    {
        pData = &SHARED_NULL;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = pData->nSize = nSize;
            pData->pValue = pData->byValue;
            IMS_MEM_Memcpy(pData->byValue, pValue_, nSize);
            pData->byValue[nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray::ByteArray(IN CONST AString& strValue)
{
    IMS_SINT32 nSize = strValue.GetLength();

    if (nSize == 0)
    {
        pData = &SHARED_NULL;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = pData->nSize = nSize;
            pData->pValue = pData->byValue;
            IMS_MEM_Memcpy(pData->byValue, strValue.GetStr(), nSize);
            pData->byValue[nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray::ByteArray(IN CONST IMS_BYTE* pValue_, IN IMS_SINT32 nSize_)
{
    if ((pValue_ == IMS_NULL) || (nSize_ <= 0))
    {
        pData = &SHARED_NULL;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize_));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = pData->nSize = nSize_;
            pData->pValue = pData->byValue;
            IMS_MEM_Memcpy(pData->byValue, pValue_, nSize_);
            pData->byValue[nSize_] = '\0';
        }
    }
}

PUBLIC
ByteArray::ByteArray(IN CONST ByteArray& objRHS)
{
    if (objRHS.pData == &SHARED_NULL)
    {
        pData = &SHARED_NULL;
    }
    else
    {
        pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + objRHS.pData->nAlloc));

        if (pData == IMS_NULL)
        {
            pData = &SHARED_NULL;
        }
        else
        {
            pData->nAlloc = objRHS.pData->nAlloc;
            pData->nSize = objRHS.pData->nSize;
            pData->pValue = pData->byValue;
            IMS_MEM_Memcpy(pData->byValue, objRHS.pData->pValue, pData->nSize);
            pData->byValue[pData->nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray::~ByteArray()
{
    Clear();
}

PUBLIC
ByteArray& ByteArray::operator=(IN CONST ByteArray& objRHS)
{
    if (this != &objRHS)
    {
        if ((objRHS.pData->nSize == 0) || (objRHS.pData->pValue == IMS_NULL))
        {
            Clear();
        }
        else
        {
            IMS_SINT32 nLen = objRHS.pData->nSize;

            if ((nLen > pData->nAlloc) || ((nLen < pData->nSize) && (nLen < (pData->nAlloc >> 1))))
            {
                Realloc(nLen);
            }

            // include NULL character
            IMS_MEM_Memcpy(pData->pValue, objRHS.pData->pValue, nLen + 1);
            pData->nSize = nLen;
        }
    }

    return (*this);
}

PUBLIC
ByteArray& ByteArray::operator=(IN IMS_BYTE byte)
{
    if ((1 > pData->nAlloc) || ((1 < pData->nSize) && (1 < (pData->nAlloc >> 1))))
    {
        Realloc(1);
    }

    pData->pValue[0] = byte;
    pData->pValue[1] = '\0';
    pData->nSize = 1;

    return (*this);
}

PUBLIC
ByteArray& ByteArray::operator+=(IN CONST ByteArray& objBA)
{
    return Append(objBA);
}

PUBLIC
ByteArray& ByteArray::operator+=(IN IMS_BYTE byte)
{
    return Append(byte);
}

PUBLIC
IMS_BYTE ByteArray::operator[](IN IMS_SINT32 nIndex) const
{
    IMS_ASSERT(nIndex >= 0 && nIndex < GetLength());

    return pData->pValue[nIndex];
}

PUBLIC
ByteArray::ByteRef ByteArray::operator[](IN IMS_SINT32 nIndex)
{
    IMS_ASSERT(nIndex >= 0 && nIndex < GetLength());

    return ByteRef(*this, nIndex);
}

PUBLIC
IMS_BYTE ByteArray::operator[](IN IMS_UINT32 nIndex) const
{
    IMS_ASSERT(nIndex < IMS_UINT32(GetLength()));

    return pData->pValue[nIndex];
}

PUBLIC
ByteArray::ByteRef ByteArray::operator[](IN IMS_UINT32 nIndex)
{
    IMS_ASSERT(nIndex < IMS_UINT32(GetLength()));

    return ByteRef(*this, nIndex);
}

PUBLIC
ByteArray& ByteArray::Append(IN IMS_BYTE byte)
{
    if (pData->nSize + 1 > pData->nAlloc)
    {
        Realloc(ByteArray_AllocateMore(pData->nSize + 1, sizeof(Data)));
    }

    pData->pValue[pData->nSize++] = byte;
    pData->pValue[pData->nSize] = '\0';

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Append(IN CONST IMS_BYTE* pValue_, IN IMS_SINT32 nSize_)
{
    if ((nSize_ <= 0) || (pValue_ == IMS_NULL))
    {
        return (*this);
    }

    if (pData->nSize + nSize_ > pData->nAlloc)
    {
        Realloc(ByteArray_AllocateMore(pData->nSize + nSize_, sizeof(Data)));
    }

    IMS_MEM_Memcpy(pData->pValue + pData->nSize, pValue_, nSize_);
    pData->nSize += nSize_;
    pData->pValue[pData->nSize] = '\0';

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Append(IN CONST ByteArray& objBA)
{
    if (pData == &SHARED_NULL)
    {
        (*this) = objBA;
    }
    else if (objBA.pData != &SHARED_NULL)
    {
        if (pData->nSize + objBA.pData->nSize > pData->nAlloc)
        {
            Realloc(ByteArray_AllocateMore(pData->nSize + objBA.pData->nSize, sizeof(Data)));
        }

        IMS_MEM_Memcpy(pData->pValue + pData->nSize, objBA.pData->pValue, objBA.pData->nSize);
        pData->nSize += objBA.pData->nSize;
        pData->pValue[pData->nSize] = '\0';
    }

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    if ((nCount <= 0) || (nOffset >= pData->nSize) || (nOffset < 0))
    {
        return (*this);
    }

    if ((nOffset + nCount) >= pData->nSize)
    {
        Resize(nOffset);
    }
    else
    {
        IMS_MEM_Memmove(pData->pValue + nOffset, pData->pValue + nOffset + nCount,
                pData->nSize - nOffset - nCount);
        Resize(pData->nSize - nCount);
    }

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Prepend(IN IMS_BYTE byte)
{
    if (pData->nSize + 1 > pData->nAlloc)
    {
        Realloc(ByteArray_AllocateMore(pData->nSize + 1, sizeof(Data)));
    }

    IMS_MEM_Memmove(pData->pValue + 1, pData->pValue, pData->nSize);
    pData->pValue[0] = byte;
    ++(pData->nSize);
    pData->pValue[pData->nSize] = '\0';

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Prepend(IN CONST IMS_BYTE* pValue_, IN IMS_SINT32 nSize_)
{
    if ((nSize_ <= 0) || (pValue_ == IMS_NULL))
    {
        return (*this);
    }

    if (pData->nSize + nSize_ > pData->nAlloc)
    {
        Realloc(ByteArray_AllocateMore(pData->nSize + nSize_, sizeof(Data)));
    }

    IMS_MEM_Memmove(pData->pValue + nSize_, pData->pValue, pData->nSize);
    IMS_MEM_Memcpy(pData->pValue, pValue_, nSize_);
    pData->nSize += nSize_;
    pData->pValue[pData->nSize] = '\0';

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Prepend(IN CONST ByteArray& objBA)
{
    if (pData == &SHARED_NULL)
    {
        (*this) = objBA;
    }
    else if (objBA.pData != &SHARED_NULL)
    {
        ByteArray objTmp = (*this);

        (*this) = objBA;
        Append(objTmp);
    }

    return (*this);
}

PUBLIC
void ByteArray::Attach(IN CONST IMS_BYTE* pValue_, IN IMS_SINT32 nSize_)
{
    Data* pNewData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data)));

    if (pValue_)
    {
        pNewData->pValue = const_cast<IMS_BYTE*>(pValue_);
    }
    else
    {
        pNewData->pValue = pNewData->byValue;
        nSize_ = 0;
    }

    pNewData->nAlloc = pNewData->nSize = nSize_;
    pNewData->byValue[0] = '\0';

    Clear();

    pData = pNewData;
}

PUBLIC
void ByteArray::Detach()
{
    if (pData->pValue != pData->byValue)
    {
        Realloc(pData->nSize);
    }
}

PUBLIC
void ByteArray::Resize(IN IMS_SINT32 nSize)
{
    if (nSize <= 0)
    {
        if (pData == &SHARED_NULL)
        {
            return;
        }

        Clear();
    }
    else
    {
        if ((nSize > pData->nAlloc) || ((nSize < pData->nSize) && (nSize < (pData->nAlloc >> 1))))
        {
            Realloc(ByteArray_AllocateMore(nSize, sizeof(Data)));
        }

        if (pData->nAlloc >= nSize)
        {
            pData->nSize = nSize;
            pData->pValue = pData->byValue;
            pData->byValue[nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray ByteArray::GetSubData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount /* = -1 */) const
{
    if ((pData == &SHARED_NULL) || (nOffset >= pData->nSize))
    {
        return ByteArray::ConstNull();
    }

    if (nCount < 0)
    {
        nCount = pData->nSize - nOffset;
    }

    if (nOffset < 0)
    {
        nCount += nOffset;
        nOffset = 0;
    }

    if ((nCount + nOffset) > pData->nSize)
    {
        nCount = pData->nSize - nOffset;
    }

    if ((nOffset == 0) && (nCount == pData->nSize))
    {
        return (*this);
    }

    return ByteArray(pData->pValue + nOffset, nCount);
}

PUBLIC
AString ByteArray::ToString() const
{
    if (pData == &SHARED_NULL)
    {
        return AString::ConstNull();
    }

    return AString(reinterpret_cast<const IMS_CHAR*>(pData->pValue), pData->nSize);
}

PUBLIC
AString ByteArray::ToHexString() const
{
    if (pData == &SHARED_NULL)
    {
        return AString::ConstNull();
    }

    AString strTemp;
    AString strHexString;

    for (IMS_SINT32 i = 0; i < pData->nSize; ++i)
    {
        strTemp.Sprintf("%02x", pData->pValue[i]);
        strHexString.Append(strTemp);
    }

    return strHexString;
}

PUBLIC GLOBAL const ByteArray& ByteArray::ConstNull()
{
    static const ByteArray CONST_NULL;

    return CONST_NULL;
}

PRIVATE
void ByteArray::Clear()
{
    if (pData == &SHARED_NULL)
    {
        return;
    }

    IMS_MEM_Free(pData);
    pData = &SHARED_NULL;
}

PRIVATE
void ByteArray::Realloc(IN IMS_SINT32 nAlloc)
{
    Data* pNewData;

    if ((pData == &SHARED_NULL) || (pData->pValue != pData->byValue))
    {
        pNewData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nAlloc));

        if (pNewData == IMS_NULL)
        {
            return;
        }

        pNewData->nSize = IMS_MIN(nAlloc, pData->nSize);
        IMS_MEM_Memcpy(pNewData->byValue, pData->pValue, pNewData->nSize);
        pNewData->byValue[pNewData->nSize] = '\0';
        pNewData->nAlloc = nAlloc;
        pNewData->pValue = pNewData->byValue;

        Clear();
    }
    else
    {
        pNewData = static_cast<Data*>(IMS_MEM_Realloc(pData, sizeof(Data) + nAlloc));

        if (pNewData == IMS_NULL)
        {
            return;
        }

        pNewData->nAlloc = nAlloc;
        pNewData->pValue = pNewData->byValue;
    }

    pData = pNewData;
}
