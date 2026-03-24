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
#include "IMtcService.h"
#include "ISipHeader.h"
#include "ImsList.h"
#include "MediaDef.h"
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoLocation.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MockISubscriberConfig.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "call/MockIMtcCallContext.h"
#include "call/message/MessageFormatter.h"
#include "configuration/ConfigDef.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialogevent/MockIMultiEndpointManager.h"
#include "helper/MtcSupplementaryService.h"
#include "service/MockIFeatureCaps.h"
#include "utility/MessageUtil.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AnyOf;
using ::testing::AnyOfArray;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const IMS_SINT32 SLOT_ID = 0;
LOCAL const AString HOME_DOMAIN = "homedomain";
LOCAL const AString PRIVATE_USER_ID = "prid";
LOCAL const AString ACCEPT_CONTACT_MMTEL =
        "*;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"";
LOCAL const AString ACCEPT_CONTACT_VIDEO =
        "*;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\";video";

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
    MockIMessageBodyPart objBodyPart;
    MockISipMessage objSipMessage;
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockISession objSession;
    MockISubscriberConfig objSubscriberConfig;
    MockMtcConfigurationProxy objConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    MockIMessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        pSupplementaryService = new MtcSupplementaryService(objContext, objConfigurationProxy);

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetSubscriberConfig).WillByDefault(Return(&objSubscriberConfig));
        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
        ON_CALL(objSession, GetNextResponse).WillByDefault(Return(&objMessage));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
        ON_CALL(objMessage, CreateBodyPart).WillByDefault(Return(&objBodyPart));

        ON_CALL(objSubscriberConfig, GetHomeDomainName).WillByDefault(ReturnRef(HOME_DOMAIN));
        ON_CALL(objSubscriberConfig, GetPrivateUserId).WillByDefault(ReturnRef(PRIVATE_USER_ID));

        pFormatter = new MessageFormatter(objContext, objSession);
    }

    virtual void TearDown() override
    {
        delete pFormatter;
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

    AString GetRejectPhrase(IMS_SINT32 nCode, IMS_SINT32 nExtraCode = -1)
    {
        CallReasonInfo objReasonInfo(nCode, nExtraCode);
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

TEST_F(MessageFormatterTest, FormStartMessageWithoutGeolocation)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY,
                    static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::INVITE)))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    static_cast<IMS_SINT32>(
                            ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR)))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _)).Times(0);
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::GEOLOCATION_ROUTING), _)).Times(0);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormStartMessageWithGeolocation)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY,
                    static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::INVITE)))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    static_cast<IMS_SINT32>(
                            ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR)))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
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
    const ByteArray objContent = ByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                           "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                           "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                           "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                           "xmlns:gml=\"http://www.opengis.net/gml\" "
                                           "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
                                           "entity=\"pres:prid\">\n"
                                           "<dm:person id=\"\">\n"
                                           "<gp:geopriv>\n"
                                           "<gp:location-info>\n"
                                           "<gs:Circle srsName=\"urn:ogc:def:crs:EPSG::4326\">\n"
                                           "<gml:pos>1 2</gml:pos>\n"
                                           "</gs:Circle>\n"
                                           "</gp:location-info>\n"
                                           "<gp:usage-rules>\n"
                                           "</gp:usage-rules>\n"
                                           "</gp:geopriv>\n"
                                           "</dm:person>\n"
                                           "</presence>\n");

    EXPECT_CALL(objBodyPart, SetContent(objContent));
    EXPECT_CALL(objBodyPart, SetHeader(AString(SipHeaderName::CONTENT_LENGTH), AString("517")));
    EXPECT_CALL(objBodyPart, SetHeader(AString(SipHeaderName::CONTENT_ID), _));
    EXPECT_CALL(objBodyPart,
            SetHeader(AString(SipHeaderName::CONTENT_TYPE), AString("application/pidf+xml")));
    EXPECT_CALL(objBodyPart,
            SetHeader(AString(SipHeaderName::CONTENT_DISPOSITION),
                    AString("render;handling=optional")));

    pSupplementaryService->Add(SuppType::CALL_COMPOSER_LOCATION_LAT, AString("1"));
    pSupplementaryService->Add(SuppType::CALL_COMPOSER_LOCATION_LONG, AString("2"));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(MessageFormatterTest, FormProvisionalResponseMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormProvisionalResponseMessage(IMS_TRUE);

    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY,
                    static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::PROVISIONAL_RESPONSE)))
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

    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY,
                    static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::FINAL_SUCCESS_RESPONSE)))
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

    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY,
                    static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::FINAL_FAILURE_RESPONSE)))
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

