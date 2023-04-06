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

#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "FeatureCaps.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoLocation.h"
#include "SipStatusCode.h"
#include "call/MockIMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "call/message/MessageFormatter.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "dialogevent/MockIMultiEndpointManager.h"
#include "helper/MtcSupplementaryService.h"
#include "service/MockIFeatureCaps.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipHeaderName.h"
#include "utility/MessageUtil.h"
#include "utility/MessageUtils.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <vector>

LOCAL IMS_SINT32 SLOT_ID = 0;

using ::testing::_;
using ::testing::AnyOf;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL const AString SRVCC_FEATURE_A(MessageUtil::STR_SRVCC_FEATURE_A);
LOCAL const AString SRVCC_FEATURE_B(MessageUtil::STR_SRVCC_FEATURE_B);
LOCAL const AString SRVCC_FEATURE_M(MessageUtil::STR_SRVCC_FEATURE_M);

class MessageFormatterTest : public ::testing::Test
{
public:
    MessageFormatter* pFormatter;

    CallInfo objCallInfo;
    MockIMessage objMessage;
    MockISipMessage objSipMessage;
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockISession objSession;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    MessageUtils objMessageUtils;
    ParticipantInfo* pParticipantInfo;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        pSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
        ON_CALL(objSession, GetNextResponse).WillByDefault(Return(&objMessage));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        pParticipantInfo = new ParticipantInfo(objContext);
        ON_CALL(objContext, GetParticipantInfo).WillByDefault(ReturnRef(*pParticipantInfo));

        pFormatter = new MessageFormatter(objContext, objSession);
    }

    virtual void TearDown() override
    {
        delete pFormatter;
        delete pConfigurationProxy;
        delete pSupplementaryService;
    }

    IMS_SINT32 GetRejectStatusCode(IMS_SINT32 nCode, IMS_SINT32 nExtraCode = -1)
    {
        CallReasonInfo objReasonInfo(nCode, nExtraCode);
        IMS_SINT32 eStatusCode;
        AString strPhrase;
        pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);
        return eStatusCode;
    }

    AString GetRejectPhrase(IMS_SINT32 nCode)
    {
        CallReasonInfo objReasonInfo(nCode);
        IMS_SINT32 eStatusCode;
        AString strPhrase;
        pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);
        return strPhrase;
    }
};

