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
#include "IMtcCallStateListener.h"
#include "MtcConnector.h"
#include "MtcContextRepository.h"
#include "MockIMtcContext.h"
#include "helper/MockICallStateProxy.h"

using ::testing::ReturnRef;

LOCAL IMS_SINT32 SLOT_ID = 0;

namespace android
{

class MtcConnectorTest : public ::testing::Test
{
public:
    MockIMtcContext objContext;
    MockICallStateProxy objCallStateProxy;

protected:
    virtual void SetUp() override
    {
        MtcContextRepository::GetInstance()->AddContext(SLOT_ID, &objContext);
        ON_CALL(objContext, GetCallStateProxy)
                .WillByDefault(ReturnRef(objCallStateProxy));
    }

    virtual void TearDown() override {}
};

TEST_F(MtcConnectorTest, AddCallStateListener)
{
    IMtcCallStateListener* pListener = reinterpret_cast<IMtcCallStateListener*>(0x01);
    EXPECT_CALL(objCallStateProxy, AddListener(pListener))
            .Times(1);
    MtcConnector::AddCallStateListener(SLOT_ID, pListener);
}

TEST_F(MtcConnectorTest, RemoveCallStateListener)
{
    IMtcCallStateListener* pListener = reinterpret_cast<IMtcCallStateListener*>(0x01);
    EXPECT_CALL(objCallStateProxy, RemoveListener(pListener))
            .Times(1);
    MtcConnector::RemoveCallStateListener(SLOT_ID, pListener);
}

}  // namespace android
