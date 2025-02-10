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

#include "CarrierConfig.h"
#include "IMessageBodyPart.h"
#include "ISipHeader.h"
#include "ImsList.h"
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MockISubscription.h"
#include "MtcDef.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "conferencecall/ConferenceConst.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceSubscription.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "conferencecall/MockConferenceFactory.h"
#include "conferencecall/MockConferenceInfoUpdater.h"
#include "conferencecall/MockConferenceParticipantList.h"
#include "conferencecall/MockIConferenceSubscriptionListener.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSubscriptionInterfaceHolder.h"
#include "service/MockIFeatureCaps.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;
using ::testing::SafeMatcherCast;

namespace android
{

LOCAL CallKey CONFERENCE_CALL_KEY = 100;

class ConferenceSubscriptionTest : public ::testing::Test
{
public:
    ConferenceSubscriptionTest() :
            objContext(),
            objCallManager(),
            objMtcService(),
            objCoreService(),
            objFactory(objContext),
            pInfoUpdater(new MockConferenceInfoUpdater(objFactory, objConfigurationProxy)),
            objParticipantList(),
            objConferenceCall(),
            objConfCallContext(),
            objMtcSession(),
            objMessageUtils(),
            objListener(),
            objConfigurationProxy(),
            objInterfaceFactory(),
            objHolderListener(),
            objSubsHolder(objHolderListener),
            objNotifyMessage(),
            objISession(),
            objSubscription(),
            pConferenceSubscription(nullptr)
    {
    }

protected:
    MockIMtcContext objContext;
    MockIMtcCallManager objCallManager;
    MockIMtcService objMtcService;
    MockICoreService objCoreService;
    MockConferenceFactory objFactory;
    MockConferenceInfoUpdater* pInfoUpdater;  // Deleted internally.
    MockConferenceParticipantList objParticipantList;
    MockIMtcCall objConferenceCall;
    MockIMtcCallContext objConfCallContext;
    MockIMtcSession objMtcSession;
    MockIMessageUtils objMessageUtils;
    MockIConferenceSubscriptionListener objListener;
    MockMtcConfigurationProxy objConfigurationProxy;

    MockIMtcSipInterfaceFactory objInterfaceFactory;
    MockIInterfaceHolderListener objHolderListener;
    MockSubscriptionInterfaceHolder objSubsHolder;

    MockIMessage objNotifyMessage;
    MockISession objISession;
    MockISubscription objSubscription;

    std::unique_ptr<ConferenceSubscription> pConferenceSubscription;

    virtual void SetUp() override
    {
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetSipInterfaceFactory).WillByDefault(ReturnRef(objInterfaceFactory));
        ON_CALL(objInterfaceFactory, GetISubscriptionHolder).WillByDefault(Return(&objSubsHolder));
        ON_CALL(objFactory, CreateInfoUpdater).WillByDefault(Return(pInfoUpdater));

        // Sets Call mockings for GetISubscription.
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objCallManager, GetCallByCallKey(CONFERENCE_CALL_KEY))
                .WillByDefault(Return(&objConferenceCall));
        ON_CALL(objConferenceCall, GetCallContext).WillByDefault(ReturnRef(objConfCallContext));
        ON_CALL(objConfCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));
        ON_CALL(objSubsHolder, GetISubscription(&objISession, _))
                .WillByDefault(Return(&objSubscription));

        // Sets Service mockings for GetISubscription.
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objMtcService));
        ON_CALL(objMtcService, GetICoreService()).WillByDefault(Return(&objCoreService));
        ON_CALL(objSubsHolder, GetISubscription(&objCoreService, _, _, _))
                .WillByDefault(Return(&objSubscription));
    }

    virtual void TearDown() override {}

    void SetConferenceSubscription(IN IMS_BOOL bInDialog)
    {
        IMS_UINT32 nType = bInDialog ? ConfigVoice::CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG
                                     : ConfigVoice::CONFERENCE_SUBSCRIBE_TYPE_OUT_OF_DIALOG;
        ON_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT))
                .WillByDefault(Return(nType));
        pConferenceSubscription = std::make_unique<ConferenceSubscription>(
                objContext, CONFERENCE_CALL_KEY, objParticipantList, objListener, objFactory);
    }
};

