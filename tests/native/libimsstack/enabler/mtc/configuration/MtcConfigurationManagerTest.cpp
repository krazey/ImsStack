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

#include "CarrierConfig.h"
#include "ICarrierConfig.h"
#include "ImsAosReason.h"
#include "ImsVector.h"
#include "MockICarrierConfig.h"
#include "ServiceConfig.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

LOCAL IMS_SINT32 DEFAULT_SLOT_ID = 0;

class MtcConfigurationManagerTest : public ::testing::Test
{
public:
    MtcConfigurationManager* pManager;

protected:
    virtual void SetUp() override
    {
        pManager = new MtcConfigurationManager();
        pManager->Init();
    }

    virtual void TearDown() override { delete pManager; }

    IMS_BOOL GetBool(IN const IMS_CHAR* pszKey)
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
        return piCc->GetBoolean(pszKey);
    }

    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey)
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
        return piCc->GetInt(pszKey);
    }

    AString GetStr(IN const IMS_CHAR* pszKey)
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
        return piCc->GetString(pszKey);
    }

    ImsVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey)
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
        return piCc->GetIntArray(pszKey);
    }
};

TEST_F(MtcConfigurationManagerTest, GetRequestUriTypeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetRequestUriType(), GetInt(CarrierConfig::Ims::KEY_REQUEST_URI_TYPE_INT));
}

TEST_F(MtcConfigurationManagerTest, IsSupportGeolocationPidfInSipInviteReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI);
    objArray.Push(CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(pManager->IsSupportGeolocationPidfInSipInvite(
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI));
    EXPECT_TRUE(pManager->IsSupportGeolocationPidfInSipInvite(
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI));
    EXPECT_FALSE(pManager->IsSupportGeolocationPidfInSipInvite(
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR));
    EXPECT_FALSE(pManager->IsSupportGeolocationPidfInSipInvite(
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR));
}