TEST_F(MessageFormatterTest, FormStartMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormStartMessageWithGeolocation)
{
    ON_CALL(*pConfigurationManager, IsMessageTypeSupportGeolocationPidf)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsSupportGeolocationPidfInSipInvite)
            .WillByDefault(Return(IMS_FALSE));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(*pConfigurationManager, IsSupportGeolocationPidfInSipInvite)
            .WillByDefault(Return(IMS_TRUE));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageWithoutCallComposerElements)
{
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::PRIORITY), _)).Times(0);
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::SUBJECT), _)).Times(0);
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::CALL_INFO), _)).Times(0);
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _)).Times(0);

    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageWithCallComposerPriority)
{
    pSupplementaryService->Add(SuppType::CALL_COMPOSER_PRIORITY, CALL_COMPOSER_PRIORITY_NONE);
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::PRIORITY), AString("none"))).Times(1);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageWithCallComposerSubject)
{
    pSupplementaryService->Add(SuppType::CALL_COMPOSER_SUBJECT, AString("subject"));
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::SUBJECT), AString("subject")))
            .Times(1);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageWithCallComposerPicture)
{
    pSupplementaryService->Add(SuppType::CALL_COMPOSER_PICTURE_URL, AString("url"));
    EXPECT_CALL(
            objMessage, AddHeader(AString(SipHeaderName::CALL_INFO), AString("<url>;purpose=icon")))
            .Times(1);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageWithIncompleteCallComposerLocation)
{
    pSupplementaryService->Delete(SuppType::CALL_COMPOSER_LOCATION_LAT);
    pSupplementaryService->Add(SuppType::CALL_COMPOSER_LOCATION_LONG, AString("2"));
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _)).Times(0);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);

    pSupplementaryService->Add(SuppType::CALL_COMPOSER_LOCATION_LAT, AString("1"));
    pSupplementaryService->Delete(SuppType::CALL_COMPOSER_LOCATION_LONG);
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _)).Times(0);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageWithCallComposerLocation)
{
    pSupplementaryService->Add(SuppType::CALL_COMPOSER_LOCATION_LAT, AString("1"));
    pSupplementaryService->Add(SuppType::CALL_COMPOSER_LOCATION_LONG, AString("2"));
    // TODO: Location is hard to test now
    // EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormProvisionalResponseMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormProvisionalResponseMessage(IMS_TRUE);

    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(*pConfigurationManager, IsMessageTypeSupportGeolocationPidf)
            .WillByDefault(Return(IMS_TRUE));

    nResult = pFormatter->FormProvisionalResponseMessage(IMS_TRUE);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormProvisionalResponseMessageFailureCase)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormProvisionalResponseMessage(IMS_TRUE);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormPrackMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormPrackMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormPrackMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormPrackMessage();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormPrackResponseMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormPrackResponseMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormPrackResponseMessageFailureCase)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormPrackResponseMessage();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormEarlyUpdateMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormEarlyUpdateMessage(UpdateType::NORMAL);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormEarlyUpdateMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormEarlyUpdateMessage(UpdateType::NORMAL);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormEarlyUpdateResponseMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormEarlyUpdateResponseMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormEarlyUpdateResponseMessageFailureCase)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormEarlyUpdateResponseMessage();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormAcceptMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormAcceptMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(*pConfigurationManager, IsMessageTypeSupportGeolocationPidf)
            .WillByDefault(Return(IMS_TRUE));

    nResult = pFormatter->FormAcceptMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormAcceptMessageFailureCase)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormAcceptMessage();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormRejectMessageWithCodeUserDecline)
{
    CallReasonInfo objReasonInfo(CODE_USER_DECLINE);
    IMS_SINT32 eStatusCode;
    AString strPhrase;

    IMS_RESULT nResult = pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);

    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(*pConfigurationManager, IsMessageTypeSupportGeolocationPidf)
            .WillByDefault(Return(IMS_TRUE));

    nResult = pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormRejectMessageFailureCase)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    CallReasonInfo objReasonInfo(CODE_USER_DECLINE);
    IMS_SINT32 eStatusCode;
    AString strPhrase;

    IMS_RESULT nResult = pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormAckMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormAckMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormAckMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormAckMessage();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormUpdateMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormUpdateMessage(UpdateType::NORMAL, IMS_TRUE);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormUpdateMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormUpdateMessage(UpdateType::NORMAL, IMS_TRUE);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormAcceptUpdateMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormAcceptUpdateMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormAcceptUpdateMessageFailureCase)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormAcceptUpdateMessage();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormCancelUpdateMessageWithCodeNone)
{
    CallReasonInfo objReasonInfo(CODE_NONE);
    IMS_RESULT nResult = pFormatter->FormCancelUpdateMessage(objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormCancelUpdateMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    CallReasonInfo objReasonInfo(CODE_NONE);
    IMS_RESULT nResult = pFormatter->FormCancelUpdateMessage(objReasonInfo);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormTerminateMessageWithCodeUserTerminated)
{
    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED);
    IMS_RESULT nResult = pFormatter->FormTerminateMessage(objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormTerminateMessageWithCodeUserTerminatedAndSipTimeout)
{
    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);
    IMS_RESULT nResult = pFormatter->FormTerminateMessage(objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormTerminateMessageWithCodeUserTerminatedAndRtpTimeout)
{
    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_RTP_TIMEOUT);
    IMS_RESULT nResult = pFormatter->FormTerminateMessage(objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormTerminateMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED);
    IMS_RESULT nResult = pFormatter->FormTerminateMessage(objReasonInfo);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormTerminateMessageAddCarrierSpecificHeaderByConfig)
{
    const AString strCarrierSpecificHeader(MessageUtil::STR_P_SKT_BYE_CAUSE);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strCarrierSpecificHeader))
            .WillByDefault(Return(IMS_TRUE));

    const AString strByeCauseNormal("normal");
    MockIMessageUtils objMockMessageUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMockMessageUtils));
    EXPECT_CALL(objMockMessageUtils,
            AddValueIfNotExists(
                    &objMessage, strByeCauseNormal, ISipHeader::UNKNOWN, strCarrierSpecificHeader));

    const CallReasonInfo objReasonInfo(CODE_USER_TERMINATED);
    pFormatter->FormTerminateMessage(objReasonInfo);
}

