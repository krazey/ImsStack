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
#include "ICarrierConfig.h"
#include "CarrierConfig.h"
#include "ImsAosParameter.h"
#include "IuMtsService.h"
#include "MockICarrierConfig.h"
#include "MtsDef.h"
#include "SipStatusCode.h"
#include "core/MockIMessage.h"
#include "message/MtsErrorHandler.h"
#include "../../include/mts/message/MockIMtsErrorHandlerListener.h"

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class MtsErrorHandlerTest : public ::testing::Test
{
public:
    MockICarrierConfig* piMockCarrierConfig;
    MtsErrorHandler* pMtsErrorHandler;

protected:
    virtual void SetUp() override
    {
        piMockCarrierConfig = new MockICarrierConfig();
        pMtsErrorHandler = new MtsErrorHandler(piMockCarrierConfig);
    }

    virtual void TearDown() override
    {
        delete piMockCarrierConfig;
        delete pMtsErrorHandler;
    }
};

TEST_F(MtsErrorHandlerTest, Constructor)
{
    ASSERT_NE(pMtsErrorHandler, nullptr);
}

TEST_F(MtsErrorHandlerTest, SetListener_Success)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);
    EXPECT_EQ(pMtsErrorHandler->GetListener(), piListener);
}

TEST_F(MtsErrorHandlerTest, Handle403Error)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_403));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_400);
    objArray.Push(SipStatusCode::SC_403);
    objArray.Push(SipStatusCode::SC_404);
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_408);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_403_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REFRESH));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::REGISTER_REFRESH)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, Handle404Error)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_404));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_400);
    objArray.Push(SipStatusCode::SC_403);
    objArray.Push(SipStatusCode::SC_404);
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_408);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_404_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::PCSCF_NEXT));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::PCSCF_NEXT)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, Handle406Error)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_406));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_400);
    objArray.Push(SipStatusCode::SC_403);
    objArray.Push(SipStatusCode::SC_404);
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_408);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_406_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::IPSEC_DISABLED));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::IPSEC_DISABLED)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, Handle408Error)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_408));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_400);
    objArray.Push(SipStatusCode::SC_403);
    objArray.Push(SipStatusCode::SC_404);
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_408);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_408_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::RETRY_COUNT_INCREASE)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, Handle500Error)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_500));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_500_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::RETRY_COUNT_INCREASE)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, Handle503Error)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_503_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::RETRY_COUNT_INCREASE)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, Handle503ErrorWithReasonHeader)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    AString strReasonPhrase = "OUTAGE";
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(*pMockMessage, GetReasonPhrase()).WillByDefault(ReturnRef(strReasonPhrase));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_503_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REINITIATE));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::REGISTER_REINITIATE)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, Handle503ErrorWithoutReasonHeader)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    AString strReasonPhrase = AString::ConstEmpty();
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(*pMockMessage, GetReasonPhrase()).WillByDefault(ReturnRef(strReasonPhrase));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_503_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REINITIATE));

    EXPECT_CALL(*piListener, NotifyControlAos(MTS_REG_RECOVERY_POLICY_NONE)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, Handle504Error)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    MockIMessage* pMockMessage = new MockIMessage();
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_504));

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_504_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::RETRY_COUNT_INCREASE)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(pMockMessage);
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

TEST_F(MtsErrorHandlerTest, HandleTimerFExpired)
{
    MockIMtsErrorHandlerListener* piListener = new MockIMtsErrorHandlerListener();
    pMtsErrorHandler->SetListener(piListener);

    IMSVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_INVALID);

    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SMS_PERMANENT_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*piMockCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_EXPIRY_TIMER_F_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE));

    EXPECT_CALL(*piListener, NotifyControlAos(ImsAosControl::RETRY_COUNT_INCREASE)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle();
    EXPECT_EQ(nResult, MO_IMS_PERM_FAILURE);
}

}  // namespace android
