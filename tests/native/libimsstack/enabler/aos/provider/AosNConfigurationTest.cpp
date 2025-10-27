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
#include <gmock/gmock.h>

#include "../../../platform/interface/MockICarrierConfig.h"

#include "interface/IAosNConfiguration.h"
#include "provider/AosNConfiguration.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgReferee;

#define DECLARE_USING(Base)                                         \
    using Base::GetNotifyTerminatedForInitRegUsedEvent;             \
    using Base::GetNotifyTerminatedForInitRegUsedEventWithWaitTime; \
    using Base::InitConfig;                                         \
    using Base::InitAssetsConfig;                                   \
    using Base::InitBundleForExtraRegErr;                           \
    using Base::InitBundleForNotifyTerminatedForInitReg;            \
    using Base::InitBundleForPcscfRecoveryConditions;               \
    using Base::InitBundleForRegErrCodeWithRaTime;                  \
    using Base::InitBundleForRegRetryInterval;                      \
    using Base::InitBundleForSubErrCodeForInitReg;                  \
    using Base::InitBundleForSubErrCodeForTerminated;               \
    using Base::InitBundleForWfcErrMessage;                         \
    using Base::InitBundlesContainers;

class TestAosNConfiguration : public ::AosNConfiguration
{
public:
    DECLARE_USING(AosNConfiguration)

    inline TestAosNConfiguration() :
            AosNConfiguration()
    {
    }
};

class AosNConfigurationTest : public ::testing::Test
{
public:
    TestAosNConfiguration* m_pAosNConfiguration;

protected:
    void SetUp() override
    {
        m_pAosNConfiguration = new TestAosNConfiguration();
        ASSERT_TRUE(m_pAosNConfiguration != nullptr);
    }

    void TearDown() override
    {
        if (m_pAosNConfiguration)
        {
            delete m_pAosNConfiguration;
        }
    }
};

