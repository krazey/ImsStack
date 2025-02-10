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

#include "MockIMtcContext.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceInfo.h"
#include "conferencecall/ConferenceInfoUpdater.h"
#include "conferencecall/ConferenceSubscription.h"
#include "conferencecall/IConferenceReference.h"
#include "conferencecall/MockConferenceParticipantList.h"
#include "conferencecall/MockIConferenceReferenceListener.h"
#include "conferencecall/MockIConferenceSubscriptionListener.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockReferenceInterfaceHolder.h"
#include "helper/sipinterfaceholder/MockSubscriptionInterfaceHolder.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

class ConferenceFactoryTest : public ::testing::Test
{
public:
    ConferenceFactory* pFactory;
    MockIMtcContext objContext;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockReferenceInterfaceHolder* pReferenceInterfaceHolder;
    MockSubscriptionInterfaceHolder* pSubscriptionInterfaceHolder;
    MockIInterfaceHolderListener objInterfaceHolderListener;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        pReferenceInterfaceHolder = new MockReferenceInterfaceHolder(objInterfaceHolderListener);
        pSubscriptionInterfaceHolder =
                new MockSubscriptionInterfaceHolder(objInterfaceHolderListener);
        ON_CALL(objSipInterfaceFactory, GetIReferenceHolder)
                .WillByDefault(Return(pReferenceInterfaceHolder));
        ON_CALL(objSipInterfaceFactory, GetISubscriptionHolder)
                .WillByDefault(Return(pSubscriptionInterfaceHolder));
        ON_CALL(objContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objSipInterfaceFactory));

        pFactory = new ConferenceFactory(objContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pReferenceInterfaceHolder;
        delete pSubscriptionInterfaceHolder;
        delete pFactory;
    }
};

TEST_F(ConferenceFactoryTest, CreateSubscriptionReturnsSubscription)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_SUBSCRIBE_TYPE_OUT_OF_DIALOG));

    MockConferenceParticipantList objList;
    MockIConferenceSubscriptionListener objListener;
    ConferenceSubscription* pSubscription = pFactory->CreateSubscription(1, objList, objListener);
    EXPECT_NE(nullptr, pSubscription);
    delete pSubscription;
}

TEST_F(ConferenceFactoryTest, CreateReferenceWithUserReturnsReference)
{
    ConfUser objUser;
    MockIConferenceReferenceListener objListener;
    IConferenceReference* pReference = pFactory->CreateReference(1, &objUser, objListener);
    EXPECT_NE(nullptr, pReference);
    delete pReference;
}

TEST_F(ConferenceFactoryTest, CreateReferenceWithUserListReturnsReference)
{
    ImsList<ConfUser*> objUsers;
    MockIConferenceReferenceListener objListener;
    IConferenceReference* pReference = pFactory->CreateReference(1, objUsers, objListener);
    EXPECT_NE(nullptr, pReference);
    delete pReference;
}

TEST_F(ConferenceFactoryTest, CreateInfoUpdaterReturnsInfoUpdater)
{
    ConferenceInfoUpdater* pUpdater = pFactory->CreateInfoUpdater();
    EXPECT_NE(nullptr, pUpdater);
    delete pUpdater;
}

TEST_F(ConferenceFactoryTest, CreateInfoReturnsInfo)
{
    ConferenceInfo* pInfo = pFactory->CreateInfo();
    EXPECT_NE(nullptr, pInfo);
    delete pInfo;
}
