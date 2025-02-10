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

#include "ByteArray.h"
#include "IMessage.h"
#include "ISipHeader.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockISipMessage.h"
#include "MtcDef.h"
#include "SipHeaderName.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const IMS_CHAR SESSION_ID[] = "f81d4fae7dec11d0a76500a0c91e6bf6";

LOCAL const AString CNV_HEADER_PASSED =
        "<sip:01030993879@fakeims.google.com;verstat=TN-Validation-Passed>";
LOCAL const AString CNV_HEADER_FAILED =
        "<sip:01030993879@fakeims.google.com;verstat=TN-Validation-Failed>";
LOCAL const AString CNV_HEADER_PASSED_POTENTIAL_SPAM =
        "potential spam <sip:01030993879@fakeims.google.com;verstat=TN-Validation-Passed>";
LOCAL const AString CNV_HEADER_NO_TN_VALIDATION =
        "<sip:01030993879@fakeims.google.com;verstat=No-TN-Validation>";
LOCAL const AString CNV_HEADER_NONE = "<sip:01030993879@fakeims.google.com>";

namespace android
{

class MtcSupplementaryServiceTest : public ::testing::Test
{
public:
    MockMtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pMtcSupplementaryService;
    MockISipMessage objMockISipMessage;
    MockIMessage objMockIMessage;
    MockIMtcCallContext objContext;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        pConfigurationProxy = new MockMtcConfigurationProxy();
        pMtcSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);

