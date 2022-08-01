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

#include "CoreService.h"
#include "Session.h"

namespace android
{

class SessionTest : public ::testing::Test
{
public:
    inline SessionTest() :
            m_pCoreService(new CoreService("test.app", "test.service")),
            m_pSession(IMS_NULL)
    {
    }
    inline virtual ~SessionTest()
    {
        if (m_pCoreService != IMS_NULL)
        {
            delete m_pCoreService;
        }

        if (m_pSession != IMS_NULL)
        {
            delete m_pSession;
        }
    }

protected:
    virtual void SetUp() override { m_pSession = new Session(m_pCoreService); }

    virtual void TearDown() override
    {
        if (m_pSession != IMS_NULL)
        {
            delete m_pSession;
            m_pSession = IMS_NULL;
        }
    }

protected:
    CoreService* m_pCoreService;
    Session* m_pSession;
};

TEST_F(SessionTest, SetConfiguration)
{
    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE, m_pSession->GetConfiguration());

    m_pSession->SetConfiguration(m_pSession->GetConfiguration() |
            Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR |
            Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED);

    EXPECT_TRUE(m_pSession->IsConfigurationSet(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE));
    EXPECT_TRUE(m_pSession->IsConfigurationSet(Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR));
    EXPECT_TRUE(
            m_pSession->IsConfigurationSet(Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED));
    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE |
                    Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR |
                    Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED,
            m_pSession->GetConfiguration());

    m_pSession->SetConfiguration(m_pSession->GetConfiguration() &
            (~Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR) &
            (~Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED));

    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE, m_pSession->GetConfiguration());
}

}  // namespace android
