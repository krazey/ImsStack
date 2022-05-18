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
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosSubscriptionListener.h"
#include "../../../engine/interface/registration/MockIRegSubscription.h"

#include "IRegContact.h"
#include "IRegSubscription.h"
#include "interface/IAosAppContext.h"
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
    }

    virtual void TearDown() override
    {
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

    void SetRetryCountRegRequired(IN IMS_UINT32 nRetryCount)
    {
        pAosSubscription->m_nRetryCountRegRequired = nRetryCount;
    }
};

TEST_F(AosSubscriptionTest, checkErrorResponse)
{
    IAosNConfiguration* pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    MockIAosNConfiguration objMockAosConfig;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockAosConfig), SLOT_ID);

    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequired(403));

    IMSVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Clear();
    objErrRegRequired.Add(403);
    objErrRegRequired.Add(404);

    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(0);
    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequired(403));

    SetRetryCountRegRequired(1);
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequired(403));

    AosProvider::GetInstance()->SetNConfiguration(pOriginAosNConfiguration, SLOT_ID);
}