TEST_F(MessageFormatterTest, FormRejectMessageWithUnsupported)
{
    const AString strUnsupportedValue("any value");
    CallReasonInfo objReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS, -1, strUnsupportedValue);
    IMS_SINT32 eStatusCode;
    AString strPhrase;

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, strUnsupportedValue, ISipHeader::UNSUPPORTED, _));

    pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);
}

TEST_F(MessageFormatterTest, FormRejectMessageWithUnsupportedSdpHeadersWithIpMismatch)
{
    const AString strWarning("301 IMS-client \"Incompatible network address formats\"");
    IMS_SINT32 eStatusCode;
    AString strPhrase;

    CallReasonInfo objReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS, MEDIA_NEGO_ERROR_IP_MISMATCH);
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, strWarning, ISipHeader::WARNING, _));

    pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);
}

TEST_F(MessageFormatterTest, FormRejectMessageWithUnsupportedSdpHeadersWithInvalidDescriptor)
{
    const AString strWarning("305 IMS-client \"Incompatible media format\"");
    IMS_SINT32 eStatusCode;
    AString strPhrase;

    CallReasonInfo objReasonInfo(
            CODE_REJECT_UNSUPPORTED_SDP_HEADERS, MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, strWarning, ISipHeader::WARNING, _));

    pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);
}

TEST_F(MessageFormatterTest, FormRejectMessageWithMediaNotAcceptableWithNoCodecMatched)
{
    const AString strWarning("304 IMS-client \"Media type not available\"");
    IMS_SINT32 eStatusCode;
    AString strPhrase;

    CallReasonInfo objReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE, MEDIA_NEGO_ERROR_NO_CODEC_MATCHED);
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, strWarning, ISipHeader::WARNING, _));

    pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);
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

TEST_F(MessageFormatterTest, FormTerminateMessageWithCodeUserTerminatedAndSipTimeoutForNormalCall)
{
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    EXPECT_CALL(objConfigurationProxy,
            GetString(ConfigVoice::
                            KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_SIP_RESPONSE_TIMEOUT_STRING))
            .WillOnce(Return(AString("reason")));

    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);
    IMS_RESULT nResult = pFormatter->FormTerminateMessage(objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageFormatterTest,
        FormTerminateMessageWithCodeUserTerminatedAndSipTimeoutForEmergencyCall)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    EXPECT_CALL(objConfigurationProxy,
            GetString(ConfigEmergency::
                            KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_SIP_RESPONSE_TIMEOUT_STRING))
            .WillOnce(Return(AString("reason")));

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
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_SKT_BYE_CAUSE))
            .WillByDefault(Return(IMS_TRUE));

    const AString strByeCauseNormal("normal");
    EXPECT_CALL(objMessageUtils,
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
    ON_CALL(objMessageUtils,
            ContainsValue(&objMessage, AnyOf(SRVCC_FEATURE_A, SRVCC_FEATURE_B, SRVCC_FEATURE_M),
                    ISipHeader::FEATURE_CAPS, AString::ConstNull()))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objFeatureCaps, AddFeature(_, _, _, _)).Times(0);
    pFormatter->FormProvisionalResponseMessage(IMS_TRUE);
}

TEST_F(MessageFormatterTest, AddSrvccFeatureByPrAnswerMessage)
{
    MockICoreService objCoreService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));
    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils,
            ContainsValue(&objMessage, AnyOf(SRVCC_FEATURE_A, SRVCC_FEATURE_B, SRVCC_FEATURE_M),
                    ISipHeader::FEATURE_CAPS, AString::ConstNull()))
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
    IMS_SINT32 nCountOfAnotherSetHeader = 2;
    pSupplementaryService->Delete(SuppType::CALLER_ID);
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, _, AString::ConstNull()))
            .Times(nCountOfAnotherSetHeader);
    pFormatter->FormStartMessage(CallType::VOIP);

    pSupplementaryService->Add(SuppType::CALLER_ID, CALLERID_RESTRICTED);
    ON_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_SESSION_PRIVACY_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::SESSION_PRIVACY_TYPE_HEADER));
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, _, AString::ConstNull()))
            .Times(nCountOfAnotherSetHeader);
    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, AString(MessageUtil::STR_HEADER), ISipHeader::PRIVACY,
                    AString::ConstNull()))
            .Times(1);
    const AString strAnonymousFromHeader("\"Anonymous\" <sip:anonymous@anonymous.invalid>");
    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, strAnonymousFromHeader, ISipHeader::FROM, AString::ConstNull()))
            .Times(1);
    pFormatter->FormStartMessage(CallType::VOIP);

    ON_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_SESSION_PRIVACY_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::SESSION_PRIVACY_TYPE_ID));
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, _, AString::ConstNull()))
            .Times(nCountOfAnotherSetHeader);
    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, AString(MessageUtil::STR_ID), ISipHeader::PRIVACY,
                    AString::ConstNull()))
            .Times(1);
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, ISipHeader::FROM, AString::ConstNull()))
            .Times(1);
    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, SetTipHeader)
{
    EXPECT_CALL(objService, IsPermanentSuppServiceEnabled(PermanentSuppType::TB_TIR))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, AString(MessageUtil::STR_ID), ISipHeader::PRIVACY,
                    AString::ConstNull()))
            .Times(0);
    pFormatter->FormProvisionalResponseMessage(IMS_FALSE);

    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, AString(MessageUtil::STR_ID), ISipHeader::PRIVACY,
                    AString::ConstNull()))
            .Times(1);
    pFormatter->FormProvisionalResponseMessage(IMS_FALSE);
}

