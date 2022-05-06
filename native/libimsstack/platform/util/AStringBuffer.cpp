/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100426  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "AStringBuffer.h"

PUBLIC
AStringBuffer::AStringBuffer() :
        strValue(AString::ConstNull())
{
}

PUBLIC
AStringBuffer::AStringBuffer(IN IMS_SINT32 nSize) :
        strValue(AString(nSize))
{
}

PUBLIC
AStringBuffer::AStringBuffer(IN CONST AString& strValue_) :
        strValue(strValue_)
{
}

PUBLIC
AStringBuffer::AStringBuffer(IN CONST AStringBuffer& objRHS) :
        strValue(objRHS.strValue)
{
}

PUBLIC
AStringBuffer::~AStringBuffer() {}

PUBLIC
AStringBuffer& AStringBuffer::operator=(IN CONST AStringBuffer& objRHS)
{
    if (this != &objRHS)
    {
        strValue = objRHS.strValue;
    }

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::operator=(IN CONST IMS_CHAR ch)
{
    strValue = ch;

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::operator=(IN CONST IMS_CHAR* pszValue)
{
    strValue = pszValue;

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::operator=(IN CONST AString& strValue)
{
    this->strValue = strValue;

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::operator+=(IN CONST AStringBuffer& objRHS)
{
    strValue += objRHS.strValue;

    return (*this);
}

PUBLIC
void AStringBuffer::SetCapacity(IN IMS_SINT32 nSize)
{
    strValue.Realloc(nSize);
}

PUBLIC
AStringBuffer& AStringBuffer::Sprintf(IN CONST IMS_CHAR* pszFormat, ...)
{
    va_list ap;

    va_start(ap, pszFormat);
    strValue.Vsprintf(pszFormat, ap);
    va_end(ap);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN CONST IMS_SINT16 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN CONST IMS_UINT16 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN CONST IMS_SINT32 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN CONST IMS_UINT32 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN CONST IMS_SLONG nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(static_cast<IMS_SINT64>(nValue));
    strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN CONST IMS_ULONG nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(static_cast<IMS_UINT64>(nValue));
    strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN CONST IMS_SINT16 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN CONST IMS_UINT16 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN CONST IMS_SINT32 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN CONST IMS_UINT32 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN CONST IMS_SLONG nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(static_cast<IMS_SINT64>(nValue));
    strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN CONST IMS_ULONG nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(static_cast<IMS_UINT64>(nValue));
    strValue.Insert(i, strTmpVal);

    return (*this);
}
