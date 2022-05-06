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
#include "SIPHeader.h"
#include "SipHeaderName.h"
#include "SIPMessage.h"
#include "SipParsingHelper.h"
#include "SIPPrivate.h"

PUBLIC GLOBAL ISipHeader* SipParsingHelper::CreateHeader(IN const AString& strName)
{
    return new SIPHeader(strName);
}

PUBLIC GLOBAL ISipHeader* SipParsingHelper::CreateHeader(
        IN const AString& strName, IN const AString& strValue)
{
    SIPHeader* pHeader = new SIPHeader(strName);

    if (pHeader == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    // Parses and sets the header body field
    if (pHeader->SetHeaderValue(strValue) != IMS_SUCCESS)
    {
        delete pHeader;
        return IMS_NULL;
    }

    return pHeader;
}

PUBLIC GLOBAL ISipHeader* SipParsingHelper::CreateHeader(
        IN IMS_SINT32 nType, IN const AString& strValue)
{
    if (!SIPHeader::IsValidType(nType))
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    SIPHeader* pHeader = new SIPHeader(nType);

    if (pHeader == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    // Parses and sets the header body field
    if (pHeader->SetHeaderValue(strValue) != IMS_SUCCESS)
    {
        delete pHeader;
        return IMS_NULL;
    }

    return pHeader;
}

PUBLIC GLOBAL ISipMessage* SipParsingHelper::CreateMessage(IN IMS_SINT32 nType)
{
    if ((nType != ISipMessage::TYPE_REQUEST) && (nType != ISipMessage::TYPE_RESPONSE))
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    return new SIPMessage(nType);
}

PUBLIC GLOBAL ISipMessage* SipParsingHelper::CreateMessage(IN const ByteArray& objMessage)
{
    return SIPMessage::CreateMessage(objMessage);
}

PUBLIC GLOBAL ISipMessageBodyPart* SipParsingHelper::CreateMessageBodyPart()
{
    return new SIPMessageBodyPart();
}

PUBLIC GLOBAL IMS_BOOL SipParsingHelper::CreateMessageBodyParts(IN_OUT ISipMessage* piSipMsg)
{
    SIPMessage* pSipMsg = DYNAMIC_CAST(SIPMessage*, piSipMsg);

    if (pSipMsg == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    return pSipMsg->CreateBodyParts();
}

PUBLIC GLOBAL const AString& SipParsingHelper::GetSipReasonHeader(
        IN const IMSList<AString>& objReasonHeaders)
{
    if (objReasonHeaders.IsEmpty())
    {
        return AString::ConstNull();
    }

    const AString strHeaderNameReason(SipHeaderName::REASON);

    for (IMS_UINT32 i = 0; i < objReasonHeaders.GetSize(); ++i)
    {
        const AString& strReasonHeader = objReasonHeaders.GetAt(i);

        ISipHeader* piHeader = CreateHeader(strHeaderNameReason, strReasonHeader);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        if (piHeader->GetValue().Equals("SIP"))
        {
            piHeader->Destroy();
            return strReasonHeader;
        }

        piHeader->Destroy();
    }

    return AString::ConstNull();
}

PUBLIC GLOBAL IMS_BOOL SipParsingHelper::ParseReasonHeader(
        IN const AString& strHeaderValue, OUT IMS_SINT32& nCause, OUT AString& strText)
{
    if (strHeaderValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    // "protocol" part may not be necessary in application layer.
    AString strProtocol;

    return ParseReasonHeader(strHeaderValue, strProtocol, nCause, strText);
}

PUBLIC GLOBAL IMS_BOOL SipParsingHelper::ParseReasonHeader(IN const AString& strHeaderValue,
        OUT AString& strProtocol, OUT IMS_SINT32& nCause, OUT AString& strText)
{
    if (strHeaderValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    ISipHeader* piHeader = CreateHeader(SipHeaderName::REASON, strHeaderValue);

    if (piHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipParameter* pCauseP = piHeader->GetParameter("cause");
    const SipParameter* pTextP = piHeader->GetParameter("text");

    strProtocol = piHeader->GetValue();

    if (pCauseP == IMS_NULL)
    {
        nCause = (-1);
    }
    else
    {
        const AString& strCause = pCauseP->GetValue();
        nCause = (strCause.GetLength() == 0) ? 0 : strCause.ToInt32();
    }

    if (pTextP == IMS_NULL)
    {
        strText = AString::ConstNull();
    }
    else
    {
        strText = pTextP->GetValue();

        if (strText.GetLength() == 0)
        {
            strText = AString::ConstEmpty();
        }
    }

    piHeader->Destroy();

    return IMS_TRUE;
}
