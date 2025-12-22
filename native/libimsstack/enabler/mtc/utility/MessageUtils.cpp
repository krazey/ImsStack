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

#include "AStringBuffer.h"
#include "CallReasonInfo.h"
#include "Const3GPP.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "IMtcContext.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipMessageBodyPart.h"
#include "Ims3gpp.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "Sip.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "TextParser.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "conferencecall/ConferenceDef.h"
#include "media/IMedia.h"
#include "utility/MessageUtil.h"
#include "utility/MessageUtils.h"
#include <tuple>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC MessageUtils::MessageUtils() {}
PUBLIC VIRTUAL MessageUtils::~MessageUtils() {}

PUBLIC IMessage* MessageUtils::GetPreviousResponse(IN const ISession* piSession,
        IN IMS_SINT32 eServiceMethod, IN IMS_SINT32 nResponseIndex /*= -1*/)
{
    if (piSession == IMS_NULL)
    {
        return IMS_NULL;
    }

    ImsList<IMessage*> lstResponses = piSession->GetPreviousResponses(eServiceMethod);
    IMS_UINT32 nResponseSize = lstResponses.GetSize();

    if (nResponseSize == 0)
    {
        return IMS_NULL;
    }

    if (nResponseIndex < 0)
    {
        nResponseIndex = static_cast<IMS_SINT32>(nResponseSize - 1);
    }

    if (static_cast<IMS_UINT32>(nResponseIndex) >= nResponseSize)
    {
        return IMS_NULL;
    }

    return lstResponses.GetAt(nResponseIndex);
}

PUBLIC IMessage* MessageUtils::GetRemotePreviousMessage(IN ISession* piSession,
        IN IMS_SINT32 eServiceMethod, IN IMS_BOOL bIsUac, IN IMS_SINT32 nResponseIndex /*= -1*/)
{
    if (piSession == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (bIsUac)
    {
        return GetPreviousResponse(piSession, eServiceMethod, nResponseIndex);
    }

    return piSession->GetPreviousRequest(eServiceMethod);
}

PUBLIC IMS_SINT32 MessageUtils::GetResponseStatusCode(
        IN ISession* piSession, IN IMS_SINT32 eServiceMethod, IN IMS_SINT32 nResponseIndex /*= -1*/)
{
    const IMessage* piPreviousMessage =
            GetPreviousResponse(piSession, eServiceMethod, nResponseIndex);
    if (piPreviousMessage == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    return piPreviousMessage->GetStatusCode();
}

PUBLIC ImsList<AString> MessageUtils::GetRemoteUris(IN ISession* piSession, IN PeerType ePeerType)
{
    ImsList<AString> lstUris;

    if (piSession == IMS_NULL)
    {
        return lstUris;
    }

    ImsList<AString> lstAddresses = piSession->GetRemoteUserId();
    if (lstAddresses.IsEmpty())
    {
        const IMessage* piPreviousMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);
        if (piPreviousMessage == IMS_NULL)
        {
            return lstUris;
        }

        IMS_SINT32 eHeaderType = ISipHeader::FROM;
        if (ePeerType == PeerType::MO)
        {
            eHeaderType = ISipHeader::TO;
        }

        lstAddresses = GetHeaders(piPreviousMessage, eHeaderType);
        if (lstAddresses.IsEmpty())
        {
            return lstUris;
        }
    }

    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        SipAddress objSipAddress;
        if (objSipAddress.Create(lstAddresses.GetAt(i)))
        {
            lstUris.Append(objSipAddress.GetUri());
        }
    }

    return lstUris;
}

PUBLIC AString MessageUtils::GetRemoteUri(IN ISession* piSession, IN PeerType ePeerType)
{
    ImsList<AString> lstUris = GetRemoteUris(piSession, ePeerType);
    if (lstUris.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstUris.GetSize(); i++)
    {
        AString strUri = lstUris.GetAt(i);
        if (strUri.GetLength() > 0)
        {
            return strUri;
        }
    }

    return AString::ConstNull();
}

PUBLIC AString MessageUtils::GetSessionId(IN ISession* piSession)
{
    if (piSession == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return piSession->GetSessionId();
}

PUBLIC ImsList<AString> MessageUtils::GetHeaders(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    const ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return ImsList<AString>();
    }

    return piSipMessage->GetHeaders(eHeaderType, strHeaderName);
}

PUBLIC AString MessageUtils::GetHeader(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstHeaders = GetHeaders(piMessage, eHeaderType, strHeaderName);
    if (lstHeaders.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        AString strHeader = lstHeaders.GetAt(i);
        if (strHeader.GetLength() > 0)
        {
            return strHeader;
        }
    }

    return AString::ConstNull();
}

PUBLIC AString MessageUtils::GetHeaderValue(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstHeaders = GetHeaders(piMessage, eHeaderType, strHeaderName);
    if (lstHeaders.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        ISipHeader* piSipHeader = SipParsingHelper::CreateHeader(eHeaderType, lstHeaders.GetAt(i));
        if (piSipHeader == IMS_NULL)
        {
            continue;
        }

        AString strValue = piSipHeader->GetValue();
        piSipHeader->Destroy();

        if (strValue.GetLength() > 0)
        {
            return strValue;
        }
    }

    return AString::ConstNull();
}

