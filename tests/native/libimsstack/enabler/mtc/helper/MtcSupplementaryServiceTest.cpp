/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" B ASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "IMessage.h"
#include "ISipHeader.h"
#include "MtcDef.h"
#include "SipHeaderName.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "../../../engine/interface/core/MockIMessage.h"
#include "../../../engine/interface/sipcore/MockISipMessage.h"

using ::testing::AnyNumber;
using ::testing::Return;

namespace android
{

class MtcSupplementaryServiceTest : public ::testing::Test
{
public:
    MockIMtcConfigurationManager* pMockIMtcConfigurationManager;
    MtcConfigurationProxy* pMtcConfigurationProxy;
    MtcSupplementaryService* pMtcSupplementaryService;
    MockISipMessage objMockISipMessage;
    MockIMessage objMockIMessage;

protected:
    virtual void SetUp() override
    {
        pMockIMtcConfigurationManager = new MockIMtcConfigurationManager();
        pMtcConfigurationProxy = new MtcConfigurationProxy(
                static_cast<IMtcConfigurationManager*>(pMockIMtcConfigurationManager));
        pMtcSupplementaryService = new MtcSupplementaryService(*pMtcConfigurationProxy);

        EXPECT_CALL(objMockIMessage, GetMessage())
                .Times(AnyNumber())
                .WillRepeatedly(Return(static_cast<ISipMessage*>(&objMockISipMessage)));
    }