TEST_F(AosNConfigurationTest, InitConfig)
{
    // init AosCarrierConfig
    EXPECT_FALSE(m_pAosNConfiguration->IsEmergencySmsOverImsSupported());
    EXPECT_FALSE(m_pAosNConfiguration->IsVoLteAvailable());
    EXPECT_FALSE(m_pAosNConfiguration->IsVtAvailable());
    EXPECT_FALSE(m_pAosNConfiguration->IsWfcImsAvailable());
    EXPECT_FALSE(m_pAosNConfiguration->IsRttSupported());
    EXPECT_FALSE(m_pAosNConfiguration->IsRttSupportedWhileRoaming());
    // bCarrierCrossSimImsAvailable (IMS_FALSE)
    EXPECT_FALSE(m_pAosNConfiguration->IsVolteTtySupported());
    EXPECT_FALSE(m_pAosNConfiguration->IsImsOverNrEnabled());
    EXPECT_EQ(CarrierConfig::USSD_OVER_CS_PREFERRED, m_pAosNConfiguration->GetUssdMethod());
    EXPECT_EQ(0, m_pAosNConfiguration->GetPcscfDiscoveryMethod().GetSize());
    EXPECT_FALSE(m_pAosNConfiguration->IsImsSingleRegistrationRequired());
    EXPECT_EQ(5060, m_pAosNConfiguration->GetPcscfPort());
    // KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL (IMS_FALSE)
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP,
            m_pAosNConfiguration->GetSipPreferredTransport());
    EXPECT_EQ(1500, m_pAosNConfiguration->GetIpv4MtuSize());
    EXPECT_EQ(1500, m_pAosNConfiguration->GetIpv6MtuSize());
    // KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY
    EXPECT_FALSE(m_pAosNConfiguration->IsIpsecEnabled());
    EXPECT_EQ(0, m_pAosNConfiguration->GetIpsecAuthenticationAlgorithms().GetSize());
    EXPECT_EQ(0, m_pAosNConfiguration->GetIpsecEncryptionAlgorithms().GetSize());

    EXPECT_EQ(2000, m_pAosNConfiguration->GetSipTimerT1());
    // KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(30000, m_pAosNConfiguration->GetRegistrationRetryBaseTime());
    EXPECT_EQ(1800000, m_pAosNConfiguration->GetRegistrationRetryMaxTime());
    EXPECT_TRUE(m_pAosNConfiguration->IsSubscription());
    // KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(2, m_pAosNConfiguration->GetSupportedRats().GetSize());

    EXPECT_TRUE(m_pAosNConfiguration->IsVoLteRoamingAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsSmsOverImsSupported());
    EXPECT_EQ(0, m_pAosNConfiguration->GetSmsOverImsSupportedRats().GetSize());
    EXPECT_FALSE(m_pAosNConfiguration->IsEmergencyCallbackModeSupported());
    EXPECT_EQ(0, m_pAosNConfiguration->GetEmergencyOverImsSupportedRats().GetSize());
    EXPECT_EQ(10000, m_pAosNConfiguration->GetEmergencyRegistrationTimerMillis());
    // KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT

    EXPECT_FALSE(m_pAosNConfiguration->IsTcpRequiredForReg());
    EXPECT_FALSE(m_pAosNConfiguration->IsUnSubscription());
    EXPECT_EQ(1, m_pAosNConfiguration->GetIsimIndexForImpu());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_DSCP_NONE, m_pAosNConfiguration->GetPreferredImsDscp());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED,
            m_pAosNConfiguration->GetRegistrationPreferredAccessTypeFeatureTag());
    EXPECT_FALSE(m_pAosNConfiguration->IsGeolocationPidfSupported(1));
    EXPECT_EQ(0, m_pAosNConfiguration->GetImsIdentityPriority().GetSize());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegPermanentErrCode().GetSize());
    EXPECT_EQ(0, m_pAosNConfiguration->GetUpdateRegistrationWithRatChange().GetSize());

    EXPECT_EQ(0, m_pAosNConfiguration->GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd());
    EXPECT_EQ(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK,
            m_pAosNConfiguration->GetPreferredEmergencyRegistration());
    EXPECT_EQ(0, m_pAosNConfiguration->GetWaitTimeMillisForReleaseEPdnAfterECallEnd());
    EXPECT_FALSE(m_pAosNConfiguration->IsSupportLimitedAdminSmsMode());
    EXPECT_TRUE(m_pAosNConfiguration->IsNetworkInitiatedUssdOverImsSupported());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegistrationPrivateHeader());

    MockICarrierConfig objCarrierConfig;

    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_CARRIER_VOLTE_AVAILABLE_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(
            objCarrierConfig, GetBoolean(CarrierConfig::KEY_CARRIER_VT_AVAILABLE_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig, GetBoolean(CarrierConfig::KEY_RTT_SUPPORTED_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_RTT_SUPPORTED_WHILE_ROAMING_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    // currently it doesn't have a function
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objCarrierNrAvailabilities;
    objCarrierNrAvailabilities.Clear();
    objCarrierNrAvailabilities.Add(CarrierConfig::Ims::CARRIER_NR_AVAILABILITY_NSA);
    objCarrierNrAvailabilities.Add(CarrierConfig::Ims::CARRIER_NR_AVAILABILITY_SA);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objCarrierNrAvailabilities));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::KEY_CARRIER_USSD_METHOD_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(CarrierConfig::USSD_OVER_IMS_PREFERRED));
    ImsVector<IMS_SINT32> objPcscfDiscoveryMethod;
    objPcscfDiscoveryMethod.Clear();
    objPcscfDiscoveryMethod.Add(0);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objPcscfDiscoveryMethod));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_IMS_SINGLE_REGISTRATION_REQUIRED_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(5060));
    // currently it doesn't have a function
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_PREFERRED_TRANSPORT_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_TCP));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_IPV4_SIP_MTU_SIZE_CELLULAR_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(1500));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(1500));

    // currently it doesn't have a function
    ImsVector<IMS_SINT32> objImsPdnEnabledInNoVopsSupport;
    objImsPdnEnabledInNoVopsSupport.Clear();
    objImsPdnEnabledInNoVopsSupport.Add(0);
    objImsPdnEnabledInNoVopsSupport.Add(1);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objImsPdnEnabledInNoVopsSupport));

    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));

    // InitIpsecAlgorithm
    ImsVector<IMS_SINT32> objAuthAlgo;
    objAuthAlgo.Clear();
    objAuthAlgo.Add(0);
    objAuthAlgo.Add(1);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objAuthAlgo));

    ImsVector<IMS_SINT32> objEncryAlgo;
    objEncryAlgo.Clear();
    objEncryAlgo.Add(0);
    objEncryAlgo.Add(1);
    objEncryAlgo.Add(2);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objEncryAlgo));

    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(600000));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_T1_MILLIS_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(2000));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(30000));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(1800000));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(600000));

    ImsVector<IMS_SINT32> objSupportedRats;
    objSupportedRats.Clear();
    objSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
    EXPECT_CALL(objCarrierConfig, GetIntArray(CarrierConfig::Ims::KEY_SUPPORTED_RATS_INT_ARRAY, _))
            .Times(4)
            .WillRepeatedly(Return(objSupportedRats));

    /// imsvoice.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsVoice::KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    /// imssms.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objSmsOverImsSupportedRats;
    objSmsOverImsSupportedRats.Clear();
    objSmsOverImsSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objSmsOverImsSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objSmsOverImsSupportedRats));

    /// imsemergency.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL,
                    IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objEmergencyOverImsSupportedRats;
    objEmergencyOverImsSupportedRats.Clear();
    objEmergencyOverImsSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::ImsEmergency::KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY,
                    _))
            .Times(2)
            .WillRepeatedly(Return(objEmergencyOverImsSupportedRats));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(1000));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(2000));

    /* carrier_config */
    /// ims.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_USE_TCP_TRANSPORT_FOR_REGISTER_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(1));

    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_PREFERRED_IMS_DSCP_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED));

    ImsVector<IMS_SINT32> objGeolocationPidfInSipRegisterSupport;
    objGeolocationPidfInSipRegisterSupport.Clear();
    objGeolocationPidfInSipRegisterSupport.Add(
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objGeolocationPidfInSipRegisterSupport));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(0);
    objImsIdentityPriority.Add(1);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objImsIdentityPriority));

    ImsVector<IMS_SINT32> objRegistrationPermanentErrorCode;
    objRegistrationPermanentErrorCode.Clear();
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objRegistrationPermanentErrorCode));

    ImsVector<IMS_SINT32> objUpdateRegistrationWithRatChange;
    objUpdateRegistrationWithRatChange.Clear();
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY, _))
            .Times(2)
            .WillRepeatedly(Return(objUpdateRegistrationWithRatChange));

    /// imsemergency.
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::
                            KEY_IPCAN_RELEASE_EMERGENCY_PDN_UPON_EMERGENCY_CALL_END_INT,
                    -1))
            .Times(2)
            .WillRepeatedly(Return(0));

    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_PREFERRED_EMERGENCY_REGISTRATION_INT, -1))
            .Times(2)
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));

    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::
                            KEY_WAIT_TIME_MILLIS_FOR_RELEASE_EPDN_AFTER_ECALL_END_INT,
                    -1))
            .Times(2)
            .WillRepeatedly(Return(240000));

    /// imssms
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsSms::KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    /// imswfc.
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsWfc::KEY_REGISTRATION_PRIVATE_HEADER_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(0));

    /// imsss.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsSs::KEY_NETWORK_INITIATED_USSD_OVER_IMS_SUPPORTED_BOOL,
                    IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosNConfiguration->InitConfig(&objCarrierConfig);

    EXPECT_TRUE(m_pAosNConfiguration->IsEmergencySmsOverImsSupported());
    EXPECT_TRUE(m_pAosNConfiguration->IsVoLteAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsVtAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsWfcImsAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsRttSupported());
    EXPECT_TRUE(m_pAosNConfiguration->IsRttSupportedWhileRoaming());
    // KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL
    EXPECT_TRUE(m_pAosNConfiguration->IsVolteTtySupported());
    EXPECT_TRUE(m_pAosNConfiguration->IsImsOverNrEnabled());
    EXPECT_EQ(CarrierConfig::USSD_OVER_IMS_PREFERRED, m_pAosNConfiguration->GetUssdMethod());
    EXPECT_EQ(1, m_pAosNConfiguration->GetPcscfDiscoveryMethod().GetSize());
    EXPECT_FALSE(m_pAosNConfiguration->IsImsSingleRegistrationRequired());
    EXPECT_EQ(5060, m_pAosNConfiguration->GetPcscfPort());
    // KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_TRANSPORT_TCP,
            m_pAosNConfiguration->GetSipPreferredTransport());
    EXPECT_EQ(1500, m_pAosNConfiguration->GetIpv4MtuSize());
    EXPECT_EQ(1500, m_pAosNConfiguration->GetIpv6MtuSize());
    // KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY
    EXPECT_TRUE(m_pAosNConfiguration->IsIpsecEnabled());
    EXPECT_EQ(2, m_pAosNConfiguration->GetIpsecAuthenticationAlgorithms().GetSize());
    EXPECT_EQ(3, m_pAosNConfiguration->GetIpsecEncryptionAlgorithms().GetSize());

    EXPECT_EQ(2000, m_pAosNConfiguration->GetSipTimerT1());
    // KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(30000, m_pAosNConfiguration->GetRegistrationRetryBaseTime());
    EXPECT_EQ(1800000, m_pAosNConfiguration->GetRegistrationRetryMaxTime());
    EXPECT_TRUE(m_pAosNConfiguration->IsSubscription());
    // KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(2, m_pAosNConfiguration->GetSupportedRats().GetSize());

    EXPECT_TRUE(m_pAosNConfiguration->IsVoLteRoamingAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsSmsOverImsSupported());
    EXPECT_EQ(2, m_pAosNConfiguration->GetSmsOverImsSupportedRats().GetSize());
    EXPECT_TRUE(m_pAosNConfiguration->IsEmergencyCallbackModeSupported());
    EXPECT_EQ(1, m_pAosNConfiguration->GetEmergencyOverImsSupportedRats().GetSize());
    EXPECT_EQ(1000, m_pAosNConfiguration->GetEmergencyRegistrationTimerMillis());
    // KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT

    EXPECT_FALSE(m_pAosNConfiguration->IsTcpRequiredForReg());
    EXPECT_FALSE(m_pAosNConfiguration->IsUnSubscription());
    EXPECT_EQ(1, m_pAosNConfiguration->GetIsimIndexForImpu());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_DSCP_NONE, m_pAosNConfiguration->GetPreferredImsDscp());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED,
            m_pAosNConfiguration->GetRegistrationPreferredAccessTypeFeatureTag());

    EXPECT_TRUE(m_pAosNConfiguration->IsGeolocationPidfSupported(2));
    EXPECT_FALSE(m_pAosNConfiguration->IsGeolocationPidfSupported(1));
    EXPECT_EQ(2, m_pAosNConfiguration->GetImsIdentityPriority().GetSize());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegPermanentErrCode().GetSize());
    EXPECT_EQ(0, m_pAosNConfiguration->GetUpdateRegistrationWithRatChange().GetSize());

    EXPECT_EQ(0, m_pAosNConfiguration->GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd());
    EXPECT_EQ(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK,
            m_pAosNConfiguration->GetPreferredEmergencyRegistration());
    EXPECT_EQ(240000, m_pAosNConfiguration->GetWaitTimeMillisForReleaseEPdnAfterECallEnd());
    EXPECT_FALSE(m_pAosNConfiguration->IsSupportLimitedAdminSmsMode());
    EXPECT_TRUE(m_pAosNConfiguration->IsNetworkInitiatedUssdOverImsSupported());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegistrationPrivateHeader());

    // 2nd call to test invoking InitConfig() several times
    m_pAosNConfiguration->InitConfig(&objCarrierConfig);

    EXPECT_TRUE(m_pAosNConfiguration->IsEmergencySmsOverImsSupported());
    EXPECT_TRUE(m_pAosNConfiguration->IsVoLteAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsVtAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsWfcImsAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsRttSupported());
    EXPECT_TRUE(m_pAosNConfiguration->IsRttSupportedWhileRoaming());
    // KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL
    EXPECT_TRUE(m_pAosNConfiguration->IsVolteTtySupported());
    EXPECT_TRUE(m_pAosNConfiguration->IsImsOverNrEnabled());
    EXPECT_EQ(CarrierConfig::USSD_OVER_IMS_PREFERRED, m_pAosNConfiguration->GetUssdMethod());
    EXPECT_EQ(1, m_pAosNConfiguration->GetPcscfDiscoveryMethod().GetSize());
    EXPECT_FALSE(m_pAosNConfiguration->IsImsSingleRegistrationRequired());
    EXPECT_EQ(5060, m_pAosNConfiguration->GetPcscfPort());
    // KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_TRANSPORT_TCP,
            m_pAosNConfiguration->GetSipPreferredTransport());
    EXPECT_EQ(1500, m_pAosNConfiguration->GetIpv4MtuSize());
    EXPECT_EQ(1500, m_pAosNConfiguration->GetIpv6MtuSize());
    // KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY
    EXPECT_TRUE(m_pAosNConfiguration->IsIpsecEnabled());
    EXPECT_EQ(2, m_pAosNConfiguration->GetIpsecAuthenticationAlgorithms().GetSize());
    EXPECT_EQ(3, m_pAosNConfiguration->GetIpsecEncryptionAlgorithms().GetSize());

    EXPECT_EQ(2000, m_pAosNConfiguration->GetSipTimerT1());
    // KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(30000, m_pAosNConfiguration->GetRegistrationRetryBaseTime());
    EXPECT_EQ(1800000, m_pAosNConfiguration->GetRegistrationRetryMaxTime());
    EXPECT_TRUE(m_pAosNConfiguration->IsSubscription());
    // KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(2, m_pAosNConfiguration->GetSupportedRats().GetSize());

    EXPECT_TRUE(m_pAosNConfiguration->IsVoLteRoamingAvailable());
    EXPECT_TRUE(m_pAosNConfiguration->IsSmsOverImsSupported());
    EXPECT_EQ(2, m_pAosNConfiguration->GetSmsOverImsSupportedRats().GetSize());
    EXPECT_TRUE(m_pAosNConfiguration->IsEmergencyCallbackModeSupported());
    EXPECT_EQ(1, m_pAosNConfiguration->GetEmergencyOverImsSupportedRats().GetSize());
    EXPECT_EQ(1000, m_pAosNConfiguration->GetEmergencyRegistrationTimerMillis());
    // KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT

    EXPECT_FALSE(m_pAosNConfiguration->IsTcpRequiredForReg());
    EXPECT_FALSE(m_pAosNConfiguration->IsUnSubscription());
    EXPECT_EQ(1, m_pAosNConfiguration->GetIsimIndexForImpu());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_DSCP_NONE, m_pAosNConfiguration->GetPreferredImsDscp());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED,
            m_pAosNConfiguration->GetRegistrationPreferredAccessTypeFeatureTag());

    EXPECT_TRUE(m_pAosNConfiguration->IsGeolocationPidfSupported(2));
    EXPECT_FALSE(m_pAosNConfiguration->IsGeolocationPidfSupported(1));
    EXPECT_EQ(2, m_pAosNConfiguration->GetImsIdentityPriority().GetSize());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegPermanentErrCode().GetSize());
    EXPECT_EQ(0, m_pAosNConfiguration->GetUpdateRegistrationWithRatChange().GetSize());

    EXPECT_EQ(0, m_pAosNConfiguration->GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd());
    EXPECT_EQ(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK,
            m_pAosNConfiguration->GetPreferredEmergencyRegistration());
    EXPECT_EQ(240000, m_pAosNConfiguration->GetWaitTimeMillisForReleaseEPdnAfterECallEnd());
    EXPECT_FALSE(m_pAosNConfiguration->IsSupportLimitedAdminSmsMode());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegistrationPrivateHeader());
    EXPECT_TRUE(m_pAosNConfiguration->IsNetworkInitiatedUssdOverImsSupported());
}