PUBLIC IMS_SINT32 MessageUtils::GetHeaderValueInt(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstHeaders = GetHeaders(piMessage, eHeaderType, strHeaderName);
    if (lstHeaders.IsEmpty())
    {
        return -1;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        ISipHeader* piSipHeader = SipParsingHelper::CreateHeader(eHeaderType, lstHeaders.GetAt(i));
        if (piSipHeader == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 nValue = piSipHeader->GetValueInt();
        piSipHeader->Destroy();
        return nValue;
    }

    return -1;
}

PUBLIC AString MessageUtils::GetParameterValue(IN const IMessage* piMessage,
        IN const AString& strParameterName, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstHeaders = GetHeaders(piMessage, eHeaderType, strHeaderName);
    if (lstHeaders.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        AString strValue;
        ISipHeader* piSipHeader = eHeaderType == ISipHeader::UNKNOWN
                ? SipParsingHelper::CreateHeader(strHeaderName, lstHeaders.GetAt(i))
                : SipParsingHelper::CreateHeader(eHeaderType, lstHeaders.GetAt(i));

        if (piSipHeader == IMS_NULL)
        {
            continue;
        }

        const SipParameter* pSipParameter = piSipHeader->GetParameter(strParameterName);
        if (pSipParameter == IMS_NULL)
        {
            const AString& strBody = piSipHeader->GetValue();
            if (strBody.GetLength() > 0 && eHeaderType == ISipHeader::UNKNOWN)
            {
                GetParameterValueFromUnknownHeaderBody(strBody, strParameterName, strValue);
            }
        }
        else
        {
            strValue = pSipParameter->GetValue();
        }

        piSipHeader->Destroy();

        if (strValue.GetLength() > 0)
        {
            return strValue;
        }
    }

    return AString::ConstNull();
}

PUBLIC ImsList<AString> MessageUtils::GetUserParts(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstUserParts;

    ImsList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return lstUserParts;
    }

    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

        AString strUserPart;
        if (objSipAddress.IsSchemeTel())
        {
            strUserPart = objSipAddress.GetHost();
        }
        else
        {
            const SipAddress::UserInfoPart* pUserInfoPart = objSipAddress.GetUserInfoPart();
            if (pUserInfoPart == IMS_NULL)
            {
                continue;
            }
            strUserPart = pUserInfoPart->GetUser();
        }

        lstUserParts.Append(strUserPart);
    }

    return lstUserParts;
}

PUBLIC AString MessageUtils::GetUserPart(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstUserParts = GetUserParts(piMessage, eHeaderType, strHeaderName);

    for (IMS_UINT32 i = 0; i < lstUserParts.GetSize(); i++)
    {
        AString strUserPart = lstUserParts.GetAt(i);
        if (strUserPart.GetLength() > 0)
        {
            return strUserPart;
        }
    }

    return AString::ConstNull();
}

PUBLIC AString MessageUtils::GetUserPart(IN const AString& strUri)
{
    SipAddress objSipAddress;
    if (!objSipAddress.Create(strUri))
    {
        return AString::ConstNull();
    }

    if (objSipAddress.IsSchemeTel())
    {
        return objSipAddress.GetHost();
    }
    else
    {
        const SipAddress::UserInfoPart* pUserInfoPart = objSipAddress.GetUserInfoPart();
        if (pUserInfoPart != IMS_NULL)
        {
            return pUserInfoPart->GetUser();
        }
    }

    return AString::ConstNull();
}

PUBLIC ImsList<AString> MessageUtils::GetUserIds(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return ImsList<AString>();
    }

    ImsList<AString> lstUserIds;
    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

        AString strUserPart;
        if (objSipAddress.IsSchemeSip())
        {
            IMS_SINT32 nSemicolonIndex =
                    objSipAddress.GetUser().GetIndexOf(TextParser::CHAR_SEMICOLON);
            strUserPart = objSipAddress.GetUser().GetSubStr(0, nSemicolonIndex);
        }
        else if (objSipAddress.IsSchemeTel())
        {
            strUserPart = objSipAddress.GetHost();
        }

        lstUserIds.Append(strUserPart);
    }

    return lstUserIds;
}

PUBLIC AString MessageUtils::GetUserId(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstUserIds = GetUserIds(piMessage, eHeaderType, strHeaderName);
    if (lstUserIds.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstUserIds.GetSize(); i++)
    {
        AString strUserId = lstUserIds.GetAt(i);
        if (strUserId.GetLength() > 0)
        {
            return strUserId;
        }
    }

    return AString::ConstNull();
}

PUBLIC ImsList<AString> MessageUtils::GetDisplayNames(IN IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return ImsList<AString>();
    }

    ImsList<AString> lstDisplayNames;
    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

        lstDisplayNames.Append(objSipAddress.GetDisplayName());
    }

    return lstDisplayNames;
}

PUBLIC AString MessageUtils::GetDisplayName(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstDisplayNames = GetDisplayNames(piMessage, eHeaderType, strHeaderName);
    if (lstDisplayNames.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstDisplayNames.GetSize(); i++)
    {
        AString strDisplayName = lstDisplayNames.GetAt(i);
        if (strDisplayName.GetLength() > 0)
        {
            return TextParser::DoPercentDecoding(strDisplayName);
        }
    }

    return AString::ConstNull();
}