        EXPECT_CALL(objMockIMessage, GetMessage())
                .Times(AnyNumber())
                .WillRepeatedly(Return(static_cast<ISipMessage*>(&objMockISipMessage)));
    }

    virtual void TearDown() override
    {
        if (pConfigurationProxy)
        {
            delete pConfigurationProxy;
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

TEST_F(MtcSupplementaryServiceTest, UpdateCallerIdDoesNotCheckPrivacyHeaderIfFromIsAnonymous)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    ImsList<AString> objHeadersAnonymous;
    objHeadersAnonymous.Append(AString("\"Anonymous\" <sip:Anonymous@fakeims.google.com>"));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillOnce(Return(objHeadersAnonymous));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::PRIVACY, _)).Times(0);

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallerIdChecksPrivacyHeaderIfFromIsNotAnonymous)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    ImsList<AString> objHeaders;
    objHeaders.Append(AString("\"testDisplay\" <sip:01030993879@fakeims.google.com>"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillOnce(Return(objHeaders));

    ImsList<AString> objPrivacyHeadersHaveId;
    objPrivacyHeadersHaveId.Append(AString("id"));
    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::PRIVACY, AString::ConstNull()))
            .WillOnce(Return(objPrivacyHeadersHaveId));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::RESTRICTED));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallerIdUsingFromOnly)
{
    // config-From, config-no fallback -> IDENTITY -> (No Privacy) IDENTITY
    // config-From, config-no fallback, no From header -> INVALID -> (No Privacy) NONE
    // config-From, config-no fallback, anonymous -> RESTRICTED
    // config-From, config-no fallback, unavailable, unavailable_config: RESTRICTED
    // config-From, config-no fallback, unavailable, unavailable_config: -> UNAVAILABLE
    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL))
            .Times(5)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_OIP_TYPE_FOR_UNAVAILABLE_INT))
            .Times(2)
            .WillOnce(Return(static_cast<IMS_SINT32>(OipType::RESTRICTED)))
            .WillOnce(Return(static_cast<IMS_SINT32>(OipType::UNAVAILABLE)));

    ImsList<AString> objHeaders;
    objHeaders.Append(AString("\"testDisplay\" <sip:01030993879@fakeims.google.com>"));
    ImsList<AString> objHeadersEmpty;
    ImsList<AString> objHeadersAnonymous;
    objHeadersAnonymous.Append(AString("\"Anonymous\" <sip:Anonymous@fakeims.google.com>"));
    ImsList<AString> objHeadersUnavailable;
    objHeadersUnavailable.Append(AString("\"unavailable\" <sip:unavailable@fakeims.google.com>"));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::PRIVACY, _))
            .Times(2)
            .WillRepeatedly(Return(objHeadersEmpty));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .Times(5)
            .WillOnce(Return(objHeaders))
            .WillOnce(Return(objHeadersEmpty))
            .WillOnce(Return(objHeadersAnonymous))
            .WillOnce(Return(objHeadersUnavailable))
            .WillOnce(Return(objHeadersUnavailable));

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
            static_cast<IMS_SINT32>(OipType::RESTRICTED));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::UNAVAILABLE));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallerIdUsingPaidByFallback)
{
    // config-From, config-fallback, no From header -> IDENTITY
    // config-From, config-fallback, no From header, no Paid header -> NONE
    // config-From, config-fallback, no From header, anonymouns -> RESTRICTED
    // config-From, config-fallback, no From header, unavailable, unavailable_config: RESTRICTED
    // config-From, config-fallback, no From header, unavailable, unavailable_config: -> UNAVAILABLE
    // -> RESTRICTED
    EXPECT_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL))
            .Times(5)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_OIP_TYPE_FOR_UNAVAILABLE_INT))
            .Times(2)
            .WillOnce(Return(static_cast<IMS_SINT32>(OipType::RESTRICTED)))
            .WillOnce(Return(static_cast<IMS_SINT32>(OipType::UNAVAILABLE)));

    ImsList<AString> objHeaders;
    objHeaders.Append(AString("\"testDisplay\" <sip:01030993879@fakeims.google.com>"));
    ImsList<AString> objHeadersEmpty;
    ImsList<AString> objHeadersAnonymous;
    objHeadersAnonymous.Append(AString("\"Anonymous\" <sip:Anonymous@fakeims.google.com>"));
    ImsList<AString> objHeadersUnavailable;
    objHeadersUnavailable.Append(AString("\"unavailable\" <sip:unavailable@fakeims.google.com>"));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::PRIVACY, _))
            .Times(2)
            .WillRepeatedly(Return(objHeadersEmpty));

    EXPECT_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .Times(5)
            .WillRepeatedly(Return(objHeadersEmpty));

    EXPECT_CALL(
            objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .Times(5)
            .WillOnce(Return(objHeaders))
            .WillOnce(Return(objHeadersEmpty))
            .WillOnce(Return(objHeadersAnonymous))
            .WillOnce(Return(objHeadersUnavailable))
            .WillOnce(Return(objHeadersUnavailable));

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
            static_cast<IMS_SINT32>(OipType::RESTRICTED));

    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::UNAVAILABLE));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallerIdWithDisplayName)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_OIP_TYPE_FOR_UNAVAILABLE_INT))
            .WillByDefault(Return(static_cast<IMS_SINT32>(OipType::UNAVAILABLE)));

    ImsList<AString> objFromHeaders;
    objFromHeaders.Append(AString("\"Coin line/payphone\" <sip:anonymous@anonymous.invalid>"));
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillByDefault(Return(objFromHeaders));
    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::PAYPHONE));

    objFromHeaders.Clear();
    objFromHeaders.Append(
            AString("\"Interaction with other service\" <sip:anonymous@anonymous.invalid>"));
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillByDefault(Return(objFromHeaders));
    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::UNAVAILABLE));

    objFromHeaders.Clear();
    objFromHeaders.Append(AString("\"Unavailable\" <sip:anonymous@anonymous.invalid>"));
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillByDefault(Return(objFromHeaders));
    pMtcSupplementaryService->UpdateCallerId(static_cast<IMessage*>(&objMockIMessage));
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLER_ID)->nValue,
            static_cast<IMS_SINT32>(OipType::UNAVAILABLE));
}

TEST_F(MtcSupplementaryServiceTest, UpdateCnap)
{
    // config-From, config-no fallback
    // config-Paid, config-no fallback
    // config-From, config-fallback, no From
    // config-Paid, config-fallback, no Paid

    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL))
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
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

TEST_F(MtcSupplementaryServiceTest, UpdateCallingNumberVerificationWhenPaidHeaderContainsPassed)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_PASSED);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_VERIFIED);
}

TEST_F(MtcSupplementaryServiceTest,
        UpdateCallingNumberVerificationWhenPaidHeaderContainsPassedAndPotentialSpam)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_PASSED_POTENTIAL_SPAM);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_NOT_VERIFIED);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallingNumberVerificationWhenPaidHeaderContainsFailed)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_FAILED);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_NOT_VERIFIED);
}