TEST_F(ConferenceSubscriptionTest, SubscriptionNotifyDoesNothingIfSubStateIsTerminated)
{
    SetConferenceSubscription(IMS_TRUE);

    ON_CALL(objMessageUtils,
            GetHeaderValue(&objNotifyMessage, ISipHeader::SUBSCRIPTION_STATE, AString::ConstNull()))
            .WillByDefault(Return(AString("terminated")));

    EXPECT_CALL(objNotifyMessage, GetBodyParts).Times(0);

    pConferenceSubscription->SubscriptionNotify(&objSubscription, &objNotifyMessage);

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionNotifyDoesNothingIfNoMessageBody)
{
    SetConferenceSubscription(IMS_TRUE);

    ON_CALL(objMessageUtils,
            GetHeaderValue(&objNotifyMessage, ISipHeader::SUBSCRIPTION_STATE, AString::ConstNull()))
            .WillByDefault(Return(AString("active")));

    ImsList<IMessageBodyPart*> objBodyParts;
    EXPECT_CALL(objNotifyMessage, GetBodyParts).WillOnce(Return(objBodyParts));
    EXPECT_CALL(objFactory, CreateInfoUpdater).Times(0);

    pConferenceSubscription->SubscriptionNotify(&objSubscription, &objNotifyMessage);

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionNotifyDoesNothingIfBodyIsEmpty)
{
    SetConferenceSubscription(IMS_TRUE);

    ON_CALL(objMessageUtils,
            GetHeaderValue(&objNotifyMessage, ISipHeader::SUBSCRIPTION_STATE, AString::ConstNull()))
            .WillByDefault(Return(AString("active")));

    MockIMessageBodyPart objBodyPartEmpty;
    ImsList<IMessageBodyPart*> objBodyParts;
    objBodyParts.Append(&objBodyPartEmpty);
    EXPECT_CALL(objNotifyMessage, GetBodyParts).WillOnce(Return(objBodyParts));

    const ByteArray objValueEmpty(AString::ConstEmpty());
    EXPECT_CALL(objBodyPartEmpty, GetContent).WillOnce(ReturnRef(objValueEmpty));
    EXPECT_CALL(objFactory, CreateInfoUpdater).Times(0);

    pConferenceSubscription->SubscriptionNotify(&objSubscription, &objNotifyMessage);

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionNotifyInvokesUpdateAndNotifiesResult)
{
    SetConferenceSubscription(IMS_TRUE);

    ON_CALL(objMessageUtils,
            GetHeaderValue(&objNotifyMessage, ISipHeader::SUBSCRIPTION_STATE, AString::ConstNull()))
            .WillByDefault(Return(AString("active")));

    MockIMessageBodyPart objBodyPartValid;
    ImsList<IMessageBodyPart*> objBodyParts;
    objBodyParts.Append(IMS_NULL);
    objBodyParts.Append(&objBodyPartValid);
    EXPECT_CALL(objNotifyMessage, GetBodyParts).WillOnce(Return(objBodyParts));

    const ByteArray objValueValid(AString("valid information"));
    EXPECT_CALL(objBodyPartValid, GetContent).WillOnce(ReturnRef(objValueValid));
    EXPECT_CALL(*pInfoUpdater, Update(&objParticipantList, objValueValid.ToString()))
            .WillOnce(Return(ConferenceInfoUpdater::RESULT_UPDATED));
    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::NOTIFY_RECEIVED));

    pConferenceSubscription->SubscriptionNotify(&objSubscription, &objNotifyMessage);

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionNotifyInvokesUpdateAndDoesNothingIfResultIsInvalid)
{
    SetConferenceSubscription(IMS_TRUE);

    ON_CALL(objMessageUtils,
            GetHeaderValue(&objNotifyMessage, ISipHeader::SUBSCRIPTION_STATE, AString::ConstNull()))
            .WillByDefault(Return(AString("active")));

    MockIMessageBodyPart objBodyPartValid;
    ImsList<IMessageBodyPart*> objBodyParts;
    objBodyParts.Append(IMS_NULL);
    objBodyParts.Append(&objBodyPartValid);
    ON_CALL(objNotifyMessage, GetBodyParts).WillByDefault(Return(objBodyParts));

    EXPECT_CALL(objListener, OnSubscriptionUpdated(_)).Times(0);
    const ByteArray objValueValid(AString("valid information"));
    ON_CALL(objBodyPartValid, GetContent).WillByDefault(ReturnRef(objValueValid));

    EXPECT_CALL(*pInfoUpdater, Update(&objParticipantList, objValueValid.ToString()))
            .WillOnce(Return(ConferenceInfoUpdater::RESULT_MALFORMED_XML));
    pConferenceSubscription->SubscriptionNotify(&objSubscription, &objNotifyMessage);

    pInfoUpdater = new MockConferenceInfoUpdater(objFactory, objConfigurationProxy);
    ON_CALL(objFactory, CreateInfoUpdater).WillByDefault(Return(pInfoUpdater));
    EXPECT_CALL(*pInfoUpdater, Update(&objParticipantList, objValueValid.ToString()))
            .WillOnce(Return(ConferenceInfoUpdater::RESULT_INFO_DELETED));
    pConferenceSubscription->SubscriptionNotify(&objSubscription, &objNotifyMessage);

    pInfoUpdater = new MockConferenceInfoUpdater(objFactory, objConfigurationProxy);
    ON_CALL(objFactory, CreateInfoUpdater).WillByDefault(Return(pInfoUpdater));
    EXPECT_CALL(*pInfoUpdater, Update(&objParticipantList, objValueValid.ToString()))
            .WillOnce(Return(ConferenceInfoUpdater::RESULT_AMBIGUOUS));
    pConferenceSubscription->SubscriptionNotify(&objSubscription, &objNotifyMessage);

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionNotifyTriesResubscribtionIfVersionIsInvalid)
{
    SetConferenceSubscription(IMS_TRUE);

    ON_CALL(objMessageUtils,
            GetHeaderValue(&objNotifyMessage, ISipHeader::SUBSCRIPTION_STATE, AString::ConstNull()))
            .WillByDefault(Return(AString("active")));

    MockIMessageBodyPart objBodyPartValid;
    ImsList<IMessageBodyPart*> objBodyParts;
    objBodyParts.Append(IMS_NULL);
    objBodyParts.Append(&objBodyPartValid);
    EXPECT_CALL(objNotifyMessage, GetBodyParts).WillOnce(Return(objBodyParts));

    const ByteArray objValueValid(AString("valid information"));
    EXPECT_CALL(objBodyPartValid, GetContent).WillOnce(ReturnRef(objValueValid));
    EXPECT_CALL(*pInfoUpdater, Update(&objParticipantList, objValueValid.ToString()))
            .WillOnce(Return(ConferenceInfoUpdater::RESULT_INVALID_VERSION));

    // Resubscribe is failed since there is no subscription existing.
    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::FAILED));

    pConferenceSubscription->SubscriptionNotify(&objSubscription, &objNotifyMessage);

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, SubscribeInvokesSubscribeInDialogAndSetsStateSubscribing)
{
    SetConferenceSubscription(IMS_TRUE);

    // To skip SetHeaders() without exceptions.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());

    EXPECT_CALL(objSubscription, SetListener(pConferenceSubscription.get()));
    EXPECT_CALL(objSubscription, Subscribe).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objSubsHolder, GetISubscription(&objISession, _))
            .WillOnce(Return(&objSubscription));

    pConferenceSubscription->Subscribe(AString::ConstEmpty());

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);
}