TEST_F(AosNConfigurationTest, InitAssetsConfig)
{
    MockICarrierConfig objCarrierConfig;

    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_B2C_CALL_COMPOSER_FEATURE_TAG_IN_REG_CONTACT_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_BLOCK_PCSCF_ON_REG_FAILURE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_BLOCK_REG_ON_CS_CALL_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_CALL_END_AND_PDN_REACTIVATION_BY_REG_TERMINATED_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_DESTROY_UNSECURE_TCP_SOCKET_ON_ACCOMPLISHING_REG_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::
                               KEY_ECALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REG_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_EREG_ON_RANDOM_PCSCF_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsEmergency::KEY_EREG_SET_TCP_ONLY_IN_ROAMING_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsEmergency::KEY_EREG_USING_FIRST_IMPU_IN_ISIM_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsEmergency::KEY_SUPPORT_EREG_WHEN_EATTACH_WITH_VALID_SIM_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_HOLD_REG_WITH_IPCAN_CHANGED_DURING_IMS_CALL_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_IGNORE_MTU_FROM_NETWORK_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_IMS_DEREG_ON_3G_NETWORK_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_IMSI_BASED_URI_PRIORITIZED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_INIT_IPSEC_SETTING_WITH_NEW_PCSCF_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_INIT_SUB_UPON_SUB_TERMINATED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_KEEP_EPDN_UPON_PCSCF_UNAVAILABLE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_KEEP_EREG_RETRY_ON_WLAN_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::
                               KEY_KEEP_EXISTING_PCSCF_ON_PCSCF_CHANGE_DURING_THE_CALL_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_KEEP_REG_RETRY_CNT_UPON_PDN_RECONNECT_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_KEEP_REG_RETRY_TIMER_ON_ALL_ENABLERS_DETACHED_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_REG_TIMER_FOR_ECALL_TIMEOUT_AS_FAILURE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::
                               KEY_REG_TIMER_FOR_ECALL_WITH_RAT_CHECK_ENABLED_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_STOP_EREG_TIMER_ON_EPDN_CONNECTED_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_PDN_RECONNECT_ON_ALL_PCSCFS_UNAVAILABLE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::
                               KEY_PLMN_BLOCK_WITH_TIMEOUT_ON_VOICE_CALL_UNAVAILABLE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_REG_CONTACT_VALIDATION_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_REG_RETRY_WITH_IP_VER_FALLBACK_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_RELEASE_EPDN_OF_UNAVAILABLE_NETWORK_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsEmergency::KEY_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_REQUIRED_INIT_REG_AFTER_IMS_CALL_END_ON_REG_HELD_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_REQUIRED_INIT_REG_AFTER_IMS_ECALL_END_ON_REG_HELD_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_REQUIRED_VOLTE_BLOCK_BY_SSAC_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsWfc::KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsWfc::KEY_GEOLOCATION_PIDF_IN_WFC_INIT_REG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsWfc::KEY_PANI_HEADER_IN_WFC_INIT_REG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsWfc::KEY_REREG_WITH_CHANGED_COUNTRY_ON_WIFI_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_AVAILABLE_WITHOUT_VOICE_CAPA_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_SUPPORT_ANONYMOUS_ECALL_ACTION_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_SUPPORT_EREREG_ON_IPCAN_CHANGE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_SUPPORT_GIBA_FOR_EREG_IN_ROAMING_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_VERSTAT_BASED_ON_NETWORK_FOR_REG_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_VERSTAT_FOR_REG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_SUPPORT_VIDEO_FOR_EREG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::
                               KEY_UPDATE_ONGOING_REG_RETRY_TIMER_ON_IMS_EST_TIMER_EXPIRY_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::
                               KEY_USE_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_USE_REGINFO_CONTACT_WITHOUT_URI_CHECK_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_USE_SECURITY_SERVER_PORT_IN_INIT_REG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsEmergency::KEY_USE_REG_RETRY_RULE_FOR_EREG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::
                               KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INIT_REG_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsWfc::KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsWfc::KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_AUTH_FAILURE_RETRY_MAX_CNT_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_CONTACT_USER_INFO_POLICY_FOR_NON_REG_MESSAGE_INT, -1))
            .WillOnce(Return(1));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_EPDN_PREFERRED_IPTYPE_INT, -1))
            .WillOnce(Return(1));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::ImsEmergency::KEY_EREG_RETRY_MAX_CNT_INT, -1))
            .WillOnce(Return(2));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_EREG_RETRY_TIMER_MILLIS_INT, -1))
            .WillOnce(Return(3000));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_FORMING_POLICY_INT, -1))
            .WillOnce(Return(1));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_IMS_ESTABLISHMENT_TIME_FOR_LTE_SEC_INT, -1))
            .WillOnce(Return(120));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_IMS_ESTABLISHMENT_TIME_FOR_NR_SEC_INT, -1))
            .WillOnce(Return(180));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_IMS_PREFERRED_IPTYPE_INT, -1))
            .WillOnce(Return(1));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_IMS_SIGNALLING_DSCP_INT, -1))
            .WillOnce(Return(46));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::
                            KEY_PDN_RECONNECT_DELAY_ON_WFC_SETUP_FAIL_ALL_PCSCFS_WITH_CS_ROAM_SEC_INT,
                    -1))
            .WillOnce(Return(0));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_DEFAULT_WAIT_TIME_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_OUT_OF_SERVICE_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::REG_OOS_POLICY_DEFAULT));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_PCSCF_UPDATE_POLICY_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_RETRY_305_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_RETRY_503_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::SIP_503_CODE_POLICY_DEFAULT));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_ON_SINGLE_PCSCF_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_PER_PCSCF_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_PER_PCSCF_WITH_RA_TIME_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_RESET_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REG_RETRY_CNT_WITH_IPSEC_ON_AUTH_FAILURE_INT, -1))
            .WillOnce(Return(3));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_RETRY_DEFAULT_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::DEFAULT_RETRY_POLICY_SPEC));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REG_RETRY_TIMER_F_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::TIMER_F_POLICY_NONE));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_REG_TIMER_FOR_ECALL_MILLIS_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_RELEASE_PDN_DELAY_SEC_AFTER_TEMP_PLMN_BLOCK_INT, -1))
            .WillOnce(Return(5));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REREG_RETRY_305_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_ROAMING_PREFERRED_EREG_INT, -1))
            .WillOnce(Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT, -1))
            .WillOnce(Return(200));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SUB_RETRY_503_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::SIP_503_CODE_POLICY_3GPP));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::USAT_REG_EVENT_NOT_DOWNLOAD));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::ImsVoice::KEY_VOLTE_HYS_TIME_SEC_INT, -1))
            .WillOnce(Return(60));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsWfc::
                            KEY_SUB_CONSECUTIVE_RETRY_CNT_FOR_REG_FORBIDDEN_IN_WIFI_INT,
                    -1))
            .WillOnce(Return(0));

    ImsVector<IMS_SINT32> objKeepRegWithMmtelFeatureTagPolicy;
    objKeepRegWithMmtelFeatureTagPolicy.Clear();
    objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_3G);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::Ims::KEY_KEEP_REG_WITH_MMTEL_FEATURE_TAG_POLICY_INT_ARRAY, _))
            .WillOnce(Return(objKeepRegWithMmtelFeatureTagPolicy));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Clear();
    objRegErrCodeForPcscfDiscovery.Add(408);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY, _))
            .WillOnce(Return(objRegErrCodeForPcscfDiscovery));

    ImsVector<IMS_SINT32> objRegPermanentErrMaxCnt;
    objRegPermanentErrMaxCnt.Clear();
    objRegPermanentErrMaxCnt.Add(2);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_REG_PERMANENT_ERR_MAX_CNT_INT_ARRAY, _))
            .WillOnce(Return(objRegPermanentErrMaxCnt));

    ImsVector<IMS_SINT32> objRegRetryErrCodeWithoutIpsec;
    objRegRetryErrCodeWithoutIpsec.Clear();
    objRegRetryErrCodeWithoutIpsec.Add(406);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY, _))
            .WillOnce(Return(objRegRetryErrCodeWithoutIpsec));

    ImsVector<IMS_SINT32> objTempPlmnBlockRats;
    objTempPlmnBlockRats.Clear();
    objTempPlmnBlockRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::Ims::KEY_REG_TEMP_PLMN_BLOCK_RATS_ON_ALL_PCSCFS_FAIL_INT_ARRAY,
                    _))
            .WillOnce(Return(objTempPlmnBlockRats));

    ImsVector<IMS_SINT32> objReregErrForCallEnd;
    objReregErrForCallEnd.Clear();
    objReregErrForCallEnd.Add(403);
    objReregErrForCallEnd.Add(406);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_REREG_ERR_CODE_FOR_CALL_END_INT_ARRAY, _))
            .WillOnce(Return(objReregErrForCallEnd));

    ImsVector<IMS_SINT32> objReregErrCodeForInitRegWithAvailablePcscf;
    objReregErrCodeForInitRegWithAvailablePcscf.Clear();
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::
                                KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY,
                    _))
            .WillOnce(Return(objReregErrCodeForInitRegWithAvailablePcscf));

    ImsVector<IMS_SINT32> objReregErrCodeForImsPdnReactivation;
    objReregErrCodeForImsPdnReactivation.Clear();
    objReregErrCodeForImsPdnReactivation.Add(408);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::Ims::KEY_REREG_ERR_CODE_FOR_IMS_PDN_REACTIVATION_INT_ARRAY, _))
            .WillOnce(Return(objReregErrCodeForImsPdnReactivation));

    ImsVector<IMS_SINT32> objReregRetryErrCodeForInitRegWithSamePcscf;
    objReregRetryErrCodeForInitRegWithSamePcscf.Clear();
    objReregRetryErrCodeForInitRegWithSamePcscf.Add(407);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::
                                KEY_REREG_RETRY_ERR_CODE_FOR_INIT_REG_WITH_SAME_PCSCF_INT_ARRAY,
                    _))
            .WillOnce(Return(objReregRetryErrCodeForInitRegWithSamePcscf));

    ImsVector<IMS_SINT32> objSubErrorCodeForInitRegWithNextPcscf;
    objSubErrorCodeForInitRegWithNextPcscf.Clear();
    objSubErrorCodeForInitRegWithNextPcscf.Add(404);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_NEXT_PCSCF_INT_ARRAY, _))
            .WillOnce(Return(objSubErrorCodeForInitRegWithNextPcscf));

    ImsVector<IMS_SINT32> objSubErrorCodeForStoppingByExpirationTime;
    objSubErrorCodeForStoppingByExpirationTime.Clear();
    objSubErrorCodeForStoppingByExpirationTime.Add(606);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY,
                    _))
            .WillOnce(Return(objSubErrorCodeForStoppingByExpirationTime));

    ImsVector<IMS_SINT32> objSupportedRoamingRats;
    objSupportedRoamingRats.Clear();
    objSupportedRoamingRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);
    objSupportedRoamingRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objSupportedRoamingRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY, _))
            .WillOnce(Return(objSupportedRoamingRats));

    ImsVector<IMS_SINT32> objTestMode;
    EXPECT_CALL(objCarrierConfig, GetIntArray(CarrierConfig::Ims::KEY_TEST_MODE_INT_ARRAY, _))
            .WillOnce(Return(objTestMode));

    ImsVector<IMS_SINT32> objUnavailableFeaturesInLimitedReg;
    objUnavailableFeaturesInLimitedReg.Clear();
    objUnavailableFeaturesInLimitedReg.Add(CarrierConfig::Ims::REG_FEATURE_MMTEL);
    objUnavailableFeaturesInLimitedReg.Add(CarrierConfig::Ims::REG_FEATURE_VIDEO);
    objUnavailableFeaturesInLimitedReg.Add(CarrierConfig::Ims::REG_FEATURE_TEXT);
    objUnavailableFeaturesInLimitedReg.Add(CarrierConfig::Ims::REG_FEATURE_SMS);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_UNAVAILABLE_FEATURES_IN_LIMITED_REG_INT_ARRAY, _))
            .WillOnce(Return(objUnavailableFeaturesInLimitedReg));

    ImsVector<IMS_SINT32> objERegErrCodeNotSupportedCommonPolicy;
    objERegErrCodeNotSupportedCommonPolicy.Add(423);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::ImsEmergency::
                                KEY_EREG_ERR_CODE_NOT_SUPPORTED_COMMON_POLICY_INT_ARRAY,
                    _))
            .WillOnce(Return(objERegErrCodeNotSupportedCommonPolicy));

    ImsVector<IMS_SINT32> objNetworkAttachRejectCausesForCrossStackRedial;
    objNetworkAttachRejectCausesForCrossStackRedial.Add(3);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::ImsEmergency::
                                KEY_NETWORK_ATTACH_REJECT_CAUSES_FOR_CROSS_STACK_REDIAL_INT_ARRAY,
                    _))
            .WillOnce(Return(objNetworkAttachRejectCausesForCrossStackRedial));

    ImsVector<IMS_SINT32> objVowifiSubErrorCodeForInitReg;
    objVowifiSubErrorCodeForInitReg.Clear();
    objVowifiSubErrorCodeForInitReg.Add(0);
    objVowifiSubErrorCodeForInitReg.Add(403);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::ImsWfc::KEY_VOWIFI_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY, _))
            .WillOnce(Return(objVowifiSubErrorCodeForInitReg));

    ImsVector<AString> objPlmnsReleaseEPdnUponECallEndInFakeMode;
    objPlmnsReleaseEPdnUponECallEndInFakeMode.Add("310410");
    EXPECT_CALL(objCarrierConfig,
            GetStringArray(CarrierConfig::ImsEmergency::
                                   KEY_PLMNS_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_STRING_ARRAY,
                    _))
            .WillOnce(Return(objPlmnsReleaseEPdnUponECallEndInFakeMode));

    m_pAosNConfiguration->InitAssetsConfig(&objCarrierConfig);

    EXPECT_FALSE(m_pAosNConfiguration->IsCdmalessFeatureTagRequired());
    EXPECT_TRUE(m_pAosNConfiguration->IsB2cCallComposerFeatureTagInRegContact());
    EXPECT_TRUE(m_pAosNConfiguration->IsBlockPcscfOnRegFailure());
    EXPECT_TRUE(m_pAosNConfiguration->IsBlockRegOnCsCall());
    EXPECT_FALSE(m_pAosNConfiguration->IsCallEndAndPdnReactivationByRegTerminated());
    EXPECT_FALSE(m_pAosNConfiguration->IsUnsecureTcpSocketOnAccomplishingRegDestroyed());
    EXPECT_FALSE(m_pAosNConfiguration->IsEmergencyCallBasedOnPauOfNormalRegistrationSupported());
    EXPECT_TRUE(m_pAosNConfiguration->IsEmcRegOnRandomPcscf());
    EXPECT_TRUE(m_pAosNConfiguration->IsERegWithOnlyTcpInRoaming());
    EXPECT_TRUE(m_pAosNConfiguration->IsERegUsingFirstImpuInIsim());
    EXPECT_TRUE(m_pAosNConfiguration->IsSupportERegWhenEAttachWithValidSim());
    EXPECT_FALSE(m_pAosNConfiguration->IsRegWithIpcanChangedDuringImsCallHeld());
    EXPECT_TRUE(m_pAosNConfiguration->IsVopsIgnoredForVolteEnabled());
    EXPECT_FALSE(m_pAosNConfiguration->IsDeregOn3gNetwork());
    EXPECT_FALSE(m_pAosNConfiguration->IsImsiBasedUriPrioritized());
    EXPECT_FALSE(m_pAosNConfiguration->IsIpsecInitializedWithNewPcscf());
    EXPECT_FALSE(m_pAosNConfiguration->IsInitSubUponSubTerminated());
    EXPECT_FALSE(m_pAosNConfiguration->IsKeepEPdnUponPcscfUnavailable());
    EXPECT_FALSE(m_pAosNConfiguration->IsKeepERegRetryOnWlanRequired());
    EXPECT_TRUE(m_pAosNConfiguration->IsKeepRegRetryCntUponPdnReconnect());
    EXPECT_TRUE(m_pAosNConfiguration->IsKeepRegRetryTimerOnAllEnablersDetached());
    EXPECT_TRUE(m_pAosNConfiguration->IsRegTimerForECallTimeoutAsFailure());
    EXPECT_TRUE(m_pAosNConfiguration->IsRegTimerForECallWithRatCheckEnabled());
    EXPECT_FALSE(m_pAosNConfiguration->IsStopERegTimerOnEpdnConnected());
    EXPECT_FALSE(m_pAosNConfiguration->IsPdnReconnectOnAllPcscfsUnavailable());
    EXPECT_FALSE(m_pAosNConfiguration->IsPlmnBlockWithTimeoutOnVoiceCallUnavailable());
    EXPECT_FALSE(m_pAosNConfiguration->IsContactUriValidationChecked());
    EXPECT_FALSE(m_pAosNConfiguration->IsRegRetryWithIpVerFallback());
    EXPECT_TRUE(m_pAosNConfiguration->IsReleaseEPdnUponECallEndInFakeMode());
    EXPECT_FALSE(m_pAosNConfiguration->IsOldSaOnEstablishingSaRemoved());
    EXPECT_TRUE(m_pAosNConfiguration->IsRegRequiredAfterImsCallEndOnRegHeld());
    EXPECT_TRUE(m_pAosNConfiguration->IsRegRequiredAfterImsECallEndOnRegHeld());
    EXPECT_FALSE(m_pAosNConfiguration->IsRequiredVolteBlockBySsac());
    EXPECT_FALSE(m_pAosNConfiguration->IsRequiredWfcBlockByAirplaneMode());
    EXPECT_FALSE(m_pAosNConfiguration->IsGeolocationPidfInWfcInitReg());
    EXPECT_FALSE(m_pAosNConfiguration->IsPaniHeaderInWfcInitReg());
    EXPECT_FALSE(m_pAosNConfiguration->IsReregRetryWithChangedCountryOnWifi());
    EXPECT_TRUE(m_pAosNConfiguration->IsSipOverIpsecInRoamingEnabled());
    EXPECT_TRUE(m_pAosNConfiguration->IsSmsOverImsAvailableWithoutVoiceCapability());
    EXPECT_FALSE(m_pAosNConfiguration->IsAnonymousECallActionSupported());
    EXPECT_FALSE(m_pAosNConfiguration->IsVerstatForRegistrationSupported());
    EXPECT_FALSE(m_pAosNConfiguration->IsVerstatSupportedBasedOnNetworkForReg());
    EXPECT_TRUE(m_pAosNConfiguration->IsUpdateOngoingRegRetryTimerOnImsEstTimerExpiry());
    EXPECT_FALSE(m_pAosNConfiguration->IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType());
    EXPECT_FALSE(m_pAosNConfiguration->IsSecurityServerPortInInitRegUsed());
    EXPECT_FALSE(m_pAosNConfiguration->IsRegRetryRuleForERegUsed());
    EXPECT_FALSE(m_pAosNConfiguration->IsSecurityServerPortInRegContactOfInitRegUsed());
    EXPECT_FALSE(m_pAosNConfiguration->UseWfcCountryCodeAvailabilityCheck());
    EXPECT_FALSE(m_pAosNConfiguration->IsVideoOverWifiSupportedWithoutVoice());
    EXPECT_TRUE(m_pAosNConfiguration->IsUseRegInfoContactWithoutUriCheck());
    EXPECT_TRUE(m_pAosNConfiguration->IsIgnoreMtuFromNetwork());
    EXPECT_FALSE(m_pAosNConfiguration->IsTestModeEnabled(
            CarrierConfig::Ims::TEST_MODE_PERMANENT_FAILURE_WITHOUT_IMS_PDN_DEACTIVATION));
    EXPECT_FALSE(m_pAosNConfiguration->ShouldKeepExistingPcscfOnPcscfChangeDuringTheCall());

    EXPECT_EQ(0, m_pAosNConfiguration->GetAuthFailureRetryMaxCnt());
    EXPECT_EQ(1, m_pAosNConfiguration->GetUserInfoPolicyForNonRegisterMessage());
    EXPECT_EQ(1, m_pAosNConfiguration->GetEmergencyPreferredIpType());
    EXPECT_EQ(2, m_pAosNConfiguration->GetEmcRegRetryMaxCnt());
    EXPECT_EQ(3000, m_pAosNConfiguration->GetEmcRegRetryTimerMillis());
    EXPECT_EQ(1, m_pAosNConfiguration->GetGeolocationPidfFormingPolicy());
    EXPECT_EQ(120, m_pAosNConfiguration->GetImsEstablishmentTimeForLte());
    EXPECT_EQ(180, m_pAosNConfiguration->GetImsEstablishmentTimeForNr());
    EXPECT_EQ(1, m_pAosNConfiguration->GetPreferredIpType());
    EXPECT_EQ(46, m_pAosNConfiguration->GetImsSignallingDscp());
    EXPECT_EQ(0, m_pAosNConfiguration->GetPdnReconnectDelayOnWfcSetupFailAllPcscfsWithCsRoam());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegActualWaitTimePolicy());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegDefaultWaitTime());
    EXPECT_EQ(CarrierConfig::Ims::REG_OOS_POLICY_DEFAULT,
            m_pAosNConfiguration->GetRegOutOfServicePolicy());
    EXPECT_EQ(CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT,
            m_pAosNConfiguration->GetRegRetrySip305CodePolicy());
    EXPECT_EQ(CarrierConfig::Ims::SIP_503_CODE_POLICY_DEFAULT,
            m_pAosNConfiguration->GetRegRetrySip503CodePolicy());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegRetryCountOnSinglePcscf());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegRetryCountPerPcscf());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegRetryCountPerPcscfWithRaTime());
    EXPECT_EQ(CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_REGISTRATION,
            m_pAosNConfiguration->GetRegRetryCountResetPolicy());
    EXPECT_EQ(3, m_pAosNConfiguration->GetRegRetryCountWithIpsecOnAuthFailure());
    EXPECT_EQ(CarrierConfig::Ims::DEFAULT_RETRY_POLICY_SPEC,
            m_pAosNConfiguration->GetRegRetryDefaultPolicy());
    EXPECT_EQ(CarrierConfig::Ims::TIMER_F_POLICY_NONE,
            m_pAosNConfiguration->GetRegRetryTimerFPolicy());
    EXPECT_EQ(0, m_pAosNConfiguration->GetRegistrationPcscfUpdatePolicy());

    EXPECT_EQ(0, m_pAosNConfiguration->GetRegTimerForEmcCall());
    EXPECT_EQ(5, m_pAosNConfiguration->GetReleasePdnDelaySecAfterTempPlmnBlock());
    EXPECT_EQ(CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT,
            m_pAosNConfiguration->GetReregRetrySip305CodePolicy());
    EXPECT_EQ(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL,
            m_pAosNConfiguration->GetRoamingPreferredEmcReg());
    EXPECT_EQ(200, m_pAosNConfiguration->GetSipMessageThresholdForTransportChange());
    EXPECT_EQ(CarrierConfig::Ims::SIP_503_CODE_POLICY_3GPP,
            m_pAosNConfiguration->GetSubRetrySip503CodePolicy());
    EXPECT_EQ(CarrierConfig::Ims::USAT_REG_EVENT_NOT_DOWNLOAD,
            m_pAosNConfiguration->GetUsatRegEventDownloadPolicy());
    EXPECT_EQ(60, m_pAosNConfiguration->GetVolteHysTime());
    EXPECT_EQ(0, m_pAosNConfiguration->GetSubConsecutiveRetryCntForRegForbiddenInWifi());

    ImsVector<IMS_SINT32> objFeatures = m_pAosNConfiguration->GetUnavailableFeaturesInLimitedReg();
    EXPECT_TRUE(objFeatures.Contains(CarrierConfig::Ims::REG_FEATURE_MMTEL));
    EXPECT_TRUE(objFeatures.Contains(CarrierConfig::Ims::REG_FEATURE_VIDEO));
    EXPECT_TRUE(objFeatures.Contains(CarrierConfig::Ims::REG_FEATURE_TEXT));
    EXPECT_TRUE(objFeatures.Contains(CarrierConfig::Ims::REG_FEATURE_SMS));

    ImsVector<IMS_SINT32> objNetworkTypes =
            m_pAosNConfiguration->GetRegTempPlmnBlockRatsOnAllPcscfsFail();
    EXPECT_EQ(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN, objNetworkTypes.GetAt(0));

    ImsVector<IMS_SINT32> objPolicy = m_pAosNConfiguration->GetKeepRegWithMmtelFeatureTagPolicy();
    EXPECT_EQ(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_3G, objPolicy.GetAt(0));
    ImsVector<IMS_SINT32> objErrCode = m_pAosNConfiguration->GetRegErrCodeForPcscfDiscovery();
    EXPECT_EQ(408, objErrCode.GetAt(0));
    ImsVector<IMS_SINT32> objCount = m_pAosNConfiguration->GetRegPermanentErrMaxCount();
    EXPECT_EQ(1, objCount.GetSize());
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetRegErrCodeWithoutIpsec();
    EXPECT_EQ(406, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetReregErrCodeForCallEnd();
    IMS_UINT32 cnt = 0;
    for (IMS_UINT32 i = 0; i < objErrCode.GetSize(); i++)
    {
        if (objErrCode.GetAt(i) == 403 || objErrCode.GetAt(i) == 406)
        {
            cnt++;
        }
    }
    EXPECT_EQ(2, cnt);

    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetReregErrCodeForImsPdnReactivation();
    EXPECT_EQ(1, objErrCode.GetSize());
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetReregErrCodeForInitRegWithAvailablePcscf();
    EXPECT_EQ(0, objErrCode.GetSize());
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetReregRetryErrCodeForInitRegWithSamePcscf();
    EXPECT_EQ(407, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetSubErrorRegRequiredWithNextPcscf();
    EXPECT_EQ(404, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetSubErrorStoppingResub();
    EXPECT_EQ(606, objErrCode.GetAt(0));
    ImsVector<IMS_SINT32> objRats = m_pAosNConfiguration->GetSupportedRoamingRats();
    IMS_BOOL bNrSupported = IMS_FALSE;
    for (IMS_UINT32 i = 0; i < objErrCode.GetSize(); i++)
    {
        if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN)
        {
            bNrSupported = IMS_TRUE;
        }
    }
    EXPECT_TRUE(bNrSupported);
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetERegErrCodeNotSupportedCommonPolicy();
    EXPECT_EQ(423, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetNetworkAttachRejectCausesForCrossStackRedial();
    EXPECT_EQ(3, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = m_pAosNConfiguration->GetVowifiSubErrorRegRequired();
    cnt = 0;
    for (IMS_UINT32 i = 0; i < objErrCode.GetSize(); i++)
    {
        if (objErrCode.GetAt(i) == 0 || objErrCode.GetAt(i) == 403)
        {
            cnt++;
        }
    }
    EXPECT_EQ(2, cnt);
    objErrCode.Clear();
    ImsVector<AString> objPlmns = m_pAosNConfiguration->GetPlmnsReleaseEPdnUponECallEndInFakeMode();
    EXPECT_EQ(AString("310410"), objPlmns.GetAt(0));
    objPlmns.Clear();
}

TEST_F(AosNConfigurationTest, InitBundleConfigForExtraRegErr)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objExtraRegErr;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_BUNDLE))
            .WillByDefault(Return(&objExtraRegErr));
    ON_CALL(objExtraRegErr,
            GetBoolean(CarrierConfig::Ims::
                               KEY_EXTRA_REG_ERR_CODE_AS_FAILURE_IN_ROAMING_FOR_UPDATE_BOOL,
                    IMS_FALSE))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objExtraRegErr,
            GetBoolean(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL,
                    IMS_FALSE))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objExtraRegErr,
            GetInt(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_FINAL_TYPE_INT,
                    CarrierConfig::Ims::ERROR_TYPE_NOT_SPECIFIED))
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_NOT_SPECIFIED));
    ON_CALL(objExtraRegErr, GetInt(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_MAX_CNT_INT, 0))
            .WillByDefault(Return(0));
    ON_CALL(objExtraRegErr,
            GetInt(CarrierConfig::Ims::
                            KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_EPS_5GS_ONLY_ATTACHED_INT,
                    0))
            .WillByDefault(Return(1));
    ON_CALL(objExtraRegErr,
            GetInt(CarrierConfig::Ims::
                            KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_LTE_COMBINED_ATTACHED_INT,
                    0))
            .WillByDefault(Return(2));
    ON_CALL(objExtraRegErr,
            GetInt(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_POLICY_INT,
                    CarrierConfig::Ims::ERROR_POLICY_NOT_SPECIFIED))
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_POLICY_PCSCF_FAILED));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(400);
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ON_CALL(objExtraRegErr, GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraRegErrCode)));
    ImsVector<IMS_SINT32> objExtraReregErrCode;
    objExtraReregErrCode.Add(500);
    ON_CALL(objExtraRegErr,
            GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraReregErrCode)));
    ImsVector<IMS_SINT32> objExtraRegErrWaitTimeSec;
    objExtraRegErrWaitTimeSec.Add(30);
    ON_CALL(objExtraRegErr,
            GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraRegErrWaitTimeSec)));

    // WHEN
    m_pAosNConfiguration->InitBundleForExtraRegErr(&objCarrierConfig);

    // THEN
    EXPECT_FALSE(m_pAosNConfiguration->IsExtraReregErrInRoamingAsFailureHandled());
    EXPECT_TRUE(m_pAosNConfiguration->IsExtraRegErrRetryCntSharedForRegAndSubRequired());
    EXPECT_EQ(CarrierConfig::Ims::ERROR_TYPE_NOT_SPECIFIED,
            m_pAosNConfiguration->GetExtraRegErrFinalType());
    EXPECT_EQ(0, m_pAosNConfiguration->GetExtraRegErrMaxCount());
    EXPECT_EQ(1, m_pAosNConfiguration->GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached());
    EXPECT_EQ(2, m_pAosNConfiguration->GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached());
    EXPECT_EQ(CarrierConfig::Ims::ERROR_POLICY_PCSCF_FAILED,
            m_pAosNConfiguration->GetExtraRegErrPolicy());
    ImsVector<IMS_SINT32>& objErrCode = m_pAosNConfiguration->GetExtraRegErrCode();
    EXPECT_EQ(1, objErrCode.GetSize());
    EXPECT_EQ(400, objErrCode.GetAt(0));
    ImsVector<IMS_SINT32>& objReregErrCode = m_pAosNConfiguration->GetExtraReregErrCode();
    EXPECT_EQ(1, objReregErrCode.GetSize());
    EXPECT_EQ(500, objReregErrCode.GetAt(0));
    ImsVector<IMS_SINT32>& objWaitTime = m_pAosNConfiguration->GetExtraRegErrWaitTime();
    EXPECT_EQ(1, objWaitTime.GetSize());
    EXPECT_EQ(30, objWaitTime.GetAt(0));
}