PUBLIC ImsList<AString> MessageUtils::GetHosts(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return ImsList<AString>();
    }

    ImsList<AString> lstHosts;
    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

        lstHosts.Append(objSipAddress.GetHost());
    }

    return lstHosts;
}

PUBLIC AString MessageUtils::GetHost(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstHosts = GetHosts(piMessage, eHeaderType, strHeaderName);
    if (lstHosts.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstHosts.GetSize(); i++)
    {
        AString strHost = lstHosts.GetAt(i);
        if (strHost.GetLength() > 0)
        {
            return strHost;
        }
    }

    return AString::ConstNull();
}

PUBLIC AString MessageUtils::GetParameterValueFromUri(IN IMessage* piMessage,
        IN const AString& strParameterName, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);
        AString strValue;

        if (objSipAddress.IsSchemeSip() || objSipAddress.IsSchemeSips())
        {
            const SipAddress::UserInfoPart* pUserInfoPart = objSipAddress.GetUserInfoPart();
            if (pUserInfoPart == IMS_NULL)
            {
                continue;
            }

            const SipParameter* pParameter = pUserInfoPart->GetParameter(strParameterName);
            if (pParameter != IMS_NULL)
            {
                strValue = pParameter->GetValue();
            }

            if (strValue.GetLength() > 0)
            {
                return strValue;
            }
        }

        const SipParameter* pParameter = objSipAddress.GetParameter(strParameterName);
        if (pParameter != IMS_NULL)
        {
            strValue = pParameter->GetValue();
        }

        if (strValue.GetLength() > 0)
        {
            return strValue;
        }
    }

    return AString::ConstNull();
}

PUBLIC ImsList<AString> MessageUtils::GetUris(IN const IMessage* piMessage,
        IN IMS_BOOL bWithParameters, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return ImsList<AString>();
    }

    ImsList<AString> lstUris;
    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

        if (bWithParameters)
        {
            lstUris.Append(objSipAddress.ToString());
        }
        else
        {
            lstUris.Append(objSipAddress.GetUri());
        }
    }

    return lstUris;
}

PUBLIC AString MessageUtils::GetUri(IN const IMessage* piMessage, IN IMS_BOOL bWithParameters,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstUris = GetUris(piMessage, bWithParameters, eHeaderType, strHeaderName);
    if (lstUris.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < lstUris.GetSize(); i++)
    {
        AString strUri = lstUris.GetAt(i);
        if (strUri.GetLength() > 0)
        {
            return strUri;
        }
    }

    return AString::ConstNull();
}

PUBLIC IMS_SINT32 MessageUtils::GetSosTypeFromServiceUrn(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    AString strValue;
    if (GetUrnValue(piMessage, MessageUtil::STR_SERVICE, eHeaderType, strValue, strHeaderName) ==
            IMS_FAILURE)
    {
        // b/236411658 case seems to be controlled by APDS, not IMS Stack so we might be able to
        // use GENERIC urn if there is no Contact header without any configuration.
        return EXTRA_CODE_EMERGENCYSERVICE_GENERIC;
    }

    if (!strValue.StartsWith(MessageUtil::STR_SOS))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_INVALID;
    }

    if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_GENERIC;
    }
    else if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS_AIEC))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_AIEC;
    }
    else if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS_AMBULANCE))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE;
    }
    else if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS_FIRE))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_FIRE;
    }
    else if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS_MARINE))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_MARINE;
    }
    else if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS_MIEC))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_MIEC;
    }
    else if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS_MOUNTAIN))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN;
    }
    else if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS_POLICE))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_POLICE;
    }
    else if (strValue.EqualsIgnoreCase(MessageUtil::STR_SOS_COUNTRY_SPECIFIC))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_INVALID;
    }
    else if (strValue.MakeLower().Contains(MessageUtil::STR_SOS_COUNTRY_SPECIFIC))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC;
    }

    return EXTRA_CODE_EMERGENCYSERVICE_UNSPECIFIED;
}

PUBLIC IMS_SINT32 MessageUtils::GetCauseFromReasonHeader(
        IN const IMessage* piMessage, IN const AString& strProtocol /*= AString::ConstNull()*/)
{
    return GetCauseAndTextFromReasonHeader(piMessage, strProtocol).nCause;
}

PUBLIC ReasonHeaderValue MessageUtils::GetCauseAndTextFromReasonHeader(
        IN const IMessage* piMessage, IN const AString& strProtocol /*= AString::ConstNull()*/)
{
    ReasonHeaderValue objValue;

    ImsList<AString> lstHeaders = GetHeaders(piMessage, ISipHeader::REASON);
    if (lstHeaders.IsEmpty())
    {
        return objValue;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        const AString& strHeader = lstHeaders.GetAt(i);

        AString strReceivedProtocol;
        IMS_SINT32 nCause = -1;
        AString strText;
        if (!SipParsingHelper::ParseReasonHeader(strHeader, strReceivedProtocol, nCause, strText))
        {
            continue;
        }

        if ((strProtocol.GetLength() > 0) && (!strProtocol.EqualsIgnoreCase(strReceivedProtocol)))
        {
            continue;
        }

        objValue.nCause = nCause;
        objValue.strText = strText;
        objValue.strProtocol = strReceivedProtocol;

        return objValue;
    }

    return objValue;
}