TEST_F(ConferenceSubscriptionTest, SubscribeInvokesSubscribeOutdialogAndSetsStateSubscribing)
{
    SetConferenceSubscription(IMS_FALSE);

    // To skip SetHeaders() without exceptions.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());

    EXPECT_CALL(objSubscription, SetListener(pConferenceSubscription.get()));
    EXPECT_CALL(objSubscription, Subscribe).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objSubsHolder, GetISubscription(&objCoreService, _, _, _))
            .WillOnce(Return(&objSubscription));

    pConferenceSubscription->Subscribe(AString::ConstEmpty());

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);
}

TEST_F(ConferenceSubscriptionTest, SubscribeDoesNotChangeStateToSubscribingIfSubscriptionFails)
{
    SetConferenceSubscription(IMS_TRUE);

    // To skip SetHeaders() without exceptions.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());

    EXPECT_CALL(objSubscription, SetListener(pConferenceSubscription.get()));
    EXPECT_CALL(objSubscription, Subscribe).WillOnce(Return(IMS_FAILURE));

    pConferenceSubscription->Subscribe(AString::ConstEmpty());

    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, UnsubscribeDoesNothingIfNoSubscription)
{
    SetConferenceSubscription(IMS_FALSE);

    EXPECT_CALL(objSubscription, Unsubscribe()).Times(0);
    pConferenceSubscription->UnSubscribe();
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, UnsubscribeDoesNothingAfterUnsubscribe)
{
    SetConferenceSubscription(IMS_TRUE);

    // Sets state to Idle after Unsubscribe.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    pConferenceSubscription->UnSubscribe();
    pConferenceSubscription->SubscriptionStarted(&objSubscription);

    // Tests.
    EXPECT_CALL(objSubscription, Unsubscribe()).Times(0);
    pConferenceSubscription->UnSubscribe();
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, UnsubscribeInvokesUnsubscribeAndSetsStateUnsubscribing)
{
    SetConferenceSubscription(IMS_FALSE);

    // Sets state to Subscribing and m_pSubscription not null.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());

    // Tests.
    EXPECT_CALL(objSubscription, Unsubscribe());
    pConferenceSubscription->UnSubscribe();
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::UNSUBSCRIBING);
}

