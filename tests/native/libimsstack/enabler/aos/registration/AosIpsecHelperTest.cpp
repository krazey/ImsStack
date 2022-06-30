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
#include "../../../engine/interface/registration/MockIRegContact.h"
#include "../../../engine/interface/registration/MockIRegParameter.h"

#include "registration/AosIpsecHelper.h"

using ::testing::Return;
using ::testing::ReturnRef;

// const IMS_SINT32 SLOT_ID = 0;

class AosIpsecHelperTest : public ::testing::Test
{
public:
    AosIpsecHelper* pAosIpsecHelper;

    MockIRegContact objMockIRegContact;
    MockIRegParameter objMockIRegParameter;
    AosStaticProfile* pAosStaticProfile;
    MockAosAppContext* pMockAosAppContext;
    AString* pRegId;

    const AString strRegId = "aos_normal_reg";

protected:
    virtual void SetUp() override
    {
        pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);

        pRegId = new AString(strRegId);

        pAosIpsecHelper = new AosIpsecHelper(static_cast<IRegContact*>(&objMockIRegContact),
                static_cast<IRegParameter*>(&objMockIRegParameter),
                static_cast<IAosAppContext*>(pMockAosAppContext), *pRegId);
        ASSERT_TRUE(pAosIpsecHelper != nullptr);
        pAosIpsecHelper->InitIpsec();
    }

    virtual void TearDown() override
    {
        if (pRegId)
        {
            delete pRegId;
        }

        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }

        if (pMockAosAppContext)
        {
            delete pMockAosAppContext;
        }

        if (pAosIpsecHelper)
        {
            delete pAosIpsecHelper;
        }
    }
};

TEST_F(AosIpsecHelperTest, SetPcscfPortnSpi)
{
    IMSList<SipSecurityHeader> objSecuServerH;
    objSecuServerH.Clear();

    EXPECT_CALL(objMockIRegParameter, GetSecurityServers()).WillOnce(ReturnRef(objSecuServerH));
    EXPECT_FALSE(pAosIpsecHelper->SetPcscfPortnSpi());
}
