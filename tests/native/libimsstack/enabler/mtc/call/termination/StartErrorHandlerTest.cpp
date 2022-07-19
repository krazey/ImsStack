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
#include "call/termination/StartErrorHandler.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "CallReasonInfo.h"

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class StartErrorHandlerTest : public ::testing::Test
{
public:
    MockIMtcCallContext objCallContext;
    MockIMessage objMessage;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));
    }

    virtual void TearDown() override { delete pConfigurationProxy; }
};

TEST_F(StartErrorHandlerTest, HandleReturnsCsfbReasonIf701IsIncludedInCsfbConfiguration)
{
    const IMS_SINT32 REJECT_CODE = 701;

    ON_CALL(*pConfigurationManager, IsRejectCodeForCsfb(REJECT_CODE))
            .WillByDefault(Return(IMS_TRUE));

    CallInfo objCallInfo;  // bEmergency is false.
    ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(REJECT_CODE));

    StartErrorHandler objHandler(objCallContext);
    CallReasonInfo objResult = objHandler.Handle(&objMessage);

    EXPECT_EQ(
            CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL),
            objResult);
}

}  // namespace android
