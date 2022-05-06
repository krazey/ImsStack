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
#include "SIPPrivate.h"
#include "SipHeaderName.h"
#include "SIPHeader.h"
#include "SIPUnknownHeaders.h"

PUBLIC
SIPUnknownHeaders::Header::Header(IN CONST AString& strName_) :
        strCompactName(AString::ConstNull()),
        strName(AString::ConstNull())
{
    // Headers with a compact form : Subject (s), Identity (y), Identity-Info (n)
    SetName(strName_);
}

PUBLIC
SIPUnknownHeaders::Header::Header(IN CONST SIPUnknownHeaders::Header& objRHS) :
        strCompactName(objRHS.strCompactName),
        strName(objRHS.strName),
        objBodys(objRHS.objBodys)
{
}

PUBLIC
SIPUnknownHeaders::Header::~Header() {}

PUBLIC
SIPUnknownHeaders::Header& SIPUnknownHeaders::Header::operator=(
        IN CONST SIPUnknownHeaders::Header& objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        strCompactName = objRHS.strCompactName;
        strName = objRHS.strName;
        objBodys = objRHS.objBodys;
    }

    return (*this);
}

PUBLIC
void SIPUnknownHeaders::Header::Clear()
{
    //---------------------------------------------------------------------------------------------

    objBodys.Clear();
}

PUBLIC
IMS_BOOL SIPUnknownHeaders::Header::Equals(IN CONST AString& strName) const
{
    //---------------------------------------------------------------------------------------------

    if (strName.IsNULL() || strName.IsEmpty())
        return IMS_FALSE;

    if (strCompactName.EqualsIgnoreCase(strName))
        return IMS_TRUE;

    return this->strName.EqualsIgnoreCase(strName);
}

PRIVATE
void SIPUnknownHeaders::Header::SetName(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    // Headers with a compact form : Subject (s), Identity (y), Identity-Info (n)

    if (strName.EqualsIgnoreCase(SipHeaderName::CF_SUBJECT))
    {
        this->strCompactName = strName;
        this->strName = SipHeaderName::SUBJECT;
    }
    else if (strName.EqualsIgnoreCase(SipHeaderName::CF_IDENTITY))
    {
        this->strCompactName = strName;
        this->strName = SipHeaderName::IDENTITY;
    }
    else if (strName.EqualsIgnoreCase(SipHeaderName::CF_IDENTITY_INFO))
    {
        this->strCompactName = strName;
        this->strName = SipHeaderName::IDENTITY_INFO;
    }
    else
    {
        switch (strName[0])
        {
            case 's':
            case 'S':
                if (strName.EqualsIgnoreCase(SipHeaderName::SUBJECT))
                {
                    this->strCompactName = SipHeaderName::CF_SUBJECT;
                }

                this->strName = strName;
                break;

            case 'i':
            case 'I':
                if (strName.EqualsIgnoreCase(SipHeaderName::IDENTITY))
                {
                    this->strCompactName = SipHeaderName::CF_IDENTITY;
                }
                else if (strName.EqualsIgnoreCase(SipHeaderName::IDENTITY_INFO))
                {
                    this->strCompactName = SipHeaderName::CF_IDENTITY_INFO;
                }

                this->strName = strName;
                break;

            default:
                this->strName = strName;
                break;
        }
    }
}

PUBLIC
SIPUnknownHeaders::SIPUnknownHeaders() {}

PUBLIC
SIPUnknownHeaders::SIPUnknownHeaders(IN CONST SIPUnknownHeaders& objRHS)
{
    Header* pNewHeader;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objRHS.objHeaders.GetSize(); ++i)
    {
        const Header* pHeader = objRHS.objHeaders.GetAt(i);

        pNewHeader = new Header(*pHeader);

        if (pNewHeader != IMS_NULL)
        {
            objHeaders.Append(pNewHeader);
        }
    }
}

