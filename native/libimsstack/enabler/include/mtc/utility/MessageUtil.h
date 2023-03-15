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

#ifndef MESSAGE_UTIL_H_
#define MESSAGE_UTIL_H_

#include "AString.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "Ims3gpp.h"
#include "MtcContextRepository.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "utility/IMessageUtils.h"

class AStringBuffer;
class IMessage;
class IMessageBodyPart;
class ISession;
class ISipHeader;
class ISipMessage;
class IMtcCall;
class SipAddress;
class SipParameter;
struct ConfUser;

class MessageUtil
{
public:
    inline static IMessage* GetPreviousResponse(IN const ISession* piSession,
            IN IMS_SINT32 eServiceMethod, IN IMS_SINT32 nResponseIndex = INVALID_INDEX)
    {
        return GetUtils().GetPreviousResponse(piSession, eServiceMethod, nResponseIndex);
    }
    inline static IMessage* GetRemotePreviousMessage(IN ISession* piSession,
            IN IMS_SINT32 eServiceMethod, IN IMS_BOOL bIsUac,
            IN IMS_SINT32 nResponseIndex = INVALID_INDEX)
    {
        return GetUtils().GetRemotePreviousMessage(
                piSession, eServiceMethod, bIsUac, nResponseIndex);
    }
    inline static IMS_SINT32 GetResponseStatusCode(IN ISession* piSession,
            IN IMS_SINT32 eServiceMethod, IN IMS_SINT32 nResponseIndex = INVALID_INDEX)
    {
        return GetUtils().GetResponseStatusCode(piSession, eServiceMethod, nResponseIndex);
    }
    inline static IMS_RESULT GetRemoteUris(
            IN ISession* piSession, IN PeerType ePeerType, OUT ImsList<AString>& lstUris)
    {
        lstUris = GetUtils().GetRemoteUris(piSession, ePeerType);
        return lstUris.IsEmpty() ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetRemoteUri(
            IN ISession* piSession, IN PeerType ePeerType, OUT AString& strUri)
    {
        strUri = GetUtils().GetRemoteUri(piSession, ePeerType);
        return strUri.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetSessionId(IN ISession* piSession, OUT AString& strSessionId)
    {
        strSessionId = GetUtils().GetSessionId(piSession);
        return strSessionId.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetHeaders(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT ImsList<AString>& lstHeaders,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        lstHeaders = GetUtils().GetHeaders(piMessage, eHeaderType, strHeaderName);
        return lstHeaders.GetSize() == 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetHeader(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strHeader, IN const AString& strHeaderName = AString::ConstNull())
    {
        strHeader = GetUtils().GetHeader(piMessage, eHeaderType, strHeaderName);
        return strHeader.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetHeaderValue(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strValue, IN const AString& strHeaderName = AString::ConstNull())
    {
        strValue = GetUtils().GetHeaderValue(piMessage, eHeaderType, strHeaderName);
        return strValue.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_SINT32 GetHeaderValueInt(IN const IMessage* piMessage,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().GetHeaderValueInt(piMessage, eHeaderType, strHeaderName);
    }
    inline static IMS_RESULT GetParameterValue(IN const IMessage* piMessage,
            IN const AString& strParameterName, IN IMS_SINT32 eHeaderType, OUT AString& strValue,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        strValue = GetUtils().GetParameterValue(
                piMessage, strParameterName, eHeaderType, strHeaderName);
        return strValue.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static ImsList<AString> GetUserParts(IN const IMessage* piMessage,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().GetUserParts(piMessage, eHeaderType, strHeaderName);
    }
    inline static IMS_RESULT GetUserPart(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strUserPart, IN const AString& strHeaderName = AString::ConstNull())
    {
        strUserPart = GetUtils().GetUserPart(piMessage, eHeaderType, strHeaderName);
        return strUserPart.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetUserIds(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT ImsList<AString>& lstUserIds,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        lstUserIds = GetUtils().GetUserIds(piMessage, eHeaderType, strHeaderName);
        return lstUserIds.IsEmpty() ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetUserId(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strUserId, IN const AString& strHeaderName = AString::ConstNull())
    {
        strUserId = GetUtils().GetUserId(piMessage, eHeaderType, strHeaderName);
        return strUserId.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetDisplayNames(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT ImsList<AString>& lstDisplayNames,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        lstDisplayNames = GetUtils().GetDisplayNames(piMessage, eHeaderType, strHeaderName);
        return lstDisplayNames.IsEmpty() ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetDisplayName(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strDisplayName, IN const AString& strHeaderName = AString::ConstNull())
    {
        strDisplayName = GetUtils().GetDisplayName(piMessage, eHeaderType, strHeaderName);
        return strDisplayName.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetHosts(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT ImsList<AString>& lstHosts, IN const AString& strHeaderName = AString::ConstNull())
    {
        lstHosts = GetUtils().GetHosts(piMessage, eHeaderType, strHeaderName);
        return lstHosts.IsEmpty() ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetHost(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strHost, IN const AString& strHeaderName = AString::ConstNull())
    {
        strHost = GetUtils().GetHost(piMessage, eHeaderType, strHeaderName);
        return strHost.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetParameterValueFromUri(IN IMessage* piMessage,
            IN const AString& strParameterName, IN IMS_SINT32 eHeaderType, OUT AString& strValue,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        strValue = GetUtils().GetParameterValueFromUri(
                piMessage, strParameterName, eHeaderType, strHeaderName);
        return strValue.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetUris(IN IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType, OUT ImsList<AString>& lstUris,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        lstUris = GetUtils().GetUris(piMessage, bWithParameters, eHeaderType, strHeaderName);
        return lstUris.IsEmpty() ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT GetUri(IN IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType, OUT AString& strUri,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        strUri = GetUtils().GetUri(piMessage, bWithParameters, eHeaderType, strHeaderName);
        return strUri.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_SINT32 GetFeatures(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().GetFeatures(piMessage, eHeaderType, strHeaderName);
    }
    inline static IMS_SINT32 GetSosTypeFromServiceUrn(IN const IMessage* piMessage,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().GetSosTypeFromServiceUrn(piMessage, eHeaderType, strHeaderName);
    }
    inline static IMS_SINT32 GetCauseFromReasonHeader(
            IN const IMessage* piMessage, IN const AString& strProtocol = AString::ConstNull())
    {
        return GetUtils().GetCauseFromReasonHeader(piMessage, strProtocol);
    }
    inline static IMS_RESULT GetCauseAndTextFromReasonHeader(IN const IMessage* piMessage,
            OUT IMS_SINT32& nCause, OUT AString& strText,
            IN const AString& strProtocol = AString::ConstNull())
    {
        ReasonHeaderValue objValue =
                GetUtils().GetCauseAndTextFromReasonHeader(piMessage, strProtocol);
        nCause = objValue.nCause;
        strText = objValue.strText;
        return nCause == -1 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_SINT32 GetSupportedFeatures(IN IMessage* piMessage)
    {
        return GetUtils().GetSupportedFeatures(piMessage);
    }
    inline static IMS_SINT32 GetRequireFeatures(IN IMessage* piMessage)
    {
        return GetUtils().GetRequireFeatures(piMessage);
    }
    inline static IMS_RESULT GetIms3gppFromBody(
            IN const IMessage* piMessage, OUT Ims3gpp& objIms3gpp)
    {
        GetUtils().GetIms3gppFromBody(piMessage, objIms3gpp);
        return objIms3gpp.GetType() == Ims3gpp::TYPE_UNKNOWN ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_SINT32 GetStatusCodeInNotify(IN IMessage* piMessage)
    {
        return GetUtils().GetStatusCodeInNotify(piMessage);
    }
    inline static IMS_BOOL HasSdp(IN const IMessage* piMessage)
    {
        return GetUtils().HasSdp(piMessage);
    }
    inline static IMS_BOOL IsFocusConf(IN const IMessage* piMessage)
    {
        return GetUtils().IsFocusConf(piMessage);
    }
    inline static IMS_BOOL IsInitialRegistrationRequired(IN const IMessage* piMessage)
    {
        return GetUtils().IsInitialRegistrationRequired(piMessage);
    }
    inline static IMS_BOOL ContainsValue(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().ContainsValue(piMessage, strValue, eHeaderType, strHeaderName);
    }
    inline static IMS_BOOL HasValue(IN const IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().HasValue(piMessage, strValue, eHeaderType, strHeaderName);
    }
    inline static IMS_BOOL IsHeaderPresent(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().IsHeaderPresent(piMessage, eHeaderType, strHeaderName);
    }
    inline static IMS_BOOL ContainsTag(IN const AString& strHeader, IN const AString& strTag)
    {
        return GetUtils().ContainsTag(strHeader, strTag);
    }
    inline static IMS_BOOL ContainsAddressInPaid(
            IN const IMessage* piMessage, IN const AString& strAddress)
    {
        return GetUtils().ContainsAddressInPaid(piMessage, strAddress);
    }
    inline static IMS_RESULT SetHeader(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().SetHeader(piMessage, strValue, eHeaderType, strHeaderName);
    }
    inline static IMS_RESULT AddValueIfNotExists(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull())
    {
        return GetUtils().AddValueIfNotExists(piMessage, strValue, eHeaderType, strHeaderName);
    }
    inline static IMS_RESULT GenerateContentId(IN const AString& strHost, OUT AString& strContentId)
    {
        strContentId = GetUtils().GenerateContentId(strHost);
        return strContentId.GetLength() <= 0 ? IMS_FAILURE : IMS_SUCCESS;
    }
    inline static IMS_RESULT SetResourceListByConfUser(IN_OUT IMessage* piMessage,
            IN const AString& strContentId, IN ImsList<ConfUser*>& lstConfUser,
            IN IMS_BOOL bMultiPart, IN IMS_BOOL bCopyControl = IMS_TRUE)
    {
        return GetUtils().SetResourceListByConfUser(
                piMessage, strContentId, lstConfUser, bMultiPart, bCopyControl);
    }
    inline static IMS_RESULT SetResourceListByEntryUri(IN_OUT IMessage* piMessage,
            IN const AString& strContentId, IN ImsList<AString>& lstEntryUri,
            IN IMS_BOOL bMultiPart, IN IMS_BOOL bCopyControl = IMS_TRUE)
    {
        return GetUtils().SetResourceListByEntryUri(
                piMessage, strContentId, lstEntryUri, bMultiPart, bCopyControl);
    }
    inline static IMS_BOOL IsVideoFeatureIncluded(IN const IMessage* piMessage)
    {
        return GetUtils().IsVideoFeatureIncluded(piMessage);
    }
    inline static IMS_BOOL IsTextFeatureIncluded(IN const IMessage* piMessage)
    {
        return GetUtils().IsTextFeatureIncluded(piMessage);
    }
    inline static CallType GetCallType(
            IN const IMessage* piMessage, IN ISession* piSession, IN IMS_BOOL bPeerView)
    {
        return GetUtils().GetCallType(piMessage, piSession, bPeerView);
    }
    inline static CallType GetCallTypeFromSdp(IN ISession* piSession, IN IMS_BOOL bNegoSdp,
            IN IMS_BOOL bPeerView, IN IMS_BOOL bCheckPort = IMS_TRUE)
    {
        return GetUtils().GetCallTypeFromSdp(piSession, bNegoSdp, bPeerView, bCheckPort);
    }  // TODO: change name of bPeerView
    inline static IMS_BOOL IsResponseExist(IN ISession* piSession, IN IMS_SINT32 nStatusCode)
    {
        return GetUtils().IsResponseExist(piSession, nStatusCode);
    }

private:
    inline static IMessageUtils& GetUtils()
    {
        return MtcContextRepository::GetContext()->GetMessageUtils();
    }

public:
    static const IMS_CHAR STR_199[];
    static const IMS_CHAR STR_ALERT_URN_CALL_WAITING[];
    static const IMS_CHAR STR_ANONYMOUS[];
    static const IMS_CHAR STR_UNAVAILABLE[];
    static const IMS_CHAR STR_CONTENT_DISPOSITION_RECIPIENT_LIST[];
    static const IMS_CHAR STR_CONTENT_ID[];
    static const IMS_CHAR STR_CONTENT_TYPE_3GPP_IMS_XML[];
    static const IMS_CHAR STR_CONTENT_TYPE_APPLICATION[];
    static const IMS_CHAR STR_CONTENT_TYPE_RESOURCE_LISTS_XML[];
    static const IMS_CHAR STR_CONTENT_TYPE_SIP_FRAG[];
    static const IMS_CHAR STR_ACCEPT_TYPE_APPLICATION_SDP[];
    static const IMS_CHAR STR_ACCEPT_TYPE_APPLICATION_3GPP_CURRENT_LOCATION_DISCOVERY_XML[];
    static const IMS_CHAR STR_ACCEPT_TYPE_APPLICATION_3GPP_IMS_XML[];
    static const IMS_CHAR STR_HEADER[];
    static const IMS_CHAR STR_HISTINFO[];
    static const IMS_CHAR STR_ICSI[];
    static const IMS_CHAR STR_ID[];
    static const IMS_CHAR STR_NONE[];
    static const IMS_CHAR STR_PACKAGE_CURRENT_LOCATION_DISCOVERY[];
    static const IMS_CHAR STR_PARAMETER_IS_FOCUS[];
    static const IMS_CHAR STR_PRECONDITION[];
    static const IMS_CHAR STR_REASON_FAILURE_TO_TRANSITION[];
    static const IMS_CHAR STR_REASON_HANDOVER_CANCELLED[];
    static const IMS_CHAR STR_RELEASE_CAUSE_1[];
    static const IMS_CHAR STR_RELEASE_CAUSE_2[];
    static const IMS_CHAR STR_RELEASE_CAUSE_3[];
    static const IMS_CHAR STR_RELEASE_CAUSE_4[];
    static const IMS_CHAR STR_RELEASE_CAUSE_5[];
    static const IMS_CHAR STR_RELEASE_CAUSE_6[];
    static const IMS_CHAR STR_REPLACES[];
    static const IMS_CHAR STR_SERVICE[];
    static const IMS_CHAR STR_SOS_AMBULANCE[];
    static const IMS_CHAR STR_SOS_ANIMAL_CONTROL[];
    static const IMS_CHAR STR_SOS_COUNTRY_SPECIFIC[];
    static const IMS_CHAR STR_SOS_FIRE[];
    static const IMS_CHAR STR_SOS_GAS[];
    static const IMS_CHAR STR_SOS_MARINE[];
    static const IMS_CHAR STR_SOS_MOUNTAIN[];
    static const IMS_CHAR STR_SOS_PHYSICIAN[];
    static const IMS_CHAR STR_SOS_POISON[];
    static const IMS_CHAR STR_SOS_POLICE[];
    static const IMS_CHAR STR_SOS[];
    static const IMS_CHAR STR_SRVCC_FEATURE_A[];
    static const IMS_CHAR STR_SRVCC_FEATURE_B[];
    static const IMS_CHAR STR_SRVCC_FEATURE_M[];
    static const IMS_CHAR STR_SUPPORTED[];
    static const IMS_CHAR STR_TIMER[];
    static const IMS_CHAR STR_URN[];
    static const IMS_CHAR STR_VIDEO[];
    static const IMS_CHAR STR_TEXT[];

    // carrier specific
    static const IMS_CHAR STR_P_SKT_BYE_CAUSE[];
    static const IMS_CHAR STR_P_TTA_VOLTE_INFO[];
    static const IMS_CHAR STR_AVCHANGE[];
    static const IMS_CHAR STR_REASON_USER_SESSIONEXPIRED[];
    static const IMS_CHAR STR_P_COM_ENABLETRANSCODING[];

private:
    static const IMS_SINT32 INVALID_INDEX;
};

#endif
