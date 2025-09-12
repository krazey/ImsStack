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
#ifndef MOCK_I_SESSION_H_
#define MOCK_I_SESSION_H_

#include <gmock/gmock.h>
#include "AString.h"
#include "ImsList.h"
#include "IServiceMethod.h"
#include "ISession.h"
#include "SipMethod.h"

class ICapabilities;
class IMedia;
class IPublication;
class IReference;
class IRefreshListener;
class IMessage;
class IMessageMediator;
class ISessionDescriptor;
class ISessionListener;
class ISessionParameter;
class ISipClientConnection;
class ISipHeader;
class ISubscription;
class Replaces;

class MockISession : public ISession
{
public:
    ~MockISession() override = default;

    // IMethod
    MOCK_METHOD(void, Destroy, (), (override));
    MOCK_METHOD(void, SetMessageMediator, (IN IMessageMediator * piMediator), (override));

    // IServiceMethod
    MOCK_METHOD(IMessage*, GetNextRequest, (), (override));
    MOCK_METHOD(IMessage*, GetNextResponse, (), (override));
    MOCK_METHOD(IMessage*, GetPreviousRequest, (IN IMS_SINT32 nServiceMethod), (const, override));
    MOCK_METHOD(IMessage*, GetPreviousResponse, (IN IMS_SINT32 nServiceMethod), (const, override));
    MOCK_METHOD(ImsList<IMessage*>, GetPreviousResponses, (IN IMS_SINT32 nServiceMethod),
            (const, override));
    MOCK_METHOD(ImsList<AString>, GetRemoteUserId, (), (const, override));

    // ISession
    MOCK_METHOD(IMS_RESULT, Accept, (), (override));
    MOCK_METHOD(ICapabilities*, CreateCapabilities, (), (override));
    MOCK_METHOD(IMedia*, CreateMedia,
            (IN const AString& strType, IN IMS_SINT32 nDirection, IN IMS_SINT32 nCountOfDescriptor),
            (override));
    MOCK_METHOD(IReference*, CreateReference,
            (IN const AString& strReferTo, IN const AString& strReferMethod), (override));
    MOCK_METHOD(ImsList<IMedia*>, GetMedia, (), (override));
    MOCK_METHOD(ISessionDescriptor*, GetSessionDescriptor, (), (override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_BOOL, HasPendingUpdate, (), (const, override));
    MOCK_METHOD(IMS_RESULT, Reject, (), (override));
    MOCK_METHOD(IMS_RESULT, Reject, (IN IMS_SINT32 nStatusCode), (override));
    MOCK_METHOD(IMS_RESULT, RejectWithDiversion, (IN const AString& strAlternativeUserAddress),
            (override));
    MOCK_METHOD(IMS_RESULT, RemoveMedia, (IN IMedia * piMedia), (override));
    MOCK_METHOD(IMS_RESULT, Restore, (), (override));
    MOCK_METHOD(void, SetListener, (IN ISessionListener * piListener), (override));
    MOCK_METHOD(IMS_RESULT, Start, (), (override));
    MOCK_METHOD(IMS_RESULT, Terminate, (), (override));
    MOCK_METHOD(IMS_RESULT, Update, (), (override));
    MOCK_METHOD(ISubscription*, CreateSubscription, (IN const AString& strEvent), (override));
    MOCK_METHOD(IPublication*, CreatePublication, (IN const AString& strEvent), (override));
    MOCK_METHOD(
            ISipClientConnection*, CreateTransaction, (IN const SipMethod& objMethod), (override));
    MOCK_METHOD(IMS_SINT32, GetConfiguration, (), (const, override));
    MOCK_METHOD(const ISipHeader*, GetContactHeader, (), (const, override));
    MOCK_METHOD(const Replaces*, GetReplaces, (), (const, override));
    MOCK_METHOD(const AString&, GetSessionId, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetTerminationReason, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsFinalResponseReceivedForInitialInviteRequest, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsReliableProvResponseSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSdpNegotiationAllowedForNonRpr, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSdpOaInPreviewMode, (), (const, override));
    MOCK_METHOD(IMS_RESULT, RejectEx,
            (IN IMS_SINT32 nStatusCode, IN const AString& strReasonPhrase), (override));
    MOCK_METHOD(IMS_RESULT, RespondToEarlyUpdate,
            (IN IMS_SINT32 nStatusCode, IN const AString& strReason), (override));
    MOCK_METHOD(IMS_RESULT, RespondToPrack,
            (IN IMS_SINT32 nStatusCode, IN const AString& strReason), (override));
    MOCK_METHOD(IMS_RESULT, SendAck, (), (override));
    MOCK_METHOD(IMS_RESULT, SendPrack, (), (override));
    MOCK_METHOD(IMS_RESULT, SendProvisionalResponse,
            (IN IMS_SINT32 nStatusCode, IN const AString& strReason, IN IMS_SINT32 nFlags),
            (override));
    MOCK_METHOD(IMS_RESULT, SendRpr,
            (IN IMS_SINT32 nStatusCode, IN const AString& strReason, IN IMS_BOOL bSdp,
                    IN IMS_SINT32 nFlags),
            (override));
    MOCK_METHOD(IMS_RESULT, SetCallerPreference, (IN const ImsList<AString>& objCallerPreference),
            (override));
    MOCK_METHOD(void, SetConfiguration, (IN IMS_SINT32 nConfigValue), (override));
    MOCK_METHOD(IMS_RESULT, SetContactParameter,
            (IN const AString& strParameter, IN IMS_SINT32 nOperation), (override));
    MOCK_METHOD(void, SetImplicitRoutingRequired, (IN IMS_BOOL bFlag), (override));
    MOCK_METHOD(void, SetReasonForCallTermination, (IN IMS_SINT32 nReason), (override));
    MOCK_METHOD(void, SetRefreshListener, (IN IRefreshListener * piListener), (override));
    MOCK_METHOD(void, SetRefreshPolicy,
            (IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLt,
                    IN IMS_SINT32 nValueGt),
            (override));
    MOCK_METHOD(IMS_RESULT, TerminateEx, (IN IMS_BOOL bTerminateMethodBye), (override));
    MOCK_METHOD(IMS_RESULT, UpdateEarlyMedia, (), (override));
    MOCK_METHOD(
            IMS_RESULT, UpdateEx, (IN IMS_SINT32 nMethod, IN IMS_BOOL bSessionRefresh), (override));
    MOCK_METHOD(IMS_RESULT, CreateFailureSdp, (), (override));
    MOCK_METHOD(void, DestroyFailureSdp, (), (override));
    MOCK_METHOD(ISessionParameter*, GetFailureSdp, (), (const, override));
    MOCK_METHOD(ISession*, GetOwnerSession, (), (const, override));
    MOCK_METHOD(ISession*, GetVirtualSession, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSessionRefreshInProgress, (), (const, override));
    MOCK_METHOD(void, SetReasonHeaderSetter, (IN IReasonHeaderSetter * piSetter), (override));
    MOCK_METHOD(ISdpReader*, GetRemoteMediaCapabilities, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSessionCanceledOnAccepted, (), (const, override));
    MOCK_METHOD(void, AbortEarlyUpdateTransaction, (), (override));
};

#endif