TEST_F(MtcConfigurationManagerTest, IsSupportSipSessionIdHeaderReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSupportSipSessionIdHeader(),
            GetBool(CarrierConfig::Ims::KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsIncludeCallerIdServiceCodesInSipInviteReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsIncludeCallerIdServiceCodesInSipInvite(),
            GetBool(CarrierConfig::ImsVoice::
                            KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsMultiendpointSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsMultiendpointSupported(),
            GetBool(CarrierConfig::ImsVoice::KEY_MULTIENDPOINT_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsSessionTimerSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSessionTimerSupported(),
            GetBool(CarrierConfig::ImsVoice::KEY_SESSION_TIMER_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetSessionPrivacyTypeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetSessionPrivacyType(),
            GetInt(CarrierConfig::ImsVoice::KEY_SESSION_PRIVACY_TYPE_INT));
}

TEST_F(MtcConfigurationManagerTest, IsPrackSupportedFor18xReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsPrackSupportedFor18x(),
            GetBool(CarrierConfig::ImsVoice::KEY_PRACK_SUPPORTED_FOR_18X_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetConferenceSubscribeTypeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetConferenceSubscribeType(),
            GetInt(CarrierConfig::ImsVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT));
}

TEST_F(MtcConfigurationManagerTest, IsVoiceQosPreconditionSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsVoiceQosPreconditionSupported(),
            GetBool(CarrierConfig::ImsVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsVoiceOnDefaultBearerSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsVoiceOnDefaultBearerSupported(),
            GetBool(CarrierConfig::ImsVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetDedicatedBearerWaitTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetDedicatedBearerWaitTimer(),
            GetInt(CarrierConfig::ImsVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, IsSrvccTypeReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(CarrierConfig::ImsVoice::BASIC_SRVCC_SUPPORT);
    objArray.Push(CarrierConfig::ImsVoice::MIDCALL_SRVCC_SUPPORT);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig, GetIntArray(CarrierConfig::ImsVoice::KEY_SRVCC_TYPE_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(pManager->IsSrvccType(CarrierConfig::ImsVoice::BASIC_SRVCC_SUPPORT));
    EXPECT_FALSE(pManager->IsSrvccType(CarrierConfig::ImsVoice::ALERTING_SRVCC_SUPPORT));
    EXPECT_FALSE(pManager->IsSrvccType(CarrierConfig::ImsVoice::PREALERTING_SRVCC_SUPPORT));
    EXPECT_TRUE(pManager->IsSrvccType(CarrierConfig::ImsVoice::MIDCALL_SRVCC_SUPPORT));
}

TEST_F(MtcConfigurationManagerTest, GetRingingTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetRingingTimer(),
            GetInt(CarrierConfig::ImsVoice::KEY_RINGING_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetRingbackTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetRingbackTimer(),
            GetInt(CarrierConfig::ImsVoice::KEY_RINGBACK_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetConferenceFactoryUriReturnsValueInCarrierConfig)
{
    EXPECT_STREQ(pManager->GetConferenceFactoryUri().GetStr(),
            GetStr(CarrierConfig::ImsVoice::KEY_CONFERENCE_FACTORY_URI_STRING).GetStr());
}

TEST_F(MtcConfigurationManagerTest, IsOipSourceFromHeaderReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsOipSourceFromHeader(),
            GetBool(CarrierConfig::ImsVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetMoCallRequestTimeoutReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetMoCallRequestTimeout(),
            GetInt(CarrierConfig::ImsVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, IsAudioInactivityCallEndReasonReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(CarrierConfig::Ims::RTCP_INACTIVITY_ON_HOLD);
    objArray.Push(CarrierConfig::Ims::RTCP_INACTIVITY_ON_CONNECTED);
    objArray.Push(CarrierConfig::Ims::RTP_INACTIVITY_ON_CONNECTED);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(
            pManager->IsAudioInactivityCallEndReason(CarrierConfig::Ims::RTCP_INACTIVITY_ON_HOLD));
    EXPECT_TRUE(pManager->IsAudioInactivityCallEndReason(
            CarrierConfig::Ims::RTCP_INACTIVITY_ON_CONNECTED));
    EXPECT_TRUE(pManager->IsAudioInactivityCallEndReason(
            CarrierConfig::Ims::RTP_INACTIVITY_ON_CONNECTED));
    EXPECT_FALSE(pManager->IsAudioInactivityCallEndReason(
            CarrierConfig::Ims::E911_RTCP_INACTIVITY_ON_CONNECTED));
    EXPECT_FALSE(pManager->IsAudioInactivityCallEndReason(
            CarrierConfig::Ims::E911_RTP_INACTIVITY_ON_CONNECTED));
}

TEST_F(MtcConfigurationManagerTest, Get18xTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->Get18xTimer(), GetInt(CarrierConfig::ImsVoice::KEY_18X_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, IsSupportConferenceReferSubscribeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSupportConferenceReferSubscribe(),
            GetBool(CarrierConfig::ImsVoice::KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsEnableConferenceSubscribeByParticipantReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEnableConferenceSubscribeByParticipant(),
            GetBool(CarrierConfig::ImsVoice::KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetConferenceSipFlowOrderReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetConferenceSipFlowOrder(),
            GetInt(CarrierConfig::ImsVoice::KEY_CONFERENCE_SIP_FLOW_ORDER_INT));
}

TEST_F(MtcConfigurationManagerTest, GetConferenceInvitingReferTypeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetConferenceInvitingReferType(),
            GetInt(CarrierConfig::ImsVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyQosPreconditionMechanismWhileCallModificationReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyQosPreconditionMechanismWhileCallModification(),
            GetInt(CarrierConfig::ImsVoice::
                            KEY_POLICY_QOS_PRECONDITION_MECHANISM_WHILE_CALL_MODIFICATION_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetIncomingCallRejectCodeForUserDeclineReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetIncomingCallRejectCodeForUserDecline(),
            GetInt(CarrierConfig::ImsVoice::KEY_INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE_INT));
}

TEST_F(MtcConfigurationManagerTest, GetIncomingCallRejectCodeForNoAnswerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetIncomingCallRejectCodeForNoAnswer(),
            GetInt(CarrierConfig::ImsVoice::KEY_INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER_INT));
}

TEST_F(MtcConfigurationManagerTest, GetPrackUpdateResponseWaitTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPrackUpdateResponseWaitTimer(),
            GetInt(CarrierConfig::ImsVoice::KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetSessionRefreshTriggerIntervalReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetSessionRefreshTriggerInterval(),
            GetInt(CarrierConfig::ImsVoice::KEY_SESSION_REFRESH_TRIGGER_INTERVAL_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetRegistrationRestorationModeOn504ForInviteReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetRegistrationRestorationModeOn504ForInvite(),
            GetInt(CarrierConfig::ImsVoice::
                            KEY_REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE_INT));
}

TEST_F(MtcConfigurationManagerTest, GetPolicyOnAudioQosDeactivationReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyOnAudioQosDeactivation(),
            GetInt(CarrierConfig::ImsVoice::KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT));
}

TEST_F(MtcConfigurationManagerTest, IsEnableSendReinviteOnRatChangeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEnableSendReinviteOnRatChange(),
            GetBool(CarrierConfig::ImsVoice::KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyForMediaTypeRestrictionOnCellularReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForMediaTypeRestrictionOnCellular(),
            GetInt(CarrierConfig::ImsVoice::KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyForMediaTypeRestrictionOnCellularInRoamingReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForMediaTypeRestrictionOnCellularInRoaming(),
            GetInt(CarrierConfig::ImsVoice::
                            KEY_POLICY_FOR_MEDIA_TYPE_RESTRICTION_ON_CELLULAR_IN_ROAMING_INT));
}

TEST_F(MtcConfigurationManagerTest, GetPolicyOfLocalNumbersReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyOfLocalNumbers(),
            GetInt(CarrierConfig::ImsVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT));
}

TEST_F(MtcConfigurationManagerTest,
        IsDefaultEpsBearerContextUsageRestrictionOnCellularReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsDefaultEpsBearerContextUsageRestrictionOnCellular(),
            GetBool(CarrierConfig::ImsVoice::
                            KEY_DEFAULT_EPS_BEARER_CONTEXT_USAGE_RESTRICTION_ON_CELLULAR_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetSilentRedialIntervalReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetSilentRedialInterval(),
            GetInt(CarrierConfig::ImsVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetCallTypeAfterAudioAndVideoCallMergedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetCallTypeAfterAudioAndVideoCallMerged(),
            GetInt(CarrierConfig::ImsVoice::KEY_CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED_INT));
}

TEST_F(MtcConfigurationManagerTest, IsShortCallCodeReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(123);
    objArray.Push(456);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_SHORT_CALL_CODE_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(pManager->IsShortCallCode(123));
    EXPECT_TRUE(pManager->IsShortCallCode(456));
    EXPECT_FALSE(pManager->IsShortCallCode(789));
}

TEST_F(MtcConfigurationManagerTest,
        IsAllowMultipleCallIncludingVideoCallReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsAllowMultipleCallIncludingVideoCall(),
            GetBool(CarrierConfig::ImsVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsRejectCodeForCsfbReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(380);
    objArray.Push(488);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_REJECT_CODE_FOR_CSFB_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(pManager->IsRejectCodeForCsfb(380));
    EXPECT_TRUE(pManager->IsRejectCodeForCsfb(488));
    EXPECT_FALSE(pManager->IsRejectCodeForCsfb(503));
    EXPECT_FALSE(pManager->IsRejectCodeForCsfb(403));
}

TEST_F(MtcConfigurationManagerTest, GetSilentRedialMaxRetryCountReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetSilentRedialMaxRetryCount(),
            GetInt(CarrierConfig::ImsVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT));
}

TEST_F(MtcConfigurationManagerTest, GetPolicyFor403ResponseForInviteReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyFor403ResponseForInvite(),
            GetInt(CarrierConfig::ImsVoice::KEY_POLICY_FOR_403_RESPONSE_FOR_INVITE_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyForCheckingQosWhileCallUpgradingReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForCheckingQosWhileCallUpgrading(),
            GetInt(CarrierConfig::ImsVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT));
}

TEST_F(MtcConfigurationManagerTest, IsRejectOfferlessInviteReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsRejectOfferlessInvite(),
            GetBool(CarrierConfig::ImsVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetCallMaxCountReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetCallMaxCount(), GetInt(CarrierConfig::ImsVoice::KEY_CALL_MAX_COUNT_INT));
}

TEST_F(MtcConfigurationManagerTest, GetCallTerminateReasonHeaderReturnsValueInCarrierConfig)
{
    AString strReason0 = "user ends call";
    AString strReason1 = "rtp timeout";
    AString strReason2 = "user ends and rtp timeout";
    AString strReason3 = "media bearer loss";
    AString strReason4 = "sip timeout";
    AString strReason5 = "sip response timeout";
    AString strReason6 = "user ends and sip response timeout";
    AString strReason7 = "call setup timeout";
    AString strReason8 = "terminating early dialog";
    AString strReason9 = "vops off";
    AString strReason10 = "session refresh failure";
    AString strReason11 = "conference call joined";

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetString(
                    CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING,
                    _))
            .WillByDefault(Return(strReason0));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_RTP_TIMEOUT_STRING,
                    _))
            .WillByDefault(Return(strReason1));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_RTP_TIMEOUT_STRING,
                    _))
            .WillByDefault(Return(strReason2));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_LOSS_STRING,
                    _))
            .WillByDefault(Return(strReason3));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_SIP_TIMEOUT_STRING,
                    _))
            .WillByDefault(Return(strReason4));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_TERMINATE_REASON_HEADER_SIP_RESPONSE_TIMEOUT_STRING,
                    _))
            .WillByDefault(Return(strReason5));
    ON_CALL(*piMockCarrierConfig,
            GetString(
                    CarrierConfig::ImsVoice::
                            KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_SIP_RESPONSE_TIMEOUT_STRING,
                    _))
            .WillByDefault(Return(strReason6));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_TERMINATE_REASON_HEADER_CALL_SETUP_TIMEOUT_STRING,
                    _))
            .WillByDefault(Return(strReason7));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_TERMINATE_REASON_HEADER_TERMINATING_EARLYDIALOG_STRING,
                    _))
            .WillByDefault(Return(strReason8));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::KEY_CALL_TERMINATE_REASON_HEADER_VOPS_OFF_STRING, _))
            .WillByDefault(Return(strReason9));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING,
                    _))
            .WillByDefault(Return(strReason10));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_TERMINATE_REASON_HEADER_CONFERENCE_CALL_JOINED_STRING,
                    _))
            .WillByDefault(Return(strReason11));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_STREQ(pManager->GetCallTerminateReasonHeader(TerminateType::USER_ENDS_CALL).GetStr(),
            strReason0.GetStr());
    EXPECT_STREQ(pManager->GetCallTerminateReasonHeader(TerminateType::RTP_TIMEOUT).GetStr(),
            strReason1.GetStr());
    EXPECT_STREQ(
            pManager->GetCallTerminateReasonHeader(TerminateType::USER_ENDS_CALL_AND_RTP_TIMEOUT)
                    .GetStr(),
            strReason2.GetStr());
    EXPECT_STREQ(pManager->GetCallTerminateReasonHeader(TerminateType::MEDIA_BEARER_LOSS).GetStr(),
            strReason3.GetStr());
    EXPECT_STREQ(pManager->GetCallTerminateReasonHeader(TerminateType::SIP_TIMEOUT).GetStr(),
            strReason4.GetStr());
    EXPECT_STREQ(
            pManager->GetCallTerminateReasonHeader(TerminateType::SIP_RESPONSE_TIMEOUT).GetStr(),
            strReason5.GetStr());
    EXPECT_STREQ(pManager->GetCallTerminateReasonHeader(
                                 TerminateType::USER_ENDS_AND_SIP_RESPONSE_TIMEOUT)
                         .GetStr(),
            strReason6.GetStr());
    EXPECT_STREQ(pManager->GetCallTerminateReasonHeader(TerminateType::CALL_SETUP_TIMEOUT).GetStr(),
            strReason7.GetStr());
    EXPECT_STREQ(pManager->GetCallTerminateReasonHeader(TerminateType::TERMINATING_EARLY_DIALOG)
                         .GetStr(),
            strReason8.GetStr());
    EXPECT_STREQ(pManager->GetCallTerminateReasonHeader(TerminateType::VOPS_OFF).GetStr(),
            strReason9.GetStr());
    EXPECT_STREQ(
            pManager->GetCallTerminateReasonHeader(TerminateType::SESSION_REFRESH_FAILURE).GetStr(),
            strReason10.GetStr());
    EXPECT_STREQ(
            pManager->GetCallTerminateReasonHeader(TerminateType::CONFERENCE_CALL_JOINED).GetStr(),
            strReason11.GetStr());
}