TEST_F(ConferenceSubscriptionTest,
        SubscriptionStartedSetsStateActiveAndNotifiesSuccededIfInSubscribingState)
{
    SetConferenceSubscription(IMS_TRUE);

    // Sets state to Subscribing.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);

    // Tests.
    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::SUCCEEDED));
    pConferenceSubscription->SubscriptionStarted(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::ACTIVE);
}

TEST_F(ConferenceSubscriptionTest,
        SubscriptionStartedSetsStateIdleAndNotifiesUnsubscribedIfInUnsubscribingState)
{
    SetConferenceSubscription(IMS_TRUE);

    // Sets state to Unsubscribing.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    pConferenceSubscription->UnSubscribe();
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::UNSUBSCRIBING);

    // Tests.
    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::UNSUBSCRIBED));
    pConferenceSubscription->SubscriptionStarted(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionStartFailedDoesNothingIfMessageInNull)
{
    SetConferenceSubscription(IMS_TRUE);

    ON_CALL(objSubscription, GetPreviousResponse).WillByDefault(ReturnNull());

    EXPECT_CALL(objListener, OnSubscriptionUpdated(_)).Times(0);

    pConferenceSubscription->SubscriptionStartFailed(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionStartFailedNotifiesUnSubscribed)
{
    SetConferenceSubscription(IMS_TRUE);

    // Sets state to Unsubscribing.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    pConferenceSubscription->UnSubscribe();
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::UNSUBSCRIBING);

    // Tests.
    MockIMessage objMessage;
    ON_CALL(objSubscription, GetPreviousResponse).WillByDefault(Return(&objMessage));

    EXPECT_CALL(objSubsHolder, ReleaseISubscription(&objSubscription, IMS_FALSE));
    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::UNSUBSCRIBED));

    pConferenceSubscription->SubscriptionStartFailed(&objSubscription);
    EXPECT_CALL(objSubsHolder, ReleaseISubscription(IMS_NULL, IMS_FALSE));  // By Destructor.
}

TEST_F(ConferenceSubscriptionTest, SubscriptionStartFailedNotifiesFailed)
{
    SetConferenceSubscription(IMS_TRUE);

    // Sets state to Subscribing.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);

    // Tests.
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objSubscription, GetPreviousResponse).WillByDefault(Return(&objMessage));

    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::FAILED));
    EXPECT_CALL(objSubsHolder, ReleaseISubscription(&objSubscription, IMS_FALSE));

    pConferenceSubscription->SubscriptionStartFailed(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);
    EXPECT_CALL(objSubsHolder, ReleaseISubscription(IMS_NULL, IMS_FALSE));  // By Destructor.
}

TEST_F(ConferenceSubscriptionTest, SubscriptionStartFailedBy403NotifiesFailedIfInDialogMode)
{
    SetConferenceSubscription(IMS_TRUE);

    // Sets state to Subscribing.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);

    // Tests.
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_403));
    ON_CALL(objSubscription, GetPreviousResponse).WillByDefault(Return(&objMessage));

    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::FAILED));
    EXPECT_CALL(objSubsHolder, ReleaseISubscription(&objSubscription, IMS_FALSE));

    pConferenceSubscription->SubscriptionStartFailed(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);
    EXPECT_CALL(objSubsHolder, ReleaseISubscription(IMS_NULL, IMS_FALSE));  // By Destructor.
}