TEST_F(MtcSupplementaryServiceTest,
        UpdateCallingNumberVerificationWhenPaidHeaderContainsNoValidation)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_NO_TN_VALIDATION);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_NONE);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallingNumberVerificationWhenPaidHeaderNotContainsVerstat)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_NONE);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    ImsList<AString> objEmptyHeaders;
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillByDefault(Return(objEmptyHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION), IMS_NULL);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallingNumberVerificationWhenFromHeaderContainsPassed)
{
    ImsList<AString> objEmptyHeaders;
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objEmptyHeaders));

    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_PASSED);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_VERIFIED);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallingNumberVerificationWhenFromHeaderContainsFailed)
{
    ImsList<AString> objEmptyHeaders;
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objEmptyHeaders));

    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_FAILED);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_NOT_VERIFIED);
}

TEST_F(MtcSupplementaryServiceTest,
        UpdateCallingNumberVerificationWhenFromHeaderContainsNoValidation)
{
    ImsList<AString> objEmptyHeaders;
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objEmptyHeaders));

    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_NO_TN_VALIDATION);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION)->nValue,
            CALLING_NUM_VERSTAT_NONE);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallingNumberVerificationWhenFromHeaderNotContainsVerstat)
{
    ImsList<AString> objEmptyHeaders;
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, AString::ConstNull()))
            .WillByDefault(Return(objEmptyHeaders));

    ImsList<AString> objHeaders;
    objHeaders.Append(CNV_HEADER_NONE);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::FROM, AString::ConstNull()))
            .WillByDefault(Return(objHeaders));

    pMtcSupplementaryService->UpdateCallingNumberVerification(
            static_cast<IMessage*>(&objMockIMessage));

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALLING_NUM_VERIFICATION), IMS_NULL);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallComposerElementsFromEmptyMessage)
{
    pMtcSupplementaryService->UpdateCallComposerElements(&objMockIMessage);

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_PRIORITY), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_SUBJECT), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_PICTURE_URL), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_LOCATION_LAT), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_LOCATION_LONG), nullptr);
    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_IS_BUSINESS), nullptr);
}

TEST_F(MtcSupplementaryServiceTest, UpdateCallComposerElementsFromMessageWithCallComposerInfo)
{
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

    MockIMessageBodyPart objLocationBody;
    ByteArray objLocationContent(  // From RCC.20
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "entity=\"tel:+491711234567\">"
            "<dm:person id=\"sh2204\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<gs:Circle srsName=\"urn:ogc:def:crs:EPSG::4326\">"
            "<gml:pos>47.577866 -122.164080</gml:pos>"
            "<gs:radius uom=\"urn:ogc:def:uom:EPSG::9001\">30</gs:radius>"
            "</gs:Circle>"
            "</gp:location-info>"
            "<gp:usage-rules/>"
            "</gp:geopriv>"
            "</dm:person>"
            "</presence>");
    ON_CALL(objLocationBody, GetHeader(AString(SipHeaderName::CONTENT_TYPE)))
            .WillByDefault(Return(AString("application/pidf+xml")));
    ON_CALL(objLocationBody, GetContent).WillByDefault(ReturnRef(objLocationContent));
    ImsList<IMessageBodyPart*> lstMessageBodies;
    lstMessageBodies.Append(&objLocationBody);
    ON_CALL(objMockIMessage, GetBodyParts).WillByDefault(Return(lstMessageBodies));

    pMtcSupplementaryService->UpdateCallComposerElements(&objMockIMessage);

    EXPECT_EQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_PRIORITY)->nValue,
            CALL_COMPOSER_PRIORITY_NONE);
    EXPECT_STREQ(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_SUBJECT)->strValue.GetStr(),
            "subject");
    EXPECT_STREQ(
            pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_PICTURE_URL)->strValue.GetStr(),
            "https://it-is-a/picture.jpg");
    EXPECT_STREQ(
            pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_LOCATION_LAT)->strValue.GetStr(),
            "47.577866");
    EXPECT_STREQ(
            pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_LOCATION_LONG)->strValue.GetStr(),
            "-122.164080");
    EXPECT_TRUE(pMtcSupplementaryService->Get(SuppType::CALL_COMPOSER_IS_BUSINESS)->bValue);
}

TEST_F(MtcSupplementaryServiceTest, UpdateSessionId)
{
    ImsList<AString> lstSessionIds;
    lstSessionIds.Append(SESSION_ID);
    ON_CALL(objMockISipMessage, GetHeaders(ISipHeader::SESSION_ID, AString::ConstNull()))
            .WillByDefault(Return(lstSessionIds));

    pMtcSupplementaryService->UpdateSessionId(&objMockIMessage);

    EXPECT_TRUE(pMtcSupplementaryService->Get(SuppType::SESSION_ID));
}

}  // namespace android
