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

#include "IMessage.h"
#include "ISipHeader.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "MtcDef.h"
#include "SipHeaderName.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "helper/MtcSupplementaryService.h"
#include "sipcore/MockISipMessage.h"
#include "utility/MessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

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
    MockIMtcCallContext objContext;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        pMockIMtcConfigurationManager = new MockIMtcConfigurationManager();
        pMtcConfigurationProxy = new MtcConfigurationProxy(
                static_cast<IMtcConfigurationManager*>(pMockIMtcConfigurationManager));
        pMtcSupplementaryService = new MtcSupplementaryService(objContext, *pMtcConfigurationProxy);

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

    ImsMap<SuppType, SuppService*> objInSuppService;
    objInSuppService.Add(SuppType::CNAP, pTestSupp1);
    objInSuppService.Add(SuppType::CALLER_ID, pTestSupp2);
    objInSuppService.Add(SuppType::CW, pTestSupp3);

    pMtcSupplementaryService->UpdateOutgoingServices(objInSuppService);

    const ImsMap<SuppType, SuppService*>& objOutSuppService =
            pMtcSupplementaryService->GetServices();

    EXPECT_EQ(objOutSuppService.GetSize(), 3);
    EXPECT_EQ(objOutSuppService.GetValue(SuppType::CNAP)->strValue, strTest);
    EXPECT_EQ(objOutSuppService.GetValue(SuppType::CALLER_ID)->nValue, 1);
    EXPECT_TRUE(objOutSuppService.GetValue(SuppType::CW)->bValue);

    SuppService* pTestSupp4 = new SuppService();
    pTestSupp4->bValue = IMS_FALSE;
    ImsMap<SuppType, SuppService*> objInSuppService2;
    objInSuppService2.Add(SuppType::CW, pTestSupp4);
    pMtcSupplementaryService->UpdateOutgoingServices(objInSuppService2);

    EXPECT_FALSE(objOutSuppService.GetValue(SuppType::CW)->bValue);
}