PUBLIC
SIPUnknownHeaders::~SIPUnknownHeaders()
{
    Clear();
}

/*

Remarks

*/
PUBLIC
SIPUnknownHeaders& SIPUnknownHeaders::operator=(IN CONST SIPUnknownHeaders& objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        Header* pNewHeader;

        Clear();

        for (IMS_UINT32 i = 0; i < objRHS.objHeaders.GetSize(); ++i)
        {
            const Header* pHeader = objRHS.objHeaders.GetAt(i);

            pNewHeader = new Header(*pHeader);

            if (pNewHeader != IMS_NULL)
            {
                objHeaders.Append(pNewHeader);
            }
        }
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPUnknownHeaders::AddHeader(IN CONST AString& strName, IN CONST AString& strBody)
{
    Header* pHeader = FindHeader(strName);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
    {
        pHeader = new Header(strName);

        if (pHeader == IMS_NULL)
        {
            SIPPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }

        if (!pHeader->objBodys.Append(strBody))
        {
            delete pHeader;

            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }

        if (!objHeaders.Append(pHeader))
        {
            delete pHeader;

            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }
    else
    {
        if (!pHeader->objBodys.Append(strBody))
        {
            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void SIPUnknownHeaders::Clear()
{
    //---------------------------------------------------------------------------------------------

    while (!objHeaders.IsEmpty())
    {
        Header* pHeader = objHeaders.GetAt(0);

        if (pHeader != IMS_NULL)
            delete pHeader;

        objHeaders.RemoveAt(0);
    }
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPUnknownHeaders::GetCount() const
{
    //---------------------------------------------------------------------------------------------

    return objHeaders.GetSize();
}

/*

Remarks

*/
PUBLIC
AString SIPUnknownHeaders::GetHeader(
        IN CONST AString& strName, IN IMS_SINT32 nIndex /* = 0 */) const
{
    Header* pHeader = FindHeader(strName);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
        return AString::ConstNull();

    return pHeader->objBodys.GetAt(nIndex);
}

/*

Remarks

*/
PUBLIC
AString SIPUnknownHeaders::GetHeaderBodys(IN IMS_SINT32 nPos) const
{
    Header* pHeader = objHeaders.GetAt(nPos);

    //---------------------------------------------------------------------------------------------

    if (pHeader != IMS_NULL)
    {
        AString strBodys;

        if (!pHeader->objBodys.IsEmpty())
        {
            strBodys = pHeader->objBodys.GetAt(0);
        }

        for (IMS_UINT32 i = 1; i < pHeader->objBodys.GetSize(); ++i)
        {
            strBodys.Append(TextParser::CHAR_COMMA);
            strBodys.Append(TextParser::CHAR_SP);
            strBodys.Append(pHeader->objBodys.GetAt(i));
        }

        return strBodys;
    }

    return AString::ConstEmpty();
}

/*

Remarks

*/
PUBLIC
AString SIPUnknownHeaders::GetHeaderBodys(IN CONST AString& strName) const
{
    Header* pHeader = FindHeader(strName);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
        return AString::ConstNull();

    AString strBodys;

    if (!pHeader->objBodys.IsEmpty())
    {
        strBodys = pHeader->objBodys.GetAt(0);
    }

    for (IMS_UINT32 i = 1; i < pHeader->objBodys.GetSize(); ++i)
    {
        strBodys.Append(TextParser::CHAR_COMMA);
        strBodys.Append(TextParser::CHAR_SP);
        strBodys.Append(pHeader->objBodys.GetAt(i));
    }

    return strBodys;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPUnknownHeaders::GetHeaderCount(IN CONST AString& strName) const
{
    Header* pHeader = FindHeader(strName);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
        return 0;

    return pHeader->objBodys.GetSize();
}

/*

Remarks

*/
PUBLIC
const AString& SIPUnknownHeaders::GetHeaderName(
        IN IMS_SINT32 nPos, IN IMS_BOOL bCompactForm /* = IMS_FALSE */) const
{
    Header* pHeader = objHeaders.GetAt(nPos);

    //---------------------------------------------------------------------------------------------

    if (pHeader != IMS_NULL)
    {
        if (bCompactForm)
            return (pHeader->strCompactName.GetLength() > 0) ? pHeader->strCompactName
                                                             : pHeader->strName;
        else
            return pHeader->strName;
    }

    return AString::ConstNull();
}

/*

Remarks

*/
PUBLIC
IMSList<AString> SIPUnknownHeaders::GetHeaders(IN CONST AString& strName) const
{
    Header* pHeader = FindHeader(strName);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
        return IMSList<AString>();  // ???, Throw exception

    return pHeader->objBodys;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPUnknownHeaders::IsHeaderPresent(IN CONST AString& strName) const
{
    //---------------------------------------------------------------------------------------------

    return (FindHeader(strName) != IMS_NULL);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPUnknownHeaders::OverwriteHeaders(IN CONST SIPUnknownHeaders& objOther)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objOther.objHeaders.GetSize(); ++i)
    {
        Header* pHeader = objOther.objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            DeleteHeader(pHeader->strName);

            for (IMS_UINT32 j = 0; j < pHeader->objBodys.GetSize(); ++j)
            {
                const AString& strBody = pHeader->objBodys.GetAt(j);

                if (AddHeader(pHeader->strName, strBody) != IMS_SUCCESS)
                {
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPUnknownHeaders::PrependHeader(IN CONST AString& strName, IN CONST AString& strBody)
{
    Header* pHeader = FindHeader(strName);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
    {
        pHeader = new Header(strName);

        if (pHeader == IMS_NULL)
        {
            SIPPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }

        if (!pHeader->objBodys.Prepend(strBody))
        {
            delete pHeader;

            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }

        if (!objHeaders.Append(pHeader))
        {
            delete pHeader;

            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }
    else
    {
        if (!pHeader->objBodys.Prepend(strBody))
        {
            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void SIPUnknownHeaders::RemoveHeader(IN CONST AString& strName)
{
    Header* pHeader = FindHeader(strName);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
        return;

    pHeader->objBodys.RemoveAt(0);

    if (pHeader->objBodys.GetSize() == 0)
        DeleteHeader(strName);
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPUnknownHeaders::SetHeader(IN CONST AString& strName, IN CONST AString& strBody)
{
    Header* pHeader = FindHeader(strName);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
    {
        pHeader = new Header(strName);

        if (pHeader == IMS_NULL)
        {
            SIPPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }

        if (pHeader->objBodys.IsEmpty())
        {
            if (!pHeader->objBodys.InsertAt(strBody, 0))
            {
                delete pHeader;

                SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
                return IMS_FAILURE;
            }
        }
        else
        {
            if (!pHeader->objBodys.SetAt(strBody, 0))
            {
                delete pHeader;

                SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
                return IMS_FAILURE;
            }
        }

        if (!objHeaders.Append(pHeader))
        {
            delete pHeader;

            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }
    else
    {
        if (pHeader->objBodys.IsEmpty())
        {
            if (!pHeader->objBodys.InsertAt(strBody, 0))
            {
                SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
                return IMS_FAILURE;
            }
        }
        else
        {
            if (!pHeader->objBodys.SetAt(strBody, 0))
            {
                SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
                return IMS_FAILURE;
            }
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
void SIPUnknownHeaders::DeleteHeader(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        Header* pHeader = objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            if (pHeader->Equals(strName))
            {
                delete pHeader;
                objHeaders.RemoveAt(i);
                return;
            }
        }
    }
}

/*

Remarks

*/
PRIVATE
SIPUnknownHeaders::Header* SIPUnknownHeaders::FindHeader(IN CONST AString& strName) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        Header* pHeader = objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            if (pHeader->Equals(strName))
            {
                return pHeader;
            }
        }
    }

    return IMS_NULL;
}