TEST_F(MtcConfigurationManagerTest, GetCallRejectReasonPhraseReturnsValueInCarrierConfig)
{
    AString strReason0 = "on cs call";
    AString strReason1 = "on vilte and no lte";
    AString strReason2 = "on connecting call";
    AString strReason3 = "exceeds max call";
    AString strReason4 = "on converting";
    AString strReasonEmpty = AString::ConstNull();

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CSCALL_STRING, _))
            .WillByDefault(Return(strReason0));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_REJECT_REASON_PHRASE_ON_VILTE_AND_NO_LTE_STRING,
                    _))
            .WillByDefault(Return(strReason1));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_REJECT_REASON_PHRASE_ON_CONNECTING_CALL_STRING,
                    _))
            .WillByDefault(Return(strReason2));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_REJECT_REASON_PHRASE_EXCEEDS_MAX_CALL_COUNT_STRING,
                    _))
            .WillByDefault(Return(strReason3));
    ON_CALL(*piMockCarrierConfig,
            GetString(
                    CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONVERTING_STRING, _))
            .WillByDefault(Return(strReason4));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::
                              KEY_CALL_REJECT_REASON_PHRASE_NEGOTIATION_FAILURE_STRING,
                    _))
            .WillByDefault(Return(strReasonEmpty));
    ON_CALL(*piMockCarrierConfig,
            GetString(
                    CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_NO_ANSWER_BY_USER_STRING,
                    _))
            .WillByDefault(Return(strReasonEmpty));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING, _))
            .WillByDefault(Return(strReasonEmpty));
    ON_CALL(*piMockCarrierConfig,
            GetString(CarrierConfig::ImsVoice::KEY_CALL_REJECT_REASON_PHRASE_USER_REJECT_STRING, _))
            .WillByDefault(Return(strReasonEmpty));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::ON_CS_CALL).GetStr(),
            strReason0.GetStr());
    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::ON_VI_LTE_AND_NO_LTE).GetStr(),
            strReason1.GetStr());
    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::ON_CONNECTING_CALL).GetStr(),
            strReason2.GetStr());
    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::EXCEEDS_MAX_CALL).GetStr(),
            strReason3.GetStr());
    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::ON_CONVERTING).GetStr(),
            strReason4.GetStr());
    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::NEGOTIATION_FAILURE).GetStr(),
            strReasonEmpty.GetStr());
    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::NO_ANSWER_BY_USER).GetStr(),
            strReasonEmpty.GetStr());
    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::VOWIFI_OFF).GetStr(),
            strReasonEmpty.GetStr());
    EXPECT_STREQ(pManager->GetCallRejectReasonPhrase(RejectType::USER_REJECT).GetStr(),
            strReasonEmpty.GetStr());
}

