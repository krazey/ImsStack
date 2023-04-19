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

#include "ImsList.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "call/IMtcSession.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceReference.h"
#include "conferencecall/MockIConferenceReferenceListener.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockIReference.h"
#include "core/MockISession.h"
#include "helper/MockICallStateProxy.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockReferenceInterfaceHolder.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipStatusCode.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL CallKey CONFERENCE_CALL_KEY = 100;
LOCAL CallKey JOINING_CALL_KEY = 101;
LOCAL IMS_UINT32 JOINING_CALL_ID = 1000;

class ConferenceReferenceTest : public ::testing::Test
{
public:
    ConferenceReference* pConferenceReference;
    MockIMtcContext objMockContext;
    MockIConferenceReferenceListener objMockListener;
    MtcConfigurationProxy* pConfigurationProxy;
    MockIMtcConfigurationManager* pMockConfigurationManager;
    MockICallStateProxy objMockCallStateProxy;
    MockIMtcSipInterfaceFactory objMockInterfaceFactory;
    MockReferenceInterfaceHolder* pMockReferenceInterfaceHolder;
    MockIInterfaceHolderListener objMockHolderListener;
    MockCallConnectionIdManager* pMockConnectionIdManager;
    MockIMtcCallManager objMockCallManager;
    MockIMtcCall objMockConferenceCall;
    MockIMtcCall objMockJoiningCall;
    MockIMtcCallContext objMockConfCallContext;
    MockIMtcCallContext objMockJoiningCallContext;
    MockIMtcService objMockService;
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcPreconditionManager objMockPreconditionManager;
    CallInfo objCallInfo;
    ConfUser objUser;
    MockIMessageUtils objMessageUtils;

    MockISession objMockConfSession;
    MockIMtcSession objMockConfMtcSession;
    MockISession objMockJoiningSession;
    MockIMtcSession objMockJoiningMtcSession;
    MockIReference objMockReference;

protected:
    virtual void SetUp() override
    {
        pMockConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pMockConfigurationManager);
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objMockContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        ON_CALL(objMockContext, GetCallStateProxy).WillByDefault(ReturnRef(objMockCallStateProxy));
        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));
        // ON_CALL(objMockContext, GetServiceByType).WillByDefault(ReturnRef(objMockCallManager));

        ON_CALL(objMockContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objMockInterfaceFactory));

        pMockReferenceInterfaceHolder = new MockReferenceInterfaceHolder(objMockHolderListener);
        ON_CALL(objMockInterfaceFactory, GetIReferenceHolder)
                .WillByDefault(Return(pMockReferenceInterfaceHolder));

        pMockConnectionIdManager = new MockCallConnectionIdManager(objMockContext);
        ON_CALL(*pMockConnectionIdManager, GetCallKey(JOINING_CALL_ID))
                .WillByDefault(Return(JOINING_CALL_KEY));

        CreateCalls();
        CreateReference();
    }

    virtual void TearDown() override
    {
        delete pConferenceReference;
        delete pConfigurationProxy;
        delete pMockReferenceInterfaceHolder;
        delete pMockConnectionIdManager;
    }

    void CreateReference()
    {
        // TODO: when multiple ConfUser logic is added, this should be updated
        objUser.strUserEntity = "sip:testUser@ims.google.com";
        objUser.nConnectionId = JOINING_CALL_ID;

        pConferenceReference = new ConferenceReference(
                objMockContext, CONFERENCE_CALL_KEY, &objUser, objMockListener);
    }

    void CreateCalls()
    {
        // conference call
        ON_CALL(objMockConferenceCall, GetKey).WillByDefault(Return(CONFERENCE_CALL_KEY));

        ON_CALL(objMockCallManager, GetCallByCallKey(CONFERENCE_CALL_KEY))
                .WillByDefault(Return(&objMockConferenceCall));

        ON_CALL(objMockConferenceCall, GetCallContext())
                .WillByDefault(ReturnRef(objMockConfCallContext));

        SetUpCallContext(objMockConfCallContext);

        ON_CALL(objMockConfMtcSession, GetISession()).WillByDefault(ReturnRef(objMockConfSession));

        ON_CALL(objMockConfCallContext, GetSession()).WillByDefault(Return(&objMockConfMtcSession));

        ON_CALL(objMockConfMtcSession, GetISession()).WillByDefault(ReturnRef(objMockConfSession));

        // joining call
        ON_CALL(objMockJoiningCall, GetKey).WillByDefault(Return(JOINING_CALL_KEY));

        ON_CALL(objMockCallManager, GetCallByCallKey(JOINING_CALL_KEY))
                .WillByDefault(Return(&objMockJoiningCall));

        ON_CALL(objMockJoiningCall, GetCallContext())
                .WillByDefault(ReturnRef(objMockJoiningCallContext));

        SetUpCallContext(objMockJoiningCallContext);

        ON_CALL(objMockJoiningMtcSession, GetISession())
                .WillByDefault(ReturnRef(objMockJoiningSession));

        ON_CALL(objMockJoiningCallContext, GetSession())
                .WillByDefault(Return(&objMockJoiningMtcSession));

        ON_CALL(objMockJoiningMtcSession, GetISession())
                .WillByDefault(ReturnRef(objMockJoiningSession));

        ImsList<AString> lstAddresses;
        lstAddresses.Append("sip:testUri@ims.google.com");
        ON_CALL(objMockJoiningSession, GetRemoteUserId).WillByDefault(Return(lstAddresses));
    }

    void SetUpCallContext(IN MockIMtcCallContext& objMockCallContext)
    {
        ON_CALL(objMockCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        ON_CALL(objMockCallContext, GetService).WillByDefault(ReturnRef(objMockService));

        ON_CALL(objMockCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));

        ON_CALL(objMockCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objMockPreconditionManager));

        ON_CALL(objMockCallContext, GetConfigurationProxy())
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objMockCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
    }
};