TEST_F(AosNConfigurationTest, InitBundleConfigForExtraRegErrWithoutIntArrayKeys)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objExtraRegErr;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_BUNDLE))
            .WillByDefault(Return(&objExtraRegErr));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(400);
    IMS_BOOL bKeyExistValue = IMS_FALSE;
    ON_CALL(objExtraRegErr, GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraRegErrCode)));
    ImsVector<IMS_SINT32> objExtraReregErrCode;
    objExtraReregErrCode.Add(500);
    ON_CALL(objExtraRegErr,
            GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraReregErrCode)));
    ImsVector<IMS_SINT32> objExtraRegErrWaitTimeSec;
    objExtraRegErrWaitTimeSec.Add(30);
    ON_CALL(objExtraRegErr,
            GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraRegErrWaitTimeSec)));

    // WHEN
    m_pAosNConfiguration->InitBundleForExtraRegErr(&objCarrierConfig);

    // THEN
    ImsVector<IMS_SINT32>& objErrCode = m_pAosNConfiguration->GetExtraRegErrCode();
    EXPECT_EQ(0, objErrCode.GetSize());
    ImsVector<IMS_SINT32>& objReregErrCode = m_pAosNConfiguration->GetExtraReregErrCode();
    EXPECT_EQ(0, objReregErrCode.GetSize());
    ImsVector<IMS_SINT32>& objWaitTime = m_pAosNConfiguration->GetExtraRegErrWaitTime();
    EXPECT_EQ(0, objWaitTime.GetSize());
}