TEST_F(ConferenceSubscriptionTest,
        SubscriptionStartFailedBy403TriggersNewSubscriptionIfOutDialogMode)
{
    SetConferenceSubscription(IMS_FALSE);

    // Sets state to Subscribing.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);

    // Tests.
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_403));
    ON_CALL(objSubscription, GetPreviousResponse).WillByDefault(Return(&objMessage));

    MockISubscription objNewSubscription;
    // Dialog type is changed to In-Dialog after receiving 403 so objISession is used.
    EXPECT_CALL(objSubsHolder, GetISubscription(&objISession, _))
            .WillOnce(Return(&objNewSubscription));
    EXPECT_CALL(objNewSubscription, Subscribe).WillOnce(Return(IMS_SUCCESS));

    pConferenceSubscription->SubscriptionStartFailed(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionStartFailedBy403AndStartedPutsConfigurationToCache)
{
    SetConferenceSubscription(IMS_FALSE);

    // Sets state to Subscribing.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);

    // Tests.
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_403));
    ON_CALL(objSubscription, GetPreviousResponse).WillByDefault(Return(&objMessage));

    MockISubscription objNewSubscription;
    // Dialog type is changed to In-Dialog after receiving 403 so objISession is used.
    EXPECT_CALL(objSubsHolder, GetISubscription(&objISession, _))
            .WillOnce(Return(&objNewSubscription));
    EXPECT_CALL(objNewSubscription, Subscribe).WillOnce(Return(IMS_SUCCESS));

    pConferenceSubscription->SubscriptionStartFailed(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);

    EXPECT_CALL(objConfigurationProxy,
            PutCache(ConfigVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT,
                    SafeMatcherCast<IMS_SINT32>(ConfigVoice::CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG)));
    pConferenceSubscription->SubscriptionStarted(&objSubscription);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionStartFailedBy423NotifiesFailedIfNoMinExpires)
{
    SetConferenceSubscription(IMS_TRUE);

    // Sets state to Subscribing.
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(ReturnNull());
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);

    // Tests.
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_423));
    ON_CALL(objSubscription, GetPreviousResponse).WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMessageUtils,
            GetHeaderValueInt(&objMessage, ISipHeader::MIN_EXPIRES, AString::ConstNull()))
            .WillOnce(Return(-1));

    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::FAILED));
    EXPECT_CALL(objSubsHolder, ReleaseISubscription(&objSubscription, IMS_FALSE));

    pConferenceSubscription->SubscriptionStartFailed(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);
    EXPECT_CALL(objSubsHolder, ReleaseISubscription(IMS_NULL, IMS_FALSE));  // By Destructor.
}

TEST_F(ConferenceSubscriptionTest, SubscriptionStartFailedBy423TriggersNewSubscriptionWithExpires)
{
    SetConferenceSubscription(IMS_FALSE);

    // Sets state to Subscribing.
    MockIMessage objResponse;
    ON_CALL(objSubscription, GetNextRequest).WillByDefault(Return(&objResponse));
    ON_CALL(objResponse, GetMessage).WillByDefault(ReturnNull());  // To increase coverage.
    ON_CALL(objSubscription, Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pConferenceSubscription->Subscribe(AString::ConstEmpty());
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);

    // Tests.
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_423));
    ON_CALL(objSubscription, GetPreviousResponse).WillByDefault(Return(&objMessage));

    const IMS_SINT32 nMinExpires = 100;
    const IMS_SINT32 nExpectedExpires = 150;  // 100 * 1.5
    EXPECT_CALL(objMessageUtils,
            GetHeaderValueInt(&objMessage, ISipHeader::MIN_EXPIRES, AString::ConstNull()))
            .WillOnce(Return(nMinExpires));

    MockISubscription objNewSubscription;
    EXPECT_CALL(objSubsHolder, GetISubscription(&objCoreService, _, _, _))
            .WillOnce(Return(&objNewSubscription));
    EXPECT_CALL(objNewSubscription, Subscribe).WillOnce(Return(IMS_SUCCESS));

    // Sets Headers
    MockIMessage objRequest;
    ON_CALL(objNewSubscription, GetNextRequest).WillByDefault(Return(&objRequest));
    MockISipMessage objSipRequest;
    ON_CALL(objRequest, GetMessage).WillByDefault(Return(&objSipRequest));
    const AString strValue(ConferenceConst::APPLICATION_CONFERENCEINFO);
    EXPECT_CALL(objSipRequest, AddHeader(ISipHeader::ACCEPT, strValue, _));
    MockIFeatureCaps objFeatureCaps;
    EXPECT_CALL(objCoreService, GetFeatureCaps).WillOnce(Return(&objFeatureCaps));
    EXPECT_CALL(objFeatureCaps,
            AddFeature(AString("+g.3gpp.mid-call"), AString::ConstEmpty(), SipMethod::SUBSCRIBE,
                    ISipMessage::TYPE_REQUEST));
    AString strExpires;
    strExpires.SetNumber(nExpectedExpires);
    EXPECT_CALL(
            objSipRequest, SetHeader(ISipHeader::EXPIRES_SEC, strExpires, AString::ConstNull()));

    pConferenceSubscription->SubscriptionStartFailed(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::SUBSCRIBING);
}

TEST_F(ConferenceSubscriptionTest, SubscriptionTerminatedNotifiesTerminated)
{
    SetConferenceSubscription(IMS_TRUE);

    EXPECT_CALL(objListener, OnSubscriptionUpdated(SubscriptionUpdateType::TERMINATED));

    pConferenceSubscription->SubscriptionTerminated(&objSubscription);
    EXPECT_EQ(pConferenceSubscription->GetState(), SubscriptionState::IDLE);
}

}  // namespace android