TEST_F(MtcConfigurationManagerTest, IsVideoOnDefaultBearerSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsVideoOnDefaultBearerSupported(),
            GetBool(CarrierConfig::ImsVt::KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsVideoQosPreconditionSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsVideoQosPreconditionSupported(),
            GetBool(CarrierConfig::ImsVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetConvertRemoteResponseTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetConvertRemoteResponseTimer(),
            GetInt(CarrierConfig::ImsVt::KEY_CONVERT_REMOTE_RESPONSE_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetConvertUserResponseTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetConvertUserResponseTimer(),
            GetInt(CarrierConfig::ImsVt::KEY_CONVERT_USER_RESPONSE_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetPolicyOnVideoQosDeactivationReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyOnVideoQosDeactivation(),
            GetInt(CarrierConfig::ImsVt::KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT));
}

TEST_F(MtcConfigurationManagerTest, IsSupportEarlySessionReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSupportEarlySession(),
            GetBool(CarrierConfig::ImsVt::KEY_SUPPORT_EARLY_SESSION_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetPolicyForTextWithVideoReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForTextWithVideo(),
            GetInt(CarrierConfig::ImsVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetMinimumBatteryLevelForLimitVideoCallReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetMinimumBatteryLevelForLimitVideoCall(),
            GetInt(CarrierConfig::ImsVt::KEY_MINIMUM_BATTERY_LEVEL_FOR_LIMIT_VIDEO_CALL_INT));
}

TEST_F(MtcConfigurationManagerTest, IsTextOnDefaultBearerSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsTextOnDefaultBearerSupported(),
            GetBool(CarrierConfig::ImsRtt::KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsTextQosPreconditionSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsTextQosPreconditionSupported(),
            GetBool(CarrierConfig::ImsRtt::KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetPolicyOnTextQosDeactivationReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyOnTextQosDeactivation(),
            GetInt(CarrierConfig::ImsRtt::KEY_POLICY_ON_TEXT_QOS_DEACTIVATION_INT));
}

TEST_F(MtcConfigurationManagerTest, IsPidfShortCodeReturnsValueInCarrierConfig)
{
    AString strCode1 = "123";
    AString strCode2 = "456";
    AString strCode3 = "789";
    AString strCode4 = "123456";

    ImsVector<AString> objArray;
    objArray.Push(strCode1);
    objArray.Push(strCode2);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetStringArray(CarrierConfig::ImsWfc::KEY_PIDF_SHORT_CODE_STRING_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(pManager->IsPidfShortCode(strCode1));
    EXPECT_TRUE(pManager->IsPidfShortCode(strCode2));
    EXPECT_FALSE(pManager->IsPidfShortCode(strCode3));
    EXPECT_FALSE(pManager->IsPidfShortCode(strCode4));
}

TEST_F(MtcConfigurationManagerTest, IsEmergencyCallOverEmergencyPdnReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEmergencyCallOverEmergencyPdn(),
            GetBool(CarrierConfig::ImsWfc::KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetCountryCodeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetCountryCode(), GetInt(CarrierConfig::ImsWfc::KEY_COUNTRY_CODE_INT));
}

TEST_F(MtcConfigurationManagerTest, IsRetryEmergencyOnImsPdnBoolReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsRetryEmergencyOnImsPdnBool(),
            GetBool(CarrierConfig::ImsEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsEmergencyQosPreconditionSupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEmergencyQosPreconditionSupported(),
            GetBool(CarrierConfig::ImsEmergency::KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsEmergencyCallOverEmergencyPdnOnCellularReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEmergencyCallOverEmergencyPdnOnCellular(),
            GetBool(CarrierConfig::ImsEmergency::
                            KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_ON_CELLULAR_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCallReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall(),
            GetBool(CarrierConfig::ImsEmergency::
                            KEY_EMERGENCY_RETRY_WITHOUT_CHECKING_380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetEmergencyTCallTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetEmergencyTCallTimer(),
            GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_TCALL_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetEmergencyRingbackTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetEmergencyRingbackTimer(),
            GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_RINGBACK_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetEmergency18xTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetEmergency18xTimer(),
            GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_18X_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetPolicyForEmergencyUrnEscvMappingReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForEmergencyUrnEscvMapping(),
            GetInt(CarrierConfig::ImsEmergency::KEY_POLICY_FOR_EMERGENCY_URN_ESCV_MAPPING_INT));
}

TEST_F(MtcConfigurationManagerTest, IsCheckConferenceEventPackageVersionReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsCheckConferenceEventPackageVersion(),
            GetBool(CarrierConfig::Assets::KEY_CHECK_CONFERENCE_EVENT_PACKAGE_VERSION_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsConferenceReferToUriSourcePaidReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsConferenceReferToUriSourcePaid(),
            GetBool(CarrierConfig::Assets::KEY_CONFERENCE_REFER_TO_URI_SOURCE_PAID_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        GetConferenceDropReferToUriSourceTypeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetConferenceDropReferToUriSourceType(),
            GetInt(CarrierConfig::Assets::KEY_CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE_INT));
}

TEST_F(MtcConfigurationManagerTest, IsEnableFakeQosCallFlowOnWifiReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEnableFakeQosCallFlowOnWifi(),
            GetBool(CarrierConfig::Assets::KEY_ENABLE_FAKE_QOS_CALL_FLOW_ON_WIFI_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetMediaTypeForOfferlessReinviteReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetMediaTypeForOfferlessReinvite(),
            GetInt(CarrierConfig::Assets::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT));
}

TEST_F(MtcConfigurationManagerTest,
        IsSupportVideoCallUpgradeRegardlessOfFeatureTagsReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSupportVideoCallUpgradeRegardlessOfFeatureTags(),
            GetBool(CarrierConfig::Assets::
                            KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetOipTypeForUnavailableReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetOipTypeForUnavailable(),
            GetInt(CarrierConfig::Assets::KEY_OIP_TYPE_FOR_UNAVAILABLE_INT));
}

TEST_F(MtcConfigurationManagerTest, IsEnableOipHeaderPolicyFallBackReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEnableOipHeaderPolicyFallBack(),
            GetBool(CarrierConfig::Assets::KEY_ENABLE_OIP_HEADER_POLICY_FALLBACK_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetEmergencyRttGuardTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetEmergencyRttGuardTimer(),
            GetInt(CarrierConfig::Assets::KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest,
        IsRetryEmergencyCallOverEmergencyPdnWithNextPcscfReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf(),
            GetBool(CarrierConfig::Assets::
                            KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetPreAlertingTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPreAlertingTimer(),
            GetInt(CarrierConfig::Assets::KEY_PREALERTING_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyForTcallTimerExpiryOfVolteCallReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForTcallTimerExpiryOfVolteCall(),
            GetInt(CarrierConfig::Assets::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyForTcallTimerExpiryOfVolteEmergencyCallReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForTcallTimerExpiryOfVolteEmergencyCall(),
            GetInt(CarrierConfig::Assets::
                            KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyForTcallTimerExpiryOfVowifiCallReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForTcallTimerExpiryOfVowifiCall(),
            GetInt(CarrierConfig::Assets::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT));
}

TEST_F(MtcConfigurationManagerTest, IsCarrierSpecificSipHeaderReturnsValueInCarrierConfig)
{
    AString strHeader1 = "Test-Header1";
    AString strHeader2 = "Test-Header2";
    AString strHeader3 = "Test-Header3";
    AString strHeader4 = "Test-Header4";

    ImsVector<AString> objArray;
    objArray.Push(strHeader1);
    objArray.Push(strHeader2);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetStringArray(CarrierConfig::Assets::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(pManager->IsCarrierSpecificSipHeader(strHeader1));
    EXPECT_TRUE(pManager->IsCarrierSpecificSipHeader(strHeader2));
    EXPECT_FALSE(pManager->IsCarrierSpecificSipHeader(strHeader3));
    EXPECT_FALSE(pManager->IsCarrierSpecificSipHeader(strHeader4));
}

TEST_F(MtcConfigurationManagerTest,
        IsCheckAvchangeFeatureForCallConvertingCapabilityReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsCheckAvchangeFeatureForCallConvertingCapability(),
            GetBool(CarrierConfig::Assets::
                            KEY_CHECK_AVCHANGE_FEATURE_FOR_CALL_CONVERTING_CAPABILITY_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsSupportRegistrationRecoveryForFailureOfSessionRefreshReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSupportRegistrationRecoveryForFailureOfSessionRefresh(),
            GetBool(CarrierConfig::Assets::
                            KEY_SUPPORT_REGISTRATION_RECOVERY_FOR_FAILURE_OF_SESSION_REFRESH_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsCallMaintainingOnRegistrationSuspendedReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(ImsAosReason::SUSPEND_NO_RAT_COVERAGE);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_POLICY_FOR_CALL_MAINTAINING_ON_REGISTRATION_SUSPENDED_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_FALSE(pManager->IsCallMaintainingOnRegistrationSuspended(ImsAosReason::SUSPEND_NONE));
    EXPECT_FALSE(pManager->IsCallMaintainingOnRegistrationSuspended(
            ImsAosReason::SUSPEND_OUT_OF_SERVICE));
    EXPECT_TRUE(pManager->IsCallMaintainingOnRegistrationSuspended(
            ImsAosReason::SUSPEND_NO_RAT_COVERAGE));
}

TEST_F(MtcConfigurationManagerTest,
        IsRequiringEmergencyCallWhenVideoEmergencyCallFailedReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(488);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_POLICY_FOR_REQUIRING_EMERGENCY_CALL_WHEN_VIDEO_EMERGENCY_CALL_FAILED_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_FALSE(pManager->IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(380));
    EXPECT_FALSE(pManager->IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(404));
    EXPECT_TRUE(pManager->IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(488));
    EXPECT_FALSE(pManager->IsRequiringEmergencyCallWhenVideoEmergencyCallFailed(503));
}

TEST_F(MtcConfigurationManagerTest, IsUseMcidSupplementaryServiceReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsUseMcidSupplementaryService(),
            GetBool(CarrierConfig::Assets::KEY_USE_MCID_SUPPLEMENTARY_SERVICE_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsUseMmcSupplementaryServiceReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsUseMmcSupplementaryService(),
            GetBool(CarrierConfig::Assets::KEY_USE_MMC_SUPPLEMENTARY_SERVICE_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsUseLtePreferredStatusForServiceCapabilityReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsUseLtePreferredStatusForServiceCapability(),
            GetBool(CarrierConfig::Assets::
                            KEY_USE_LTE_PREFERRED_STATUS_FOR_SERVICE_CAPABILITY_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsAllowIncomingHoldRequestDuringConferenceCallReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsAllowIncomingHoldRequestDuringConferenceCall(),
            GetBool(CarrierConfig::Assets::
                            KEY_ALLOW_INCOMING_HOLD_REQUEST_DURING_CONFERENCE_CALL_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsIgnore180After183ResponseReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsIgnore180After183Response(),
            GetBool(CarrierConfig::Assets::KEY_IGNORE_180_AFTER_183_RESPONSE_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsAddReplaceHeaderForConferenceReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsAddReplaceHeaderForConference(),
            GetBool(CarrierConfig::Assets::KEY_ADD_REPLACE_HEADER_FOR_CONFERENCE_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsVilteToVolteRetryFailureResponseCodeReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(488);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_VILTE_TO_VOLTE_RETRY_FAILURE_RESPONSE_CODE_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_FALSE(pManager->IsVilteToVolteRetryFailureResponseCode(480));
    EXPECT_TRUE(pManager->IsVilteToVolteRetryFailureResponseCode(488));
    EXPECT_FALSE(pManager->IsVilteToVolteRetryFailureResponseCode(503));
}

TEST_F(MtcConfigurationManagerTest,
        IsUseEmergencyNumberTranslationInRoamingStatusReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsUseEmergencyNumberTranslationInRoamingStatus(),
            GetBool(CarrierConfig::Assets::
                            KEY_USE_EMERGENCY_NUMBER_TRANSLATION_IN_ROAMING_STATUS_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsIgnorePrackDeliveryFailureReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsIgnorePrackDeliveryFailure(),
            GetBool(CarrierConfig::Assets::KEY_IGNORE_PRACK_DELIVERY_FAILURE_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsSupportVideoCallOnlyInVopsOffStatusReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSupportVideoCallOnlyInVopsOffStatus(),
            GetBool(CarrierConfig::Assets::KEY_SUPPORT_VIDEO_CALL_ONLY_IN_VOPS_OFF_STATUS_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsBlockWifiEmergencyCallIfNotProvisionedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsBlockWifiEmergencyCallIfNotProvisioned(),
            GetBool(CarrierConfig::Assets::KEY_BLOCK_WIFI_EMERGENCY_CALL_IF_NOT_PROVISIONED_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsRegistrationDisconnectReasonToTerminateOngoingCallReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(ImsAosReason::POWER_OFF);
    objArray.Push(ImsAosReason::OUT_OF_SERVICE);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_FALSE(
            pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(ImsAosReason::NONE));
    EXPECT_TRUE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::POWER_OFF));
    EXPECT_FALSE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::DATA_DISCONNECTED));
    EXPECT_FALSE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::NO_RAT_COVERAGE));
    EXPECT_TRUE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::OUT_OF_SERVICE));
    EXPECT_FALSE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::REG_TERMINATED));
}

TEST_F(MtcConfigurationManagerTest,
        IsRegistrationDisconnectReasonToTerminateOngoingCallReturnsTrueBy999)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(999);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::POWER_OFF));
    EXPECT_TRUE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::DATA_DISCONNECTED));
    EXPECT_TRUE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::NO_RAT_COVERAGE));
    EXPECT_TRUE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::OUT_OF_SERVICE));
    EXPECT_TRUE(pManager->IsRegistrationDisconnectReasonToTerminateOngoingCall(
            ImsAosReason::REG_TERMINATED));
}

TEST_F(MtcConfigurationManagerTest, GetWifiEmergency18xTimerReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetWifiEmergency18xTimer(),
            GetInt(CarrierConfig::Assets::KEY_WIFI_EMERGENCY_18X_TIMER_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, IsSupportCanidInfoReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSupportCanidInfo(),
            GetBool(CarrierConfig::Assets::KEY_SUPPORT_CANID_INFO_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsUseCarrierSpecificContactHeaderForOptionsResponseReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsUseCarrierSpecificContactHeaderForOptionsResponse(),
            GetBool(CarrierConfig::Assets::
                            KEY_USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistrationReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration(),
            GetBool(CarrierConfig::Assets::
                            KEY_USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsEnableRegistrationRecoveryWhenCallRejectedByServerErrorReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEnableRegistrationRecoveryWhenCallRejectedByServerError(),
            GetBool(CarrierConfig::Assets::
                            KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_REJECTED_BY_SERVER_ERROR_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsEnableRegistrationRecoveryWhenCallRetryUnavailableReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEnableRegistrationRecoveryWhenCallRetryUnavailable(),
            GetBool(CarrierConfig::Assets::
                            KEY_ENABLE_REGISTRATION_RECOVERY_WHEN_CALL_RETRY_UNAVAILABLE_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsRejectVowifiVoiceCallWhenVowifiSettingOffReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsRejectVowifiVoiceCallWhenVowifiSettingOff(),
            GetBool(CarrierConfig::Assets::
                            KEY_REJECT_VOWIFI_VOICE_CALL_WHEN_VOWIFI_SETTING_OFF_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsCheckServerOutageReasonForVxlteCallReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsCheckServerOutageReasonForVxlteCall(),
            GetBool(CarrierConfig::Assets::KEY_CHECK_SERVER_OUTAGE_REASON_FOR_VXLTE_CALL_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionTypeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsSetVideoTextFeatureExclusivelyInContactHeaderBySessionType(),
            GetBool(CarrierConfig::Assets::
                            KEY_SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        GetMaximumWaitTimerForGeolocationPidfInfoReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetMaximumWaitTimerForGeolocationPidfInfo(),
            GetInt(CarrierConfig::Assets::
                            KEY_MAXIMUM_WAIT_TIMER_FOR_GEOLOCATION_PIDF_INFO_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest,
        IsMaintainMultipleEarlySessionsByForkingReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsMaintainMultipleEarlySessionsByForking(),
            GetBool(CarrierConfig::Assets::KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsStopRingbackTimerBy183WithSdpBodyReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsStopRingbackTimerBy183WithSdpBody(),
            GetBool(CarrierConfig::Assets::KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL));
}

TEST_F(MtcConfigurationManagerTest, GetInformationLevelOfGeolocationPidfReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_LAT_AND_LONG);
    objArray.Push(CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_LAT_AND_LONG_AND_CIVIC);
    objArray.Push(CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY);
    objArray.Push(CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY);

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_EQ(pManager->GetInformationLevelOfGeolocationPidf(IMS_TRUE, IMS_FALSE, IMS_TRUE),
            CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_LAT_AND_LONG);
    EXPECT_EQ(pManager->GetInformationLevelOfGeolocationPidf(IMS_TRUE, IMS_TRUE, IMS_TRUE),
            CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_LAT_AND_LONG_AND_CIVIC);
    EXPECT_EQ(pManager->GetInformationLevelOfGeolocationPidf(IMS_FALSE, IMS_FALSE, IMS_TRUE),
            CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY);
    EXPECT_EQ(pManager->GetInformationLevelOfGeolocationPidf(IMS_FALSE, IMS_TRUE, IMS_TRUE),
            CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_AND_STATE);
    EXPECT_EQ(pManager->GetInformationLevelOfGeolocationPidf(IMS_FALSE, IMS_TRUE, IMS_FALSE),
            CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY);
}

TEST_F(MtcConfigurationManagerTest, IsMessageTypeSupportGeolocationPidfReturnsValueInCarrierConfig)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::INVITE));
    objArray.Push(static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::FINAL_SUCCESS_RESPONSE));

    MockICarrierConfig* piMockCarrierConfig = new MockICarrierConfig();
    ON_CALL(*piMockCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY))
            .WillByDefault(Return(objArray));

    pManager->UpdateFullConfig(piMockCarrierConfig);

    EXPECT_TRUE(
            pManager->IsMessageTypeSupportGeolocationPidf(MessageTypeForGeolocationPidf::INVITE));
    EXPECT_FALSE(pManager->IsMessageTypeSupportGeolocationPidf(
            MessageTypeForGeolocationPidf::PROVISIONAL_RESPONSE));
    EXPECT_TRUE(pManager->IsMessageTypeSupportGeolocationPidf(
            MessageTypeForGeolocationPidf::FINAL_SUCCESS_RESPONSE));
    EXPECT_FALSE(pManager->IsMessageTypeSupportGeolocationPidf(
            MessageTypeForGeolocationPidf::FINAL_FAILURE_RESPONSE));
}

TEST_F(MtcConfigurationManagerTest, IsInitializePemWhenNoHeaderReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsInitializePemWhenNoHeader(),
            GetBool(CarrierConfig::Assets::KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyForLocalRingbackToneWith180ResponseReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForLocalRingbackToneWith180Response(),
            GetInt(CarrierConfig::Assets::
                            KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT));
}

TEST_F(MtcConfigurationManagerTest, GetEpsFallbackWatchdogTimeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetEpsFallbackWatchdogTime(),
            GetInt(CarrierConfig::Assets::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest, GetSendUdpKeepAliveIntervalTimeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetSendUdpKeepAliveIntervalTime(),
            GetInt(CarrierConfig::Assets::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT));
}

TEST_F(MtcConfigurationManagerTest,
        GetCallRejectCodeForNotAcceptableCallTypeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetCallRejectCodeForNotAcceptableCallType(),
            GetInt(CarrierConfig::Assets::KEY_CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE_INT));
}

TEST_F(MtcConfigurationManagerTest,
        IsReleaseEmergencyPdnWithEmergencyCallFailReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsReleaseEmergencyPdnWithEmergencyCallFail(),
            GetBool(CarrierConfig::Assets::
                            KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_FAIL_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        GetPolicyForAlertNotUsingPreconditionMechanismReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->GetPolicyForAlertNotUsingPreconditionMechanism(),
            GetInt(CarrierConfig::Assets::
                            KEY_POLICY_FOR_ALERT_NOT_USING_PRECONDITION_MECHANISM_INT));
}

TEST_F(MtcConfigurationManagerTest, IsRequiredCdmalessFeatureTag)
{
    EXPECT_EQ(pManager->IsRequiredCdmalessFeatureTag(),
            GetBool(CarrierConfig::Assets::KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL));
}

TEST_F(MtcConfigurationManagerTest,
        IsEmergencyCallCurrentLocationDiscoverySupportedReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsEmergencyCallCurrentLocationDiscoverySupported(),
            GetBool(CarrierConfig::Assets::
                            KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL));
}

TEST_F(MtcConfigurationManagerTest, IsCheckUiConditionForIncomingResumeReturnsValueInCarrierConfig)
{
    EXPECT_EQ(pManager->IsCheckUiConditionForIncomingResume(),
            GetBool(CarrierConfig::Assets::KEY_CHECK_UI_CONDITION_FOR_INCOMING_RESUME_BOOL));
}

}  // namespace android
