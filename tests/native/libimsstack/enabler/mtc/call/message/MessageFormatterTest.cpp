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
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/message/MessageFormatter.h"
#include "call/MockIMtcSessionContext.h"
#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "FeatureCaps.h"
#include "helper/MtcSupplementaryService.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoLocation.h"
#include "sipcore/MockISipMessage.h"
#include "SipStatusCode.h"
#include "utility/MessageUtil.h"

LOCAL IMS_SINT32 SLOT_ID = 0;

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class MessageFormatterTest : public ::testing::Test
{
public:
    MessageFormatter* pFormatter;

    CallInfo objCallInfo;
    MockIMessage objMessage;
    MockISipMessage objSipMessage;
    MockIMtcSessionContext objContext;
    MockIMtcService objService;
    MockISession objSession;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcExtensionSet* pExtensionSet;
    MtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        pSupplementaryService = new MtcSupplementaryService(*pConfigurationProxy);

        ImsList<IMtcExtension*> lstExtensions;
        pExtensionSet = new MtcExtensionSet(lstExtensions);

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetCallType).WillByDefault(Return(CallType::VOIP));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetExtensionSet).WillByDefault(ReturnRef(*pExtensionSet));
        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
        ON_CALL(objSession, GetNextResponse).WillByDefault(Return(&objMessage));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        pFormatter = new MessageFormatter(objContext, objSession);
    }

    virtual void TearDown() override
    {
        delete pFormatter;
        delete pConfigurationProxy;
        delete pSupplementaryService;
        delete pExtensionSet;
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
    IMS_RESULT nResult = pFormatter->FormStartMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormStartMessage();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, FormStartMessageWithGeolocation)
{
    ON_CALL(*pConfigurationManager, IsMessageTypeSupportGeolocationPidf)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsSupportGeolocationPidfInSipInvite)
            .WillByDefault(Return(IMS_FALSE));

    IMS_RESULT nResult = pFormatter->FormStartMessage();
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(*pConfigurationManager, IsSupportGeolocationPidfInSipInvite)
            .WillByDefault(Return(IMS_TRUE));

    nResult = pFormatter->FormStartMessage();

    EXPECT_EQ(nResult, IMS_SUCCESS);
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

TEST_F(MessageFormatterTest, FormTerminateMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED);
    IMS_RESULT nResult = pFormatter->FormTerminateMessage(objReasonInfo);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageFormatterTest, SetAcceptContactHeader)
{
    ON_CALL(objContext, GetCallType).WillByDefault(Return(CallType::VT));
    pFormatter->FormStartMessage();
}

TEST_F(MessageFormatterTest, AddSrvccFeature)
{
    ON_CALL(objService, GetICoreService).WillByDefault(Return(nullptr));
    pFormatter->FormStartMessage();

    MockICoreService objCoreService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    FeatureCaps* pFeatureCaps = new FeatureCaps();
    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(pFeatureCaps));
    pFormatter->FormStartMessage();

    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(nullptr));
    pFormatter->FormProvisionalResponseMessage(IMS_TRUE);

    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(&objMessage));
    ImsList<AString> lstHeaders;
    lstHeaders.Append(MessageUtil::STR_SRVCC_FEATURE_A);
    lstHeaders.Append(MessageUtil::STR_SRVCC_FEATURE_B);
    lstHeaders.Append(MessageUtil::STR_SRVCC_FEATURE_M);
    ON_CALL(objSipMessage, GetHeaders).WillByDefault(Return(lstHeaders));
    pFormatter->FormProvisionalResponseMessage(IMS_TRUE);

    delete pFeatureCaps;
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
    pFormatter->FormStartMessage();

    pSupplementaryService->Add(SuppType::CALLER_ID, CALLERID_RESTRICTED);
    ON_CALL(*pConfigurationManager, GetSessionPrivacyType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::SESSION_PRIVACY_TYPE_HEADER));
    pFormatter->FormStartMessage();
    ON_CALL(*pConfigurationManager, GetSessionPrivacyType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::SESSION_PRIVACY_TYPE_NONE));
    pFormatter->FormStartMessage();

    pSupplementaryService->Delete(SuppType::CALLER_ID);
    pSupplementaryService->Add(SuppType::CALLER_ID, CALLERID_IDENTITY);
    pFormatter->FormStartMessage();
}