TEST_F(ConferenceReferenceTest, Constructor)
{
    ASSERT_NE(pConferenceReference, nullptr);
    EXPECT_EQ(pConferenceReference->GetType(), REFERENCE_TYPE_INVALID);
}

TEST_F(ConferenceReferenceTest, OnReferenceDeliveredAndSubsSupported)
{
    ON_CALL(*pMockConfigurationManager, IsSupportConferenceReferSubscribe)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMockListener, OnReferenceStarted(_)).Times(1);

    pConferenceReference->ReferenceDelivered(&objMockReference);
}

TEST_F(ConferenceReferenceTest, OnReferenceDeliveredAndNoSubsSupported)
{
    ON_CALL(*pMockConfigurationManager, IsSupportConferenceReferSubscribe)
            .WillByDefault(Return(IMS_FALSE));

    MockIMessage objMockMessage;
    ON_CALL(objMockReference, GetPreviousResponse(IMessage::REFERENCE_REFER))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode).WillByDefault(Return(200));

    EXPECT_CALL(objMockListener, OnReferenceStarted(_)).Times(1);

    pConferenceReference->ReferenceDelivered(&objMockReference);
}

TEST_F(ConferenceReferenceTest, OnReferenceDeliveryFailed)
{
    EXPECT_CALL(objMockListener, OnReferenceStartFailed(_)).Times(1);

    pConferenceReference->ReferenceDeliveryFailed(&objMockReference);
}

TEST_F(ConferenceReferenceTest, OnReferenceNotifyWithStateActive)
{
    MockIMessage objMockMessage;
    ON_CALL(objMessageUtils, GetHeaderValue(&objMockMessage, ISipHeader::SUBSCRIPTION_STATE, _))
            .WillByDefault(Return("active"));

    EXPECT_CALL(objMockListener, OnReferenceUpdated(_, _, ReferSubscriptionState::ACTIVE)).Times(1);

    pConferenceReference->ReferenceNotify(&objMockReference, &objMockMessage);
}

TEST_F(ConferenceReferenceTest, OnReferenceNotifyWithStateTerminated)
{
    MockIMessage objMockMessage;
    ON_CALL(objMessageUtils, GetHeaderValue(&objMockMessage, ISipHeader::SUBSCRIPTION_STATE, _))
            .WillByDefault(Return("terminated"));

    EXPECT_CALL(objMockListener, OnReferenceUpdated(_, _, ReferSubscriptionState::TERMINATED))
            .Times(1);

    pConferenceReference->ReferenceNotify(&objMockReference, &objMockMessage);
}

TEST_F(ConferenceReferenceTest, OnReferenceTerminated)
{
    EXPECT_CALL(objMockListener,
            OnReferenceUpdated(pConferenceReference, _, ReferSubscriptionState::TERMINATED))
            .Times(1);

    pConferenceReference->ReferenceTerminated(&objMockReference);
}

TEST_F(ConferenceReferenceTest, SendInviteWithSingleUser)
{
    ON_CALL(*pMockConfigurationManager, IsConferenceReferToUriSourcePaid)
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pMockReferenceInterfaceHolder, GetIReference(_, _, _))
            .WillByDefault(Return(&objMockReference));

    ON_CALL(objMessageUtils, GetSessionId(&objMockJoiningSession)).WillByDefault(Return("12345"));

    // TODO: let UriFormatter returns test uri
    AString strReferToUri = "sip:testuri@ims.google.com";
    ON_CALL(objMessageUtils, GetRemoteUri(_, _)).WillByDefault(Return(strReferToUri));
    IMS_RESULT nResult = pConferenceReference->SendInvite(strReferToUri, *pMockConnectionIdManager);
    EXPECT_EQ(nResult, IMS_SUCCESS);
    EXPECT_EQ(pConferenceReference->GetType(), REFERENCE_TYPE_INVITE);
}

}  // namespace android