TEST_F(AosNConfigurationTest, InitBundleConfigForNotifyTerminatedForInitReg)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objNotifyTerminated;

    ON_CALL(objCarrierConfig,
            GetBundle(CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE))
            .WillByDefault(Return(&objNotifyTerminated));
    ON_CALL(objNotifyTerminated,
            GetInt(CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_WITH_WAIT_TIME_INT, 0))
            .WillByDefault(Return(40));
    ImsVector<IMS_SINT32> objEventForInitRegOnTerminatedState;
    objEventForInitRegOnTerminatedState.Add(1);
    objEventForInitRegOnTerminatedState.Add(2);
    objEventForInitRegOnTerminatedState.Add(3);
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ON_CALL(objNotifyTerminated,
            GetIntArray(
                    CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY, _))
            .WillByDefault(DoAll(
                    SetArgReferee<1>(bKeyExistValue), Return(objEventForInitRegOnTerminatedState)));

    ImsVector<IMS_SINT32> objEventWithWtForInitRegOnTerminatedState;
    objEventWithWtForInitRegOnTerminatedState.Add(2);
    objEventWithWtForInitRegOnTerminatedState.Add(5);
    ON_CALL(objNotifyTerminated,
            GetIntArray(
                    CarrierConfig::Ims ::
                            KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY,
                    _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue),
                    Return(objEventWithWtForInitRegOnTerminatedState)));

    // WHEN
    m_pAosNConfiguration->InitBundleForNotifyTerminatedForInitReg(&objCarrierConfig);

    // THEN
    EXPECT_EQ(40, m_pAosNConfiguration->GetNotifyWaitTime());

    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_PROBATION &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_FALSE(IAosNConfiguration::NOTIFY_TERMINATED_UNREGISTERED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegistration()));

    EXPECT_FALSE(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegWithWaitTime()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegWithWaitTime()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegWithWaitTime()));
}

