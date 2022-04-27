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
#include "SIPMessageBodyPart.h"

__IMS_TRACE_TAG_SIP__;



PUBLIC
SIPMessageBodyPart::SIPMessageBodyPart(IN IMS_BOOL bSDPBody_ /* = IMS_FALSE */)
    : bSDPBody(bSDPBody_)
    , pstMsgBody(IMS_NULL)
{
    //---------------------------------------------------------------------------------------------

    pstMsgBody = SIPStack::CreateMessageBody();
}

PUBLIC
SIPMessageBodyPart::SIPMessageBodyPart(IN SipMsgBody *pstMsgBody_,
        IN IMS_BOOL bSDPBody_ /* = IMS_FALSE */)
    : bSDPBody(bSDPBody_)
    , pstMsgBody(pstMsgBody_)
{
    //---------------------------------------------------------------------------------------------

    if (pstMsgBody != IMS_NULL)
    {
        SIPStack::AddReference(pstMsgBody);

        (void) SIPStack::CreateMIMEHeader(pstMsgBody);
        ExtractProperties();
    }
}

PUBLIC VIRTUAL
SIPMessageBodyPart::~SIPMessageBodyPart()
{
    //---------------------------------------------------------------------------------------------

    SIPStack::FreeMessageBody(pstMsgBody);

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SIPMessageBodyPart", 0, 0, 0);
#endif
}