TEST_F(MessageFormatterTest, SetAcceptContactHeader)
{
    pFormatter->FormStartMessage(CallType::VT);
}

TEST_F(MessageFormatterTest, AddSrvccFeatureByStartMessage)
{
    // ICoreService is null case. Nothing to check.
    ON_CALL(objService, GetICoreService).WillByDefault(Return(nullptr));
    pFormatter->FormStartMessage(CallType::VOIP);

    // IFeatureCaps is null case. Nothing to check.
    MockICoreService objCoreService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(nullptr));
    pFormatter->FormStartMessage(CallType::VOIP);

    // Normal case
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));

    EXPECT_CALL(objFeatureCaps,
            AddFeature(SRVCC_FEATURE_A, AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_REQUEST));
    EXPECT_CALL(objFeatureCaps,
            AddFeature(SRVCC_FEATURE_B, AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_REQUEST));
    EXPECT_CALL(objFeatureCaps,
            AddFeature(SRVCC_FEATURE_M, AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_REQUEST));
    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, AddNoSrvccFeatureByPrAnswerMessage)
{
    // TODO: change all Tests in this file to use MockIMessageUtils.
    MockIMessageUtils objMockMessageUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMockMessageUtils));

    MockICoreService objCoreService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));

    // Previous Message is null case
    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(nullptr));
    EXPECT_CALL(objFeatureCaps, AddFeature(_, _, _, _)).Times(0);
    pFormatter->FormProvisionalResponseMessage(IMS_TRUE);

    // No FeatureCaps in INVITE case
    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(&objMessage));
    const AString strFeatureCaps(SipHeaderName::FEATURE_CAPS);
    ON_CALL(objMockMessageUtils,
            ContainsValue(&objMessage, AnyOf(SRVCC_FEATURE_A, SRVCC_FEATURE_B, SRVCC_FEATURE_M),
                    ISipHeader::UNKNOWN, strFeatureCaps))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objFeatureCaps, AddFeature(_, _, _, _)).Times(0);
    pFormatter->FormProvisionalResponseMessage(IMS_TRUE);
}

TEST_F(MessageFormatterTest, AddSrvccFeatureByPrAnswerMessage)
{
    MockIMessageUtils objMockMessageUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMockMessageUtils));
    MockICoreService objCoreService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));
    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(&objMessage));

    const AString strFeatureCaps(SipHeaderName::FEATURE_CAPS);
    ON_CALL(objMockMessageUtils,
            ContainsValue(&objMessage, AnyOf(SRVCC_FEATURE_A, SRVCC_FEATURE_B, SRVCC_FEATURE_M),
                    ISipHeader::UNKNOWN, strFeatureCaps))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objFeatureCaps,
            AddFeature(SRVCC_FEATURE_A, AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_RESPONSE));
    EXPECT_CALL(objFeatureCaps,
            AddFeature(SRVCC_FEATURE_B, AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_RESPONSE));
    EXPECT_CALL(objFeatureCaps,
            AddFeature(SRVCC_FEATURE_M, AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_RESPONSE));
    pFormatter->FormProvisionalResponseMessage(IMS_TRUE);
}

TEST_F(MessageFormatterTest, SetSrvccContactParameter)
{
    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(nullptr));
    pFormatter->FormAcceptMessage();

    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(&objMessage));
    ImsList<AString> lstHeaders;
    lstHeaders.Append(MessageUtil::STR_SRVCC_FEATURE_A);
    lstHeaders.Append(MessageUtil::STR_SRVCC_FEATURE_B);
    lstHeaders.Append(MessageUtil::STR_SRVCC_FEATURE_M);
    ON_CALL(objSipMessage, GetHeaders).WillByDefault(Return(lstHeaders));
    pFormatter->FormAcceptMessage();
}

