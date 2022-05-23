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

#include "app/MockAosAppContext.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosSubscriptionListener.h"
#include "../../../engine/interface/registration/MockIRegSubscription.h"

#include "IRegContact.h"
#include "IRegSubscription.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosConnection.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosSubscription.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class AosSubscriptionTest : public ::testing::Test
{
public:
    AosSubscription* pAosSubscription;

    AosStaticProfile* pAosStaticProfile;
    MockAosAppContext* pMockAosAppContext;
    MockIRegSubscription objMockIRegSubscription;
    AString* pAor;
    SipAddress* pContactAddress;

    MockIAosSubscriptionListener objMockIAosSubscriptionListener;

    // for AosNConfig info
    IAosNConfiguration* pOriginAosNConfiguration;
    MockIAosNConfiguration objMockAosConfig;

protected:
    virtual void SetUp() override
    {
        pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);

        pAor = new AString("sip:1234@ims.google.com");
        pContactAddress = new SipAddress();

        EXPECT_CALL(*pMockAosAppContext, GetSlotId()).WillRepeatedly(Return(SLOT_ID));

        pAosSubscription = new AosSubscription(static_cast<IAosAppContext*>(pMockAosAppContext),
                static_cast<IRegSubscription*>(&objMockIRegSubscription), *pAor, *pContactAddress);
        ASSERT_TRUE(pAosSubscription != nullptr);

        pAosSubscription->SetListener(&objMockIAosSubscriptionListener);
        EXPECT_CALL(objMockIAosSubscriptionListener, Subscription_StateChanged(_, _))
                .WillRepeatedly(Return());

        pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&objMockAosConfig), SLOT_ID);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(pOriginAosNConfiguration, SLOT_ID);

        if (pAor)
        {
            delete pAor;
        }

        if (pContactAddress)
        {
            delete pContactAddress;
        }

        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }

        if (pMockAosAppContext)
        {
            delete pMockAosAppContext;
        }

        if (pAosSubscription)
        {
            pAosSubscription->SetListener(IMS_NULL);
            delete pAosSubscription;
        }
    }

    void SetState(IN IMS_SINT32 nState) { pAosSubscription->m_nState = nState; }

    void SetpiRegSubscription(IN IRegSubscription* piRegSubscription)
    {
        pAosSubscription->m_piRegSubscription = piRegSubscription;
    }

    void SetRetryCountRegRequired(IN IMS_UINT32 nRetryCount)
    {
        pAosSubscription->m_nRetryCountRegRequired = nRetryCount;
    }
};

TEST_F(AosSubscriptionTest, aosSubscriptionStart)
{
    SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(pAosSubscription->Start());

    SetState(AosSubscription::STATE_OFFLINE);

    SetpiRegSubscription(IMS_NULL);
    EXPECT_FALSE(pAosSubscription->Start());

    SetState(AosSubscription::STATE_SUBSCRIBED);
    SetpiRegSubscription(static_cast<IRegSubscription*>(&objMockIRegSubscription));
    EXPECT_CALL(objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_FAILURE))
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_FALSE(pAosSubscription->Start());
    EXPECT_TRUE(pAosSubscription->Start());
}

TEST_F(AosSubscriptionTest, checkInitialRegRequired)
{
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequired(403));

    IMSVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Clear();
    objErrRegRequired.Add(404);
    objErrRegRequired.Add(403);

    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(0);
    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequired(403));

    SetRetryCountRegRequired(1);
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequired(403));

    objErrRegRequired.Clear();
    objErrRegRequired.Add(5);
    objErrRegRequired.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(1);
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequired(504));
}

TEST_F(AosSubscriptionTest, checkInitialRegWithNextPcscfRequired)
{
    IMSVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403));

    objErrRegRequiredWithNextPcscf.Add(404);
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403));

    objErrRegRequiredWithNextPcscf.Clear();
    objErrRegRequiredWithNextPcscf.Add(6);
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(603));
}

TEST_F(AosSubscriptionTest, checkInitialRegRequiredInWifi)
{
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(4)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403));

    IMSVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Clear();
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403));

    objErrRegRequiredInWifi.Add(404);
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403));

    objErrRegRequiredInWifi.Clear();
    objErrRegRequiredInWifi.Add(3);
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequiredInWifi(301));
}

TEST_F(AosSubscriptionTest, checkIsReSubscriptionStopped)
{
    IMSVector<IMS_SINT32> objErrResubStopped;
    objErrResubStopped.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));

    EXPECT_FALSE(pAosSubscription->IsResubscriptionStopped(403));

    objErrResubStopped.Add(404);
    objErrResubStopped.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));
    EXPECT_TRUE(pAosSubscription->IsResubscriptionStopped(403));

    objErrResubStopped.Clear();
    objErrResubStopped.Add(403);
    objErrResubStopped.Add(5);
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));
    EXPECT_TRUE(pAosSubscription->IsResubscriptionStopped(503));
}