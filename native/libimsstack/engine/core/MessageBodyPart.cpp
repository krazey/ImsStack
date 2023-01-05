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
#include "ServiceMemory.h"

#include "IMessage.h"
#include "MessageBodyPart.h"
#include "SipHeaderName.h"
#include "base/Ims.h"

PUBLIC
MessageBodyPart::MessageBodyPart(IN IMessage* piMessage, IN ISipMessageBodyPart* piBodyPart) :
        m_piMessage(piMessage),
        m_piBodyPart(piBodyPart)
{
}

PRIVATE VIRTUAL AString MessageBodyPart::GetHeader(IN const AString& strName) const
{
    if (strName.IsNULL())
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return AString();
    }

    AString strTmp = strName.MakeUpper();

    if (!strTmp.StartsWith("CONTENT-"))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return AString();
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return m_piBodyPart->GetHeader(GetHeaderType(strTmp), strName);
}

PRIVATE VIRTUAL IMS_RESULT MessageBodyPart::SetContent(IN const ByteArray& objContent)
{
    if (m_piMessage->GetState() != IMessage::STATE_UNSENT)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    m_piBodyPart->SetContent(objContent);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MessageBodyPart::SetHeader(
        IN const AString& strName, IN const AString& strValue)
{
    if (strName.IsNULL() || strValue.IsNULL())
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Check the syntax : strValue

    if (m_piMessage->GetState() != IMessage::STATE_UNSENT)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    m_piBodyPart->SetHeader(GetHeaderType(strName), strValue, strName);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE GLOBAL IMS_SINT32 MessageBodyPart::GetHeaderType(IN const AString& strName)
{
    if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_TYPE))
    {
        return ISipMessageBodyPart::CONTENT_TYPE;
    }
    else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_DISPOSITION))
    {
        return ISipMessageBodyPart::CONTENT_DISPOSITION;
    }
    else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_TRANSFER_ENCODING))
    {
        return ISipMessageBodyPart::CONTENT_TRANSFER_ENCODING;
    }
    else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_ID))
    {
        return ISipMessageBodyPart::CONTENT_ID;
    }
    else if (strName.EqualsIgnoreCase(SipHeaderName::CONTENT_DESCRIPTION))
    {
        return ISipMessageBodyPart::CONTENT_DESCRIPTION;
    }
    else
    {
        return ISipMessageBodyPart::CONTENT_UNKNOWN;
    }
}