TEST_F(MessageFormatterTest, SetPEarlyMediaHeader)
{
    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, _, _, _)).Times(AnyNumber());
    const AString strSupported(MessageUtil::STR_SUPPORTED);
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, strSupported, ISipHeader::P_EARLY_MEDIA, _));
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
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING))
            .WillByDefault(Return(strReason));

    pFormatter->FormTerminateMessage(objReasonInfo);
}

TEST_F(MessageFormatterTest, SetCarrierSpecificHeaders)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_SKT_BYE_CAUSE))
            .WillByDefault(Return(IMS_TRUE));
    pFormatter->FormStartMessage(CallType::VOIP);
    pFormatter->FormAcceptMessage();
    pFormatter->FormUpdateMessage(UpdateType::NORMAL, IMS_TRUE);
    pFormatter->FormAcceptUpdateMessage();
}

TEST_F(MessageFormatterTest, SetCarrierSpecificHeadersSetsTranscodingHeaderIfCallPull)
{
    pSupplementaryService->Add(SuppType::CALL_PULL, IMS_FALSE);
    const AString strTranscodingHeader(MessageUtil::STR_P_COM_ENABLETRANSCODING);

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, _, ISipHeader::UNKNOWN, strTranscodingHeader));
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

    pSupplementaryService->Add(SuppType::CALL_PULL, IMS_FALSE);

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, strReplaces, ISipHeader::REPLACES, _));
    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, GetRejectStatusCode)
{
    const IMS_SINT32 nTestStatusCode = 999;

    EXPECT_EQ(GetRejectStatusCode(CODE_NONE), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_UNSPECIFIED), SipStatusCode::SC_480);
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE_INT))
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
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE_INT))
            .WillByDefault(Return(nTestStatusCode));
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE),
            nTestStatusCode);
    EXPECT_EQ(GetRejectStatusCode(CODE_SIP_REQUEST_PENDING), SipStatusCode::SC_491);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_ONGOING_CALL_UPGRADE), SipStatusCode::SC_486);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_INTERNAL_ERROR), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED),
            SipStatusCode::SC_580);
    EXPECT_EQ(GetRejectStatusCode(CODE_MEDIA_INIT_FAILED), SipStatusCode::SC_480);
    EXPECT_EQ(GetRejectStatusCode(CODE_MEDIA_NOT_ACCEPTABLE), SipStatusCode::SC_488);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_UNSUPPORTED_SDP_HEADERS), SipStatusCode::SC_488);
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER_INT))
            .WillByDefault(Return(nTestStatusCode));
    EXPECT_EQ(GetRejectStatusCode(CODE_TIMEOUT_NO_ANSWER), nTestStatusCode);
    EXPECT_EQ(GetRejectStatusCode(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE), SipStatusCode::SC_603);
    EXPECT_EQ(GetRejectStatusCode(CODE_NETWORK_RESP_TIMEOUT), SipStatusCode::SC_500);
    EXPECT_EQ(GetRejectStatusCode(CODE_BLACKLISTED_CALL_ID), SipStatusCode::SC_603);
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(nTestStatusCode));
    EXPECT_EQ(GetRejectStatusCode(CODE_USER_REJECTED_SESSION_MODIFICATION), nTestStatusCode);
    EXPECT_EQ(GetRejectStatusCode(CODE_ACCESS_CLASS_BLOCKED), SipStatusCode::SC_488);
    EXPECT_EQ(GetRejectStatusCode(CODE_REJECT_CALL_TYPE_NOT_ALLOWED), SipStatusCode::SC_488);
}