TEST_F(AosNConfigurationTest, InitBundleConfigForNotifyTerminatedForInitRegWithoutIntArrayKeys)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objNotifyTerminated;

    ON_CALL(objCarrierConfig,
            GetBundle(CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE))
            .WillByDefault(Return(&objNotifyTerminated));
    ImsVector<IMS_SINT32> objEventForInitRegOnTerminatedState;
    IMS_BOOL bKeyExistValue = IMS_FALSE;
    ON_CALL(objNotifyTerminated,
            GetIntArray(
                    CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY, _))
            .WillByDefault(DoAll(
                    SetArgReferee<1>(bKeyExistValue), Return(objEventForInitRegOnTerminatedState)));
    ImsVector<IMS_SINT32> objEventWithWtForInitRegOnTerminatedState;
    ON_CALL(objNotifyTerminated,
            GetIntArray(
                    CarrierConfig::Ims ::
                            KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY,
                    _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue),
                    Return(objEventWithWtForInitRegOnTerminatedState)));

    // WHEN
    m_pAosNConfiguration->InitBundleForNotifyTerminatedForInitReg(&objCarrierConfig);

    // THEN
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_PROBATION &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_FALSE(IAosNConfiguration::NOTIFY_TERMINATED_UNREGISTERED &
            (m_pAosNConfiguration->GetNotifyEventForInitialRegistration()));

    EXPECT_EQ(0, m_pAosNConfiguration->GetNotifyEventForInitialRegWithWaitTime());
}

TEST_F(AosNConfigurationTest, InitBundleConfigForPcscfRecoveryConditions)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objPcscfRecoveryConditionsBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE))
            .WillByDefault(Return(&objPcscfRecoveryConditionsBundle));
    ON_CALL(objPcscfRecoveryConditionsBundle,
            GetInt(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_MAX_CNT_INT, 3))
            .WillByDefault(Return(3));
    ON_CALL(objPcscfRecoveryConditionsBundle,
            GetInt(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_WAIT_TIME_SEC_INT, 20))
            .WillByDefault(Return(20));
    ON_CALL(objPcscfRecoveryConditionsBundle,
            GetInt(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_BASE_TIME_SEC_INT, 20))
            .WillByDefault(Return(20));
    ON_CALL(objPcscfRecoveryConditionsBundle,
            GetInt(CarrierConfig::Ims::KEY_PCSCF_RECOVERY_MAX_TIME_SEC_INT, 1800))
            .WillByDefault(Return(1800));

    // WHEN
    m_pAosNConfiguration->InitBundleForPcscfRecoveryConditions(&objCarrierConfig);

    // THEN
    EXPECT_EQ(3, m_pAosNConfiguration->GetPcscfRecoveryMaxRetryCnt());
    EXPECT_EQ(20, m_pAosNConfiguration->GetPcscfRecoveryWaitTime());
    EXPECT_EQ(20, m_pAosNConfiguration->GetPcscfRecoveryBaseTime());
    EXPECT_EQ(1800, m_pAosNConfiguration->GetPcscfRecoveryMaxTime());
}

TEST_F(AosNConfigurationTest, InitBundleConfigForRegErrCodeWithRaTime)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objRegErrCodeWithRaTimeBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE))
            .WillByDefault(Return(&objRegErrCodeWithRaTimeBundle));
    ON_CALL(objRegErrCodeWithRaTimeBundle,
            GetBoolean(
                    CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL, IMS_FALSE))
            .WillByDefault(Return(IMS_FALSE));
    ImsVector<IMS_SINT32> objRegErrCodeWithRaTime;
    objRegErrCodeWithRaTime.Add(486);
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ON_CALL(objRegErrCodeWithRaTimeBundle,
            GetIntArray(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objRegErrCodeWithRaTime)));
    ImsVector<IMS_SINT32> objReregErrCodeWithRaTime;
    objReregErrCodeWithRaTime.Add(486);
    ON_CALL(objRegErrCodeWithRaTimeBundle,
            GetIntArray(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objReregErrCodeWithRaTime)));

    // WHEN
    m_pAosNConfiguration->InitBundleForRegErrCodeWithRaTime(&objCarrierConfig);

    // THEN
    EXPECT_FALSE(m_pAosNConfiguration->IsRegErrCodeWithRetryAfterTimeOnlyDefined());
    ImsVector<IMS_SINT32>& objErrCode = m_pAosNConfiguration->GetRegErrCodeWithRetryAfterTime();
    EXPECT_EQ(1, objErrCode.GetSize());
    EXPECT_EQ(486, objErrCode.GetAt(0));
    ImsVector<IMS_SINT32>& objReregErrCode =
            m_pAosNConfiguration->GetReregErrCodeWithRetryAfterTime();
    EXPECT_EQ(1, objReregErrCode.GetSize());
    EXPECT_EQ(486, objReregErrCode.GetAt(0));
}

TEST_F(AosNConfigurationTest, InitBundleConfigForRegErrCodeWithRaTimeWithoutIntArrayKeys)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objRegErrCodeWithRaTimeBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE))
            .WillByDefault(Return(&objRegErrCodeWithRaTimeBundle));
    ImsVector<IMS_SINT32> objRegErrCodeWithRaTime;
    objRegErrCodeWithRaTime.Add(486);
    IMS_BOOL bKeyExistValue = IMS_FALSE;
    ON_CALL(objRegErrCodeWithRaTimeBundle,
            GetIntArray(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objRegErrCodeWithRaTime)));
    ImsVector<IMS_SINT32> objReregErrCodeWithRaTime;
    objReregErrCodeWithRaTime.Add(486);
    ON_CALL(objRegErrCodeWithRaTimeBundle,
            GetIntArray(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objReregErrCodeWithRaTime)));

    // WHEN
    m_pAosNConfiguration->InitBundleForRegErrCodeWithRaTime(&objCarrierConfig);

    // THEN
    ImsVector<IMS_SINT32>& objErrCode = m_pAosNConfiguration->GetRegErrCodeWithRetryAfterTime();
    EXPECT_EQ(0, objErrCode.GetSize());
    ImsVector<IMS_SINT32>& objReregErrCode =
            m_pAosNConfiguration->GetReregErrCodeWithRetryAfterTime();
    EXPECT_EQ(0, objReregErrCode.GetSize());
}

