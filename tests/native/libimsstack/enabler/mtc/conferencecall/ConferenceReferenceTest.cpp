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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MtcContextRepository.h"
#include "conferencecall/ConferenceReference.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/MockIConferenceReferenceListener.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "core/MockIReference.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "sipcore/MockISipMessage.h"
#include "helper/MockICallStateProxy.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockReferenceInterfaceHolder.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "conferencecall/MockCallConnectionIdManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "sipcore/SipStatusCode.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/IMtcSession.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "media/MockIMtcMediaManager.h"
#include "call/MtcUiNotifier.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL CallKey CONFERENCE_CALL_KEY = 100;
LOCAL CallKey JOINING_CALL_KEY = 101;
LOCAL IMS_UINT32 JOINGING_CALL_ID = 1000;
LOCAL IMS_SINT32 SLOT_ID = 0;

class ConferenceReferenceTest : public ::testing::Test
{
public:
    ConferenceReference* pConferenceReference;
    MockIMtcContext objMockContext;
    MockIConferenceReferenceListener objMockListener;
    MtcConfigurationProxy* pConfigurationProxy;
    MockICallStateProxy objMockCallStateProxy;
    MockIMtcSipInterfaceFactory* pMockInterfaceFactory;
    MockReferenceInterfaceHolder* pMockReferenceInterfaceHolder;
    MockIInterfaceHolderListener* pMockHolderListener;
    MockIMtcConfigurationManager objMockConfigurationManager;
    MockCallConnectionIdManager* pMockConnectionIdManager;
    MockIMtcCallManager objMockCallManager;
    MockIMtcCall* pMockConferenceCall;
    MockIMtcCall* pMockJoiningCall;
    MockIMtcCallContext objMockConfCallContext;
    MockIMtcCallContext objMockJoiningCallContext;
    MockIMtcService objMockService;
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcPreconditionManager objMockPreconditionManager;
    CallInfo objCallInfo;

    MockISession* pMockConfSession;
    MockIMtcSession* pMockConfMtcSession;
    MockISession* pMockJoiningSession;
    MockIMtcSession* pMockJoiningMtcSession;

protected:
    virtual void SetUp() override
    {
        MtcContextRepository::GetInstance()->AddContext(SLOT_ID, &objMockContext);

        pConfigurationProxy = new MtcConfigurationProxy(&objMockConfigurationManager);
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objMockContext, GetCallStateProxy).WillByDefault(ReturnRef(objMockCallStateProxy));
        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));
        // ON_CALL(objMockContext, GetServiceByType).WillByDefault(ReturnRef(objMockCallManager));

        pMockInterfaceFactory = new MockIMtcSipInterfaceFactory();
        ON_CALL(objMockContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(*pMockInterfaceFactory));

        pMockHolderListener = new MockIInterfaceHolderListener();
        pMockReferenceInterfaceHolder = new MockReferenceInterfaceHolder(*pMockHolderListener);
        ON_CALL(*pMockInterfaceFactory, GetIReferenceHolder)
                .WillByDefault(Return(pMockReferenceInterfaceHolder));

        pMockConnectionIdManager = new MockCallConnectionIdManager(objMockContext);
        ON_CALL(*pMockConnectionIdManager, GetCallKey(JOINGING_CALL_ID))
                .WillByDefault(Return(JOINING_CALL_KEY));

        // TODO: when multiple ConfUser logic is added, this should be updated
        ConfUser* pUser = new ConfUser();
        pUser->strUserEntity = "sip:testUser@ims.google.com";
        pUser->nConnectionId = JOINGING_CALL_ID;

        CreateCalls();
        CreateReference(pUser);
    }

    virtual void TearDown() override
    {
        delete pConferenceReference;
        delete pMockInterfaceFactory;
        delete pMockReferenceInterfaceHolder;
        delete pMockHolderListener;
        delete pMockConnectionIdManager;
    }

    void CreateReference(IN ConfUser* pUser)
    {
        pConferenceReference = new ConferenceReference(
                objMockContext, CONFERENCE_CALL_KEY, pUser, objMockListener);
    }

    void CreateCalls()
    {
        // conference call
        pMockConferenceCall = new MockIMtcCall();
        ON_CALL(*pMockConferenceCall, GetKey).WillByDefault(Return(CONFERENCE_CALL_KEY));

        ON_CALL(objMockCallManager, GetCallByCallKey(CONFERENCE_CALL_KEY))
                .WillByDefault(Return(pMockConferenceCall));

        ON_CALL(*pMockConferenceCall, GetCallContext())
                .WillByDefault(ReturnRef(objMockConfCallContext));

        SetUpCallContext(objMockConfCallContext);

        pMockConfSession = new MockISession();  // TODO: delete
        pMockConfMtcSession = new MockIMtcSession();
        ON_CALL(*pMockConfMtcSession, GetISession()).WillByDefault(ReturnRef(*pMockConfSession));

        ON_CALL(objMockConfCallContext, GetSession()).WillByDefault(Return(pMockConfMtcSession));

        ON_CALL(*pMockConfMtcSession, GetISession()).WillByDefault(ReturnRef(*pMockConfSession));

        // joining call
        pMockJoiningCall = new MockIMtcCall();
        ON_CALL(*pMockJoiningCall, GetKey).WillByDefault(Return(JOINING_CALL_KEY));

        ON_CALL(objMockCallManager, GetCallByCallKey(JOINING_CALL_KEY))
                .WillByDefault(Return(pMockJoiningCall));

        ON_CALL(*pMockJoiningCall, GetCallContext())
                .WillByDefault(ReturnRef(objMockJoiningCallContext));

        SetUpCallContext(objMockJoiningCallContext);

        pMockJoiningSession = new MockISession();  // TODO: delete
        pMockJoiningMtcSession = new MockIMtcSession();
        ON_CALL(*pMockJoiningMtcSession, GetISession())
                .WillByDefault(ReturnRef(*pMockJoiningSession));

        ON_CALL(objMockJoiningCallContext, GetSession())
                .WillByDefault(Return(pMockJoiningMtcSession));

        ON_CALL(*pMockJoiningMtcSession, GetISession())
                .WillByDefault(ReturnRef(*pMockJoiningSession));

        IMSList<AString> lstAddresses;
        lstAddresses.Append("sip:testUri@ims.google.com");
        ON_CALL(*pMockJoiningSession, GetRemoteUserId).WillByDefault(Return(lstAddresses));
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

        MtcUiNotifier* pUiNotifier = new MtcUiNotifier(objMockCallContext);
        ON_CALL(objMockCallContext, GetUiNotifier).WillByDefault(ReturnRef(*pUiNotifier));
    }
};