TEST_F(MessageFormatterTest, GetRejectPhrase)
{
    const IMS_CHAR pszTestPhrase[] = "TEST_PHRASE";
    std::vector<std::string> expectedKeys = {
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CSCALL_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_VILTE_AND_NO_LTE_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONNECTING_CALL_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_EXCEEDS_MAX_CALL_COUNT_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONVERTING_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_NEGOTIATION_FAILURE_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_NO_ANSWER_BY_USER_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_USER_REJECT_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ACCESS_CLASS_BLOCKED_STRING,
            ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_VOPS_OFF_STRING};

    ON_CALL(objConfigurationProxy, GetString(AnyOfArray(expectedKeys)))
            .WillByDefault(Return(AString(pszTestPhrase)));

    EXPECT_TRUE(GetRejectPhrase(CODE_NONE).GetLength() < 1);
    EXPECT_STREQ(GetRejectPhrase(CODE_REJECT_CALL_TYPE_NOT_ALLOWED).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_USER_DECLINE).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_REJECT_ONGOING_CS_CALL).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_LOCAL_CALL_BUSY).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_REJECT_ONGOING_CALL_SETUP).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_REJECT_MAX_CALL_LIMIT_REACHED).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_TIMEOUT_NO_ANSWER).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_REJECT_ONGOING_CALL_UPGRADE).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_MEDIA_NOT_ACCEPTABLE).GetStr(), pszTestPhrase);
    EXPECT_STREQ(GetRejectPhrase(CODE_ACCESS_CLASS_BLOCKED).GetStr(), pszTestPhrase);
}

TEST_F(MessageFormatterTest, GetRejectPhraseForBusyRttOn)
{
    CallReasonInfo objReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_RTT_ON);

    IMS_SINT32 eStatusCode;
    AString strPhrase;
    pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);

    EXPECT_STREQ("RTT on", strPhrase.GetStr());
}

TEST_F(MessageFormatterTest, GetRejectPhraseForBusyVowifiOff)
{
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING))
            .WillByDefault(Return(AString("VoWiFi OFF")));
    CallReasonInfo objReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF);

    IMS_SINT32 eStatusCode;
    AString strPhrase;
    pFormatter->FormRejectMessage(objReasonInfo, eStatusCode, strPhrase);

    EXPECT_STREQ("VoWiFi OFF", strPhrase.GetStr());
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

    objReasonInfo.nCode = CODE_RADIO_OFF;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_OEM_CAUSE_3;
    pFormatter->FormTerminateMessage(objReasonInfo);

    objReasonInfo.nCode = CODE_MEDIA_NOT_ACCEPTABLE;
    pFormatter->FormTerminateMessage(objReasonInfo);
}

TEST_F(MessageFormatterTest, FormTerminateMessageWithMediaBearerNotMet)
{
    const AString strReason = "SIP;text=\"QOS not met\"";
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_NOT_MET_STRING))
            .WillByDefault(Return(strReason));
    EXPECT_CALL(objContext, IsEstablished()).WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, strReason, ISipHeader::REASON, _))
            .Times(1);

    const CallReasonInfo objReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);
    pFormatter->FormTerminateMessage(objReasonInfo);
}