TEST_F(AosNConfigurationTest, InitBundleConfigForRegRetryInterval)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objRegRetryInterval;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_BUNDLE))
            .WillByDefault(Return(&objRegRetryInterval));
    ON_CALL(objRegRetryInterval,
            GetBoolean(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_USED_FOR_SUB_BOOL, IMS_TRUE))
            .WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objRegRetryRandomUpperValueSec;
    objRegRetryRandomUpperValueSec.Add(0);
    objRegRetryRandomUpperValueSec.Add(0);
    objRegRetryRandomUpperValueSec.Add(15);
    objRegRetryRandomUpperValueSec.Add(0);
    objRegRetryRandomUpperValueSec.Add(0);
    objRegRetryRandomUpperValueSec.Add(0);
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ON_CALL(objRegRetryInterval,
            GetIntArray(
                    CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY, _))
            .WillByDefault(DoAll(
                    SetArgReferee<1>(bKeyExistValue), Return(objRegRetryRandomUpperValueSec)));
    ImsVector<IMS_SINT32> objRegRetryIntervalSec;
    objRegRetryIntervalSec.Add(30);
    objRegRetryIntervalSec.Add(30);
    objRegRetryIntervalSec.Add(60);
    objRegRetryIntervalSec.Add(120);
    objRegRetryIntervalSec.Add(480);
    objRegRetryIntervalSec.Add(900);
    ON_CALL(objRegRetryInterval,
            GetIntArray(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objRegRetryIntervalSec)));

    // WHEN
    m_pAosNConfiguration->InitBundleForRegRetryInterval(&objCarrierConfig);

    // THEN
    EXPECT_TRUE(m_pAosNConfiguration->IsRegRetryIntervalsUsedForSub());
    ImsVector<IMS_SINT32>& objRandomInterval = m_pAosNConfiguration->GetRegRandomRetryIntervals();
    ImsVector<IMS_SINT32>& objInterval = m_pAosNConfiguration->GetRegRetryIntervals();
    EXPECT_EQ(objRandomInterval.GetSize(), objInterval.GetSize());
}

TEST_F(AosNConfigurationTest, InitBundleConfigForRegRetryIntervalWithoutIntArrayKeys)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objRegRetryInterval;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_BUNDLE))
            .WillByDefault(Return(&objRegRetryInterval));
    ImsVector<IMS_SINT32> objRegRetryRandomUpperValueSec;
    objRegRetryRandomUpperValueSec.Add(15);
    IMS_BOOL bKeyExistValue = IMS_FALSE;
    ON_CALL(objRegRetryInterval,
            GetIntArray(
                    CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY, _))
            .WillByDefault(DoAll(
                    SetArgReferee<1>(bKeyExistValue), Return(objRegRetryRandomUpperValueSec)));
    ImsVector<IMS_SINT32> objRegRetryIntervalSec;
    objRegRetryIntervalSec.Add(30);
    ON_CALL(objRegRetryInterval,
            GetIntArray(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objRegRetryIntervalSec)));

    // WHEN
    m_pAosNConfiguration->InitBundleForRegRetryInterval(&objCarrierConfig);

    // THEN
    ImsVector<IMS_SINT32>& objRandomInterval = m_pAosNConfiguration->GetRegRandomRetryIntervals();
    ImsVector<IMS_SINT32>& objInterval = m_pAosNConfiguration->GetRegRetryIntervals();
    EXPECT_EQ(0, objRandomInterval.GetSize());
    EXPECT_EQ(0, objInterval.GetSize());
}

TEST_F(AosNConfigurationTest, InitBundleConfigForSubErrCodeForInitReg)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objSubErrCodeForInitRegBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE))
            .WillByDefault(Return(&objSubErrCodeForInitRegBundle));
    ON_CALL(objSubErrCodeForInitRegBundle,
            GetInt(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_RETRY_MAX_CNT_INT, 0))
            .WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objSubErrCodeForInitReg;
    objSubErrCodeForInitReg.Add(408);
    objSubErrCodeForInitReg.Add(504);
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ON_CALL(objSubErrCodeForInitRegBundle,
            GetIntArray(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objSubErrCodeForInitReg)));

    // WHEN
    m_pAosNConfiguration->InitBundleForSubErrCodeForInitReg(&objCarrierConfig);

    // THEN
    EXPECT_EQ(2, m_pAosNConfiguration->GetRetryCountSubErrorRegRequired());
    ImsVector<IMS_SINT32>& objErrCode = m_pAosNConfiguration->GetSubErrorRegRequired();
    EXPECT_EQ(2, objErrCode.GetSize());
}

TEST_F(AosNConfigurationTest,
        InitBundleConfigForSubErrCodeForInitRegWithoutKeyForSubErrCodeForInitReg)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objSubErrCodeForInitRegBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE))
            .WillByDefault(Return(&objSubErrCodeForInitRegBundle));
    ImsVector<IMS_SINT32> objSubErrCodeForInitReg;
    objSubErrCodeForInitReg.Add(408);
    IMS_BOOL bKeyExistValue = IMS_FALSE;
    ON_CALL(objSubErrCodeForInitRegBundle,
            GetIntArray(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objSubErrCodeForInitReg)));

    // WHEN
    m_pAosNConfiguration->InitBundleForSubErrCodeForInitReg(&objCarrierConfig);

    // THEN
    ImsVector<IMS_SINT32>& objErrCode = m_pAosNConfiguration->GetSubErrorRegRequired();
    EXPECT_EQ(0, objErrCode.GetSize());
}

TEST_F(AosNConfigurationTest, InitBundleConfigForSubErrCodeForTerminated)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objSubErrCodeForTerminatedBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE))
            .WillByDefault(Return(&objSubErrCodeForTerminatedBundle));
    ON_CALL(objSubErrCodeForTerminatedBundle,
            GetInt(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_WITH_RETRY_MAX_CNT_INT, 0))
            .WillByDefault(Return(0));
    ImsVector<IMS_SINT32> objSubErrCodeForTerminated;
    objSubErrCodeForTerminated.Add(491);
    objSubErrCodeForTerminated.Add(500);
    objSubErrCodeForTerminated.Add(606);
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ON_CALL(objSubErrCodeForTerminatedBundle,
            GetIntArray(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objSubErrCodeForTerminated)));

    // WHEN
    m_pAosNConfiguration->InitBundleForSubErrCodeForTerminated(&objCarrierConfig);

    // THEN
    EXPECT_EQ(0, m_pAosNConfiguration->GetRetryCountSubErrorSubTerminated());
    ImsVector<IMS_SINT32>& objErrCode = m_pAosNConfiguration->GetSubErrorSubTerminated();
    EXPECT_EQ(3, objErrCode.GetSize());
}

TEST_F(AosNConfigurationTest,
        InitBundleConfigForSubErrCodeForTerminatedWithoutKeyForSubErrCodeForTerminated)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objSubErrCodeForTerminatedBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE))
            .WillByDefault(Return(&objSubErrCodeForTerminatedBundle));
    ImsVector<IMS_SINT32> objSubErrCodeForTerminated;
    objSubErrCodeForTerminated.Add(491);
    IMS_BOOL bKeyExistValue = IMS_FALSE;
    ON_CALL(objSubErrCodeForTerminatedBundle,
            GetIntArray(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objSubErrCodeForTerminated)));

    // WHEN
    m_pAosNConfiguration->InitBundleForSubErrCodeForTerminated(&objCarrierConfig);

    // THEN
    ImsVector<IMS_SINT32>& objErrCode = m_pAosNConfiguration->GetSubErrorSubTerminated();
    EXPECT_EQ(0, objErrCode.GetSize());
}

TEST_F(AosNConfigurationTest, InitBundleConfigForBundleForWfcErrMessage)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objWfcErrMessageBundle;

    AString strWfcErrorReg403("REG90 - Unable to Connect");
    AString strWfcErrorReg500("REG91 - Unable to Connect");
    AString strWfcErrorNotSupportedCountry("REG92 - Wi-Fi Calling isn't supported in this country");
    AString strWfcErrorSub403("REG09 - Missing 911 Address");
    AString strWfcErrorNotifyTerminated("REG09 - Missing 911 Address");
    AString strWfcErrorOtherFailures = AString::ConstNull();
    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::ImsWfc::KEY_WFC_ERR_MESSAGE_BUNDLE))
            .WillByDefault(Return(&objWfcErrMessageBundle));
    ON_CALL(objWfcErrMessageBundle, GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_REG_403_STRING, _))
            .WillByDefault(Return(strWfcErrorReg403));
    ON_CALL(objWfcErrMessageBundle, GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_REG_500_STRING, _))
            .WillByDefault(Return(strWfcErrorReg500));
    ON_CALL(objWfcErrMessageBundle,
            GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING, _))
            .WillByDefault(Return(strWfcErrorNotSupportedCountry));
    ON_CALL(objWfcErrMessageBundle, GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_SUB_403_STRING, _))
            .WillByDefault(Return(strWfcErrorSub403));
    ON_CALL(objWfcErrMessageBundle,
            GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_NOTIFY_TERMINATED_STRING, _))
            .WillByDefault(Return(strWfcErrorNotifyTerminated));
    ON_CALL(objWfcErrMessageBundle,
            GetString(CarrierConfig::ImsWfc::KEY_WFC_ERR_OTHER_FAILURES_STRING, _))
            .WillByDefault(Return(strWfcErrorOtherFailures));

    // WHEN
    m_pAosNConfiguration->InitBundleForWfcErrMessage(&objCarrierConfig);

    // THEN
    EXPECT_TRUE(m_pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::ImsWfc::WFC_ERROR_REG_403));
    EXPECT_TRUE(m_pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::ImsWfc::WFC_ERROR_REG_500));
    EXPECT_TRUE(m_pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::ImsWfc::WFC_ERROR_NOT_SUPPORTED_COUNTRY));
    EXPECT_TRUE(m_pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::ImsWfc::WFC_ERROR_SUB_403));
    EXPECT_TRUE(m_pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::ImsWfc::WFC_ERROR_NOTIFY_TERMINATED));
    EXPECT_FALSE(m_pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::ImsWfc::WFC_ERROR_OTHER_FAILURES));
}