TEST_F(MessageFormatterTest, SetPreconditionHeader)
{
    pFormatter->FormStartMessage();

    MockIMtcExtension* pExtension = new MockIMtcExtension();
    const AString strTag = MtcExtensionSet::OPTION_TAG_PRECONDITION;
    ON_CALL(*pExtension, GetOptionTag).WillByDefault(ReturnRef(strTag));

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);
    MtcExtensionSet* pExtensionSet = new MtcExtensionSet(lstExtensions);
    ON_CALL(objContext, GetExtensionSet).WillByDefault(ReturnRef(*pExtensionSet));

    ON_CALL(*pExtension, IsAvailableOnRemote).WillByDefault(Return(IMS_FALSE));
    pFormatter->FormAcceptMessage();

    ON_CALL(*pExtension, IsAvailableOnRemote).WillByDefault(Return(IMS_TRUE));
    pFormatter->FormStartMessage();
    pFormatter->FormEarlyUpdateMessage(UpdateType::NORMAL);
    pFormatter->FormUpdateMessage(UpdateType::NORMAL, IMS_TRUE);
    pFormatter->FormProvisionalResponseMessage(IMS_TRUE);
    pFormatter->FormPrackResponseMessage();
    pFormatter->FormEarlyUpdateResponseMessage();
    pFormatter->FormAcceptMessage();
    pFormatter->FormAcceptUpdateMessage();

    delete pExtensionSet;
}

TEST_F(MessageFormatterTest, SetPEarlyMediaHeader)
{
    objCallInfo.bUssi = IMS_TRUE;
    pFormatter->FormStartMessage();

    objCallInfo.bUssi = IMS_FALSE;
    pFormatter->FormStartMessage();
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
    pFormatter->FormStartMessage();
    pFormatter->FormAcceptMessage();
    pFormatter->FormUpdateMessage(UpdateType::NORMAL, IMS_TRUE);
    pFormatter->FormAcceptUpdateMessage();
}

TEST_F(MessageFormatterTest, GetRejectStatusCode)
{
    const IMS_SINT32 nTestStatusCode = 999;

    EXPECT_EQ(GetRejectStatusCode(CODE_NONE), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_UNSPECIFIED), SipStatusCode::SC_480);
    ON_CALL(*pConfigurationManager, GetIncomingCallRejectCodeForUserDecline)
            .WillByDefault(Return(nTestStatusCode));
    EXPECT_EQ(GetRejectStatusCode(CODE_USER_DECLINE), nTestStatusCode);
    EXPECT_EQ(GetRejectStatusCode(CODE_USER_NOANSWER), SipStatusCode::SC_603);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOW_BATTERY), SipStatusCode::SC_603);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_CALL_END_UNSPECIFIED), SipStatusCode::SC_603);
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
    EXPECT_EQ(GetRejectStatusCode(CODE_USER_IGNORE), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_UNSUPPORTED_SIP_HEADERS), SipStatusCode::SC_420);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE), SipStatusCode::SC_406);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_SIP_406),
            SipStatusCode::SC_406);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_SIP_488),
            SipStatusCode::SC_488);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_SIP_606),
            SipStatusCode::SC_606);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_ONGOING_CALL_UPDATE), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_SESSION_INTERNAL_ERROR), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED),
            SipStatusCode::SC_580);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_MEDIA_INIT_FAILED), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_MEDIA_NOT_ACCEPTABLE), SipStatusCode::SC_488);
    ON_CALL(*pConfigurationManager, GetIncomingCallRejectCodeForNoAnswer)
            .WillByDefault(Return(nTestStatusCode));
    EXPECT_EQ(GetRejectStatusCode(CODE_TIMEOUT_NO_ANSWER), nTestStatusCode);
    EXPECT_EQ(GetRejectStatusCode(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE), SipStatusCode::SC_603);
    EXPECT_EQ(GetRejectStatusCode(CODE_NETWORK_RESP_TIMEOUT), SipStatusCode::SC_500);
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
    EXPECT_EQ(GetRejectPhrase(CODE_REJECT_ONGOING_CALL_UPDATE), strTestPhrase);
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

    objReasonInfo.nCode = CODE_EARLYDIALOG_FORKED_TERMINATED_INTERNALONLY;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE;
    pFormatter->FormTerminateMessage(objReasonInfo);
}

}  // namespace android