TEST_F(MessageFormatterTest, SetCallerIdHeader)
{
    pSupplementaryService->Delete(SuppType::CALLER_ID);
    pFormatter->FormStartMessage(CallType::VOIP);

    pSupplementaryService->Add(SuppType::CALLER_ID, CALLERID_RESTRICTED);
    ON_CALL(*pConfigurationManager, GetSessionPrivacyType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::SESSION_PRIVACY_TYPE_HEADER));
    pFormatter->FormStartMessage(CallType::VOIP);
    ON_CALL(*pConfigurationManager, GetSessionPrivacyType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::SESSION_PRIVACY_TYPE_NONE));
    pFormatter->FormStartMessage(CallType::VOIP);

    pSupplementaryService->Delete(SuppType::CALLER_ID);
    pSupplementaryService->Add(SuppType::CALLER_ID, CALLERID_IDENTITY);
    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, SetPEarlyMediaHeader)
{
    objCallInfo.bUssi = IMS_TRUE;
    pFormatter->FormStartMessage(CallType::VOIP);

    objCallInfo.bUssi = IMS_FALSE;
    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, SetAlertInfoHeader)
{
    pFormatter->FormProvisionalResponseMessage(IMS_TRUE);

    pFormatter->FormProvisionalResponseMessage(IMS_FALSE);
}

TEST_F(MessageFormatterTest, SetReasonHeader)
{
    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED);
    pFormatter->FormTerminateMessage(objReasonInfo);

    const AString strReason = "TEST_REASON";
    ON_CALL(*pConfigurationManager, GetCallTerminateReasonHeader).WillByDefault(Return(strReason));

    pFormatter->FormTerminateMessage(objReasonInfo);
}

TEST_F(MessageFormatterTest, SetCarrierSpecificHeaders)
{
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader).WillByDefault(Return(IMS_TRUE));
    pFormatter->FormStartMessage(CallType::VOIP);
    pFormatter->FormAcceptMessage();
    pFormatter->FormUpdateMessage(UpdateType::NORMAL, IMS_TRUE);
    pFormatter->FormAcceptUpdateMessage();
}