TEST_F(AosNConfigurationTest, ShouldInitializeExtraRegErrContainersWhenInitBundlesContainers)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objExtraRegErr;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_BUNDLE))
            .WillByDefault(Return(&objExtraRegErr));
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(400);
    ON_CALL(objExtraRegErr, GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraRegErrCode)));
    ImsVector<IMS_SINT32> objExtraReregErrCode;
    objExtraReregErrCode.Add(500);
    ON_CALL(objExtraRegErr,
            GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraReregErrCode)));
    ImsVector<IMS_SINT32> objExtraRegErrWaitTimeSec;
    objExtraRegErrWaitTimeSec.Add(30);
    ON_CALL(objExtraRegErr,
            GetIntArray(CarrierConfig::Ims::KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objExtraRegErrWaitTimeSec)));
    m_pAosNConfiguration->InitBundleForExtraRegErr(&objCarrierConfig);

    // WHEN
    m_pAosNConfiguration->InitBundlesContainers();

    // THEN
    ImsVector<IMS_SINT32>& objRegErrCode = m_pAosNConfiguration->GetExtraRegErrCode();
    EXPECT_EQ(0, objRegErrCode.GetSize());
    ImsVector<IMS_SINT32>& objReregErrCode = m_pAosNConfiguration->GetExtraReregErrCode();
    EXPECT_EQ(0, objReregErrCode.GetSize());
    ImsVector<IMS_SINT32>& objRegErrWaitTimeSec = m_pAosNConfiguration->GetExtraRegErrWaitTime();
    EXPECT_EQ(0, objRegErrWaitTimeSec.GetSize());
}

TEST_F(AosNConfigurationTest, ShouldInitializeNotifyTerminatedContainersWhenInitBundlesContainers)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objNotifyTerminated;

    ON_CALL(objCarrierConfig,
            GetBundle(CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE))
            .WillByDefault(Return(&objNotifyTerminated));
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ImsVector<IMS_SINT32> objEventForInitRegOnTerminatedState;
    objEventForInitRegOnTerminatedState.Add(1);
    objEventForInitRegOnTerminatedState.Add(2);
    ON_CALL(objNotifyTerminated,
            GetIntArray(
                    CarrierConfig::Ims::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY, _))
            .WillByDefault(DoAll(
                    SetArgReferee<1>(bKeyExistValue), Return(objEventForInitRegOnTerminatedState)));
    ImsVector<IMS_SINT32> objEventWithWtForInitRegOnTerminatedState;
    objEventWithWtForInitRegOnTerminatedState.Add(2);
    objEventWithWtForInitRegOnTerminatedState.Add(5);
    ON_CALL(objNotifyTerminated,
            GetIntArray(
                    CarrierConfig::Ims::
                            KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY,
                    _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue),
                    Return(objEventWithWtForInitRegOnTerminatedState)));
    m_pAosNConfiguration->InitBundleForNotifyTerminatedForInitReg(&objCarrierConfig);

    // WHEN
    m_pAosNConfiguration->InitBundlesContainers();

    // THEN
    ImsVector<IMS_SINT32>& objEvents =
            m_pAosNConfiguration->GetNotifyTerminatedForInitRegUsedEvent();
    EXPECT_EQ(3, objEvents.GetSize());
    ImsVector<IMS_SINT32>& objEventsWithWaitTime =
            m_pAosNConfiguration->GetNotifyTerminatedForInitRegUsedEventWithWaitTime();
    EXPECT_EQ(0, objEventsWithWaitTime.GetSize());
}

TEST_F(AosNConfigurationTest,
        ShouldInitializeRegErrCodeWithRaTimeContainersWhenInitBundlesContainers)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objRegErrCodeWithRaTimeBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE))
            .WillByDefault(Return(&objRegErrCodeWithRaTimeBundle));
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ImsVector<IMS_SINT32> objRegErrCodeWithRaTime;
    objRegErrCodeWithRaTime.Add(486);
    ON_CALL(objRegErrCodeWithRaTimeBundle,
            GetIntArray(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objRegErrCodeWithRaTime)));
    ImsVector<IMS_SINT32> objReregErrCodeWithRaTime;
    objReregErrCodeWithRaTime.Add(486);
    ON_CALL(objRegErrCodeWithRaTimeBundle,
            GetIntArray(CarrierConfig::Ims::KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objReregErrCodeWithRaTime)));
    m_pAosNConfiguration->InitBundleForRegErrCodeWithRaTime(&objCarrierConfig);

    // WHEN
    m_pAosNConfiguration->InitBundlesContainers();

    // THEN
    ImsVector<IMS_SINT32>& objRegErrCode = m_pAosNConfiguration->GetRegErrCodeWithRetryAfterTime();
    EXPECT_EQ(0, objRegErrCode.GetSize());
    ImsVector<IMS_SINT32>& objReregErrCode =
            m_pAosNConfiguration->GetReregErrCodeWithRetryAfterTime();
    EXPECT_EQ(0, objReregErrCode.GetSize());
}

TEST_F(AosNConfigurationTest, ShouldInitializeRegRetryIntervalContainersWhenInitBundlesContainers)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objRegRetryInterval;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_BUNDLE))
            .WillByDefault(Return(&objRegRetryInterval));
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ImsVector<IMS_SINT32> objRegRetryRandomUpperValueSec;
    objRegRetryRandomUpperValueSec.Add(0);
    ON_CALL(objRegRetryInterval,
            GetIntArray(
                    CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY, _))
            .WillByDefault(DoAll(
                    SetArgReferee<1>(bKeyExistValue), Return(objRegRetryRandomUpperValueSec)));
    ImsVector<IMS_SINT32> objRegRetryIntervalSec;
    objRegRetryIntervalSec.Add(30);
    ON_CALL(objRegRetryInterval,
            GetIntArray(CarrierConfig::Ims::KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY, _))
            .WillByDefault(DoAll(SetArgReferee<1>(bKeyExistValue), Return(objRegRetryIntervalSec)));
    m_pAosNConfiguration->InitBundleForRegRetryInterval(&objCarrierConfig);

    // WHEN
    m_pAosNConfiguration->InitBundlesContainers();

    // THEN
    ImsVector<IMS_SINT32>& objIntervalSec = m_pAosNConfiguration->GetRegRetryIntervals();
    EXPECT_EQ(0, objIntervalSec.GetSize());
    ImsVector<IMS_SINT32>& objRandomUpperValueSec =
            m_pAosNConfiguration->GetRegRandomRetryIntervals();
    EXPECT_EQ(0, objRandomUpperValueSec.GetSize());
}

TEST_F(AosNConfigurationTest,
        ShouldInitializeSubErrCodeForInitRegContainersWhenInitBundlesContainers)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objSubErrCodeForInitRegBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE))
            .WillByDefault(Return(&objSubErrCodeForInitRegBundle));
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ImsVector<IMS_SINT32> objSubErrCodeForInitReg;
    objSubErrCodeForInitReg.Add(408);
    ON_CALL(objSubErrCodeForInitRegBundle,
            GetIntArray(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objSubErrCodeForInitReg)));
    m_pAosNConfiguration->InitBundleForSubErrCodeForInitReg(&objCarrierConfig);

    // WHEN
    m_pAosNConfiguration->InitBundlesContainers();

    // THEN
    ImsVector<IMS_SINT32>& objErrCodeForInitReg = m_pAosNConfiguration->GetSubErrorRegRequired();
    EXPECT_EQ(0, objErrCodeForInitReg.GetSize());
}

TEST_F(AosNConfigurationTest,
        ShouldInitializeSubErrCodeForTerminatedContainersWhenInitBundlesContainers)
{
    // GIVEN
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objSubErrCodeForTerminatedBundle;

    ON_CALL(objCarrierConfig, GetBundle(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE))
            .WillByDefault(Return(&objSubErrCodeForTerminatedBundle));
    IMS_BOOL bKeyExistValue = IMS_TRUE;
    ImsVector<IMS_SINT32> objSubErrCodeForTerminated;
    objSubErrCodeForTerminated.Add(491);
    ON_CALL(objSubErrCodeForTerminatedBundle,
            GetIntArray(CarrierConfig::Ims::KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY, _))
            .WillByDefault(
                    DoAll(SetArgReferee<1>(bKeyExistValue), Return(objSubErrCodeForTerminated)));
    m_pAosNConfiguration->InitBundleForSubErrCodeForTerminated(&objCarrierConfig);

    // WHEN
    m_pAosNConfiguration->InitBundlesContainers();

    // THEN
    ImsVector<IMS_SINT32>& objErrCodeForTerminated =
            m_pAosNConfiguration->GetSubErrorSubTerminated();
    EXPECT_EQ(0, objErrCodeForTerminated.GetSize());
}

TEST_F(AosNConfigurationTest, ShouldNoFeaturesContainedIfNoFeaturesUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeaturesInLimitedReg;
    objUnavailableFeaturesInLimitedReg.Clear();
    MockICarrierConfig objCarrierConfig;
    ON_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_UNAVAILABLE_FEATURES_IN_LIMITED_REG_INT_ARRAY, _))
            .WillByDefault(Return(objUnavailableFeaturesInLimitedReg));

    // WHEN
    ImsVector<IMS_SINT32> objFeatures = m_pAosNConfiguration->GetUnavailableFeaturesInLimitedReg();

    // THEN
    EXPECT_EQ(objFeatures.GetSize(), 0);
    EXPECT_FALSE(objFeatures.Contains(CarrierConfig::Ims::REG_FEATURE_MMTEL));
    EXPECT_FALSE(objFeatures.Contains(CarrierConfig::Ims::REG_FEATURE_VIDEO));
    EXPECT_FALSE(objFeatures.Contains(CarrierConfig::Ims::REG_FEATURE_TEXT));
    EXPECT_FALSE(objFeatures.Contains(CarrierConfig::Ims::REG_FEATURE_SMS));
}
