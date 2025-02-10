/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "AString.h"
#include "ByteArray.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "ISipHeader.h"
#include "ISubscription.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockISipMessage.h"
#include "MockISubscription.h"
#include "SipStatusCode.h"
#include "dialogevent/DialogSubscription.h"
#include "dialogevent/MockIDialogSubscription.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSubscriptionInterfaceHolder.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const AString strTarget("sip:anyaddress");

namespace android
{

class DialogSubscriptionTest : public ::testing::Test
{
public:
    DialogSubscriptionTest() :
            objContext(),
            objService(),
            objCoreService(),
            objISubscription(),
            objSipFactory(),
            objHolderListener(),
            objSubscriptionHolder(objHolderListener),
            objSubscription(objContext, objListener, strTarget)
    {
    }

protected:
    MockIMtcContext objContext;
    MockIMtcService objService;
    MockICoreService objCoreService;
    MockISubscription objISubscription;
    MockIMtcSipInterfaceFactory objSipFactory;
    MockIInterfaceHolderListener objHolderListener;
    MockSubscriptionInterfaceHolder objSubscriptionHolder;
    MockIDialogSubscriptionListener objListener;

    DialogSubscription objSubscription;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));
        ON_CALL(objContext, GetSipInterfaceFactory).WillByDefault(ReturnRef(objSipFactory));
        ON_CALL(objSipFactory, GetISubscriptionHolder)
                .WillByDefault(Return(&objSubscriptionHolder));

        ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    }

    virtual void TearDown() override {}
};

TEST_F(DialogSubscriptionTest, DestructorReleasesInterface)
{
    EXPECT_CALL(objSubscriptionHolder, ReleaseISubscription(_, _));
}

TEST_F(DialogSubscriptionTest, SubscribeDoesNotInvokeSubscribeIfISubscriptionIsNull)
{
    ON_CALL(objSubscriptionHolder, GetISubscription(_, _, _, _)).WillByDefault(Return(nullptr));
    EXPECT_CALL(objISubscription, Subscribe).Times(0);  // meaningless
    objSubscription.Subscribe();
}

TEST_F(DialogSubscriptionTest, SubscribeInvokesSubscribeEvenIfIMessageIsNull)
{
    ON_CALL(objSubscriptionHolder, GetISubscription(_, _, _, _))
            .WillByDefault(Return(&objISubscription));
    EXPECT_CALL(objISubscription, SetListener(&objSubscription));
    EXPECT_CALL(objISubscription, Subscribe);
    MockIMessage objMessage;
    ON_CALL(objISubscription, GetNextRequest).WillByDefault(Return(nullptr));
    objSubscription.Subscribe();
}

TEST_F(DialogSubscriptionTest, SubscribeInvokesSubscribeEvenIfISipMessageIsNull)
{
    ON_CALL(objSubscriptionHolder, GetISubscription(_, _, _, _))
            .WillByDefault(Return(&objISubscription));
    EXPECT_CALL(objISubscription, SetListener(&objSubscription));
    EXPECT_CALL(objISubscription, Subscribe);
    MockIMessage objMessage;
    ON_CALL(objISubscription, GetNextRequest).WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(nullptr));
    objSubscription.Subscribe();
}

TEST_F(DialogSubscriptionTest, SubscribeInvokesSubscribe)
{
    ON_CALL(objSubscriptionHolder, GetISubscription(_, _, _, _))
            .WillByDefault(Return(&objISubscription));
    EXPECT_CALL(objISubscription, SetListener(&objSubscription));
    EXPECT_CALL(objISubscription, Subscribe);
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    ON_CALL(objISubscription, GetNextRequest).WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    objSubscription.Subscribe();
}

TEST_F(DialogSubscriptionTest, SubscribeFailsIfAlreadySubscribed)
{
    ON_CALL(objSubscriptionHolder, GetISubscription(_, _, _, _))
            .WillByDefault(Return(&objISubscription));
    objSubscription.Subscribe();

    EXPECT_EQ(objSubscription.Subscribe(), IMS_FAILURE);
}

TEST_F(DialogSubscriptionTest, UnsubscribeInvokesUnsubscribeIfSubscribed)
{
    // make be subscribed so that m_piSubscription can be created.
    ON_CALL(objSubscriptionHolder, GetISubscription(_, _, _, _))
            .WillByDefault(Return(&objISubscription));
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    ON_CALL(objISubscription, GetNextRequest).WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    objSubscription.Subscribe();

    ON_CALL(objISubscription, GetState).WillByDefault(Return(ISubscription::STATE_ACTIVE));
    EXPECT_CALL(objISubscription, Unsubscribe);
    objSubscription.Unsubscribe();
}

TEST_F(DialogSubscriptionTest, UnsubscribeDoesNotInvokeUnsubscribeIfNotSubscribed)
{
    ON_CALL(objISubscription, GetState).WillByDefault(Return(ISubscription::STATE_INACTIVE));
    EXPECT_CALL(objISubscription, Unsubscribe).Times(0);
    objSubscription.Unsubscribe();
}

