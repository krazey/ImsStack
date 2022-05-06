/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090605  lovil@                    Created
    </table>

    Description
    A MessageBodyPart can contain different kinds of content, for example text,
    an image or an audio clip.
*/

#include "ServiceMemory.h"
#include "SipHeaderName.h"
#include "base/IMS.h"
#include "IMessage.h"
#include "MessageBodyPart.h"

PUBLIC
MessageBodyPart::MessageBodyPart(IN IMessage* piMessage_, IN ISipMessageBodyPart* piBodyPart_) :
        piMessage(piMessage_),
        piBodyPart(piBodyPart_)
{
}

PUBLIC VIRTUAL MessageBodyPart::~MessageBodyPart() {}

PUBLIC
ISipMessageBodyPart* MessageBodyPart::GetBodyPart() const
{
    //---------------------------------------------------------------------------------------------

    return piBodyPart;
}

PUBLIC
void MessageBodyPart::SetBodyPart(IN ISipMessageBodyPart* piNewBodyPart)
{
    //---------------------------------------------------------------------------------------------

    piBodyPart = piNewBodyPart;
}

PRIVATE VIRTUAL const ByteArray& MessageBodyPart::GetContent() const
{
    //---------------------------------------------------------------------------------------------

    return piBodyPart->GetContent();
}

PRIVATE VIRTUAL AString MessageBodyPart::GetHeader(IN CONST AString& strName) const
{
    //---------------------------------------------------------------------------------------------

    if (strName.IsNULL())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return AString();
    }

    AString strTmp = strName.MakeUpper();

    if (!strTmp.StartsWith("CONTENT-"))
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return AString();
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return piBodyPart->GetHeader(GetHeaderType(strTmp), strName);
}

PRIVATE VIRTUAL IMS_RESULT MessageBodyPart::SetContent(IN CONST ByteArray& objContent)
{
    //---------------------------------------------------------------------------------------------

    if (piMessage->GetState() != IMessage::STATE_UNSENT)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    piBodyPart->SetContent(objContent);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MessageBodyPart::SetHeader(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    //---------------------------------------------------------------------------------------------

    if (strName.IsNULL() || strValue.IsNULL())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check the syntax : strValue

    if (piMessage->GetState() != IMessage::STATE_UNSENT)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    piBodyPart->SetHeader(GetHeaderType(strName), strValue, strName);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE GLOBAL IMS_SINT32 MessageBodyPart::GetHeaderType(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_TYPE))
        return ISipMessageBodyPart::CONTENT_TYPE;
    else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_DISPOSITION))
        return ISipMessageBodyPart::CONTENT_DISPOSITION;
    else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_TRANSFER_ENCODING))
        return ISipMessageBodyPart::CONTENT_TRANSFER_ENCODING;
    else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_ID))
        return ISipMessageBodyPart::CONTENT_ID;
    else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_DESCRIPTION))
        return ISipMessageBodyPart::CONTENT_DESCRIPTION;
    else
        return ISipMessageBodyPart::CONTENT_UNKNOWN;
}
