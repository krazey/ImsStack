/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_I_MESSAGE_UTILS_H_
#define MOCK_I_MESSAGE_UTILS_H_

#include "AString.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "utility/IMessageUtils.h"
#include <gmock/gmock.h>

class IMessage;
class IMtcContext;
class Ims3gpp;
class ISession;
struct ConfUser;

class MockIMessageUtils : public IMessageUtils
{
public:
    ~MockIMessageUtils() {}

    MOCK_METHOD(IMessage*, GetPreviousResponse, (IN const ISession*, IN IMS_SINT32, IN IMS_SINT32),
            (override));
    MOCK_METHOD(IMessage*, GetRemotePreviousMessage,
            (IN ISession*, IN IMS_SINT32, IN IMS_BOOL, IN IMS_SINT32), (override));
    MOCK_METHOD(IMS_SINT32, GetResponseStatusCode, (IN ISession*, IN IMS_SINT32, IN IMS_SINT32),
            (override));
    MOCK_METHOD(ImsList<AString>, GetRemoteUris, (IN ISession*, IN PeerType), (override));
    MOCK_METHOD(AString, GetRemoteUri, (IN ISession*, IN PeerType), (override));
    MOCK_METHOD(AString, GetSessionId, (IN ISession*), (override));
    MOCK_METHOD(ImsList<AString>, GetHeaders,
            (IN const IMessage*, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(
            AString, GetHeader, (IN const IMessage*, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(AString, GetHeaderValue, (IN const IMessage*, IN IMS_SINT32, IN const AString&),
            (override));
    MOCK_METHOD(IMS_SINT32, GetHeaderValueInt,
            (IN const IMessage*, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(AString, GetParameterValue,
            (IN const IMessage*, IN const AString&, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(ImsList<AString>, GetUserParts,
            (IN const IMessage*, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(AString, GetUserPart, (IN const IMessage*, IN IMS_SINT32, IN const AString&),
            (override));
    MOCK_METHOD(ImsList<AString>, GetUserIds, (IN IMessage*, IN IMS_SINT32, IN const AString&),
            (override));
    MOCK_METHOD(AString, GetUserId, (IN IMessage*, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(ImsList<AString>, GetDisplayNames, (IN IMessage*, IN IMS_SINT32, IN const AString&),
            (override));
    MOCK_METHOD(
            AString, GetDisplayName, (IN IMessage*, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(ImsList<AString>, GetHosts, (IN IMessage*, IN IMS_SINT32, IN const AString&),
            (override));
    MOCK_METHOD(AString, GetHost, (IN IMessage*, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(AString, GetParameterValueFromUri,
            (IN IMessage*, IN const AString&, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(ImsList<AString>, GetUris,
            (IN const IMessage*, IN IMS_BOOL, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(AString, GetUri,
            (IN const IMessage*, IN IMS_BOOL, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(IMS_SINT32, GetSosTypeFromServiceUrn,
            (IN const IMessage*, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(IMS_SINT32, GetCauseFromReasonHeader, (IN const IMessage*, IN const AString&),
            (override));
    MOCK_METHOD(ReasonHeaderValue, GetCauseAndTextFromReasonHeader,
            (IN const IMessage*, IN const AString&), (override));
    MOCK_METHOD(Ims3gpp&, GetIms3gppFromBody, (IN const IMessage*, OUT Ims3gpp&), (override));
    MOCK_METHOD(Ims3gppData, GetIms3gppData, (IN const IMessage*), (override));
    MOCK_METHOD(IMS_SINT32, GetStatusCodeInNotify, (IN IMessage*), (override));
    MOCK_METHOD(IMS_BOOL, HasSdp, (IN const IMessage*), (override));
    MOCK_METHOD(IMS_BOOL, IsFocusConf, (IN const IMessage*), (override));
    MOCK_METHOD(IMS_BOOL, IsInitialRegistrationRequired, (IN const IMessage*), (override));
    MOCK_METHOD(IMS_BOOL, IsInitialEmergencyRegistrationRequired, (IN const IMessage*), (override));
    MOCK_METHOD(IMS_BOOL, ContainsValue,
            (IN IMessage*, IN const AString&, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(IMS_BOOL, HasValue,
            (IN const IMessage*, IN const AString&, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(IMS_BOOL, IsHeaderPresent, (IN const IMessage*, IN IMS_SINT32, IN const AString&),
            (override));
    MOCK_METHOD(IMS_BOOL, ContainsTag, (IN const AString&, IN const AString&), (override));
    MOCK_METHOD(
            IMS_BOOL, ContainsAddressInPaid, (IN const IMessage*, IN const AString&), (override));
    MOCK_METHOD(IMS_RESULT, SetHeader,
            (IN IMessage*, IN const AString&, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(IMS_RESULT, AddValueIfNotExists,
            (IN IMessage*, IN const AString&, IN IMS_SINT32, IN const AString&), (override));
    MOCK_METHOD(AString, GenerateContentId, (IN const AString&), (override));
    MOCK_METHOD(IMS_RESULT, SetResourceList,
            (IN_OUT IMessage*, IN IMtcContext&, IN const ImsList<ConfUser*>&, IN IMS_BOOL,
                    IN IMS_BOOL),
            (override));
    MOCK_METHOD(IMS_BOOL, IsVideoFeatureIncluded, (IN const IMessage*), (override));
    MOCK_METHOD(IMS_BOOL, IsTextFeatureIncluded, (IN const IMessage*), (override));
    MOCK_METHOD(CallType, GetCallType, (IN const IMessage*, IN ISession*, IN IMS_BOOL), (override));
    MOCK_METHOD(CallType, GetCallTypeFromSdp, (IN ISession*, IN IMS_BOOL, IN IMS_BOOL, IN IMS_BOOL),
            (override));
    MOCK_METHOD(IMS_BOOL, IsResponseExist, (IN ISession*, IN IMS_SINT32), (override));
    MOCK_METHOD(IMS_UINT32, GetNumberOfPreviousResponses, (IN const ISession*, IN IMS_SINT32),
            (const, override));
};

#endif