    virtual void TearDown() override
    {
        if (pMtcConfigurationProxy)
        {
            // pMockIMtcConfigurationManager will be deleted by below.
            delete pMtcConfigurationProxy;
        }

        if (pMtcSupplementaryService)
        {
            delete pMtcSupplementaryService;
        }
    }
};

TEST_F(MtcSupplementaryServiceTest, UpdateOutgoingServices)
{
    SuppService* pTestSupp1 = new SuppService();
    AString strTest("testDisplay");
    pTestSupp1->strValue = strTest;
    SuppService* pTestSupp2 = new SuppService();
    pTestSupp2->nValue = 1;
    SuppService* pTestSupp3 = new SuppService();
    pTestSupp3->bValue = IMS_TRUE;

    IMSMap<SuppType, SuppService*> objInSuppService;
    objInSuppService.Add(SuppType::CNAP, pTestSupp1);
    objInSuppService.Add(SuppType::CALLER_ID, pTestSupp2);
    objInSuppService.Add(SuppType::CW, pTestSupp3);

    pMtcSupplementaryService->UpdateOutgoingServices(objInSuppService);

    IMSMap<SuppType, SuppService*> objOutSuppService = pMtcSupplementaryService->GetServices();

    EXPECT_EQ(objOutSuppService.GetSize(), 3);
    EXPECT_EQ(objOutSuppService.GetValue(SuppType::CNAP)->strValue, strTest);
    EXPECT_EQ(objOutSuppService.GetValue(SuppType::CALLER_ID)->nValue, 1);
    EXPECT_TRUE(objOutSuppService.GetValue(SuppType::CW)->bValue);
}

TEST_F(MtcSupplementaryServiceTest, UpdateTip)
{
    IMSList<AString> objPrivacyHeadersHaveId;
    objPrivacyHeadersHaveId.Append(AString("id"));
    IMSList<AString> objPrivacyHeadersEmpty;

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::PRIVACY, AString::ConstNull()))
            .Times(AnyNumber())
            .WillOnce(Return(objPrivacyHeadersHaveId))
            .WillOnce(Return(objPrivacyHeadersEmpty))
            .WillRepeatedly(Return(objPrivacyHeadersHaveId));

    EXPECT_CALL(objMockISipMessage,
            IsHeaderPresent(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    IMSList<AString> objPaidHeaders;
    objPaidHeaders.Append(AString("\"testDisplay\" <sip:01030993879@fakeims.google.com>"));
    EXPECT_CALL(
            objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .Times(AnyNumber())
            .WillRepeatedly(Return(objPaidHeaders));

    pMtcSupplementaryService->UpdateTip(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::TIP)->nValue, TIP_TYPE_RESTRICTED);

    pMtcSupplementaryService->UpdateTip(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::TIP)->nValue, TIP_TYPE_NONE);

    pMtcSupplementaryService->UpdateTip(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::TIP)->nValue, TIP_TYPE_IDENTITY);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::TIP)->strValue,
            AString("01030993879,testDisplay"));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallerId)
{
    IMSList<AString> objPrivacyHeadersHaveId;
    objPrivacyHeadersHaveId.Append(AString("id"));
    IMSList<AString> objPrivacyHeadersEmpty;

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::PRIVACY, AString::ConstNull()))
            .Times(AnyNumber())
            .WillOnce(Return(objPrivacyHeadersHaveId))
            .WillRepeatedly(Return(objPrivacyHeadersEmpty));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));

    IMSMap<SuppType, SuppService*> objOutSuppService1 = pMtcSupplementaryService->GetServices();
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::RESTRICTED));

    // config-From, config-no fallback -> IDENTITY
    // config-From, config-no fallback, no From header -> NONE
    // config-From, config-no fallback, anonymouns -> RESTRICTED
    // config-From, config-no fallback, unavailable, unavailable_none -> NONE
    // config-From, config-no fallback, unavailable, unavailable_restriected -> RESTRICTED
    EXPECT_CALL(*pMockIMtcConfigurationManager, IsEnableOipHeaderPolicyFallBack())
            .Times(5)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(*pMockIMtcConfigurationManager, IsOipSourceFromHeader())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(*pMockIMtcConfigurationManager, GetOipTypeForUnavailable())
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(1));

    IMSList<AString> objHeaders;
    objHeaders.Append(AString("\"testDisplay\" <sip:01030993879@fakeims.google.com>"));
    IMSList<AString> objHeadersEmpty;
    IMSList<AString> objHeadersAnonymous;
    objHeadersAnonymous.Append(AString("\"Anonymous\" <sip:Anonymous@fakeims.google.com>"));
    IMSList<AString> objHeadersunavailable;
    objHeadersunavailable.Append(AString("\"unavailable\" <sip:unavailable@fakeims.google.com>"));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .Times(5)
            .WillOnce(Return(objHeaders))
            .WillOnce(Return(objHeadersEmpty))
            .WillOnce(Return(objHeadersAnonymous))
            .WillOnce(Return(objHeadersunavailable))
            .WillOnce(Return(objHeadersunavailable));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::IDENTITY));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::NONE));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::RESTRICTED));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::NONE));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::RESTRICTED));

    // config-From, config-fallback, no From header -> IDENTITY
    // config-From, config-fallback, no From header, no Paid header -> NONE
    // config-From, config-fallback, no From header, anonymouns -> RESTRICTED
    // config-From, config-fallback, no From header, unavailable, unavailable_none -> NONE
    // config-From, config-fallback, no From header, unavailable, unavailable_restriected
    // -> RESTRICTED
    EXPECT_CALL(*pMockIMtcConfigurationManager, IsEnableOipHeaderPolicyFallBack())
            .Times(5)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(*pMockIMtcConfigurationManager, GetOipTypeForUnavailable())
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(1));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .Times(5)
            .WillRepeatedly(Return(objHeadersEmpty));

    EXPECT_CALL(
            objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .Times(5)
            .WillOnce(Return(objHeaders))
            .WillOnce(Return(objHeadersEmpty))
            .WillOnce(Return(objHeadersAnonymous))
            .WillOnce(Return(objHeadersunavailable))
            .WillOnce(Return(objHeadersunavailable));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::IDENTITY));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::NONE));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::RESTRICTED));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::NONE));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::RESTRICTED));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCnap)
{
    // config-From, config-no fallback
    // config-Paid, config-no fallback
    // config-From, config-fallback, no From
    // config-Paid, config-fallback, no Paid

    EXPECT_CALL(*pMockIMtcConfigurationManager, IsEnableOipHeaderPolicyFallBack())
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(*pMockIMtcConfigurationManager, IsOipSourceFromHeader())
            .Times(4)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    IMSList<AString> objHeaders;
    objHeaders.Append(AString("\"testDisplay\" <sip:01030993879@fakeims.google.com>"));
    IMSList<AString> objHeadersEmpty;

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .Times(3)
            .WillOnce(Return(objHeaders))
            .WillOnce(Return(objHeadersEmpty))
            .WillOnce(Return(objHeaders));

    EXPECT_CALL(
            objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .Times(3)
            .WillOnce(Return(objHeaders))
            .WillOnce(Return(objHeaders))
            .WillOnce(Return(objHeadersEmpty));

    pMtcSupplementaryService->UpdateCnap(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CNAP)->strValue, AString("testDisplay"));

    pMtcSupplementaryService->UpdateCnap(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CNAP)->strValue, AString("testDisplay"));

    pMtcSupplementaryService->UpdateCnap(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CNAP)->strValue, AString("testDisplay"));

    pMtcSupplementaryService->UpdateCnap(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CNAP)->strValue, AString("testDisplay"));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCdivCause)
{
    IMSList<AString> objHeaders;
    objHeaders.Append(AString("<sip:diverting_user1_address?Privacy=history>;index=1"));
    objHeaders.Append(
            AString("<sip:diverting_user2_address;cause=302?Privacy=none>;index=1.1;mp=1"));
    objHeaders.Append(AString("<sip:last_diverting_target;cause=486>;index=1.1.1;mp=1.1"));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));

    pMtcSupplementaryService->UpdateCdivCause(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_CAUSE)->nValue,
            static_cast<IMS_SINT32>(CdivCause::BUSY));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCdivHistory)
{
    IMSList<AString> objHeaders;
    objHeaders.Append(AString("<sip:bob@example.com>;index=1"));
    objHeaders.Append(AString("<sip:bob@192.0.2.4>;index=1.1;mp=1"));
    objHeaders.Append(AString("<sip:office@example.com;cause=486>;index=1.1.1;mp=1.1"));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));

    pMtcSupplementaryService->UpdateCdivHistory(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_HISTORY)->strValue, AString("office"));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCw)
{
    EXPECT_CALL(objMockISipMessage,
            IsHeaderPresent(ISipHeader::UNKNOWN, AString(SipHeaderName::ALERT_INFO)))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    pMtcSupplementaryService->UpdateCw(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CW)->bValue, IMS_TRUE);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallingNumVerification)
{
    EXPECT_CALL(*pMockIMtcConfigurationManager, IsOipSourceFromHeader())
            .Times(3)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objMockISipMessage,
            IsHeaderPresent(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    IMSList<AString> objHeadersPass;
    AString strCnvPass = "<sip:01030993879@fakeims.google.com;verstat=TN-Validation-Passed>";
    objHeadersPass.Append(AString(strCnvPass));

    IMSList<AString> objHeadersFail;
    AString strCnvFail = "<sip:01030993879@fakeims.google.com;verstat=TN-Validation-Failed>";
    objHeadersFail.Append(strCnvFail);

    IMSList<AString> objHeadersNone;
    objHeadersNone.Append(AString("<sip:01030993879@fakeims.google.com>"));

    EXPECT_CALL(
            objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeadersPass));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .Times(2)
            .WillOnce(Return(objHeadersFail))
            .WillOnce(Return(objHeadersNone));

    pMtcSupplementaryService->UpdateCallingNumVerification(
            static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_VERIFIED);
    pMtcSupplementaryService->Delete(SuppType::CALLING_NUM_VERIFICATION);

    pMtcSupplementaryService->UpdateCallingNumVerification(
            static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_NOT_VERIFIED);
    pMtcSupplementaryService->Delete(SuppType::CALLING_NUM_VERIFICATION);

    pMtcSupplementaryService->UpdateCallingNumVerification(
            static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION),
            static_cast<SuppService*>(IMS_NULL));
}

}  // namespace android