TEST_F(MessageFormatterTest, SetCarrierSpecificHeadersSetsTranscodingHeaderIfCallPull)
{
    MockIMessageUtils objMockMessageUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMockMessageUtils));
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader).WillByDefault(Return(IMS_FALSE));
    pSupplementaryService->Add(SuppType::CALL_PULL, IMS_FALSE);
    const AString strTranscodingHeader(MessageUtil::STR_P_COM_ENABLETRANSCODING);

    // TODO: make this be checked isolated.
    // EXPECT_CALL(objMockMessageUtils, AddValueIfNotExists(
    //        &objMessage, _, ISipHeader::UNKNOWN, strTranscodingHeader));
    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, FormStartMessageSetsReplaceHeaderIfCallPull)
{
    MockIMultiEndpointManager objMepManager;
    ON_CALL(objContext, GetMultiEndpointManager).WillByDefault(Return(&objMepManager));
    IMultiEndpointManager::PullingDialogInfo objDialogInfo;
    objDialogInfo.strCallId = "anyCallId";
    objDialogInfo.strLocalTag = "anyLocalTag";
    objDialogInfo.strRemoteTag = "anyRemoteTag";
    ON_CALL(objMepManager, GetDialogInfo(_)).WillByDefault(Return(objDialogInfo));
    AString strReplaces("anyCallId;from-tag=anyLocalTag;to-tag=anyRemoteTag");

    MockIMessageUtils objMockMessageUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMockMessageUtils));

    pSupplementaryService->Add(SuppType::CALL_PULL, IMS_FALSE);

    // TODO: make this be checked isolated.
    // EXPECT_CALL(objMockMessageUtils, AddValueIfNotExists(
    //        &objMessage, strReplaces, ISipHeader::REPLACES, _));
    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, GetRejectStatusCode)
{
    const IMS_SINT32 nTestStatusCode = 999;

    EXPECT_EQ(GetRejectStatusCode(CODE_NONE), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_UNSPECIFIED), SipStatusCode::SC_480);
    ON_CALL(*pConfigurationManager, GetIncomingCallRejectCodeForUserDecline)
            .WillByDefault(Return(nTestStatusCode));
    EXPECT_EQ(GetRejectStatusCode(CODE_USER_DECLINE), nTestStatusCode);
    EXPECT_EQ(GetRejectStatusCode(CODE_USER_NOANSWER), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOW_BATTERY), SipStatusCode::SC_486);
    EXPECT_EQ(
            GetRejectStatusCode(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_SERVICE_UNAVAILABLE), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_VT_TTY_NOT_ALLOWED), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_ONGOING_CS_CALL), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_ONGOING_E911_CALL), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_CALL_ON_OTHER_SUB), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_ONGOING_CALL_SETUP), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_MAX_CALL_LIMIT_REACHED), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_CALL_EXCEEDED), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_CALL_BUSY), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_CALL_DECLINE), SipStatusCode::SC_603);
    EXPECT_EQ(GetRejectStatusCode(CODE_USER_IGNORE), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_UNSUPPORTED_SIP_HEADERS), SipStatusCode::SC_420);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE), SipStatusCode::SC_406);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_SIP_406),
            SipStatusCode::SC_406);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_SIP_488),
            SipStatusCode::SC_488);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_SIP_606),
            SipStatusCode::SC_606);
    ON_CALL(*pConfigurationManager, GetCallRejectCodeForNotAcceptableCallType)
            .WillByDefault(Return(nTestStatusCode));
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE),
            nTestStatusCode);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_ONGOING_CALL_UPGRADE), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_INTERNAL_ERROR), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED),
            SipStatusCode::SC_580);
    EXPECT_EQ(GetRejectStatusCode(CODE_MEDIA_INIT_FAILED), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_MEDIA_NOT_ACCEPTABLE), SipStatusCode::SC_488);
    ON_CALL(*pConfigurationManager, GetIncomingCallRejectCodeForNoAnswer)
            .WillByDefault(Return(nTestStatusCode));
    EXPECT_EQ(GetRejectStatusCode(CODE_TIMEOUT_NO_ANSWER), nTestStatusCode);
    EXPECT_EQ(GetRejectStatusCode(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE), SipStatusCode::SC_603);
    EXPECT_EQ(GetRejectStatusCode(CODE_NETWORK_RESP_TIMEOUT), SipStatusCode::SC_500);
    EXPECT_EQ(GetRejectStatusCode(CODE_BLACKLISTED_CALL_ID), SipStatusCode::SC_603);
    EXPECT_EQ(GetRejectStatusCode(CODE_USER_REJECTED_SESSION_MODIFICATION), SipStatusCode::SC_603);
}

TEST_F(MessageFormatterTest, GetRejectPhrase)
{
    const AString strTestPhrase = "TEST_PHRASE";
    ON_CALL(*pConfigurationManager, GetCallRejectReasonPhrase).WillByDefault(Return(strTestPhrase));

    EXPECT_TRUE(GetRejectPhrase(CODE_NONE).GetLength() < 1);
    EXPECT_EQ(GetRejectPhrase(CODE_USER_DECLINE), strTestPhrase);
    EXPECT_EQ(GetRejectPhrase(CODE_REJECT_ONGOING_CS_CALL), strTestPhrase);
    EXPECT_EQ(GetRejectPhrase(CODE_LOCAL_CALL_BUSY), strTestPhrase);
    EXPECT_EQ(GetRejectPhrase(CODE_REJECT_ONGOING_CALL_SETUP), strTestPhrase);
    EXPECT_EQ(GetRejectPhrase(CODE_REJECT_MAX_CALL_LIMIT_REACHED), strTestPhrase);
    EXPECT_EQ(GetRejectPhrase(CODE_TIMEOUT_NO_ANSWER), strTestPhrase);
    EXPECT_EQ(GetRejectPhrase(CODE_REJECT_ONGOING_CALL_UPGRADE), strTestPhrase);
    EXPECT_EQ(GetRejectPhrase(CODE_MEDIA_NOT_ACCEPTABLE), strTestPhrase);
}