TEST_F(DialogSubscriptionTest, SubscriptionForkedNotifyDoesNothing)
{
    objSubscription.SubscriptionForkedNotify(&objISubscription, &objISubscription);
}

TEST_F(DialogSubscriptionTest, SubscriptionNotifyDoesNotNotifyToListenerIfBodyIsEmpty)
{
    ImsList<IMessageBodyPart*> objBodyParts;
    MockIMessage objMessage;
    ON_CALL(objMessage, GetBodyParts).WillByDefault(Return(objBodyParts));
    EXPECT_CALL(objListener, OnSubscriptionNotified).Times(0);
    objSubscription.SubscriptionNotify(&objISubscription, &objMessage);
}

TEST_F(DialogSubscriptionTest, SubscriptionNotifyNotifiesToListener)
{
    ImsList<IMessageBodyPart*> objBodyParts;
    objBodyParts.Append(IMS_NULL);
    MockIMessage objMessage;
    ON_CALL(objMessage, GetBodyParts).WillByDefault(Return(objBodyParts));
    EXPECT_CALL(objListener, OnSubscriptionNotified);
    objSubscription.SubscriptionNotify(&objISubscription, &objMessage);
}

TEST_F(DialogSubscriptionTest, SubscriptionNotifyNotifiesToListenerWithValidInformation)
{
    MockIMessageBodyPart objBodyPart;
    ByteArray objContent("anyContent");
    ON_CALL(objBodyPart, GetContent).WillByDefault(ReturnRef(objContent));
    ImsList<IMessageBodyPart*> objBodyParts;
    objBodyParts.Append(&objBodyPart);
    MockIMessage objMessage;
    ON_CALL(objMessage, GetBodyParts).WillByDefault(Return(objBodyParts));
    EXPECT_CALL(objListener, OnSubscriptionNotified(objContent.ToString()));
    objSubscription.SubscriptionNotify(&objISubscription, &objMessage);
}

TEST_F(DialogSubscriptionTest, SubscriptionStartedNotifiesToListener)
{
    EXPECT_CALL(objListener, OnSubscriptionStarted);
    objSubscription.SubscriptionStarted(&objISubscription);
}

TEST_F(DialogSubscriptionTest, SubscriptionStartFailedNotifiesToListener)
{
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_403));
    ON_CALL(objISubscription, GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objListener, OnSubscriptionStartFailed);
    objSubscription.SubscriptionStartFailed(&objISubscription);
}

TEST_F(DialogSubscriptionTest, SubscriptionStartFailedDoesNotRetryIfCodeIs423ButMinSeIsInvalid)
{
    MockIMessage objResponseMessage;
    MockIMessageUtils objUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objUtils));
    ON_CALL(objUtils, GetHeaderValueInt(&objResponseMessage, ISipHeader::MIN_EXPIRES, _))
            .WillByDefault(Return(100));
    ON_CALL(objResponseMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_423));
    ON_CALL(objISubscription, GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE))
            .WillByDefault(Return(&objResponseMessage));
    EXPECT_CALL(objListener, OnSubscriptionStartFailed);
    objSubscription.SubscriptionStartFailed(&objISubscription);
}

TEST_F(DialogSubscriptionTest, SubscriptionStartFailedRetriesInternallyIfCodeIs423)
{
    MockIMessage objResponseMessage;
    MockIMessageUtils objUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objUtils));
    ON_CALL(objUtils, GetHeaderValueInt(&objResponseMessage, ISipHeader::MIN_EXPIRES, _))
            .WillByDefault(Return(10000));
    ON_CALL(objResponseMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_423));
    ON_CALL(objISubscription, GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE))
            .WillByDefault(Return(&objResponseMessage));
    EXPECT_CALL(objListener, OnSubscriptionStartFailed).Times(0);

    MockISubscription obj2ndISubscription;
    ON_CALL(objSubscriptionHolder, GetISubscription(_, _, _, _))
            .WillByDefault(Return(&obj2ndISubscription));
    MockISipMessage objSipMessage;
    MockIMessage objNextMessage;
    ON_CALL(obj2ndISubscription, GetNextRequest).WillByDefault(Return(&objNextMessage));
    ON_CALL(objNextMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    EXPECT_CALL(obj2ndISubscription, SetListener(&objSubscription));
    EXPECT_CALL(obj2ndISubscription, Subscribe);

    objSubscription.SubscriptionStartFailed(&objISubscription);
}

TEST_F(DialogSubscriptionTest, SubscriptionTerminatedNotifiesToListener)
{
    EXPECT_CALL(objListener, OnSubscriptionTerminated);
    objSubscription.SubscriptionTerminated(&objISubscription);
}

}  // namespace android