TEST_F(ConferenceReferenceTest, Constructor)
{
    ASSERT_NE(pConferenceReference, nullptr);
    EXPECT_EQ(pConferenceReference->GetType(), REFERENCE_TYPE_INVALID);
}

TEST_F(ConferenceReferenceTest, OnReferenceDeliveredAndSubsSupported)
{
    ON_CALL(objMockConfigurationManager, IsSupportConferenceReferSubscribe)
            .WillByDefault(Return(IMS_TRUE));

    MockIReference* pMockReference = new MockIReference();

    EXPECT_CALL(objMockListener, OnReferenceStarted(_)).Times(1);

    pConferenceReference->ReferenceDelivered(pMockReference);
}

TEST_F(ConferenceReferenceTest, OnReferenceDeliveredAndNoSubsSupported)
{
    ON_CALL(objMockConfigurationManager, IsSupportConferenceReferSubscribe)
            .WillByDefault(Return(IMS_FALSE));

    MockIReference* pMockReference = new MockIReference();

    MockIMessage* pMockMessage = new MockIMessage();
    ON_CALL(*pMockReference, GetPreviousResponse(IMessage::REFERENCE_REFER))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetStatusCode).WillByDefault(Return(200));

    EXPECT_CALL(objMockListener, OnReferenceStarted(_)).Times(1);

    pConferenceReference->ReferenceDelivered(pMockReference);
    // delete pMockReference;
    // delete pMockMessage;
}

TEST_F(ConferenceReferenceTest, OnReferenceDeliveryFailed)
{
    MockIReference* pMockReference = new MockIReference();

    EXPECT_CALL(objMockListener, OnReferenceStartFailed(_)).Times(1);

    pConferenceReference->ReferenceDeliveryFailed(pMockReference);
    // delete pMockReference;
}

TEST_F(ConferenceReferenceTest, OnReferenceNotifyWithStateActive)
{
    MockIReference* pMockReference = new MockIReference();
    MockIMessage* pMockMessage = new MockIMessage();
    MockISipMessage objMockISipMessage;
    ON_CALL(*pMockMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

    ImsList<AString> objSubStates;
    objSubStates.Append("active");
    ON_CALL(objMockISipMessage, GetHeaders(_, _)).WillByDefault(Return(objSubStates));

    EXPECT_CALL(objMockListener, OnReferenceUpdated(_, _, ReferSubscriptionState::ACTIVE)).Times(1);

    pConferenceReference->ReferenceNotify(pMockReference, pMockMessage);
    // delete pMockReference;
    // delete pMockMessage;
}

TEST_F(ConferenceReferenceTest, OnReferenceNotifyWithStateTerminated)
{
    MockIReference* pMockReference = new MockIReference();
    MockIMessage* pMockMessage = new MockIMessage();
    MockISipMessage objMockISipMessage;
    ON_CALL(*pMockMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

    ImsList<AString> objSubStates;
    objSubStates.Append("terminated");
    ON_CALL(objMockISipMessage, GetHeaders(_, _)).WillByDefault(Return(objSubStates));

    EXPECT_CALL(objMockListener, OnReferenceUpdated(_, _, ReferSubscriptionState::TERMINATED))
            .Times(1);

    pConferenceReference->ReferenceNotify(pMockReference, pMockMessage);
    // delete pMockReference;
    // delete pMockMessage;
}

TEST_F(ConferenceReferenceTest, OnReferenceTerminated)
{
    MockIReference* pMockReference = new MockIReference();

    EXPECT_CALL(objMockListener,
            OnReferenceUpdated(pConferenceReference, _, ReferSubscriptionState::TERMINATED))
            .Times(1);

    pConferenceReference->ReferenceTerminated(pMockReference);
    // delete pMockReference;
}

TEST_F(ConferenceReferenceTest, SendInviteWithSingleUser)
{
    ON_CALL(objMockConfigurationManager, IsConferenceReferToUriSourcePaid)
            .WillByDefault(Return(IMS_TRUE));

    MockIReference* pMockReference = new MockIReference();
    ON_CALL(*pMockReferenceInterfaceHolder, GetIReference(_, _, _))
            .WillByDefault(Return(pMockReference));

    AString strSessionId = "12345";
    ON_CALL(*pMockJoiningSession, GetSessionId()).WillByDefault(ReturnRef(strSessionId));

    // TODO: let UriFormatter returns test uri
    AString strReferToUri = "sip:testuri@ims.google.com";
    IMS_RESULT nResult = pConferenceReference->SendInvite(strReferToUri, *pMockConnectionIdManager);
    EXPECT_EQ(nResult, IMS_SUCCESS);
    EXPECT_EQ(pConferenceReference->GetType(), REFERENCE_TYPE_INVITE);
}

}  // namespace android