PUBLIC ReasonHeaderValue MessageUtils::GetPrioritizedReasonHeader(IN const IMessage* piMessage,
        IN const std::initializer_list<AString>& lstPrioritizedProtocols)
{
    ReasonHeaderValue objResaultValue;
    if (!piMessage)
    {
        return objResaultValue;
    }

    for (const auto& strProtocol : lstPrioritizedProtocols)
    {
        IMS_TRACE_D("GetPrioritizedReasonHeader: Trying protocol [%s]",
                strProtocol.GetStr() ? strProtocol.GetStr() : "any", 0, 0);

        ReasonHeaderValue objValue = GetCauseAndTextFromReasonHeader(piMessage, strProtocol);

        if (objValue.nCause != -1 || objValue.strText.GetLength() > 0)
        {
            objResaultValue.nCause = objValue.nCause;
            objResaultValue.strText = objValue.strText;
            objResaultValue.strProtocol = objValue.strProtocol;

            return objResaultValue;
        }
    }
    IMS_TRACE_D("GetPrioritizedReasonHeader: No matching Reason Header found.", 0, 0, 0);
    return objResaultValue;
}

PUBLIC Ims3gpp& MessageUtils::GetIms3gppFromBody(
        IN const IMessage* piMessage, OUT Ims3gpp& objIms3gpp)
{
    const ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return objIms3gpp;
    }

    ImsList<ISipMessageBodyPart*> lstBodyParts = piSipMessage->GetBodyParts();
    for (IMS_UINT32 i = 0; i < lstBodyParts.GetSize(); i++)
    {
        const ISipMessageBodyPart* piBodyPart = lstBodyParts.GetAt(i);
        if (piBodyPart == IMS_NULL)
        {
            continue;
        }

        AString strContentType = piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);

        AString strType;
        AString strSubType;
        TextParser::ParseMediaType(strContentType, strType, strSubType);

        if (!strType.EqualsIgnoreCase(MessageUtil::STR_CONTENT_TYPE_APPLICATION) ||
                !strSubType.EqualsIgnoreCase(MessageUtil::STR_CONTENT_TYPE_3GPP_IMS_XML))
        {
            continue;
        }

        if (objIms3gpp.Parse(piBodyPart->GetContent().ToString()))
        {
            return objIms3gpp;
        }
    }

    return objIms3gpp;
}

PUBLIC Ims3gppData MessageUtils::GetIms3gppData(IN const IMessage* piMessage)
{
    Ims3gppData objIms3gppData;
    const ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return objIms3gppData;
    }

    ImsList<ISipMessageBodyPart*> lstBodyParts = piSipMessage->GetBodyParts();
    for (IMS_UINT32 i = 0; i < lstBodyParts.GetSize(); i++)
    {
        const ISipMessageBodyPart* piBodyPart = lstBodyParts.GetAt(i);
        if (piBodyPart == IMS_NULL)
        {
            continue;
        }

        AString strContentType = piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);

        AString strType;
        AString strSubType;
        TextParser::ParseMediaType(strContentType, strType, strSubType);

        if (!strType.EqualsIgnoreCase(MessageUtil::STR_CONTENT_TYPE_APPLICATION) ||
                !strSubType.EqualsIgnoreCase(MessageUtil::STR_CONTENT_TYPE_3GPP_IMS_XML))
        {
            continue;
        }

        Ims3gpp objIms3gpp;
        if (objIms3gpp.Parse(piBodyPart->GetContent().ToString()))
        {
            objIms3gppData.eType = objIms3gpp.GetType();
            objIms3gppData.eAlternativeServiceType = objIms3gpp.GetAlternativeService().GetType();
            objIms3gppData.eAlternativeServiceAction =
                    objIms3gpp.GetAlternativeService().GetAction();
        }
    }

    return objIms3gppData;
}

PUBLIC IMS_SINT32 MessageUtils::GetStatusCodeInNotify(IN IMessage* piMessage)
{
    if (!ContainsValue(piMessage, MessageUtil::STR_CONTENT_TYPE_SIP_FRAG, ISipHeader::CONTENT_TYPE))
    {
        return SipStatusCode::SC_INVALID;
    }

    const ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    IMS_SINT32 eStatusCode = SipStatusCode::SC_INVALID;
    ImsList<ISipMessageBodyPart*> lstBodyParts = piSipMessage->GetBodyParts();
    for (IMS_UINT32 i = 0; i < lstBodyParts.GetSize(); i++)
    {
        const ISipMessageBodyPart* piBodyPart = lstBodyParts.GetAt(i);
        if (piBodyPart == IMS_NULL)
        {
            continue;
        }

        ByteArray objContent = piBodyPart->GetContent();
        // To cover RJIL network issue: b/247729585
        objContent.Append(TextParser::CHAR_CR);
        objContent.Append(TextParser::CHAR_LF);

        ISipMessage* piSipFrag = SipParsingHelper::CreateMessage(objContent);
        if (piSipFrag == IMS_NULL)
        {
            continue;
        }

        eStatusCode = piSipFrag->GetStatusCode();
        piSipFrag->Destroy();
    }

    return eStatusCode;
}