TEST_F(MtcSupplementaryServiceTest, UpdateTip)
{
    ImsList<AString> objPrivacyHeadersHaveId;
    objPrivacyHeadersHaveId.Append(AString("id"));
    ImsList<AString> objPrivacyHeadersEmpty;

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

    ImsList<AString> objPaidHeaders;
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
    ImsList<AString> objPrivacyHeadersHaveId;
    objPrivacyHeadersHaveId.Append(AString("id"));
    ImsList<AString> objPrivacyHeadersEmpty;

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::PRIVACY, AString::ConstNull()))
            .Times(AnyNumber())
            .WillOnce(Return(objPrivacyHeadersHaveId))
            .WillRepeatedly(Return(objPrivacyHeadersEmpty));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));

    ImsMap<SuppType, SuppService*> objOutSuppService1 = pMtcSupplementaryService->GetServices();
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

    ImsList<AString> objHeaders;
    objHeaders.Append(AString("\"testDisplay\" <sip:01030993879@fakeims.google.com>"));
    ImsList<AString> objHeadersEmpty;
    ImsList<AString> objHeadersAnonymous;
    objHeadersAnonymous.Append(AString("\"Anonymous\" <sip:Anonymous@fakeims.google.com>"));
    ImsList<AString> objHeadersunavailable;
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

    ImsList<AString> objHeaders;
    objHeaders.Append(AString("\"testDisplay\" <sip:01030993879@fakeims.google.com>"));
    ImsList<AString> objHeadersEmpty;

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
    ImsList<AString> objHeaders;
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

    objHeaders.Append(AString("<sip:last_diverting_target;cause=302>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivCause(static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_CAUSE)->nValue,
            static_cast<IMS_SINT32>(CdivCause::UNCONDITION));

    objHeaders.Append(AString("<sip:last_diverting_target;cause=404>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivCause(static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_CAUSE)->nValue,
            static_cast<IMS_SINT32>(CdivCause::NOT_LOGGED_IN));

    objHeaders.Append(AString("<sip:last_diverting_target;cause=408>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivCause(static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_CAUSE)->nValue,
            static_cast<IMS_SINT32>(CdivCause::NO_REPLY));

    objHeaders.Append(AString("<sip:last_diverting_target;cause=480>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivCause(static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_CAUSE)->nValue,
            static_cast<IMS_SINT32>(CdivCause::DEFLECTION));

    objHeaders.Append(AString("<sip:last_diverting_target;cause=487>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivCause(static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_CAUSE)->nValue,
            static_cast<IMS_SINT32>(CdivCause::DEFLECTION_ALERTING));

    objHeaders.Append(AString("<sip:last_diverting_target;cause=503>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivCause(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_CAUSE)->nValue,
            static_cast<IMS_SINT32>(CdivCause::NOT_REACHABLE));

    objHeaders.Append(AString("<sip:last_diverting_target;cause=999>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivCause(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_CAUSE)->nValue,
            static_cast<IMS_SINT32>(CdivCause::NONE));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCdivHistory)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(AString("<sip:bob@example.com>;index=1"));
    objHeaders.Append(AString("<sip:bob@192.0.2.4>;index=1.1;mp=1"));
    objHeaders.Append(AString("<sip:office@example.com;cause=486>;index=1.1.1;mp=1.1"));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));

    pMtcSupplementaryService->UpdateCdivHistory(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_HISTORY)->strValue, AString("office"));

    objHeaders.Append(AString("<sips:bob@ims.com;transport=tcp>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivHistory(static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_HISTORY)->strValue, AString("bob"));

    objHeaders.Append(AString("<tel:+358-9-123-45678>;index=1.1.1;mp=1.1"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivHistory(static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_HISTORY)->strValue,
            AString("+358-9-123-45678"));

    objHeaders.Append(AString("358-9-123-45678"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::HISTORY_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(objHeaders));
    pMtcSupplementaryService->UpdateCdivHistory(static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CDIV_HISTORY)->strValue,
            AString("+358-9-123-45678"));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCw)
{
    EXPECT_CALL(objMockISipMessage, IsHeaderPresent(ISipHeader::ALERT_INFO, AString::ConstNull()))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    pMtcSupplementaryService->UpdateCw(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_TRUE(pMtcSupplementaryService->Get(SuppType::CW)->bValue);

    pMtcSupplementaryService->Add(SuppType::CW, IMS_FALSE);
    EXPECT_FALSE(pMtcSupplementaryService->Get(SuppType::CW)->bValue);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallingNumVerification)
{
    ImsList<AString> objNoHeaders;

    ImsList<AString> objHeadersPass;
    AString strCnvPass = "<sip:01030993879@fakeims.google.com;verstat=TN-Validation-Passed>";
    objHeadersPass.Append(AString(strCnvPass));

    ImsList<AString> objHeadersFail;
    AString strCnvFail = "<sip:01030993879@fakeims.google.com;verstat=TN-Validation-Failed>";
    objHeadersFail.Append(strCnvFail);

    ImsList<AString> objHeadersNone;
    objHeadersNone.Append(AString("<sip:01030993879@fakeims.google.com>"));

    ImsList<AString> objHeadersNoValidation;
    objHeadersNoValidation.Append(
            AString("<sip:01030993879@fakeims.google.com;verstat=No-TN-Validation>"));

    EXPECT_CALL(
            objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .Times(6)
            .WillOnce(Return(objHeadersPass))
            .WillOnce(Return(objNoHeaders))
            .WillOnce(Return(objNoHeaders))
            .WillOnce(Return(objHeadersNoValidation))
            .WillOnce(Return(objHeadersNone))
            .WillOnce(Return(objNoHeaders));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .Times(4)
            .WillOnce(Return(objHeadersFail))
            .WillOnce(Return(objHeadersNone))
            .WillOnce(Return(objHeadersFail))
            .WillOnce(Return(objHeadersPass));

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
    pMtcSupplementaryService->Delete(SuppType::CALLING_NUM_VERIFICATION);

    pMtcSupplementaryService->UpdateCallingNumVerification(&objMockIMessage);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_NONE);
    pMtcSupplementaryService->Delete(SuppType::CALLING_NUM_VERIFICATION);

    pMtcSupplementaryService->UpdateCallingNumVerification(&objMockIMessage);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_NOT_VERIFIED);
    pMtcSupplementaryService->Delete(SuppType::CALLING_NUM_VERIFICATION);

    pMtcSupplementaryService->UpdateCallingNumVerification(&objMockIMessage);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_VERIFIED);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallComposerElements)
{
    pMtcSupplementaryService->UpdateCallComposerElements(&objMockIMessage);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_PRIORITY), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_SUBJECT), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_PICTURE_URL), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_LOCATION_LAT), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_LOCATION_LONG), nullptr);
    EXPECT_FALSE(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_IS_BUSINESS));

    ImsList<AString> lstPriorityHeaders;
    lstPriorityHeaders.Append("none");
    ON_CALL(objMockIMessage, GetHeaders(AString(SipHeaderName::PRIORITY)))
            .WillByDefault(Return(lstPriorityHeaders));

    ImsList<AString> lstSubjectHeaders;
    lstSubjectHeaders.Append("subject");
    ON_CALL(objMockIMessage, GetHeaders(AString(SipHeaderName::SUBJECT)))
            .WillByDefault(Return(lstSubjectHeaders));

    ImsList<AString> lstCallInfoHeaders;
    lstCallInfoHeaders.Append("<https://it-is-a/picture.jpg>;purpose=icon");
    ON_CALL(objMockIMessage, GetHeaders(AString(SipHeaderName::CALL_INFO)))
            .WillByDefault(Return(lstCallInfoHeaders));

    ImsList<AString> lstOrganizationHeaders;
    lstOrganizationHeaders.Append("some_org");
    ON_CALL(objMockIMessage, GetHeaders(AString(SipHeaderName::ORGANIZATION)))
            .WillByDefault(Return(lstOrganizationHeaders));

    pMtcSupplementaryService->UpdateCallComposerElements(&objMockIMessage);
    EXPECT_NE(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_PRIORITY), nullptr);
    EXPECT_NE(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_SUBJECT), nullptr);
    EXPECT_NE(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_PICTURE_URL), nullptr);
    // TODO: Location is hard to test now
    EXPECT_TRUE(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_IS_BUSINESS));
}

}  // namespace android