/*

Remarks

*/
PUBLIC
SIPMessageBodyPart& SIPMessageBodyPart::operator=(IN CONST SIPMessageBodyPart &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        SIPStack::FreeMessageBody(pstMsgBody);

        bSDPBody = objRHS.bSDPBody;

        pstMsgBody = objRHS.pstMsgBody;
        SIPStack::AddReference(pstMsgBody);

        (void) SIPStack::CreateMIMEHeader(pstMsgBody);

        objOtherMimeHeaders = objRHS.objOtherMimeHeaders;
        objContent = objRHS.objContent;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPMessageBodyPart::Destroy()
{
    //---------------------------------------------------------------------------------------------

    delete this;
}

/*

Remarks

*/
PUBLIC VIRTUAL
ISIPMessageBodyPart* SIPMessageBodyPart::Clone() const
{
    SIPMessageBodyPart *pNewBodyPart = new SIPMessageBodyPart(bSDPBody);

    //---------------------------------------------------------------------------------------------

    if (pNewBodyPart == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::NO_MEMORY);
        return IMS_NULL;
    }

    if (pstMsgBody != IMS_NULL)
    {
        SIPStack::FreeMessageBody(pNewBodyPart->pstMsgBody);
        pNewBodyPart->pstMsgBody = SIPStack::CloneMessageBody(pstMsgBody);

        if (pNewBodyPart->pstMsgBody == IMS_NULL)
        {
            delete pNewBodyPart;

            SIPPrivate::SetLastError(SIPError::NO_MEMORY);
            return IMS_NULL;
        }
    }

    pNewBodyPart->objOtherMimeHeaders = objOtherMimeHeaders;
    pNewBodyPart->objContent = objContent;

    SIPPrivate::SetLastError(SIPError::NO_ERROR);
    return pNewBodyPart;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPMessageBodyPart::CopyFrom(IN CONST ISIPMessageBodyPart *piBodyPart)
{
    //---------------------------------------------------------------------------------------------

    if (piBodyPart == IMS_NULL)
    {
        return;
    }

    const SIPMessageBodyPart *pBodyPart = DYNAMIC_CAST(const SIPMessageBodyPart*, piBodyPart);

    if (pBodyPart == IMS_NULL)
    {
        return;
    }

    operator=(*pBodyPart);
}

/*

Remarks

*/
PUBLIC VIRTUAL
AString SIPMessageBodyPart::GetHeader(IN IMS_SINT32 nType,
        IN CONST AString &strName /* = AString::ConstNull() */) const
{
    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
    case CONTENT_TYPE:
    case CONTENT_DISPOSITION:
    case CONTENT_TRANSFER_ENCODING:
    case CONTENT_ID:
    case CONTENT_DESCRIPTION:
        return SIPStack::GetMIMEHeader(pstMsgBody, nType);

    case CONTENT_UNKNOWN:
        if (strName.IsNULL())
            return AString::ConstNull();

        return objOtherMimeHeaders.GetHeader(strName);

    default:
        break;
    }

    return AString::ConstNull();
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPMessageBodyPart::SetHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
        IN CONST AString &strName /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
    case CONTENT_TYPE:
    case CONTENT_DISPOSITION:
    case CONTENT_TRANSFER_ENCODING:
    case CONTENT_ID:
    case CONTENT_DESCRIPTION:
        if (!SIPStack::SetMIMEHeader(nType, strName, strValue, pstMsgBody))
            return; // throw exception
        break;

    case CONTENT_UNKNOWN:
        if (strName.IsNULL())
            return; // throw exception ?

        objOtherMimeHeaders.AddHeader(strName, strValue);
        break;

    default:
        break;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPMessageBodyPart::SetContent(IN CONST ByteArray &objContent)
{
    //---------------------------------------------------------------------------------------------

    // Set the content
    if (!SIPStack::SetContent(objContent.GetData(), objContent.GetLength(), pstMsgBody))
        return;

    // Load the content from SIP message body
    IMS_BYTE *pContent = IMS_NULL;
    IMS_SINT32 nContentLength = 0;

    if (SIPStack::GetContent(pstMsgBody, pContent, nContentLength))
    {
        // Set the reference of SIP message body
        this->objContent.Attach(pContent, nContentLength);
    }
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessageBodyPart::FormMessageBody()
{
    //---------------------------------------------------------------------------------------------

    // Set MIME content headers
    if (objOtherMimeHeaders.GetCount() > 0)
    {
        AString strName;
        AString strBody;

        for (IMS_SINT32 i = 0; i < objOtherMimeHeaders.GetCount(); ++i)
        {
            strName = objOtherMimeHeaders.GetHeaderName(i);
            strBody = objOtherMimeHeaders.GetHeaderBodys(i);

            if (!SIPStack::SetMIMEHeader(CONTENT_UNKNOWN, strName, strBody, pstMsgBody))
            {
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
void SIPMessageBodyPart::SetHeader(IN SipHeaderBase *pstHeader,
        IN IMS_SINT32 nType /* = ISIPMessageBodyPart::CONTENT_UNKNOWN */)
{
    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
    case CONTENT_TYPE:
    case CONTENT_DISPOSITION:
    case CONTENT_TRANSFER_ENCODING:
    case CONTENT_ID:
    case CONTENT_DESCRIPTION:
        if (!SIPStack::SetMIMEHeader(nType, pstHeader, pstMsgBody))
            return; // throw exception
        break;

    case CONTENT_UNKNOWN:
        objOtherMimeHeaders.AddHeader(
                SIPStack::GetUnknownHeaderName(pstHeader),
                SIPStack::GetUnknownHeaderBody(pstHeader));
        break;

    default:
        break;
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPMessageBodyPart::ExtractProperties()
{
    AString strHeader;

    IMS_SINT32 nHCount = SIPStack::GetMIMEHeaderCount(pstMsgBody, CONTENT_UNKNOWN);

    //---------------------------------------------------------------------------------------------

    // Extract an additional MIME content headers
    for (IMS_SINT32 i = 0; i < nHCount; ++i)
    {
        strHeader = SIPStack::GetMIMEHeader(pstMsgBody, CONTENT_UNKNOWN, i);

        IMS_SINT32 nIndex = strHeader.GetIndexOf(TextParser::CHAR_COLON);

        if (nIndex != AString::NPOS)
        {
            AString strHName = strHeader.GetSubStr(0, nIndex).Trim();
            AString strHBody = strHeader.GetSubStr(nIndex + 1).Trim();

            objOtherMimeHeaders.AddHeader(strHName, strHBody);
        }
    }

    // Extract the content
    IMS_BYTE *pContent = IMS_NULL;
    IMS_SINT32 nContentLength = 0;

    if (SIPStack::GetContent(pstMsgBody, pContent, nContentLength))
    {
        // Set the reference of SIP message body
        objContent.Attach(pContent, nContentLength);
    }

    return IMS_TRUE;
}