PUBLIC IMS_BOOL MessageUtils::HasSdp(IN const IMessage* piMessage)
{
    const ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (piSipMessage->GetSdpBodyPart() == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC IMS_BOOL MessageUtils::IsFocusConf(IN const IMessage* piMessage)
{
    ImsList<AString> lstHeaders = GetHeaders(piMessage, ISipHeader::CONTACT_NORMAL);
    if (lstHeaders.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        ISipHeader* piSipHeader =
                SipParsingHelper::CreateHeader(ISipHeader::CONTACT_NORMAL, lstHeaders.GetAt(i));
        if (piSipHeader == IMS_NULL)
        {
            continue;
        }

        const SipParameter* pSipParameter =
                piSipHeader->GetParameter(MessageUtil::STR_PARAMETER_IS_FOCUS);
        if (pSipParameter == IMS_NULL)
        {
            piSipHeader->Destroy();
            continue;
        }

        if (pSipParameter->IsNameOnly() ||
                pSipParameter->GetValue().EqualsIgnoreCase(TextParser::STR_SMALL_TRUE))
        {
            piSipHeader->Destroy();
            return IMS_TRUE;
        }

        piSipHeader->Destroy();
    }

    return IMS_FALSE;
}

PUBLIC IMS_BOOL MessageUtils::IsInitialRegistrationRequired(IN const IMessage* piMessage)
{
    Ims3gpp objIms3gpp;
    GetIms3gppFromBody(piMessage, objIms3gpp);
    if (objIms3gpp.GetAlternativeService().GetType() ==
                    Ims3gpp::AlternativeService::TYPE_RESTORATION &&
            objIms3gpp.GetAlternativeService().GetAction() ==
                    Ims3gpp::AlternativeService::ACTION_INITIAL_REGISTRATION)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC IMS_BOOL MessageUtils::IsInitialEmergencyRegistrationRequired(IN const IMessage* piMessage)
{
    Ims3gpp objIms3gpp;
    GetIms3gppFromBody(piMessage, objIms3gpp);
    if (objIms3gpp.GetAlternativeService().GetType() ==
                    Ims3gpp::AlternativeService::TYPE_EMERGENCY &&
            objIms3gpp.GetAlternativeService().GetAction() ==
                    Ims3gpp::AlternativeService::ACTION_EMERGENCY_REGISTRATION)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC IMS_BOOL MessageUtils::ContainsValue(IN const IMessage* piMessage,
        IN const AString& strValue, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ImsList<AString> lstHeaders = GetHeaders(piMessage, eHeaderType, strHeaderName);
    if (lstHeaders.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        if (lstHeaders.GetAt(i).Contains(strValue))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC IMS_BOOL MessageUtils::IsHeaderPresent(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    const ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return piSipMessage->IsHeaderPresent(eHeaderType, strHeaderName);
}

PUBLIC IMS_BOOL MessageUtils::ContainsTag(IN const AString& strHeader, IN const AString& strTag)
{
    if (strHeader.GetLength() < 1 || strTag.GetLength() < 1)
    {
        return IMS_FALSE;
    }

    if (strHeader.Contains(strTag))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC IMS_BOOL MessageUtils::ContainsAddressInPaid(
        IN const IMessage* piMessage, IN const AString& strAddress)
{
    if (strAddress.GetLength() <= 0)
    {
        return IMS_FALSE;
    }
    SipAddress objAddress(strAddress);

    ImsList<AString> lstPaid = GetHeaders(piMessage, ISipHeader::P_ASSERTED_IDENTITY);

    AString strAddressInHeader;
    for (IMS_UINT32 index = 0; index < lstPaid.GetSize(); index++)
    {
        if (lstPaid.GetAt(index).Contains(objAddress.GetScheme()))
        {
            strAddressInHeader = lstPaid.GetAt(index);
            break;
        }
    }

    if (strAddressInHeader.GetLength() <= 0)
    {
        return IMS_FALSE;
    }

    SipAddress objAddressInHeader(strAddressInHeader);

    // Don't check the ports, just in case
    objAddress.SetPort(0);
    objAddressInHeader.SetPort(0);

    return objAddress.Equals(objAddressInHeader);
}

PUBLIC IMS_RESULT MessageUtils::SetHeader(IN IMessage* piMessage, IN const AString& strValue,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return piSipMessage->SetHeader(eHeaderType, strValue, strHeaderName);
}

PUBLIC IMS_RESULT MessageUtils::AddValueIfNotExists(IN IMessage* piMessage,
        IN const AString& strValue, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    if (ContainsValue(piMessage, strValue, eHeaderType, strHeaderName))
    {
        return IMS_SUCCESS;
    }

    ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return piSipMessage->AddHeader(eHeaderType, strValue, strHeaderName);
}

PUBLIC AString MessageUtils::GenerateContentId(IN const AString& strHost)
{
    AString strContentId;
    IMS_UINT32 nRandom = IMS_SYS_GetSRandom0();
    IMS_UINT32 nMicroSeconds = IMS_SYS_GetTimeInMicroSeconds();

    if (strHost.GetLength() < 1)
    {
        strContentId.Sprintf("%05x%05x", nMicroSeconds, nRandom);
    }
    else
    {
        strContentId.Sprintf("%05x%05x@%s", nMicroSeconds, nRandom, strHost.GetStr());
    }

    return strContentId;
}

PUBLIC IMS_RESULT MessageUtils::SetResourceList(IN_OUT IMessage* piMessage,
        IN IMtcContext& objContext, IN const ImsList<ConfUser*>& lstConfUser,
        IN IMS_BOOL bWithDialogId, IN IMS_BOOL bMultiPart)
{
    ImsList<std::tuple<AString, AString, AString>> objEntries;
    for (IMS_UINT32 i = 0; i < lstConfUser.GetSize(); i++)
    {
        const ConfUser* pConfUser = lstConfUser.GetAt(i);
        if (pConfUser == IMS_NULL)
        {
            continue;
        }

        AString strEntry = CreateEntryUri(objContext, *pConfUser, bWithDialogId);

        AString strCc;
        switch (pConfUser->eCcType)
        {
            case COPYCONTROLTYPE_TO:
                strCc = "to";
                break;
            case COPYCONTROLTYPE_CC:
                strCc = "cc";
                break;
            case COPYCONTROLTYPE_BCC:
                strCc = "bcc";
                break;
        }

        AString strAnonymize;
        if (pConfUser->bAnonymize)
        {
            strAnonymize = "true";
        }

        objEntries.Append(std::make_tuple(strEntry, strCc, strAnonymize));
    }

    return SetResourceListWithHeaders(piMessage, bMultiPart, CreateResourceListXml(objEntries));
}

PUBLIC std::optional<IMS_BOOL> MessageUtils::IsMmtelFeatureIncluded(IN const IMessage* piMessage)
{
    AString strContact = GetHeader(piMessage, ISipHeader::CONTACT_NORMAL);
    if (strContact.GetLength() <= 0)
    {
        return std::nullopt;
    }
    return ContainsTag(
            strContact, AString(Const3GPP::ICSI_MMTEL).Replace(TextParser::CHAR_COLON, "%3A"));
}

PUBLIC std::optional<IMS_BOOL> MessageUtils::IsVideoFeatureIncluded(IN const IMessage* piMessage)
{
    if (!IsMmtelFeatureIncluded(piMessage).value_or(IMS_FALSE))
    {
        return std::nullopt;
    }
    AString strContact = GetHeader(piMessage, ISipHeader::CONTACT_NORMAL);
    return ContainsTag(strContact, MessageUtil::STR_VIDEO);
}

PUBLIC std::optional<IMS_BOOL> MessageUtils::IsTextFeatureIncluded(IN const IMessage* piMessage)
{
    if (!IsMmtelFeatureIncluded(piMessage).value_or(IMS_FALSE))
    {
        return std::nullopt;
    }
    AString strContact = GetHeader(piMessage, ISipHeader::CONTACT_NORMAL);
    return ContainsTag(strContact, MessageUtil::STR_TEXT);
}

PUBLIC CallType MessageUtils::GetCallType(
        IN const IMessage* piMessage, IN ISession* piSession, IN IMS_BOOL bCheckRemote)
{
    if (HasSdp(piMessage))
    {
        return GetCallTypeFromSdp(piSession, IMS_FALSE, bCheckRemote);
    }
    return CallType::UNKNOWN;
}

PUBLIC CallType MessageUtils::GetCallTypeFromSdp(IN ISession* piSession,
        IN IMS_BOOL bActiveMediaOnly, IN IMS_BOOL bCheckRemote, IN IMS_BOOL bIgnorePort0)
{
    IMS_BOOL bAudio = IMS_FALSE;
    IMS_BOOL bVideo = IMS_FALSE;
    IMS_BOOL bText = IMS_FALSE;

    if (piSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetCallTypeFromSdp : piSession is NULL", 0, 0, 0);
        return CallType::UNKNOWN;
    }

    ImsList<IMedia*> lstIMedia = piSession->GetMedia();
    for (IMS_UINT32 nIndex = 0; nIndex < lstIMedia.GetSize(); nIndex++)
    {
        const IMedia* piMedia = lstIMedia.GetAt(nIndex);
        const IMediaDescriptor* pDescriptor = IMS_NULL;
        if (bCheckRemote)
        {
            if (piMedia->GetUpdateState() == IMedia::UPDATE_MODIFIED)
            {
                pDescriptor = piMedia->GetProposal()->GetMediaDescriptor();
            }
            else
            {
                pDescriptor = piMedia->GetMediaDescriptor();
            }
        }
        else
        {
            pDescriptor = piMedia->GetMediaDescriptor();
        }
        if (pDescriptor == IMS_NULL)
        {
            return CallType::UNKNOWN;
        }

        const SdpMedia* pSdpMedia = IMS_NULL;
        if (bCheckRemote)
        {
            pSdpMedia = pDescriptor->GetMediaDescriptionEx();
        }
        else
        {
            pSdpMedia = pDescriptor->GetMediaDescriptionExAsLocal();
        }
        if (pSdpMedia == IMS_NULL)
        {
            continue;
        }

        if (bIgnorePort0 && pSdpMedia->GetPort() == 0)
        {
            continue;
        }

        if (pSdpMedia->GetType() == SdpMedia::TYPE_AUDIO)
        {
            bAudio = IMS_TRUE;
        }
        else if (pSdpMedia->GetType() == SdpMedia::TYPE_VIDEO)
        {
            IMS_TRACE_D("GetCallTypeFromSdp : media state [%d]", piMedia->GetState(), 0, 0);
            if (!bActiveMediaOnly || piMedia->GetState() != IMedia::STATE_DELETED)
            {
                bVideo = IMS_TRUE;
            }
        }
        else if (pSdpMedia->GetType() == SdpMedia::TYPE_TEXT)
        {
            IMS_TRACE_D("GetCallTypeFromSdp : media state [%d]", piMedia->GetState(), 0, 0);
            if (!bActiveMediaOnly || piMedia->GetState() != IMedia::STATE_DELETED)
            {
                bText = IMS_TRUE;
            }
        }
    }
    IMS_TRACE_D("GetCallTypeFromSdp : audio[%s] video[%s] text[%s]", _TRACE_B_(bAudio),
            _TRACE_B_(bVideo), _TRACE_B_(bText));

    if (!bAudio)
    {
        return CallType::UNKNOWN;
    }

    if (bVideo && bText)
    {
        return CallType::VIDEO_RTT;
    }
    else if (bVideo)
    {
        return CallType::VT;
    }
    else if (bText)
    {
        return CallType::RTT;
    }
    return CallType::VOIP;
}

PUBLIC IMS_BOOL MessageUtils::IsResponseExist(IN ISession* piSession, IN IMS_SINT32 nStatusCode)
{
    IMS_BOOL bExist = IMS_FALSE;
    ImsList<IMessage*> objResponses = piSession->GetPreviousResponses(IMessage::SESSION_START);

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); i++)
    {
        if (objResponses.GetAt(i)->GetStatusCode() == nStatusCode)
        {
            bExist = IMS_TRUE;
            break;
        }
    }

    IMS_TRACE_D("IsResponseExist : [%d][%s]", nStatusCode, _TRACE_B_(bExist), 0);
    return bExist;
}

PUBLIC IMS_UINT32 MessageUtils::GetNumberOfPreviousResponses(
        IN const ISession* piSession, IN IMS_SINT32 eServiceMethod) const
{
    if (piSession == IMS_NULL)
    {
        return 0;
    }

    ImsList<IMessage*> lstResponses = piSession->GetPreviousResponses(eServiceMethod);
    return lstResponses.GetSize();
}

PRIVATE ISipMessage* MessageUtils::GetSipMessage(IN const IMessage* piMessage)
{
    if (piMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piMessage->GetMessage();
}

PRIVATE IMS_RESULT MessageUtils::GetAddresses(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, OUT ImsList<SipAddress>& lstAddresses,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    lstAddresses.Clear();

    ImsList<AString> lstHeaders = GetHeaders(piMessage, eHeaderType, strHeaderName);
    if (lstHeaders.IsEmpty())
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        SipAddress objSipAddress;
        if (objSipAddress.Create(lstHeaders.GetAt(i)))
        {
            lstAddresses.Append(objSipAddress);
        }
    }

    if (lstAddresses.IsEmpty())
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE void MessageUtils::GetParameterValueFromUnknownHeaderBody(
        IN const AString& strBody, IN const AString& strParameterName, OUT AString& strValue)
{
    strValue = AString::ConstNull();

    ImsList<AString> lstParameters = strBody.Split(TextParser::CHAR_SEMICOLON);
    for (IMS_UINT32 i = 0; i < lstParameters.GetSize(); i++)
    {
        const AString& strParameter = lstParameters.GetAt(i);
        IMS_SINT32 nEqualPosition =
                TextParser::GetIndexOfDelimiter(strParameter, TextParser::CHAR_EQUAL);

        if (nEqualPosition == AString::NPOS)
        {
            continue;
        }

        if ((strParameter.GetSubStr(0, nEqualPosition)).Equals(strParameterName))
        {
            strValue = strParameter.GetSubStr(nEqualPosition + 1);
            return;
        }
    }
}

PRIVATE IMS_RESULT MessageUtils::GetUrnValue(IN const IMessage* piMessage, IN const AString& strId,
        IN IMS_SINT32 eHeaderType, OUT AString& strValue,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strValue = AString::ConstNull();

    AString strHeader = GetHeader(piMessage, eHeaderType, strHeaderName);
    if (strHeader.IsEmpty())
    {
        return IMS_FAILURE;
    }

    SipAddress objAddress(strHeader);
    if (!objAddress.GetScheme().EqualsIgnoreCase(MessageUtil::STR_URN))
    {
        return IMS_FAILURE;
    }

    AString strHost = objAddress.GetHost();
    if (!strHost.StartsWith(strId))
    {
        return IMS_FAILURE;
    }

    strValue = strHost.Erase(0, strId.GetLength() + 1);

    if (strValue.GetLength() < 1)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE IMS_RESULT MessageUtils::SetResourceListWithHeaders(
        IN_OUT IMessage* piMessage, IN IMS_BOOL bMultiPart, IN const AString& strXml)
{
    if (piMessage == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    IMessageBodyPart* piBodyPart = piMessage->CreateBodyPart();
    if (piBodyPart == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    piBodyPart->SetHeader(
            SipHeaderName::CONTENT_TYPE, MessageUtil::STR_CONTENT_TYPE_RESOURCE_LISTS_XML);
    if (bMultiPart)
    {
        piBodyPart->SetHeader(SipHeaderName::CONTENT_DISPOSITION,
                MessageUtil::STR_CONTENT_DISPOSITION_RECIPIENT_LIST);

        AString strContentLength;
        strContentLength.SetNumber(strXml.GetLength());
        piBodyPart->SetHeader(SipHeaderName::CONTENT_LENGTH, strContentLength);
    }

    return piBodyPart->SetContent(ByteArray(strXml));
}

PRIVATE AString MessageUtils::CreateResourceListXml(
        IN const ImsList<std::tuple<AString, AString, AString>>& objEntries)
{
    AStringBuffer objXml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    objXml += "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\"";
    objXml += " xmlns:cp=\"urn:ietf:params:xml:ns:copyControl\">\n";
    objXml += "<list>\n";

    for (IMS_UINT32 i = 0; i < objEntries.GetSize(); i++)
    {
        objXml += "<entry uri=\"";
        objXml += std::get<0>(objEntries.GetAt(i));
        objXml += "\"";

        if (std::get<1>(objEntries.GetAt(i)).GetLength() > 0)
        {
            objXml += " cp:copyControl=\"";
            objXml += std::get<1>(objEntries.GetAt(i));
            objXml += "\"";
        }

        if (std::get<2>(objEntries.GetAt(i)).GetLength() > 0)
        {
            objXml += " cp:anonymize=\"true\"";
        }

        objXml += " />\n";
    }

    objXml += "</list>\n"
              "</resource-lists>";

    return objXml.GetString();
}

PRIVATE AString MessageUtils::CreateEntryUri(
        IN IMtcContext& objContext, IN const ConfUser& objUser, IN IMS_BOOL bWithDialogId)
{
    if (!bWithDialogId)
    {
        // It's not possible to have user entity before inviting. If there is a case that the host
        // drops a participant using resource-list, a user entity should be used.
        return objUser.strTarget;
    }

    // KDDI carrier requires 3GPP 24.147 5.3.1.5.4
    // <entry uri="[SIP-URI]?[Call-ID]?[From header including tag value];
    // [To header including tag value];[Session-ID]
    // <entry uri="B?Call-ID=1a&amp;From=A%3Btag%3Da1&amp;To=B%3Btag%3Db&amp;Session-ID=1"
    // cp:copyControl="to"/>

    CallKey nKey = objContext.GetCallConnectionIdManager().GetCallKey(objUser.nConnectionId);
    if (nKey == 0)
    {
        IMS_TRACE_E(0, "No call exists.", 0, 0, 0);
        return AString::ConstNull();
    }

    IMtcCallContext& objCallContext =
            objContext.GetCallManager().GetCallByCallKey(nKey)->GetCallContext();
    ISession& objSession = objCallContext.GetSession()->GetISession();

    // To get a dialog ID, a confirmed dialog message is needed.
    IMessage* piMessage = objSession.GetPreviousResponse(IMessage::SESSION_START);
    if (!piMessage)
    {
        return AString::ConstNull();
    }

    AString strRemoteUri = GetRemoteUri(&objSession, objCallContext.GetCallInfo().ePeerType);
    SipAddress objSipAddress;
    if (!objSipAddress.Create(strRemoteUri))
    {
        return AString::ConstNull();
    }

    objSipAddress.SetHeader(ISipHeader::CALL_ID, GetHeader(piMessage, ISipHeader::CALL_ID));

    // SIP URI + "?" + Call-ID
    AString strEntryUri(objSipAddress.ToString());

    AString strFromPart;
    AString strToPart;
    if (objCallContext.GetCallInfo().ePeerType == PeerType::MO)
    {
        strFromPart = CreateFromToPartWithTagValue(piMessage, ISipHeader::FROM);
        strToPart = CreateFromToPartWithTagValue(piMessage, ISipHeader::TO);
    }
    else
    {
        strFromPart = CreateFromToPartWithTagValue(piMessage, ISipHeader::TO);
        strToPart = CreateFromToPartWithTagValue(piMessage, ISipHeader::FROM);
    }

    strEntryUri.Append("&amp;From=");
    strEntryUri.Append(strFromPart);

    strEntryUri.Append("&amp;To=");
    strEntryUri.Append(strToPart);

    const IMessage* piInitialInvite = objSession.GetPreviousRequest(IMessage::SESSION_START);
    AString strSessionId = GetHeader(piInitialInvite, ISipHeader::SESSION_ID);
    if (strSessionId.GetLength() > 0)
    {
        strEntryUri.Append("&amp;Session-ID=");
        strEntryUri.Append(strSessionId);
    }

    return strEntryUri;
}

PRIVATE
AString MessageUtils::CreateFromToPartWithTagValue(
        IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType)
{
    AString strHeader = GetHeader(piMessage, eHeaderType);
    ISipHeader* piHeader = SipParsingHelper::CreateHeader(eHeaderType, strHeader);
    if (!piHeader)
    {
        IMS_TRACE_E(0, "piHeader is null.", 0, 0, 0);
        return AString::ConstNull();
    }

    const AString strEscaped("\"<>");
    AString strValue(TextParser::DoPercentEncoding(piHeader->GetValue(), strEscaped));
    const SipParameter* pTagParam = piHeader->GetParameter("tag");
    if (pTagParam)
    {
        strValue.Append(TextParser::DoPercentEncoding(";tag="));
        strValue.Append(pTagParam->GetValue());
    }

    piHeader->Destroy();
    return strValue;
}
