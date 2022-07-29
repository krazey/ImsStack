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
#include "MockIMtsService.h"
#include "core/IPageMessage.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"

using ::testing::_;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_SINT32 SEQ_ID = 1;

class MtsMessageControllerTest : public ::testing::Test
{
public:
    MtsMessageController* pMtsMessageController;
    MtsDynamicLoader* pMtsDynamicLoader;
    MockIMtsService* pMtsService;

protected:
    virtual void SetUp() override
    {
        pMtsDynamicLoader = new MtsDynamicLoader(SLOT_ID);
        pMtsDynamicLoader->Initialize();
        pMtsService = new MockIMtsService();
        pMtsMessageController =
                new MtsMessageController(SLOT_ID, pMtsService, pMtsDynamicLoader);
    }

    virtual void TearDown() override
    {
        delete pMtsDynamicLoader;
        delete pMtsService;
        delete pMtsMessageController;
    }
};

TEST_F(MtsMessageControllerTest, Constructor)
{
    ASSERT_NE(pMtsMessageController, nullptr);
}

TEST_F(MtsMessageControllerTest, ReportTransmissionResult)
{
    EXPECT_CALL(*pMtsService,
            ReportMoStatus(MO_IMS_TEMP_FAILURE, SmsFormatType::SMSFORMAT_3GPP, _, SEQ_ID))
            .Times(1);
    pMtsMessageController->ReportTransmissionResult(
            SipStatusCode::SC_404, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID);
}

}  // namespace android
