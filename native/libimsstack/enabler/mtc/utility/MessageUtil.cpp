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

#include "Const3GPP.h"
#include "define/MtcStringDef.h"
#include "AStringBuffer.h"
#include "media/IMedia.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "Ims3gpp.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "utility/MessageUtil.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "Sip.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "TextParser.h"
#include "conferencecall/ConferenceDef.h"

__IMS_TRACE_TAG_COM_MTC__;

const IMS_CHAR MessageUtil::STR_199[] = "199";
const IMS_CHAR MessageUtil::STR_ALERT_URN_CALL_WAITING[] = "<urn:alert:service:call-waiting>";
const IMS_CHAR MessageUtil::STR_ANONYMOUS[] = "Anonymous";
const IMS_CHAR MessageUtil::STR_UNAVAILABLE[] = "unavailable";
const IMS_CHAR MessageUtil::STR_CONTENT_DISPOSITION_RECIPIENT_LIST[] = "recipient-list";
const IMS_CHAR MessageUtil::STR_CONTENT_ID[] = "Content-ID";
const IMS_CHAR MessageUtil::STR_CONTENT_TYPE_3GPP_IMS_XML[] = "3gpp-ims+xml";
const IMS_CHAR MessageUtil::STR_CONTENT_TYPE_APPLICATION[] = "application";
const IMS_CHAR MessageUtil::STR_CONTENT_TYPE_RESOURCE_LISTS_XML[] =
        "application/resource-lists+xml";
const IMS_CHAR MessageUtil::STR_CONTENT_TYPE_SIP_FRAG[] = "message/sipfrag";
const IMS_CHAR MessageUtil::STR_ACCEPT_TYPE_APPLICATION_SDP[] = "application/sdp";
const IMS_CHAR MessageUtil::STR_ACCEPT_TYPE_APPLICATION_3GPP_IMS_XML[] = "application/3gpp-ims+xml";
const IMS_CHAR MessageUtil::STR_HEADER[] = "header";
const IMS_CHAR MessageUtil::STR_HISTINFO[] = "histinfo";
const IMS_CHAR MessageUtil::STR_ICSI[] = "+g.3gpp.icsi-ref=";
const IMS_CHAR MessageUtil::STR_ID[] = "id";
const IMS_CHAR MessageUtil::STR_NONE[] = "none";
const IMS_CHAR MessageUtil::STR_PARAMETER_IS_FOCUS[] = "isfocus";
const IMS_CHAR MessageUtil::STR_PRECONDITION[] = "precondition";
const IMS_CHAR MessageUtil::STR_REASON_FAILURE_TO_TRANSITION[] =
        "SIP;cause=487;text=\"failure to transition to CS domain\"";
const IMS_CHAR MessageUtil::STR_REASON_HANDOVER_CANCELLED[] =
        "SIP;cause=487;text=\"handover cancelled\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_1[] = "RELEASE_CAUSE;cause=1;text=\"User ends call\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_2[] =
        "RELEASE_CAUSE;cause=2;text=\"RTP/RTCP time-out\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_3[] =
        "RELEASE_CAUSE;cause=3;text=\"Media bearer loss\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_4[] =
        "RELEASE_CAUSE;cause=4;text=\"SIP timeout - no ACK\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_5[] =
        "RELEASE_CAUSE;cause=5;text=\"SIP response time-out\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_6[] =
        "RELEASE_CAUSE;cause=6;text=\"Call-setup time-out\"";
const IMS_CHAR MessageUtil::STR_REPLACES[] = "replaces";
const IMS_CHAR MessageUtil::STR_SERVICE[] = "service";
const IMS_CHAR MessageUtil::STR_SOS[] = "sos";
const IMS_CHAR MessageUtil::STR_SOS_AMBULANCE[] = "sos.ambulance";
const IMS_CHAR MessageUtil::STR_SOS_ANIMAL_CONTROL[] = "sos.animal-control";
const IMS_CHAR MessageUtil::STR_SOS_COUNTRY_SPECIFIC[] = "sos.country-specific";
const IMS_CHAR MessageUtil::STR_SOS_FIRE[] = "sos.fire";
const IMS_CHAR MessageUtil::STR_SOS_GAS[] = "sos.gas";
const IMS_CHAR MessageUtil::STR_SOS_MARINE[] = "sos.marine";
const IMS_CHAR MessageUtil::STR_SOS_MOUNTAIN[] = "sos.mountain";
const IMS_CHAR MessageUtil::STR_SOS_PHYSICIAN[] = "sos.physician";
const IMS_CHAR MessageUtil::STR_SOS_POISON[] = "sos.poison";
const IMS_CHAR MessageUtil::STR_SOS_POLICE[] = "sos.police";
const IMS_CHAR MessageUtil::STR_SRVCC_FEATURE_A[] = "+g.3gpp.srvcc-alerting";
const IMS_CHAR MessageUtil::STR_SRVCC_FEATURE_B[] = "+g.3gpp.ps2cs-srvcc-orig-pre-alerting";
const IMS_CHAR MessageUtil::STR_SRVCC_FEATURE_M[] = "+g.3gpp.mid-call";
const IMS_CHAR MessageUtil::STR_SUPPORTED[] = "supported";
const IMS_CHAR MessageUtil::STR_TIMER[] = "timer";
const IMS_CHAR MessageUtil::STR_URN[] = "urn";
const IMS_CHAR MessageUtil::STR_VIDEO[] = "video";
const IMS_CHAR MessageUtil::STR_TEXT[] = "text";
const IMS_CHAR MessageUtil::STR_P_TTA_VOLTE_INFO[] = "P-TTA-VoLTE-Info";
const IMS_CHAR MessageUtil::STR_AVCHANGE[] = "avchange";