TEST_F(MessageFormatterTest, FormTerminateMessageWithMediaBearerLoss)
{
    const AString strReason = "USER;text=\"Media bearer loss\"";
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_LOSS_STRING))
            .WillByDefault(Return(strReason));
    EXPECT_CALL(objContext, IsEstablished()).WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, strReason, ISipHeader::REASON, _))
            .Times(1);

    const CallReasonInfo objReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);
    pFormatter->FormTerminateMessage(objReasonInfo);
}

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetHeaderDoesNotSetReasonHeadersByNoConfiguration)
{
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

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetHeaderSetsReasonHeadersByConfiguration1)
{
    const AString strReasonHeaderValue("USER;text=\"Session Expired\"");
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING))
            .WillByDefault(Return(strReasonHeaderValue));
    MockISipMessage objMessage;
    EXPECT_CALL(objMessage, AddHeader(ISipHeader::REASON, strReasonHeaderValue, _)).Times(2);
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

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetHeaderSetsReasonHeadersByConfiguration2)
{
    const AString strCarrierSpecificHeader(MessageUtil::STR_P_SKT_BYE_CAUSE);
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_SKT_BYE_CAUSE))
            .WillByDefault(Return(IMS_TRUE));

    const AString strReasonHeaderSip("SIP; cause=103; text=\"Session-Expire\"; fc=9602");
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING))
            .WillByDefault(Return(strReasonHeaderSip));
    const AString strReasonHeaderUser("USER; cause=101;text=\"USER triggered\"; fc=9501");
    const AString strReasonHeaderEtc("ETC; cause=104; text=\"Unknown\"; fc=9999");
    const AString strByeCauseNormal("normal");
    const AString strByeCauseNoUpd("no_upd");
    MockISipMessage objMessage;
    EXPECT_CALL(objMessage, AddHeader(ISipHeader::REASON, strReasonHeaderSip, _)).Times(2);
    EXPECT_CALL(objMessage, AddHeader(ISipHeader::REASON, strReasonHeaderUser, _)).Times(1);
    EXPECT_CALL(objMessage, AddHeader(ISipHeader::REASON, strReasonHeaderEtc, _)).Times(7);
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

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetPrivateHeaderSetsPrivateHeaderByConfiguration)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_SKT_BYE_CAUSE))
            .WillByDefault(Return(IMS_TRUE));

    const AString strCarrierSpecificByeCause(MessageUtil::STR_P_SKT_BYE_CAUSE);
    MockISipMessage objOldMessage;
    MockISipMessage objNewMessage;

    ON_CALL(objOldMessage, GetHeader(ISipHeader::UNKNOWN, 0, strCarrierSpecificByeCause))
            .WillByDefault(Return(AString::ConstEmpty()));
    EXPECT_CALL(objNewMessage, SetHeader(_, _, _)).Times(0);
    pFormatter->ReasonHeaderSetter_SetPrivateHeader(&objOldMessage, &objNewMessage);

    const AString strAnyCause("anyCause");
    ON_CALL(objOldMessage, GetHeader(ISipHeader::UNKNOWN, 0, strCarrierSpecificByeCause))
            .WillByDefault(Return(strAnyCause));
    EXPECT_CALL(
            objNewMessage, SetHeader(ISipHeader::UNKNOWN, strAnyCause, strCarrierSpecificByeCause));

    pFormatter->ReasonHeaderSetter_SetPrivateHeader(&objOldMessage, &objNewMessage);
}

TEST_F(MessageFormatterTest, ReasonHeaderSetterSetHeaderSetsReasonHeaderForRefreshTimeouts)
{
    const AString strReasonHeaderValue("SIP;cause=408;text=\"Session Timeout\"");
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING))
            .WillByDefault(Return(strReasonHeaderValue));

    MockISipMessage objMessage;
    // Verify that the Reason header is added for both timeout types
    EXPECT_CALL(objMessage, AddHeader(ISipHeader::REASON, strReasonHeaderValue, _)).Times(2);

    pFormatter->ReasonHeaderSetter_SetHeader(
            &objMessage, ISession::TERMINATION_REASON_REFRESH_TIMEOUT);
    pFormatter->ReasonHeaderSetter_SetHeader(
            &objMessage, ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT);

    // Verify that no header is added if the configuration returns an empty string
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING))
            .WillByDefault(Return(AString::ConstEmpty()));
    EXPECT_CALL(objMessage, AddHeader(ISipHeader::REASON, _, _)).Times(0);
    pFormatter->ReasonHeaderSetter_SetHeader(
            &objMessage, ISession::TERMINATION_REASON_REFRESH_TIMEOUT);
}

TEST_F(MessageFormatterTest, SetAcceptContactHeaderWithVideoTagRegardlessCallType)
{
    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigVt::KEY_ADD_VIDEO_FEATURE_TAG_IN_ACCEPT_CONTACT_ALWAYS_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, ACCEPT_CONTACT_VIDEO, ISipHeader::ACCEPT_CONTACT, _))
            .Times(1);

    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, SetAcceptContactHeaderWithoutVideoTagWhenVoipCall)
{
    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigVt::KEY_ADD_VIDEO_FEATURE_TAG_IN_ACCEPT_CONTACT_ALWAYS_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, ACCEPT_CONTACT_MMTEL, ISipHeader::ACCEPT_CONTACT, _))
            .Times(1);

    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(MessageFormatterTest, SetAcceptContactHeaderWithVideoTagWhenVtCall)
{
    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigVt::KEY_ADD_VIDEO_FEATURE_TAG_IN_ACCEPT_CONTACT_ALWAYS_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, ACCEPT_CONTACT_VIDEO, ISipHeader::ACCEPT_CONTACT, _))
            .Times(1);

    pFormatter->FormStartMessage(CallType::VIDEO_RTT);
}
}  // namespace android
