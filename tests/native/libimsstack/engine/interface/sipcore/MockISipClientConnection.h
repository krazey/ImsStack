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

#ifndef MOCK_I_SIP_CLIENT_CONNECTION_H_
#define MOCK_I_SIP_CLIENT_CONNECTION_H_

#include <gmock/gmock.h>
#include "ImsTypeDef.h"
#include "IpAddress.h"
#include "ISipClientConnection.h"
#include "Sip.h"

class Credential;
class ISipAckPackage;
class ISipClientConnectionListener;
class ISipConnectionNotifier;
class ISipGenericChallenge;

class MockISipClientConnection : public ISipClientConnection
{
public:
    inline MockISipClientConnection() {}
    inline virtual ~MockISipClientConnection() {}

    MOCK_METHOD(IMS_RESULT, InitAck, (), (override));
    MOCK_METHOD(ISipClientConnection*, InitCancel, (), (override));
    MOCK_METHOD(IMS_RESULT, InitRequest,
            (IN const AString& strMethod, IN ISipConnectionNotifier* piScn), (override));
    MOCK_METHOD(IMS_RESULT, Receive, (IN IMS_SLONG nTimeout), (override));
    MOCK_METHOD(IMS_RESULT, SetCredentials, (IN ImsList<Credential> & objCredentials), (override));
    MOCK_METHOD(IMS_RESULT, SetCredentials, (IN const Credential& objCredential), (override));
    MOCK_METHOD(void, SetListener, (IN ISipClientConnectionListener* piListener), (override));
    MOCK_METHOD(IMS_RESULT, SetRequestUri, (IN const AString& strUri), (override));
    MOCK_METHOD(ISipGenericChallenge*, GetAuthenticationChallenge, (IN IMS_SINT32 nIndex),
            (const, override));
    MOCK_METHOD(ISipAckPackage*, GrabAck, (), (override));
    MOCK_METHOD(IMS_RESULT, InitResubmissionRequest, (), (override));
    MOCK_METHOD(void, RemoveAllChallenges, (), (override));
    MOCK_METHOD(void, RemoveAllCredentials, (), (override));
    MOCK_METHOD(IMS_RESULT, SetAuthenticationChallenge, (IN ISipGenericChallenge* piChallenge),
            (override));
    MOCK_METHOD(void, SetExtensionTokenForViaBranch, (IN const AString& strToken), (override));
    MOCK_METHOD(void, SetImplicitRouteHeader, (IN const AString& strRouteHeader), (override));
    MOCK_METHOD(void, SetTransportTuple,
            (IN const IpAddress& objIpAddr, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
                    IN IMS_SINT32 nPortFc, IN IMS_SINT32 nTransportExt),
            (override));

    MOCK_METHOD(IMS_RESULT, AddHeader, (IN const AString& strName, IN const AString& strValue),
            (override));
    MOCK_METHOD(ISipDialog*, GetDialog, (), (const, override));
    MOCK_METHOD(AString, GetHeader, (IN const AString& strName, IN IMS_SINT32 nIndex), (override));
    MOCK_METHOD(ImsList<AString>, GetHeaders, (IN const AString& strName), (override));
    MOCK_METHOD(const SipMethod&, GetMethod, (), (const, override));
    MOCK_METHOD(const AString&, GetReasonPhrase, (), (const, override));
    MOCK_METHOD(const AString&, GetRequestUri, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetStatusCode, (), (const, override));
    MOCK_METHOD(IMS_RESULT, RemoveHeader, (IN const AString& strName), (override));
    MOCK_METHOD(IMS_RESULT, Send, (), (override));
    MOCK_METHOD(void, SetErrorListener, (IN ISipErrorListener* piListener), (override));
    MOCK_METHOD(IMS_RESULT, SetHeader, (IN const AString& strName, IN const AString& strValue),
            (override));
    MOCK_METHOD(const ByteArray&, GetContent, (), (const, override));
    MOCK_METHOD(IMS_RESULT, SetContent, (IN const ByteArray& objContent), (override));
    MOCK_METHOD(IMS_SINT32, GetHeaderCount, (IN const AString& strName), (const, override));
    MOCK_METHOD(ISipMessage*, GetMessage, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(void, SetSipProfile, (IN SipProfile* pProfile), (override));
    MOCK_METHOD(void, SetTransactionTimerValues, (IN const SipTimerValues& objTv), (override));

    MOCK_METHOD(void, Close, (), (override));
};

#endif