const IMS_SINT32 MessageUtil::INVALID_INDEX = -1;

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMessage* MessageUtil::GetPreviousResponse(IN const ISession* piSession,
        IN IMS_SINT32 eServiceMethod, IN IMS_SINT32 nResponseIndex /*= INVALID_INDEX*/)
{
    if (piSession == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMSList<IMessage*> lstResponses = piSession->GetPreviousResponses(eServiceMethod);
    IMS_UINT32 nResponseSize = lstResponses.GetSize();

    if (nResponseIndex < 0)
    {
        nResponseIndex = nResponseSize - 1;
    }

    if ((IMS_UINT32)(nResponseIndex) >= nResponseSize)
    {
        return IMS_NULL;
    }

    return lstResponses.GetAt(nResponseIndex);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMessage* MessageUtil::GetRemotePreviousMessage(IN ISession* piSession,
        IN IMS_SINT32 eServiceMethod, IN IMS_BOOL bIsUac,
        IN IMS_SINT32 nResponseIndex /*= INVALID_INDEX*/)
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::GetResponseStatusCode(IN ISession* piSession,
        IN IMS_SINT32 eServiceMethod, IN IMS_SINT32 nResponseIndex /*= INVALID_INDEX*/)
{
    IMessage* piPreviousMessage = GetPreviousResponse(piSession, eServiceMethod, nResponseIndex);
    if (piPreviousMessage == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    return piPreviousMessage->GetStatusCode();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetRemoteUris(
        IN ISession* piSession, IN PeerType ePeerType, OUT IMSList<AString>& lstUris)
{
    lstUris.Clear();

    if (piSession == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    IMSList<AString> lstAddresses = piSession->GetRemoteUserId();
    if (lstAddresses.IsEmpty())
    {
        IMessage* piPreviousMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);
        if (piPreviousMessage == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        IMS_SINT32 eHeaderType = ISipHeader::FROM;
        if (ePeerType == PeerType::MO)
        {
            eHeaderType = ISipHeader::TO;
        }

        if (GetHeaders(piPreviousMessage, eHeaderType, lstAddresses) == IMS_FAILURE)
        {
            return IMS_FAILURE;
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

    if (lstUris.IsEmpty())
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetRemoteUri(
        IN ISession* piSession, IN PeerType ePeerType, OUT AString& strUri)
{
    strUri = AString::ConstNull();

    IMSList<AString> lstUris;
    if (GetRemoteUris(piSession, ePeerType, lstUris) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstUris.GetSize(); i++)
    {
        strUri = lstUris.GetAt(i);
        if (strUri.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetSessionId(
        IN ISession* piSession, OUT AString& strSessionId)
{
    strSessionId = AString::ConstNull();

    if (piSession == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    strSessionId = piSession->GetSessionId();
    if (strSessionId.GetLength() < 1)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetHeaders(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, OUT IMSList<AString>& lstHeaders,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    lstHeaders.Clear();

    ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    lstHeaders = piSipMessage->GetHeaders(eHeaderType, strHeaderName);
    if (lstHeaders.IsEmpty())
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetHeader(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, OUT AString& strHeader,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strHeader = AString::ConstNull();

    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, eHeaderType, lstHeaders, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        strHeader = lstHeaders.GetAt(i);
        if (strHeader.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetHeaderValue(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, OUT AString& strValue,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strValue = AString::ConstNull();

    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, eHeaderType, lstHeaders, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        ISipHeader* piSipHeader = SipParsingHelper::CreateHeader(eHeaderType, lstHeaders.GetAt(i));
        if (piSipHeader == IMS_NULL)
        {
            continue;
        }

        strValue = piSipHeader->GetValue();
        piSipHeader->Destroy();

        if (strValue.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::GetHeaderValueInt(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, eHeaderType, lstHeaders, strHeaderName) == IMS_FAILURE)
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetParameterValue(IN const IMessage* piMessage,
        IN const AString& strParameterName, IN IMS_SINT32 eHeaderType, OUT AString& strValue,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strValue = AString::ConstNull();

    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, eHeaderType, lstHeaders, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        ISipHeader* piSipHeader = SipParsingHelper::CreateHeader(eHeaderType, lstHeaders.GetAt(i));
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
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMSList<AString> MessageUtil::GetUserParts(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    IMSList<AString> lstUserParts;

    IMSList<SipAddress> lstAddresses;
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetUserPart(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, OUT AString& strUserPart,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strUserPart = AString::ConstNull();

    IMSList<AString> lstUserParts = GetUserParts(piMessage, eHeaderType, strHeaderName);

    for (IMS_UINT32 i = 0; i < lstUserParts.GetSize(); i++)
    {
        strUserPart = lstUserParts.GetAt(i);
        if (strUserPart.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetUserIds(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        OUT IMSList<AString>& lstUserIds,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    lstUserIds.Clear();

    IMSList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

        AString strUserPart;
        if (objSipAddress.IsSchemeSip())
        {
            // TODO, Contains / GetIndexOf
            if (objSipAddress.GetUser().Contains(TextParser::CHAR_SEMICOLON))
            {
                strUserPart = objSipAddress.GetUser().GetSubStr(
                        0, objSipAddress.GetUser().GetIndexOf(TextParser::CHAR_SEMICOLON));
            }
            else
            {
                strUserPart = objSipAddress.GetUser();
            }
        }
        else if (objSipAddress.IsSchemeTel())
        {
            strUserPart = objSipAddress.GetHost();
        }

        lstUserIds.Append(strUserPart);
    }

    if (lstUserIds.IsEmpty())
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetUserId(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        OUT AString& strUserId, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strUserId = AString::ConstNull();

    IMSList<AString> lstUserIds;
    if (GetUserIds(piMessage, eHeaderType, lstUserIds, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstUserIds.GetSize(); i++)
    {
        strUserId = lstUserIds.GetAt(i);
        if (strUserId.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetDisplayNames(IN IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, OUT IMSList<AString>& lstDisplayNames,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    lstDisplayNames.Clear();

    IMSList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

        lstDisplayNames.Append(objSipAddress.GetDisplayName());
    }

    if (lstDisplayNames.IsEmpty())
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetDisplayName(IN IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, OUT AString& strDisplayName,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strDisplayName = AString::ConstNull();

    IMSList<AString> lstDisplayNames;
    if (GetDisplayNames(piMessage, eHeaderType, lstDisplayNames, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstDisplayNames.GetSize(); i++)
    {
        strDisplayName = lstDisplayNames.GetAt(i);
        if (strDisplayName.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetHosts(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        OUT IMSList<AString>& lstHosts, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    lstHosts.Clear();

    IMSList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

        lstHosts.Append(objSipAddress.GetHost());
    }

    if (lstHosts.IsEmpty())
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetHost(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        OUT AString& strHost, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strHost = AString::ConstNull();

    IMSList<AString> lstHosts;
    if (GetHosts(piMessage, eHeaderType, lstHosts, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstHosts.GetSize(); i++)
    {
        strHost = lstHosts.GetAt(i);
        if (strHost.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetParameterValueFromUri(IN IMessage* piMessage,
        IN const AString& strParameterName, IN IMS_SINT32 eHeaderType, OUT AString& strValue,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strValue = AString::ConstNull();

    IMSList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstAddresses.GetSize(); i++)
    {
        const SipAddress& objSipAddress = lstAddresses.GetAt(i);

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
                return IMS_SUCCESS;
            }
        }

        const SipParameter* pParameter = objSipAddress.GetParameter(strParameterName);
        if (pParameter != IMS_NULL)
        {
            strValue = pParameter->GetValue();
        }

        if (strValue.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetUris(IN IMessage* piMessage, IN IMS_BOOL bWithParameters,
        IN IMS_SINT32 eHeaderType, OUT IMSList<AString>& lstUris,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    lstUris.Clear();

    IMSList<SipAddress> lstAddresses;
    if (GetAddresses(piMessage, eHeaderType, lstAddresses, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

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

    if (lstUris.IsEmpty())
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetUri(IN IMessage* piMessage, IN IMS_BOOL bWithParameters,
        IN IMS_SINT32 eHeaderType, OUT AString& strUri,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strUri = AString::ConstNull();

    IMSList<AString> lstUris;
    if (GetUris(piMessage, bWithParameters, eHeaderType, lstUris, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstUris.GetSize(); i++)
    {
        strUri = lstUris.GetAt(i);
        if (strUri.GetLength() > 0)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::GetFeatures(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, eHeaderType, lstHeaders, strHeaderName) == IMS_FAILURE)
    {
        return FEATURE_NONE;
    }

    IMS_SINT32 eFeatures = FEATURE_NONE;

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        if (lstHeaders.GetAt(i).Equals(STR_TIMER))
        {
            eFeatures |= FEATURE_TIMER;
            continue;
        }

        if (lstHeaders.GetAt(i).Equals(Sip::STR_100REL))
        {
            eFeatures |= FEATURE_100REL;
            continue;
        }

        if (lstHeaders.GetAt(i).Equals(STR_PRECONDITION))
        {
            eFeatures |= FEATURE_PRECONDITION;
            continue;
        }
    }

    return eFeatures;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::GetSosTypeFromServiceUrn(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    AString strValue;
    if (GetUrnValue(piMessage, STR_SERVICE, eHeaderType, strValue, strHeaderName) == IMS_FAILURE)
    {
        return EXTRA_CODE_EMERGENCYSERVICE_INVALID;
    }

    if (!strValue.StartsWith(STR_SOS))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_INVALID;
    }

    if (strValue.EqualsIgnoreCase(STR_SOS))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_GENERIC;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_AMBULANCE))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_ANIMAL_CONTROL))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_ANIMAL_CONTROL;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_FIRE))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_FIRE;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_GAS))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_GAS;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_MARINE))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_MARINE;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_MOUNTAIN))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_PHYSICIAN))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_PHYSICIAN;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_POISON))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_POISON;
    }
    else if (strValue.EqualsIgnoreCase(STR_SOS_POLICE))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_POLICE;
    }
    else if (strValue.MakeLower().Contains(STR_SOS_COUNTRY_SPECIFIC))
    {
        return EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC;
    }

    return EXTRA_CODE_EMERGENCYSERVICE_INVALID;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::GetCauseFromReasonHeader(
        IN const IMessage* piMessage, IN const AString& strProtocol /*= AString::ConstNull()*/)
{
    AString strText;
    IMS_SINT32 nCause;

    GetCauseAndTextFromReasonHeader(piMessage, nCause, strText, strProtocol);

    return nCause;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetCauseAndTextFromReasonHeader(IN const IMessage* piMessage,
        OUT IMS_SINT32& nCause, OUT AString& strText,
        IN const AString& strProtocol /*= AString::ConstNull()*/)
{
    nCause = -1;
    strText = AString::ConstNull();

    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, ISipHeader::UNKNOWN, lstHeaders, SipHeaderName::REASON) ==
            IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        const AString& strHeader = lstHeaders.GetAt(i);

        AString strReceivedProtocol;
        if (!SipParsingHelper::ParseReasonHeader(strHeader, strReceivedProtocol, nCause, strText))
        {
            continue;
        }

        if ((strProtocol.GetLength() > 0) && (!strProtocol.EqualsIgnoreCase(strReceivedProtocol)))
        {
            nCause = -1;
            strText = AString::ConstNull();
            continue;
        }

        return IMS_SUCCESS;
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::GetSupportedFeatures(IN IMessage* piMessage)
{
    return GetFeatures(piMessage, ISipHeader::SUPPORTED);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::GetRequireFeatures(IN IMessage* piMessage)
{
    return GetFeatures(piMessage, ISipHeader::REQUIRE);
}

/* -------------------------------------------------------------------------------------------------
    only one Ims3gpp type body is applicable
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GetIms3gppFromBody(
        IN const IMessage* piMessage, OUT Ims3gpp& objIms3gpp)
{
    ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    IMSList<ISipMessageBodyPart*> lstBodyParts = piSipMessage->GetBodyParts();
    for (IMS_UINT32 i = 0; i < lstBodyParts.GetSize(); i++)
    {
        ISipMessageBodyPart* piBodyPart = lstBodyParts.GetAt(i);
        if (piBodyPart == IMS_NULL)
        {
            continue;
        }

        AString strContentType = piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);

        AString strType;
        AString strSubType;
        TextParser::ParseMediaType(strContentType, strType, strSubType);

        if (!strType.EqualsIgnoreCase(STR_CONTENT_TYPE_APPLICATION) ||
                !strSubType.EqualsIgnoreCase(STR_CONTENT_TYPE_3GPP_IMS_XML))
        {
            continue;
        }

        if (objIms3gpp.Parse(piBodyPart->GetContent().ToString()))
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::GetStatusCodeInNotify(IN IMessage* piMessage)
{
    if (!ContainsValue(piMessage, STR_CONTENT_TYPE_SIP_FRAG, ISipHeader::CONTENT_TYPE))
    {
        return SipStatusCode::SC_INVALID;
    }

    ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    IMS_SINT32 eStatusCode = SipStatusCode::SC_INVALID;
    IMSList<ISipMessageBodyPart*> lstBodyParts = piSipMessage->GetBodyParts();
    for (IMS_UINT32 i = 0; i < lstBodyParts.GetSize(); i++)
    {
        ISipMessageBodyPart* piBodyPart = lstBodyParts.GetAt(i);
        if (piBodyPart == IMS_NULL)
        {
            continue;
        }

        ByteArray objContent;
        objContent = piBodyPart->GetContent();
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::HasSdp(IN const IMessage* piMessage)
{
    ISipMessage* piSipMessage = GetSipMessage(piMessage);
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::IsFocusConf(IN const IMessage* piMessage)
{
    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, ISipHeader::CONTACT_NORMAL, lstHeaders) == IMS_FAILURE)
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

        const SipParameter* pSipParameter = piSipHeader->GetParameter(STR_PARAMETER_IS_FOCUS);
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::IsInitialRegistrationRequired(IN const IMessage* piMessage)
{
    Ims3gpp objIms3gpp;
    if (GetIms3gppFromBody(piMessage, objIms3gpp) == IMS_FAILURE)
    {
        return IMS_FALSE;
    }

    if (objIms3gpp.GetAlternativeService().GetType() ==
                    Ims3gpp::AlternativeService::TYPE_RESTORATION &&
            objIms3gpp.GetAlternativeService().GetAction() ==
                    Ims3gpp::AlternativeService::ACTION_INITIAL_REGISTRATION)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::ContainsValue(IN IMessage* piMessage,
        IN const AString& strValue, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, eHeaderType, lstHeaders, strHeaderName) != IMS_SUCCESS)
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::HasValue(IN const IMessage* piMessage,
        IN const AString& strValue, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, eHeaderType, lstHeaders, strHeaderName) != IMS_SUCCESS)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        if (lstHeaders.GetAt(i).Equals(strValue))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::IsHeaderPresent(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return piSipMessage->IsHeaderPresent(eHeaderType, strHeaderName);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::ContainsTag(
        IN const AString& strHeader, IN const AString& strTag)
{
    if (strHeader.GetLength() < 1 || strTag.GetLength() < 1)
    {
        return IMS_FALSE;
    }

    if (strHeader.Contains(strTag))
    {
        return IMS_TRUE;
    }

    // TODO, MTC BUILD
    // AString strDecodedHeader = UCEscape::Decode((IMS_CHAR *)strHeader.GetStr());
    AString strDecodedHeader;
    if (strDecodedHeader.Contains(strTag))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::ContainsAddressInPaid(
        IN const IMessage* piMessage, IN const AString& strAddress)
{
    if (strAddress.GetLength() <= 0)
    {
        return IMS_FALSE;
    }
    SipAddress objAddress(strAddress);

    IMSList<AString> lstPaid;
    GetHeaders(piMessage, ISipHeader::P_ASSERTED_IDENTITY, lstPaid);

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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::SetHeader(IN IMessage* piMessage, IN const AString& strValue,
        IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    ISipMessage* piSipMessage = GetSipMessage(piMessage);
    if (piSipMessage == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return piSipMessage->SetHeader(eHeaderType, strValue, strHeaderName);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::AddValueIfNotExists(IN IMessage* piMessage,
        IN const AString& strValue, IN IMS_SINT32 eHeaderType,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    if (HasValue(piMessage, strValue, eHeaderType, strHeaderName) == IMS_TRUE)
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::GenerateContentId(IN const AString& strHost,
        OUT AString& strContentId, IN IMS_BOOL bAngleQuote /*= IMS_FALSE*/)
{
    strContentId = AString::ConstNull();

    IMS_UINT32 nRandom = IMS_SYS_GetSRandom0();
    IMS_UINT32 nMicroSeconds = IMS_SYS_GetTimeInMicroSeconds();

    if (bAngleQuote)
    {
        strContentId.Append(TextParser::CHAR_LAQUOT);
    }

    if (strHost.GetLength() < 1)
    {
        strContentId.Sprintf("%05x%05x", nMicroSeconds, nRandom);
    }
    else
    {
        strContentId.Sprintf("%05x%05x@%s", nMicroSeconds, nRandom, strHost.GetStr());
    }

    if (bAngleQuote)
    {
        strContentId.Append(TextParser::CHAR_RAQUOT);
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::SetResourceListByConfUser(IN_OUT IMessage* piMessage,
        IN const AString& strContentId, IN IMSList<ConfUser*>& lstConfUser, IN IMS_BOOL bMultiPart,
        IN IMS_BOOL bCopyControl /*= IMS_TRUE*/)
{
    AStringBuffer objXml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    objXml += "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\"";
    objXml += " xmlns:cp=\"urn:ietf:params:xml:ns:copyControl\">\n";
    objXml += "<list>\n";

    for (IMS_UINT32 i = 0; i < lstConfUser.GetSize(); i++)
    {
        ConfUser* pConfUser = lstConfUser.GetAt(i);
        if (pConfUser == IMS_NULL)
        {
            continue;
        }

        objXml += "<entry uri=\"";
        if (pConfUser->strUserEntity.GetLength() < 1)
        {
            objXml += pConfUser->strTarget;
        }
        else
        {
            objXml += pConfUser->strUserEntity;
        }
        objXml += "\"";

        if (bCopyControl)
        {
            objXml += " cp:copyControl=";
            switch (pConfUser->eCcType)
            {
                case COPYCONTROLTYPE_TO:
                    objXml += "\"to\"";
                    break;
                case COPYCONTROLTYPE_CC:
                    objXml += "\"cc\"";
                    break;
                case COPYCONTROLTYPE_BCC:
                    objXml += "\"bcc\"";
                    break;
                default:
                    break;
            }

            if (pConfUser->bAnonymize)
            {
                objXml += " cp:anonymize=\"true\"";
            }
        }
        objXml += " />\n";
    }

    objXml += "</list>\n"
              "</resource-lists>";

    return SetResourceList(piMessage, strContentId, bMultiPart, objXml);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_RESULT MessageUtil::SetResourceListByEntryUri(IN_OUT IMessage* piMessage,
        IN const AString& strContentId, IN IMSList<AString>& lstEntryUri, IN IMS_BOOL bMultiPart,
        IN IMS_BOOL bCopyControl /*= IMS_TRUE*/)
{
    AStringBuffer objXml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    objXml += "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\"";
    objXml += " xmlns:cp=\"urn:ietf:params:xml:ns:copyControl\">\n";
    objXml += "<list>\n";

    for (IMS_UINT32 i = 0; i < lstEntryUri.GetSize(); i++)
    {
        objXml += "<entry uri=\"";
        objXml += lstEntryUri.GetAt(i);
        objXml += "\"";

        if (bCopyControl)
        {
            objXml += " cp:copyControl=\"to\"";
        }
        objXml += " />\n";
    }

    objXml += "</list>\n"
              "</resource-lists>";

    return SetResourceList(piMessage, strContentId, bMultiPart, objXml);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::IsVideoFeatureIncluded(IN const IMessage* piMessage)
{
    AString strContact;
    GetHeader(piMessage, ISipHeader::CONTACT_NORMAL, strContact);
    return ContainsTag(strContact, STR_VIDEO) &&
            ContainsTag(strContact, AString(Const3GPP::ICSI_MMTEL).Replace(":", "%3A"));
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::IsTextFeatureIncluded(IN const IMessage* piMessage)
{
    AString strContact;
    GetHeader(piMessage, ISipHeader::CONTACT_NORMAL, strContact);
    return ContainsTag(strContact, STR_TEXT) &&
            ContainsTag(strContact, AString(Const3GPP::ICSI_MMTEL).Replace(":", "%3A"));
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE GLOBAL ISipMessage* MessageUtil::GetSipMessage(IN const IMessage* piMessage)
{
    if (piMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piMessage->GetMessage();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE GLOBAL IMS_RESULT MessageUtil::GetAddresses(IN const IMessage* piMessage,
        IN IMS_SINT32 eHeaderType, OUT IMSList<SipAddress>& lstAddresses,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    lstAddresses.Clear();

    IMSList<AString> lstHeaders;
    if (GetHeaders(piMessage, eHeaderType, lstHeaders, strHeaderName) == IMS_FAILURE)
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

    return IMS_TRUE;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE GLOBAL void MessageUtil::GetParameterValueFromUnknownHeaderBody(
        IN const AString& strBody, IN const AString& strParameterName, OUT AString& strValue)
{
    strValue = AString::ConstNull();

    // TODO, need to verify
    IMSList<AString> lstParameters = strBody.Split(TextParser::CHAR_SEMICOLON);
    for (IMS_UINT32 i = 0; i < lstParameters.GetSize(); i++)
    {
        AString& strParameter = lstParameters.GetAt(i);
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE GLOBAL IMS_RESULT MessageUtil::GetUrnValue(IN const IMessage* piMessage,
        IN const AString& strId, IN IMS_SINT32 eHeaderType, OUT AString& strValue,
        IN const AString& strHeaderName /*= AString::ConstNull()*/)
{
    strValue = AString::ConstNull();

    AString strHeader;
    if (GetHeader(piMessage, eHeaderType, strHeader, strHeaderName) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SipAddress objAddress(strHeader);
    if (!objAddress.GetScheme().EqualsIgnoreCase(STR_URN))
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE GLOBAL IMS_RESULT MessageUtil::SetResourceList(IN_OUT IMessage* piMessage,
        IN const AString& strContentId, IN IMS_BOOL bMultiPart, IN AStringBuffer& objXml)
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

    piBodyPart->SetHeader(SipHeaderName::CONTENT_TYPE, STR_CONTENT_TYPE_RESOURCE_LISTS_XML);
    if (bMultiPart)
    {
        piBodyPart->SetHeader(
                SipHeaderName::CONTENT_DISPOSITION, STR_CONTENT_DISPOSITION_RECIPIENT_LIST);

        AString strContentLength;
        strContentLength.SetNumber(objXml.GetLength());
        piBodyPart->SetHeader(SipHeaderName::CONTENT_LENGTH, strContentLength);

        if (strContentId.GetLength() < 1)
        {
            AString strHost;
            GetHost(piMessage, ISipHeader::CONTACT_NORMAL, strHost);
            AString strGeneratedContentId;
            GenerateContentId(strHost, strGeneratedContentId);

            piBodyPart->SetHeader(STR_CONTENT_ID, strGeneratedContentId);
        }
        else
        {
            piBodyPart->SetHeader(STR_CONTENT_ID, strContentId);
        }
    }

    ByteArray objContent(objXml.GetString());
    return piBodyPart->SetContent(objContent);
}

/* -------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */

// legacy API

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL ServiceType MessageUtil::CheckServiceType(IN IMessage* piMessage,
        IN IMtcCall* /*pSession = IMS_NULL*/, IN ISession* /*piSession = IMS_NULL*/)
{
    ServiceType eServiceType = ServiceType::UNKNOWN;
    AString strContact;  // = GetHeader(piMessage, ISipHeader::CONTACT_NORMAL);
    AString strVoIP;     // = UCCONFIG_GET_STR(m_nSlotID, SESSION_GET_ACCEPTCONTACT_H,
                         // SESSIONTYPE_VOIP);
    AString strVT;  // = UCCONFIG_GET_STR(m_nSlotID, SESSION_GET_ACCEPTCONTACT_H, SESSIONTYPE_VT);

    // TODO, remove
    if (piMessage == IMS_NULL)
    {
        return eServiceType;
    }

    if (strContact.GetLength() == 0)
    {
        IMS_TRACE_E(0, "feature is not present in the Contact header", 0, 0, 0);
        return eServiceType;
    }

    IMS_TRACE_I("CheckServiceType : Contact[%s] VoIP[%s] VT[%s]", strContact.GetStr(),
            strVoIP.GetStr(), strVT.GetStr());

    if (ContainsTag(strContact, strVoIP) && ContainsTag(strContact, strVT))
    {
        eServiceType = ServiceType::NORMAL;
    }
    else if (ContainsTag(strContact, strVoIP))
    {
        eServiceType = ServiceType::NORMAL;
    }
    else if (ContainsTag(strContact, strVT))
    {
        eServiceType = ServiceType::NORMAL;
    }

    return eServiceType;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL CallType MessageUtil::GetCallType(
        IN const IMessage* piMessage, IN ISession* piSession, IN IMS_BOOL bPeerView)
{
    if (HasSdp(piMessage))
    {
        return GetCallTypeFromSdp(piSession, IMS_FALSE, bPeerView);
    }
    return CallType::UNKNOWN;
    // return CheckSessionTypeByAcceptContact(piMessage, piSession); // TODO: operator specific
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL CallType MessageUtil::GetCallTypeFromSdp(IN ISession* piSession, IN IMS_BOOL bNegoSdp,
        IN IMS_BOOL bPeerView, IN IMS_BOOL bCheckPort /*= IMS_TRUE*/)
{
    IMS_BOOL bAudio = IMS_FALSE;
    IMS_BOOL bVideo = IMS_FALSE;
    IMS_BOOL bText = IMS_FALSE;

    if (piSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetCallTypeFromSdp : piSession is NULL", 0, 0, 0);
        return CallType::UNKNOWN;
    }

    IMSList<IMedia*> lstIMedia = piSession->GetMedia();
    for (IMS_UINT32 nIndex = 0; nIndex < lstIMedia.GetSize(); nIndex++)
    {
        IMedia* piMedia = lstIMedia.GetAt(nIndex);
        IMediaDescriptor* pDescriptor = IMS_NULL;
        if (bPeerView)
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

        SdpMedia* pSdpMedia = IMS_NULL;
        if (bPeerView)
        {
            pSdpMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();
        }
        else
        {
            pSdpMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionExAsLocal();
        }
        if (pSdpMedia == IMS_NULL)
        {
            continue;
        }

        if (bCheckPort && pSdpMedia->GetPort() == 0)
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
            if (!bNegoSdp || piMedia->GetState() != IMedia::STATE_DELETED)
            {
                bVideo = IMS_TRUE;
            }
        }
        else if (pSdpMedia->GetType() == SdpMedia::TYPE_TEXT)
        {
            IMS_TRACE_D("GetCallTypeFromSdp : media state [%d]", piMedia->GetState(), 0, 0);
            if (!bNegoSdp || piMedia->GetState() != IMedia::STATE_DELETED)
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

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL CallType MessageUtil::GetCallTypeFromAcceptContact(
        IN IMessage* piMessage, IN ISession* /*piSession = IMS_NULL*/)
{
    AString strAcceptContact;  // = GetHeader(piMessage, ISipHeader::ACCEPT_CONTACT);
    AString strVoIP;           // = UCCONFIG_GET_STR(m_nSlotID, SESSION_GET_ACCEPTCONTACT_H,
                               // SESSIONTYPE_VOIP);
    AString strVT;  // = UCCONFIG_GET_STR(m_nSlotID, SESSION_GET_ACCEPTCONTACT_H, SESSIONTYPE_VT);

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetCallTypeFromAcceptContact : piMessage is NULL", 0, 0, 0);
        return CallType::UNKNOWN;
    }

    if (strAcceptContact.GetLength() == 0)
    {
        IMS_TRACE_E(0, "feature is not present in the Accept-Contact header", 0, 0, 0);
        return CallType::UNKNOWN;
    }

    IMS_TRACE_I("CheckSessionTypeByAcceptContact : AcceptContact[%s] VoIP[%s] VT[%s]",
            strAcceptContact.GetStr(), strVoIP.GetStr(), strVT.GetStr());

    if (ContainsTag(strAcceptContact, strVoIP) && ContainsTag(strAcceptContact, strVT))
    {
        return CallType::VT;
    }
    else if (ContainsTag(strAcceptContact, strVoIP))
    {
        return CallType::VOIP;
    }
    return CallType::UNKNOWN;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_SINT32 MessageUtil::CheckRttUpdateRequest(
        IN IMessage* piMessage, IN ISession* piSession /*= IMS_NULL*/)
{
    IMS_SINT32 eRttUpdateRequest = GTT_MODE_INVALID;

    if (!HasSdp(piMessage))
    {
        return eRttUpdateRequest;
    }

    if (piSession == IMS_NULL)
    {
        return eRttUpdateRequest;
    }

    IMSList<IMedia*> lstIMedia = piSession->GetMedia();
    for (IMS_UINT32 index = 0; index < lstIMedia.GetSize(); index++)
    {
        IMedia* pIMedia = lstIMedia.GetAt(index);
        IMediaDescriptor* pDescriptor = IMS_NULL;
        if (pIMedia->GetUpdateState() == IMedia::UPDATE_MODIFIED)
        {
            pDescriptor = pIMedia->GetProposal()->GetMediaDescriptor();
        }
        else
        {
            pDescriptor = pIMedia->GetMediaDescriptor();
        }

        if (pDescriptor == IMS_NULL)
        {
            return eRttUpdateRequest;
        }

        SdpMedia* pSDPMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();
        if (pSDPMedia != IMS_NULL)
        {
            if (pSDPMedia->GetType() == SdpMedia::TYPE_TEXT)
            {
                if (pSDPMedia->GetPort() == 0)
                {
                    eRttUpdateRequest = GTT_MODE_INACTIVE;
                }
                else
                {
                    eRttUpdateRequest = GTT_MODE_FULL;
                }
            }
        }
    }

    IMS_TRACE_D("CheckRttUpdateRequest :: [%s]", PS_GTTMode(eRttUpdateRequest), 0, 0);
    return eRttUpdateRequest;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::IsSessionRefresh(IN ISession* piSession)
{
    IMS_BOOL bIs = IMS_FALSE;
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_UPDATE);

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "IsSessionRefresh : piMessage is NULL ", 0, 0, 0);
        goto Exit_IsSessRefresh;
    }

    /* Re-INVITE Case */
    if (piMessage->GetMethod().Equals(SipMethod::INVITE))
    {
        // 4 TODO : Check Session Refresh with Re-INVITE
        if (0)
        {
            IMS_TRACE_D("IsSessRefresh :: Re-INVITE CASE - NO SDP", 0, 0, 0);
            bIs = IMS_TRUE;
        }
    }

    if (piMessage->GetMethod().Equals(SipMethod::UPDATE))
    {
        if (!HasSdp(piSession->GetPreviousRequest(IMessage::SESSION_UPDATE)))
        {
            bIs = IMS_TRUE;
        }
    }

Exit_IsSessRefresh:

    IMS_TRACE_I("IsSessRefresh [%s]", PS_BOOL(bIs), 0, 0);
    return bIs;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC GLOBAL IMS_BOOL MessageUtil::IsTextSession(IN ISession* piSession)
{
    IMSList<IMedia*> lstIMedia = piSession->GetMedia();

    for (IMS_UINT32 index = 0; index < lstIMedia.GetSize(); index++)
    {
        IMedia* pIMedia = lstIMedia.GetAt(index);
        IMediaDescriptor* pDescriptor = IMS_NULL;

        if (pIMedia->GetUpdateState() == IMedia::UPDATE_MODIFIED)
        {
            pDescriptor = pIMedia->GetProposal()->GetMediaDescriptor();
        }
        else
        {
            pDescriptor = pIMedia->GetMediaDescriptor();
        }

        if (pDescriptor != IMS_NULL)
        {
            SdpMedia* pSDPMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();
            if (pSDPMedia != IMS_NULL)
            {
                if (pSDPMedia->GetType() == SdpMedia::TYPE_TEXT && pSDPMedia->GetPort() != 0)
                {
                    IMS_TRACE_D("IsTextSession :: TEXT SESSION", 0, 0, 0);
                    return IMS_TRUE;
                }
            }
        }
    }

    IMS_TRACE_D("IsTextSession :: NOT TEXT SESSION", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL MessageUtil::IsResponseExist(
        IN ISession* piSession, IN IMS_SINT32 nStatusCode)
{
    IMS_BOOL bExist = IMS_FALSE;
    IMSList<IMessage*> objResponses = piSession->GetPreviousResponses(IMessage::SESSION_START);

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
