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

#ifndef MESSAGE_UTILS_H_
#define MESSAGE_UTILS_H_

#include "AString.h"
#include "IMtcService.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"
#include <tuple>

class AStringBuffer;
class IMessage;
class IMessageBodyPart;
class IMtcContext;
class Ims3gpp;
class ISession;
class ISipHeader;
class ISipMessage;
class IMtcCall;
class SipAddress;
class SipParameter;
struct ConfUser;

class MessageUtils : public IMessageUtils
{
public:
    MessageUtils();
    virtual ~MessageUtils();
    MessageUtils(IN const MessageUtils&) = delete;
    MessageUtils& operator=(IN const MessageUtils&) = delete;

    IMessage* GetPreviousResponse(IN const ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_SINT32 nResponseIndex = -1) override;
    IMessage* GetRemotePreviousMessage(IN ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_BOOL bIsUac, IN IMS_SINT32 nResponseIndex = -1) override;
    IMS_SINT32 GetResponseStatusCode(IN ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_SINT32 nResponseIndex = -1) override;
    ImsList<AString> GetRemoteUris(IN ISession* piSession, IN PeerType ePeerType) override;
    AString GetRemoteUri(IN ISession* piSession, IN PeerType ePeerType) override;
    AString GetSessionId(IN ISession* piSession) override;
    ImsList<AString> GetHeaders(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetHeader(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetHeaderValue(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    IMS_SINT32 GetHeaderValueInt(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetParameterValue(IN const IMessage* piMessage, IN const AString& strParameterName,
            IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    ImsList<AString> GetUserParts(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetUserPart(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    ImsList<AString> GetUserIds(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetUserId(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    ImsList<AString> GetDisplayNames(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetDisplayName(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    ImsList<AString> GetHosts(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetHost(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetParameterValueFromUri(IN IMessage* piMessage, IN const AString& strParameterName,
            IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    ImsList<AString> GetUris(IN IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GetUri(IN IMessage* piMessage, IN IMS_BOOL bWithParameters, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    IMS_SINT32 GetSosTypeFromServiceUrn(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    IMS_SINT32 GetCauseFromReasonHeader(IN const IMessage* piMessage,
            IN const AString& strProtocol = AString::ConstNull()) override;
    ReasonHeaderValue GetCauseAndTextFromReasonHeader(IN const IMessage* piMessage,
            IN const AString& strProtocol = AString::ConstNull()) override;
    Ims3gpp& GetIms3gppFromBody(IN const IMessage* piMessage, OUT Ims3gpp& objIms3gpp) override;
    Ims3gppData GetIms3gppData(IN const IMessage* piMessage) override;
    IMS_SINT32 GetStatusCodeInNotify(IN IMessage* piMessage) override;
    IMS_BOOL HasSdp(IN const IMessage* piMessage) override;
    IMS_BOOL IsFocusConf(IN const IMessage* piMessage) override;
    IMS_BOOL IsInitialRegistrationRequired(IN const IMessage* piMessage) override;
    IMS_BOOL IsInitialEmergencyRegistrationRequired(IN const IMessage* piMessage) override;
    IMS_BOOL ContainsValue(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    IMS_BOOL HasValue(IN const IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    IMS_BOOL IsHeaderPresent(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    IMS_BOOL ContainsTag(IN const AString& strHeader, IN const AString& strTag) override;
    IMS_BOOL ContainsAddressInPaid(
            IN const IMessage* piMessage, IN const AString& strAddress) override;
    IMS_RESULT SetHeader(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    IMS_RESULT AddValueIfNotExists(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) override;
    AString GenerateContentId(IN const AString& strHost) override;
    IMS_RESULT SetResourceList(IN_OUT IMessage* piMessage, IN IMtcContext& objContext,
            IN const AString& strContentId, IN const ImsList<ConfUser*>& lstConfUser,
            IN IMS_BOOL bWithDialogId, IN IMS_BOOL bMultiPart) override;
    IMS_BOOL IsVideoFeatureIncluded(IN const IMessage* piMessage) override;
    IMS_BOOL IsTextFeatureIncluded(IN const IMessage* piMessage) override;
    CallType GetCallType(IN const IMessage* piMessage, IN ISession* piSession,
            IN IMS_BOOL bCheckRemote) override;
    CallType GetCallTypeFromSdp(IN ISession* piSession, IN IMS_BOOL bActiveMediaOnly,
            IN IMS_BOOL bCheckRemote, IN IMS_BOOL bIgnorePort0 = IMS_TRUE) override;
    IMS_BOOL IsResponseExist(IN ISession* piSession, IN IMS_SINT32 nStatusCode) override;
    IMS_UINT32 GetNumberOfPreviousResponses(
            IN const ISession* piSession, IN IMS_SINT32 eServiceMethod) const override;

private:
    static ISipMessage* GetSipMessage(IN const IMessage* piMessage);
    IMS_RESULT GetAddresses(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT ImsList<SipAddress>& lstAddresses,
            IN const AString& strHeaderName = AString::ConstNull());
    static void GetParameterValueFromUnknownHeaderBody(
            IN const AString& strBody, IN const AString& strParameterName, OUT AString& strValue);
    IMS_RESULT GetUrnValue(IN const IMessage* piMessage, IN const AString& strId,
            IN IMS_SINT32 eHeaderType, OUT AString& strValue,
            IN const AString& strHeaderName = AString::ConstNull());
    IMS_RESULT SetResourceListWithHeaders(IN_OUT IMessage* piMessage,
            IN const AString& strContentId, IN IMS_BOOL bMultiPart, IN const AString& strXml);
    AString CreateResourceListXml(
            IN const ImsList<std::tuple<AString, AString, AString>>& objEntries);
    AString CreateEntryUri(
            IN IMtcContext& objContext, IN const ConfUser& objUser, IN IMS_BOOL bWithDialogId);
    AString CreateFromToPartWithTagValue(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType);
};

#endif