TEST_F(MessageFormatterTest, SetUpdateReason)
{
    pFormatter->FormUpdateMessage(UpdateType::NORMAL, IMS_TRUE);
    pFormatter->FormUpdateMessage(UpdateType::SRVCC_RECOVERED_CANCEL, IMS_TRUE);
    pFormatter->FormUpdateMessage(UpdateType::SRVCC_RECOVERED_FAILURE, IMS_TRUE);
}

TEST_F(MessageFormatterTest, SetTerminateReason)
{
    CallReasonInfo objReasonInfo(CODE_NONE);
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_USER_TERMINATED;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_MEDIA_NO_DATA;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_SIP_REQUEST_CANCELLED;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_NETWORK_RESP_TIMEOUT;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_TIMEOUT_1XX_WAITING;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_TIMEOUT_NO_ANSWER;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE;
    pFormatter->FormTerminateMessage(objReasonInfo);
}

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetHeaderDoesNotSetReasonHeadersByNoConfiguration)
{
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(_)).WillByDefault(Return(IMS_FALSE));

    MockISipMessage objMessage;
    EXPECT_CALL(objMessage, AddHeader(_, _, _)).Times(0);

    // to cover null case
    pFormatter->ReasonHeaderSetter_SetHeader(IMS_NULL, 0);

    //clang-format off
    std::vector<IMS_SINT32> objReasons{ISession::TERMINATION_REASON_INVALID,
            ISession::TERMINATION_REASON_UNKNOWN, ISession::TERMINATION_REASON_USER_ACTION,
            ISession::TERMINATION_REASON_REMOTE_ACTION, ISession::TERMINATION_REASON_REFRESH_408,
            ISession::TERMINATION_REASON_REFRESH_481,
            ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT,
            ISession::TERMINATION_REASON_REFRESH_TIMEOUT,
            ISession::TERMINATION_REASON_SERVICE_CLOSED, ISession::TERMINATION_REASON_MAX};
    //clang-format on

    for (IMS_SINT32 eTerminationReason : objReasons)
    {
        pFormatter->ReasonHeaderSetter_SetHeader(&objMessage, eTerminationReason);
    }
}

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetHeaderSetsReasonHeadersByVzwConfiguration)
{
    const AString strReasonHeaderName(SipHeaderName::REASON);
    const AString strCarrierSpecificHeader(MessageUtil::STR_REASON_USER_SESSIONEXPIRED);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strCarrierSpecificHeader))
            .WillByDefault(Return(IMS_TRUE));

    const AString strReasonHeaderValue("USER;text=\"Session Expired\"");
    MockISipMessage objMessage;
    EXPECT_CALL(
            objMessage, AddHeader(ISipHeader::UNKNOWN, strReasonHeaderValue, strReasonHeaderName))
            .Times(2);
    pFormatter->ReasonHeaderSetter_SetHeader(
            &objMessage, ISession::TERMINATION_REASON_REFRESH_TIMEOUT);
    pFormatter->ReasonHeaderSetter_SetHeader(
            &objMessage, ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT);

    EXPECT_CALL(objMessage, AddHeader(_, _, _)).Times(0);
    //clang-format off
    std::vector<IMS_SINT32> objReasons{ISession::TERMINATION_REASON_INVALID,
            ISession::TERMINATION_REASON_UNKNOWN, ISession::TERMINATION_REASON_USER_ACTION,
            ISession::TERMINATION_REASON_REMOTE_ACTION, ISession::TERMINATION_REASON_REFRESH_408,
            ISession::TERMINATION_REASON_REFRESH_481, ISession::TERMINATION_REASON_SERVICE_CLOSED,
            ISession::TERMINATION_REASON_MAX};
    //clang-format on

    for (IMS_SINT32 eTerminationReason : objReasons)
    {
        pFormatter->ReasonHeaderSetter_SetHeader(&objMessage, eTerminationReason);
    }
}

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetHeaderSetsReasonHeadersByKrConfiguration)
{
    const AString strReasonHeaderName(SipHeaderName::REASON);
    const AString strCarrierSpecificHeader(MessageUtil::STR_P_SKT_BYE_CAUSE);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strCarrierSpecificHeader))
            .WillByDefault(Return(IMS_TRUE));

    const AString strReasonHeaderSip("SIP; cause=103; text=\"Session-Expire\"; fc=9602");
    const AString strReasonHeaderUser("USER; cause=101;text=\"USER triggered\"; fc=9501");
    const AString strReasonHeaderEtc("ETC; cause=104; text=\"Unknown\"; fc=9999");
    const AString strByeCauseNormal("normal");
    const AString strByeCauseNoUpd("no_upd");
    MockISipMessage objMessage;
    EXPECT_CALL(objMessage, AddHeader(ISipHeader::UNKNOWN, strReasonHeaderSip, strReasonHeaderName))
            .Times(2);
    EXPECT_CALL(
            objMessage, AddHeader(ISipHeader::UNKNOWN, strReasonHeaderUser, strReasonHeaderName))
            .Times(1);
    EXPECT_CALL(objMessage, AddHeader(ISipHeader::UNKNOWN, strReasonHeaderEtc, strReasonHeaderName))
            .Times(7);
    EXPECT_CALL(
            objMessage, AddHeader(ISipHeader::UNKNOWN, strByeCauseNormal, strCarrierSpecificHeader))
            .Times(8);
    EXPECT_CALL(
            objMessage, AddHeader(ISipHeader::UNKNOWN, strByeCauseNoUpd, strCarrierSpecificHeader))
            .Times(2);

    //clang-format off
    std::vector<IMS_SINT32> objReasons{ISession::TERMINATION_REASON_INVALID,
            ISession::TERMINATION_REASON_UNKNOWN, ISession::TERMINATION_REASON_USER_ACTION,
            ISession::TERMINATION_REASON_REMOTE_ACTION, ISession::TERMINATION_REASON_REFRESH_408,
            ISession::TERMINATION_REASON_REFRESH_481,
            ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT,
            ISession::TERMINATION_REASON_REFRESH_TIMEOUT,
            ISession::TERMINATION_REASON_SERVICE_CLOSED, ISession::TERMINATION_REASON_MAX};
    //clang-format on

    for (IMS_SINT32 eTerminationReason : objReasons)
    {
        pFormatter->ReasonHeaderSetter_SetHeader(&objMessage, eTerminationReason);
    }
}

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetPrivateHeaderSetsPrivateHeaderByKrConfiguration)
{
    const AString strCarrierSpecificHeader(MessageUtil::STR_P_SKT_BYE_CAUSE);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strCarrierSpecificHeader))
            .WillByDefault(Return(IMS_TRUE));

    const AString strPSktByeCause(MessageUtil::STR_P_SKT_BYE_CAUSE);
    MockISipMessage objOldMessage;
    MockISipMessage objNewMessage;

    ON_CALL(objOldMessage, GetHeader(ISipHeader::UNKNOWN, 0, strPSktByeCause))
            .WillByDefault(Return(AString::ConstEmpty()));
    EXPECT_CALL(objNewMessage, SetHeader(_, _, _)).Times(0);
    pFormatter->ReasonHeaderSetter_SetPrivateHeader(&objOldMessage, &objNewMessage);

    const AString strAnyCause("anyCause");
    ON_CALL(objOldMessage, GetHeader(ISipHeader::UNKNOWN, 0, strPSktByeCause))
            .WillByDefault(Return(strAnyCause));
    EXPECT_CALL(objNewMessage, SetHeader(ISipHeader::UNKNOWN, strAnyCause, strPSktByeCause));

    pFormatter->ReasonHeaderSetter_SetPrivateHeader(&objOldMessage, &objNewMessage);
}

}  // namespace android
