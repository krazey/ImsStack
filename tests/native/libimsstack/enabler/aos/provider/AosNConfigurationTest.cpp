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
using ::testing::Return;

class AosNConfigurationTest : public ::testing::Test
{
public:
    AosNConfiguration* pAosNConfiguration;

protected:
    virtual void SetUp() override
    {
        pAosNConfiguration = new AosNConfiguration();
        ASSERT_TRUE(pAosNConfiguration != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosNConfiguration)
        {
            delete pAosNConfiguration;
        }
    }

    void InitConfig(IN const ICarrierConfig* piCc) { pAosNConfiguration->InitConfig(piCc); }
    void InitAssetsConfig(IN const ICarrierConfig* piCc)
    {
        pAosNConfiguration->InitAssetsConfig(piCc);
    }
    void InitBundleConfig(IN const ICarrierConfig* piCc) { pAosNConfiguration->InitBundle(piCc); }
};

TEST_F(AosNConfigurationTest, InitConfig)
{
    // init AosCarrierConfig
    EXPECT_FALSE(pAosNConfiguration->IsEmergencySmsOverImsSupported());
    EXPECT_FALSE(pAosNConfiguration->IsVoLteAvailable());
    EXPECT_FALSE(pAosNConfiguration->IsVtAvailable());
    EXPECT_FALSE(pAosNConfiguration->IsWfcImsAvailable());
    EXPECT_FALSE(pAosNConfiguration->IsRttSupported());
    // bCarrierCrossSimImsAvailable (IMS_FALSE)
    EXPECT_FALSE(pAosNConfiguration->IsTtySupported());
    EXPECT_FALSE(pAosNConfiguration->IsImsOverNrEnabled());
    EXPECT_EQ(CarrierConfig::USSD_OVER_CS_PREFERRED, pAosNConfiguration->GetUssdMethod());
    EXPECT_EQ(0, pAosNConfiguration->GetPcscfDiscoveryMethod().GetSize());
    EXPECT_FALSE(pAosNConfiguration->IsImsSingleRegistrationRequired());
    EXPECT_EQ(5060, pAosNConfiguration->GetPcscfPort());
    // KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL (IMS_FALSE)
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP,
            pAosNConfiguration->GetSipPreferredTransport());
    EXPECT_EQ(1500, pAosNConfiguration->GetIpv4MtuSize());
    EXPECT_EQ(1500, pAosNConfiguration->GetIpv6MtuSize());
    // KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY
    EXPECT_FALSE(pAosNConfiguration->IsIpsecEnabled());
    EXPECT_EQ(0, pAosNConfiguration->GetIpsecAuthenticationAlgorithms().GetSize());
    EXPECT_EQ(0, pAosNConfiguration->GetIpsecEncryptionAlgorithms().GetSize());

    // KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(30000, pAosNConfiguration->GetRegistrationRetryBaseTime());
    EXPECT_EQ(1800000, pAosNConfiguration->GetRegistrationRetryMaxTime());
    EXPECT_TRUE(pAosNConfiguration->IsSubscription());
    // KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(2, pAosNConfiguration->GetSupportedRats().GetSize());

    EXPECT_TRUE(pAosNConfiguration->IsVoLteRoamingAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsSmsOverImsSupported());
    EXPECT_EQ(0, pAosNConfiguration->GetSmsOverImsSupportedRats().GetSize());
    EXPECT_FALSE(pAosNConfiguration->IsEmergencyCallbackModeSupported());
    // KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY
    EXPECT_EQ(10000, pAosNConfiguration->GetEmergencyRegistrationTimerMillis());
    // KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT

    EXPECT_FALSE(pAosNConfiguration->IsRegistrationEventForCatRequired());
    EXPECT_FALSE(pAosNConfiguration->IsUnSubscription());
    EXPECT_EQ(1, pAosNConfiguration->GetIsimIndexForImpu());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_DSCP_NONE, pAosNConfiguration->GetPreferredImsDscp());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED,
            pAosNConfiguration->GetRegistrationPreferredAccessTypeFeatureTag());

    EXPECT_FALSE(pAosNConfiguration->IsGeolocationPidfSupported(1));
    EXPECT_EQ(0, pAosNConfiguration->GetImsIdentityPriority().GetSize());
    EXPECT_EQ(0, pAosNConfiguration->GetRegPermanentErrCode().GetSize());
    EXPECT_EQ(0, pAosNConfiguration->GetUpdateRegistrationWithRatChange().GetSize());

    EXPECT_FALSE(pAosNConfiguration->IsEmergencyPdnWithEmergencyCallEndReleased());
    EXPECT_EQ(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK,
            pAosNConfiguration->GetPreferredEmergencyRegistration());
    EXPECT_FALSE(pAosNConfiguration->IsSupportLimitedAdminSmsMode());
    EXPECT_EQ(0, pAosNConfiguration->GetRegistrationPrivateHeader());

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
    EXPECT_CALL(
            objCarrierConfig, GetIntArray(CarrierConfig::KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY))
            .Times(2)
            .WillRepeatedly(Return(objCarrierNrAvailabilities));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::KEY_CARRIER_USSD_METHOD_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(CarrierConfig::USSD_OVER_IMS_PREFERRED));
    ImsVector<IMS_SINT32> objPcscfDiscoveryMethod;
    objPcscfDiscoveryMethod.Clear();
    objPcscfDiscoveryMethod.Add(0);
    EXPECT_CALL(
            objCarrierConfig, GetIntArray(CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY))
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
            GetIntArray(CarrierConfig::Ims::KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY))
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
            GetIntArray(CarrierConfig::Ims::KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY))
            .Times(2)
            .WillRepeatedly(Return(objAuthAlgo));

    ImsVector<IMS_SINT32> objEncryAlgo;
    objEncryAlgo.Clear();
    objEncryAlgo.Add(0);
    objEncryAlgo.Add(1);
    objEncryAlgo.Add(2);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY))
            .Times(2)
            .WillRepeatedly(Return(objEncryAlgo));

    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT, -1))
            .Times(2)
            .WillRepeatedly(Return(600000));
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
    EXPECT_CALL(objCarrierConfig, GetIntArray(CarrierConfig::Ims::KEY_SUPPORTED_RATS_INT_ARRAY))
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
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY))
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
                    CarrierConfig::ImsEmergency::KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY))
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
            GetBoolean(CarrierConfig::Ims::KEY_REGISTRATION_EVENT_FOR_CAT_REQUIRED_BOOL, IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL, IMS_FALSE))
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
            GetIntArray(CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY))
            .Times(2)
            .WillRepeatedly(Return(objGeolocationPidfInSipRegisterSupport));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(0);
    objImsIdentityPriority.Add(1);
    EXPECT_CALL(
            objCarrierConfig, GetIntArray(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY))
            .Times(2)
            .WillRepeatedly(Return(objImsIdentityPriority));

    ImsVector<IMS_SINT32> objRegistrationPermanentErrorCode;
    objRegistrationPermanentErrorCode.Clear();
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY))
            .Times(2)
            .WillRepeatedly(Return(objRegistrationPermanentErrorCode));

    ImsVector<IMS_SINT32> objUpdateRegistrationWithRatChange;
    objUpdateRegistrationWithRatChange.Clear();
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY))
            .Times(2)
            .WillRepeatedly(Return(objUpdateRegistrationWithRatChange));

    /// imsemergency.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::
                               KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_END_BOOL,
                    IMS_FALSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_PREFERRED_EMERGENCY_REGISTRATION_INT, -1))
            .Times(2)
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));

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

    InitConfig(static_cast<ICarrierConfig*>(&objCarrierConfig));

    EXPECT_TRUE(pAosNConfiguration->IsEmergencySmsOverImsSupported());
    EXPECT_TRUE(pAosNConfiguration->IsVoLteAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsVtAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsWfcImsAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsRttSupported());
    // KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL
    EXPECT_TRUE(pAosNConfiguration->IsTtySupported());
    EXPECT_TRUE(pAosNConfiguration->IsImsOverNrEnabled());
    EXPECT_EQ(CarrierConfig::USSD_OVER_IMS_PREFERRED, pAosNConfiguration->GetUssdMethod());
    EXPECT_EQ(1, pAosNConfiguration->GetPcscfDiscoveryMethod().GetSize());
    EXPECT_FALSE(pAosNConfiguration->IsImsSingleRegistrationRequired());
    EXPECT_EQ(5060, pAosNConfiguration->GetPcscfPort());
    // KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_TRANSPORT_TCP,
            pAosNConfiguration->GetSipPreferredTransport());
    EXPECT_EQ(1500, pAosNConfiguration->GetIpv4MtuSize());
    EXPECT_EQ(1500, pAosNConfiguration->GetIpv6MtuSize());
    // KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY
    EXPECT_TRUE(pAosNConfiguration->IsIpsecEnabled());
    EXPECT_EQ(2, pAosNConfiguration->GetIpsecAuthenticationAlgorithms().GetSize());
    EXPECT_EQ(3, pAosNConfiguration->GetIpsecEncryptionAlgorithms().GetSize());

    // KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(30000, pAosNConfiguration->GetRegistrationRetryBaseTime());
    EXPECT_EQ(1800000, pAosNConfiguration->GetRegistrationRetryMaxTime());
    EXPECT_TRUE(pAosNConfiguration->IsSubscription());
    // KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(2, pAosNConfiguration->GetSupportedRats().GetSize());

    EXPECT_TRUE(pAosNConfiguration->IsVoLteRoamingAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsSmsOverImsSupported());
    EXPECT_EQ(2, pAosNConfiguration->GetSmsOverImsSupportedRats().GetSize());
    EXPECT_TRUE(pAosNConfiguration->IsEmergencyCallbackModeSupported());
    // KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY
    EXPECT_EQ(1000, pAosNConfiguration->GetEmergencyRegistrationTimerMillis());
    // KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT

    EXPECT_FALSE(pAosNConfiguration->IsRegistrationEventForCatRequired());
    EXPECT_FALSE(pAosNConfiguration->IsUnSubscription());
    EXPECT_EQ(1, pAosNConfiguration->GetIsimIndexForImpu());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_DSCP_NONE, pAosNConfiguration->GetPreferredImsDscp());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED,
            pAosNConfiguration->GetRegistrationPreferredAccessTypeFeatureTag());

    EXPECT_TRUE(pAosNConfiguration->IsGeolocationPidfSupported(2));
    EXPECT_FALSE(pAosNConfiguration->IsGeolocationPidfSupported(1));
    EXPECT_EQ(2, pAosNConfiguration->GetImsIdentityPriority().GetSize());
    EXPECT_EQ(0, pAosNConfiguration->GetRegPermanentErrCode().GetSize());
    EXPECT_EQ(0, pAosNConfiguration->GetUpdateRegistrationWithRatChange().GetSize());

    EXPECT_FALSE(pAosNConfiguration->IsEmergencyPdnWithEmergencyCallEndReleased());
    EXPECT_EQ(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK,
            pAosNConfiguration->GetPreferredEmergencyRegistration());
    EXPECT_FALSE(pAosNConfiguration->IsSupportLimitedAdminSmsMode());
    EXPECT_EQ(0, pAosNConfiguration->GetRegistrationPrivateHeader());

    // 2nd call to test invoking InitConfig() several times
    InitConfig(static_cast<ICarrierConfig*>(&objCarrierConfig));

    EXPECT_TRUE(pAosNConfiguration->IsEmergencySmsOverImsSupported());
    EXPECT_TRUE(pAosNConfiguration->IsVoLteAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsVtAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsWfcImsAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsRttSupported());
    // KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL
    EXPECT_TRUE(pAosNConfiguration->IsTtySupported());
    EXPECT_TRUE(pAosNConfiguration->IsImsOverNrEnabled());
    EXPECT_EQ(CarrierConfig::USSD_OVER_IMS_PREFERRED, pAosNConfiguration->GetUssdMethod());
    EXPECT_EQ(1, pAosNConfiguration->GetPcscfDiscoveryMethod().GetSize());
    EXPECT_FALSE(pAosNConfiguration->IsImsSingleRegistrationRequired());
    EXPECT_EQ(5060, pAosNConfiguration->GetPcscfPort());
    // KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_TRANSPORT_TCP,
            pAosNConfiguration->GetSipPreferredTransport());
    EXPECT_EQ(1500, pAosNConfiguration->GetIpv4MtuSize());
    EXPECT_EQ(1500, pAosNConfiguration->GetIpv6MtuSize());
    // KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY
    EXPECT_TRUE(pAosNConfiguration->IsIpsecEnabled());
    EXPECT_EQ(2, pAosNConfiguration->GetIpsecAuthenticationAlgorithms().GetSize());
    EXPECT_EQ(3, pAosNConfiguration->GetIpsecEncryptionAlgorithms().GetSize());

    // KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(30000, pAosNConfiguration->GetRegistrationRetryBaseTime());
    EXPECT_EQ(1800000, pAosNConfiguration->GetRegistrationRetryMaxTime());
    EXPECT_TRUE(pAosNConfiguration->IsSubscription());
    // KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT
    EXPECT_EQ(2, pAosNConfiguration->GetSupportedRats().GetSize());

    EXPECT_TRUE(pAosNConfiguration->IsVoLteRoamingAvailable());
    EXPECT_TRUE(pAosNConfiguration->IsSmsOverImsSupported());
    EXPECT_EQ(2, pAosNConfiguration->GetSmsOverImsSupportedRats().GetSize());
    EXPECT_TRUE(pAosNConfiguration->IsEmergencyCallbackModeSupported());
    // KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY
    EXPECT_EQ(1000, pAosNConfiguration->GetEmergencyRegistrationTimerMillis());
    // KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT

    EXPECT_FALSE(pAosNConfiguration->IsRegistrationEventForCatRequired());
    EXPECT_FALSE(pAosNConfiguration->IsUnSubscription());
    EXPECT_EQ(1, pAosNConfiguration->GetIsimIndexForImpu());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_DSCP_NONE, pAosNConfiguration->GetPreferredImsDscp());
    EXPECT_EQ(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED,
            pAosNConfiguration->GetRegistrationPreferredAccessTypeFeatureTag());

    EXPECT_TRUE(pAosNConfiguration->IsGeolocationPidfSupported(2));
    EXPECT_FALSE(pAosNConfiguration->IsGeolocationPidfSupported(1));
    EXPECT_EQ(2, pAosNConfiguration->GetImsIdentityPriority().GetSize());
    EXPECT_EQ(0, pAosNConfiguration->GetRegPermanentErrCode().GetSize());
    EXPECT_EQ(0, pAosNConfiguration->GetUpdateRegistrationWithRatChange().GetSize());

    EXPECT_FALSE(pAosNConfiguration->IsEmergencyPdnWithEmergencyCallEndReleased());
    EXPECT_EQ(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK,
            pAosNConfiguration->GetPreferredEmergencyRegistration());
    EXPECT_FALSE(pAosNConfiguration->IsSupportLimitedAdminSmsMode());
    EXPECT_EQ(0, pAosNConfiguration->GetRegistrationPrivateHeader());
}

TEST_F(AosNConfigurationTest, InitAssetConfig)
{
    MockICarrierConfig objCarrierConfig;

    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_CALL_END_AND_PDN_REACTIVATION_BY_REG_TERMINATED_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::
                               KEY_DESTROY_UNSECURE_TCP_SOCKET_ON_ACCOMPLISHING_REG_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::
                               KEY_EMC_CALL_BASED_ON_P_ASSOCIATED_URI_OF_NORMAL_REG_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_HOLD_REG_WITH_IPCAN_CHANGED_DURING_IMS_CALL_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_IMS_DEREG_ON_3G_NETWORK_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_INITIALIZE_IPSEC_SETTING_WITH_NEW_PCSCF_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_NO_INIT_REG_ON_PCSCF_CHANGE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::
                               KEY_PLMN_BLOCK_WITH_TIMEOUT_ON_VOICE_CALL_UNAVAILABLE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_REG_CONTACT_VALIDATION_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_REG_RETRY_IP_VER_FALLBACK_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_REMOVE_OLD_SA_ON_ESTABLISHING_SA_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_REQUIRED_CDMALESS_FEATURE_TAG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::
                               KEY_REQUIRED_INIT_REG_AFTER_IMS_CALL_END_ON_REG_HELD_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_REQUIRED_VOLTE_BLOCK_BY_SSAC_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_REQUIRED_WFC_BLOCK_BY_AIRPLANE_MODE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_REREG_WITH_CHANGED_COUNTRY_ON_WIFI_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_SIP_OVER_IPSEC_ENABLED_IN_ROAMING_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_SMS_OVER_IMS_AVAILABLE_WITHOUT_VOICE_CAPA_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_SUPPORT_CONTACT_USER_INFO_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_SUPPORT_REG_WITH_FEATURE_TAG_UNAVAILABLE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_SUPPORT_VERSTAT_FOR_REG_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::
                               KEY_USE_RCS_TELEPHONY_FEATURE_TAG_AS_AVAILABLE_VOICE_CALL_TYPE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_USE_SECURITY_SERVER_PORT_IN_INIT_REG_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::
                               KEY_USE_SECURITY_SERVER_PORT_IN_REG_CONTACT_OF_INIT_REG_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_USE_WFC_COUNTRY_CODE_AVAILABILITY_CHECK_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_CONTACT_USER_INFO_POLICY_FOR_NON_REG_MESSAGE_INT, -1))
            .WillOnce(Return(1));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_EMC_PREFERRED_IPTYPE_INT, -1))
            .WillOnce(Return(1));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_GEOLOCATION_PIDF_FORMING_POLICY_INT, -1))
            .WillOnce(Return(1));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_IMS_ESTABLISHMENT_TIME_SEC_INT, -1))
            .WillOnce(Return(120));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_IMS_PREFERRED_IPTYPE_INT, -1))
            .WillOnce(Return(1));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_IMS_SIGNALLING_DSCP_INT, -1))
            .WillOnce(Return(46));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REG_OUT_OF_SERVICE_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REG_PCSCF_UPDATE_POLICY_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REG_RETRY_305_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REG_RETRY_503_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Assets::SIP_503_CODE_POLICY_DEFAULT));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_REG_RETRY_CNT_ON_SINGLE_PCSCF_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REG_RETRY_CNT_PER_PCSCF_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REG_RETRY_CNT_RESET_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_REG_RETRY_CNT_WITH_IPSEC_ON_AUTH_FAILURE_INT, -1))
            .WillOnce(Return(3));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REG_RETRY_DEFAULT_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_SPEC));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REG_RETRY_TIMER_F_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_NONE));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_REG_TIMER_FOR_EMC_CALL_MILLIS_INT, -1))
            .WillOnce(Return(0));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_REREG_RETRY_305_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_ROAMING_PREFERRED_EMC_REG_INT, -1))
            .WillOnce(Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_SIP_MESSAGE_THRESHOLD_FOR_TRANSPORT_CHANGE_INT, -1))
            .WillOnce(Return(200));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Assets::KEY_VOLTE_HYS_TIME_SEC_INT, -1))
            .WillOnce(Return(60));

    ImsVector<IMS_SINT32> objEmergencyPcscfRetryWaitTimeSec;
    objEmergencyPcscfRetryWaitTimeSec.Clear();
    objEmergencyPcscfRetryWaitTimeSec.Add(2);
    objEmergencyPcscfRetryWaitTimeSec.Add(2);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_EMERGENCY_PCSCF_RETRY_WAIT_TIME_SEC_INT_ARRAY))
            .WillOnce(Return(objEmergencyPcscfRetryWaitTimeSec));

    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Clear();
    objRegErrCodeForPcscfDiscovery.Add(408);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY))
            .WillOnce(Return(objRegErrCodeForPcscfDiscovery));

    ImsVector<IMS_SINT32> objRegPermanentErrMaxCnt;
    objRegPermanentErrMaxCnt.Clear();
    objRegPermanentErrMaxCnt.Add(2);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_REG_PERMANENT_ERR_MAX_CNT_INT_ARRAY))
            .WillOnce(Return(objRegPermanentErrMaxCnt));

    ImsVector<IMS_SINT32> objRegRetryErrCodeWithoutIpsec;
    objRegRetryErrCodeWithoutIpsec.Clear();
    objRegRetryErrCodeWithoutIpsec.Add(406);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_REG_RETRY_ERR_CODE_WITHOUT_IPSEC_INT_ARRAY))
            .WillOnce(Return(objRegRetryErrCodeWithoutIpsec));

    ImsVector<IMS_SINT32> objReregErrForCallEnd;
    objReregErrForCallEnd.Clear();
    objReregErrForCallEnd.Add(403);
    objReregErrForCallEnd.Add(406);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_REREG_ERR_CODE_FOR_CALL_END_INT_ARRAY))
            .WillOnce(Return(objReregErrForCallEnd));

    ImsVector<IMS_SINT32> objReregErrCodeForInitRegWithAvailablePcscf;
    objReregErrCodeForInitRegWithAvailablePcscf.Clear();
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY))
            .WillOnce(Return(objReregErrCodeForInitRegWithAvailablePcscf));

    ImsVector<IMS_SINT32> objReregErrCodeForImsPdnReactivation;
    objReregErrCodeForImsPdnReactivation.Clear();
    objReregErrCodeForImsPdnReactivation.Add(408);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::Assets::KEY_REREG_ERR_CODE_FOR_IMS_PDN_REACTIVATION_INT_ARRAY))
            .WillOnce(Return(objReregErrCodeForImsPdnReactivation));

    ImsVector<IMS_SINT32> objReregRetryErrCodeForInitRegWithSamePcscf;
    objReregRetryErrCodeForInitRegWithSamePcscf.Clear();
    objReregRetryErrCodeForInitRegWithSamePcscf.Add(407);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_REREG_RETRY_ERR_CODE_FOR_INIT_REG_WITH_SAME_PCSCF_INT_ARRAY))
            .WillOnce(Return(objReregRetryErrCodeForInitRegWithSamePcscf));

    ImsVector<IMS_SINT32> objSubErrorCodeForInitRegWithNextPcscf;
    objSubErrorCodeForInitRegWithNextPcscf.Clear();
    objSubErrorCodeForInitRegWithNextPcscf.Add(404);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::Assets::KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_NEXT_PCSCF_INT_ARRAY))
            .WillOnce(Return(objSubErrorCodeForInitRegWithNextPcscf));

    ImsVector<IMS_SINT32> objSubErrorCodeForStoppingByExpirationTime;
    objSubErrorCodeForStoppingByExpirationTime.Clear();
    objSubErrorCodeForStoppingByExpirationTime.Add(606);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_SUB_ERR_CODE_FOR_STOPPING_BY_EXPIRATION_TIME_INT_ARRAY))
            .WillOnce(Return(objSubErrorCodeForStoppingByExpirationTime));

    ImsVector<IMS_SINT32> objSupportedRoamingRats;
    objSupportedRoamingRats.Clear();
    objSupportedRoamingRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);
    objSupportedRoamingRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objSupportedRoamingRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY))
            .WillOnce(Return(objSupportedRoamingRats));

    ImsVector<IMS_SINT32> objVowifiSubErrorCodeForInitReg;
    objVowifiSubErrorCodeForInitReg.Clear();
    objVowifiSubErrorCodeForInitReg.Add(0);
    objVowifiSubErrorCodeForInitReg.Add(403);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_VOWIFI_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY))
            .WillOnce(Return(objVowifiSubErrorCodeForInitReg));

    InitAssetsConfig(static_cast<ICarrierConfig*>(&objCarrierConfig));

    EXPECT_FALSE(pAosNConfiguration->IsCdmalessFeatureTagRequired());
    EXPECT_FALSE(pAosNConfiguration->IsCallEndAndPdnReactivationByRegTerminated());
    EXPECT_FALSE(pAosNConfiguration->IsUnsecureTcpSocketOnAccomplishingRegDestroyed());
    EXPECT_FALSE(pAosNConfiguration->IsEmergencyCallBasedOnPauOfNormalRegistrationSupported());
    EXPECT_FALSE(pAosNConfiguration->IsRegWithIpcanChangedDuringImsCallHeld());
    EXPECT_TRUE(pAosNConfiguration->IsVopsIgnoredForVolteEnabled());
    EXPECT_FALSE(pAosNConfiguration->IsDeregOn3gNetwork());
    EXPECT_FALSE(pAosNConfiguration->IsIpsecInitializedWithNewPcscf());
    EXPECT_FALSE(pAosNConfiguration->IsNoInitRegOnPcscfChange());
    EXPECT_FALSE(pAosNConfiguration->IsPlmnBlockWithTimeoutOnVoiceCallUnavailable());
    EXPECT_FALSE(pAosNConfiguration->IsContactUriValidationChecked());
    EXPECT_FALSE(pAosNConfiguration->IsRegRetryWithIpVerFallback());
    EXPECT_FALSE(pAosNConfiguration->IsOldSaOnEstablishingSaRemoved());
    EXPECT_TRUE(pAosNConfiguration->IsRegRequiredAfterImsCallEndOnRegHeld());
    EXPECT_FALSE(pAosNConfiguration->IsRequiredVolteBlockBySsac());
    EXPECT_FALSE(pAosNConfiguration->IsRequiredWfcBlockByAirplaneMode());
    EXPECT_FALSE(pAosNConfiguration->IsReregRetryWithChangedCountryOnWifi());
    EXPECT_TRUE(pAosNConfiguration->IsSipOverIpsecInRoamingEnabled());
    EXPECT_TRUE(pAosNConfiguration->IsSmsOverImsAvailableWithoutVoiceCapability());
    EXPECT_TRUE(pAosNConfiguration->IsUserInfoInContactSupported());
    EXPECT_FALSE(pAosNConfiguration->IsRegWithFeatureTagUnavailableSupported());
    EXPECT_FALSE(pAosNConfiguration->IsVerstatForRegistrationSupported());
    EXPECT_FALSE(pAosNConfiguration->IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType());
    EXPECT_FALSE(pAosNConfiguration->IsSecurityServerPortInInitRegUsed());
    EXPECT_FALSE(pAosNConfiguration->IsSecurityServerPortInRegContactOfInitRegUsed());
    EXPECT_FALSE(pAosNConfiguration->UseWfcCountryCodeAvailabilityCheck());
    EXPECT_FALSE(pAosNConfiguration->IsVideoOverWifiSupportedWithoutVoice());

    EXPECT_EQ(1, pAosNConfiguration->GetUserInfoPolicyForNonRegisterMessage());
    EXPECT_EQ(1, pAosNConfiguration->GetEmergencyPreferredIpType());
    EXPECT_EQ(1, pAosNConfiguration->GetGeolocationPidfFormingPolicy());
    EXPECT_EQ(120, pAosNConfiguration->GetImsEstablishmentTime());
    EXPECT_EQ(1, pAosNConfiguration->GetPreferredIpType());
    EXPECT_EQ(46, pAosNConfiguration->GetImsSignallingDscp());
    EXPECT_EQ(0, pAosNConfiguration->GetRegActualWaitTimePolicy());
    EXPECT_EQ(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT,
            pAosNConfiguration->GetRegOutOfServicePolicy());
    EXPECT_EQ(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT,
            pAosNConfiguration->GetRegRetrySip305CodePolicy());
    EXPECT_EQ(CarrierConfig::Assets::SIP_503_CODE_POLICY_DEFAULT,
            pAosNConfiguration->GetRegRetrySip503CodePolicy());
    EXPECT_EQ(0, pAosNConfiguration->GetRegRetryCountOnSinglePcscf());
    EXPECT_EQ(0, pAosNConfiguration->GetRegRetryCountPerPcscf());
    EXPECT_EQ(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION,
            pAosNConfiguration->GetRegRetryCountResetPolicy());
    EXPECT_EQ(3, pAosNConfiguration->GetRegRetryCountWithIpsecOnAuthFailure());
    EXPECT_EQ(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_SPEC,
            pAosNConfiguration->GetRegRetryDefaultPolicy());
    EXPECT_EQ(CarrierConfig::Assets::TIMER_F_POLICY_NONE,
            pAosNConfiguration->GetRegRetryTimerFPolicy());
    EXPECT_EQ(0, pAosNConfiguration->GetRegistrationPcscfUpdatePolicy());

    EXPECT_EQ(0, pAosNConfiguration->GetRegTimerForEmcCall());
    EXPECT_EQ(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT,
            pAosNConfiguration->GetReregRetrySip305CodePolicy());
    EXPECT_EQ(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL,
            pAosNConfiguration->GetRoamingPreferredEmcReg());
    EXPECT_EQ(200, pAosNConfiguration->GetSipMessageThresholdForTransportChange());
    EXPECT_EQ(60, pAosNConfiguration->GetVolteHysTime());

    ImsVector<IMS_SINT32> objWaitTime = pAosNConfiguration->GetEmergencyPcscfRetryWaitTime();
    EXPECT_EQ(2, objWaitTime.GetSize());
    ImsVector<IMS_SINT32> objErrCode = pAosNConfiguration->GetRegErrCodeForPcscfDiscovery();
    EXPECT_EQ(408, objErrCode.GetAt(0));
    ImsVector<IMS_SINT32> objCount = pAosNConfiguration->GetRegPermanentErrMaxCount();
    EXPECT_EQ(1, objCount.GetSize());
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetRegErrCodeWithoutIpsec();
    EXPECT_EQ(406, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetReregErrCodeForCallEnd();
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
    objErrCode = pAosNConfiguration->GetReregErrCodeForImsPdnReactivation();
    EXPECT_EQ(1, objErrCode.GetSize());
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetReregErrCodeForInitRegWithAvailablePcscf();
    EXPECT_EQ(0, objErrCode.GetSize());
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetReregRetryErrCodeForInitRegWithSamePcscf();
    EXPECT_EQ(407, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetSubErrorRegRequiredWithNextPcscf();
    EXPECT_EQ(404, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetSubErrorStoppingResub();
    EXPECT_EQ(606, objErrCode.GetAt(0));
    ImsVector<IMS_SINT32> objRats = pAosNConfiguration->GetSupportedRoamingRats();
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
    objErrCode = pAosNConfiguration->GetVowifiSubErrorRegRequired();
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
}

TEST_F(AosNConfigurationTest, InitBundleConfig)
{
    MockICarrierConfig objCarrierConfig;
    MockICarrierConfig objExtraRegErr;

    EXPECT_CALL(objCarrierConfig, GetBundle(CarrierConfig::Assets::KEY_EXTRA_REG_ERR_BUNDLE))
            .WillRepeatedly(Return(static_cast<ICarrierConfig*>(&objExtraRegErr)));

    EXPECT_CALL(objExtraRegErr,
            GetBoolean(CarrierConfig::Assets::
                               KEY_EXTRA_REG_ERR_CODE_AS_FAILURE_IN_ROAMING_FOR_UPDATE_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objExtraRegErr,
            GetBoolean(
                    CarrierConfig::Assets::KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objExtraRegErr, GetInt(CarrierConfig::Assets::KEY_EXTRA_REG_ERR_FINAL_TYPE_INT, -1))
            .WillOnce(Return(0));

    EXPECT_CALL(objExtraRegErr, GetInt(CarrierConfig::Assets::KEY_EXTRA_REG_ERR_MAX_CNT_INT, -1))
            .WillOnce(Return(0));

    EXPECT_CALL(objExtraRegErr, GetInt(CarrierConfig::Assets::KEY_EXTRA_REG_ERR_MIN_CNT_INT, -1))
            .WillOnce(Return(0));

    EXPECT_CALL(objExtraRegErr,
            GetInt(CarrierConfig::Assets::
                            KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_EPS_5GS_ONLY_ATTACHED_INT,
                    -1))
            .WillOnce(Return(1));

    EXPECT_CALL(objExtraRegErr,
            GetInt(CarrierConfig::Assets::
                            KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_LTE_COMBINDED_ATTACHED_INT,
                    -1))
            .WillOnce(Return(2));

    EXPECT_CALL(objExtraRegErr, GetInt(CarrierConfig::Assets::KEY_EXTRA_REG_ERR_POLICY_INT, -1))
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PCSCF_FAILED));

    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Clear();
    objExtraRegErrCode.Add(400);
    EXPECT_CALL(
            objExtraRegErr, GetIntArray(CarrierConfig::Assets::KEY_EXTRA_REG_ERR_CODE_INT_ARRAY))
            .WillOnce(Return(objExtraRegErrCode));

    ImsVector<IMS_SINT32> objExtraReregErrCode;
    objExtraReregErrCode.Clear();
    objExtraReregErrCode.Add(500);
    EXPECT_CALL(objExtraRegErr,
            GetIntArray(CarrierConfig::Assets::KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY))
            .WillOnce(Return(objExtraReregErrCode));

    ImsVector<IMS_SINT32> objExtraRegErrWaitTimeSec;
    objExtraRegErrWaitTimeSec.Clear();
    objExtraRegErrWaitTimeSec.Add(30);
    EXPECT_CALL(objExtraRegErr,
            GetIntArray(CarrierConfig::Assets::KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY))
            .WillOnce(Return(objExtraRegErrWaitTimeSec));

    EXPECT_CALL(objExtraRegErr, ReleaseBundle()).Times(1);

    MockICarrierConfig objNotifyTerminated;

    EXPECT_CALL(objCarrierConfig,
            GetBundle(CarrierConfig::Assets::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE))
            .WillRepeatedly(Return(static_cast<ICarrierConfig*>(&objNotifyTerminated)));

    EXPECT_CALL(objNotifyTerminated,
            GetInt(CarrierConfig::Assets::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_WITH_WAIT_TIME_INT,
                    -1))
            .WillOnce(Return(40));

    ImsVector<IMS_SINT32> objEventForInitRegOnTerminatedState;
    objEventForInitRegOnTerminatedState.Clear();
    objEventForInitRegOnTerminatedState.Add(1);
    objEventForInitRegOnTerminatedState.Add(2);
    objEventForInitRegOnTerminatedState.Add(3);
    EXPECT_CALL(objNotifyTerminated,
            GetIntArray(
                    CarrierConfig::Assets::KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY))
            .WillOnce(Return(objEventForInitRegOnTerminatedState));

    ImsVector<IMS_SINT32> objEventWithWtForInitRegOnTerminatedState;
    objEventWithWtForInitRegOnTerminatedState.Clear();
    objEventWithWtForInitRegOnTerminatedState.Add(2);
    objEventWithWtForInitRegOnTerminatedState.Add(5);
    EXPECT_CALL(objNotifyTerminated,
            GetIntArray(CarrierConfig::Assets::
                            KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY))
            .WillOnce(Return(objEventWithWtForInitRegOnTerminatedState));

    EXPECT_CALL(objNotifyTerminated, ReleaseBundle()).Times(1);

    MockICarrierConfig objRegErrCodeWithRaTimeBundle;

    EXPECT_CALL(objCarrierConfig,
            GetBundle(CarrierConfig::Assets::KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE))
            .WillRepeatedly(Return(static_cast<ICarrierConfig*>(&objRegErrCodeWithRaTimeBundle)));

    EXPECT_CALL(objRegErrCodeWithRaTimeBundle,
            GetBoolean(CarrierConfig::Assets::KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    ImsVector<IMS_SINT32> objRegErrCodeWithRaTime;
    objRegErrCodeWithRaTime.Clear();
    objRegErrCodeWithRaTime.Add(486);
    EXPECT_CALL(objRegErrCodeWithRaTimeBundle,
            GetIntArray(CarrierConfig::Assets::KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY))
            .WillOnce(Return(objRegErrCodeWithRaTime));

    ImsVector<IMS_SINT32> objReregErrCodeWithRaTime;
    objReregErrCodeWithRaTime.Clear();
    objReregErrCodeWithRaTime.Add(486);
    EXPECT_CALL(objRegErrCodeWithRaTimeBundle,
            GetIntArray(CarrierConfig::Assets::KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY))
            .WillOnce(Return(objReregErrCodeWithRaTime));

    EXPECT_CALL(objRegErrCodeWithRaTimeBundle, ReleaseBundle()).Times(1);

    MockICarrierConfig objRegRetryInterval;

    EXPECT_CALL(objCarrierConfig, GetBundle(CarrierConfig::Assets::KEY_REG_RETRY_INTERVAL_BUNDLE))
            .WillRepeatedly(Return(static_cast<ICarrierConfig*>(&objRegRetryInterval)));

    EXPECT_CALL(objRegRetryInterval,
            GetBoolean(CarrierConfig::Assets::KEY_REG_RETRY_INTERVAL_USED_FOR_SUB_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objRegRetryRandomUpperValueSec;
    objRegRetryRandomUpperValueSec.Clear();
    objRegRetryRandomUpperValueSec.Add(0);
    objRegRetryRandomUpperValueSec.Add(0);
    objRegRetryRandomUpperValueSec.Add(15);
    objRegRetryRandomUpperValueSec.Add(0);
    objRegRetryRandomUpperValueSec.Add(0);
    objRegRetryRandomUpperValueSec.Add(0);
    EXPECT_CALL(objRegRetryInterval,
            GetIntArray(
                    CarrierConfig::Assets::KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY))
            .WillOnce(Return(objRegRetryRandomUpperValueSec));

    ImsVector<IMS_SINT32> objRegRetryIntervalSec;
    objRegRetryIntervalSec.Clear();
    objRegRetryIntervalSec.Add(30);
    objRegRetryIntervalSec.Add(30);
    objRegRetryIntervalSec.Add(60);
    objRegRetryIntervalSec.Add(120);
    objRegRetryIntervalSec.Add(480);
    objRegRetryIntervalSec.Add(900);
    EXPECT_CALL(objRegRetryInterval,
            GetIntArray(CarrierConfig::Assets::KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY))
            .WillOnce(Return(objRegRetryIntervalSec));

    EXPECT_CALL(objRegRetryInterval, ReleaseBundle()).Times(1);

    MockICarrierConfig objSubErrCodeForInitRegBundle;

    EXPECT_CALL(objCarrierConfig,
            GetBundle(CarrierConfig::Assets::KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE))
            .WillRepeatedly(Return(static_cast<ICarrierConfig*>(&objSubErrCodeForInitRegBundle)));

    EXPECT_CALL(objSubErrCodeForInitRegBundle,
            GetInt(CarrierConfig::Assets::KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_RETRY_MAX_CNT_INT, -1))
            .WillOnce(Return(2));

    ImsVector<IMS_SINT32> objSubErrCodeForInitReg;
    objSubErrCodeForInitReg.Clear();
    objSubErrCodeForInitReg.Add(408);
    objSubErrCodeForInitReg.Add(504);
    EXPECT_CALL(objSubErrCodeForInitRegBundle,
            GetIntArray(CarrierConfig::Assets::KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY))
            .WillOnce(Return(objSubErrCodeForInitReg));

    EXPECT_CALL(objSubErrCodeForInitRegBundle, ReleaseBundle()).Times(1);

    MockICarrierConfig objSubErrCodeForTerminatedBundle;

    EXPECT_CALL(objCarrierConfig,
            GetBundle(CarrierConfig::Assets::KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE))
            .WillRepeatedly(
                    Return(static_cast<ICarrierConfig*>(&objSubErrCodeForTerminatedBundle)));

    EXPECT_CALL(objSubErrCodeForTerminatedBundle,
            GetInt(CarrierConfig::Assets::KEY_SUB_ERR_CODE_FOR_TERMINATED_WITH_RETRY_MAX_COUNT_INT,
                    -1))
            .WillOnce(Return(0));

    ImsVector<IMS_SINT32> objSubErrCodeForTerminated;
    objSubErrCodeForTerminated.Clear();
    objSubErrCodeForTerminated.Add(491);
    objSubErrCodeForTerminated.Add(500);
    objSubErrCodeForTerminated.Add(606);
    EXPECT_CALL(objSubErrCodeForTerminatedBundle,
            GetIntArray(CarrierConfig::Assets::KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY))
            .WillOnce(Return(objSubErrCodeForTerminated));

    EXPECT_CALL(objSubErrCodeForTerminatedBundle, ReleaseBundle()).Times(1);

    // AosWfcErrMessageBundle
    MockICarrierConfig objWfcErrMessageBundle;

    AString strWfcErrorReg403("REG90 - Unable to Connect");
    AString strWfcErrorReg500("REG91 - Unable to Connect");
    AString strWfcErrorNotSupportedCountry("REG92 - Wi-Fi Calling isn't supported in this country");
    AString strWfcErrorSub403("REG09 - Missing 911 Address");
    AString strWfcErrorNotifyTerminated("REG09 - Missing 911 Address");
    AString strWfcErrorOtherFailures("REG99 - Unable to Connect");

    EXPECT_CALL(objCarrierConfig, GetBundle(CarrierConfig::Assets::KEY_WFC_ERR_MESSAGE_BUNDLE))
            .WillRepeatedly(Return(static_cast<ICarrierConfig*>(&objWfcErrMessageBundle)));

    EXPECT_CALL(
            objWfcErrMessageBundle, GetString(CarrierConfig::Assets::KEY_WFC_ERR_REG_403_STRING, _))
            .Times(1)
            .WillOnce(Return(strWfcErrorReg403));
    EXPECT_CALL(
            objWfcErrMessageBundle, GetString(CarrierConfig::Assets::KEY_WFC_ERR_REG_500_STRING, _))
            .Times(1)
            .WillOnce(Return(strWfcErrorReg500));
    EXPECT_CALL(objWfcErrMessageBundle,
            GetString(CarrierConfig::Assets::KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING, _))
            .Times(1)
            .WillOnce(Return(strWfcErrorNotSupportedCountry));
    EXPECT_CALL(
            objWfcErrMessageBundle, GetString(CarrierConfig::Assets::KEY_WFC_ERR_SUB_403_STRING, _))
            .Times(1)
            .WillOnce(Return(strWfcErrorSub403));
    EXPECT_CALL(objWfcErrMessageBundle,
            GetString(CarrierConfig::Assets::KEY_WFC_ERR_NOTIFY_TERMINATED_STRING, _))
            .Times(1)
            .WillOnce(Return(strWfcErrorNotifyTerminated));
    EXPECT_CALL(objWfcErrMessageBundle,
            GetString(CarrierConfig::Assets::KEY_WFC_ERR_OTHER_FAILURES_STRING, _))
            .Times(1)
            .WillOnce(Return(strWfcErrorOtherFailures));

    InitBundleConfig(static_cast<ICarrierConfig*>(&objCarrierConfig));

    // AosExtraRegErrBundle
    EXPECT_FALSE(pAosNConfiguration->IsExtraReregErrInRoamingAsFailureHandled());
    EXPECT_TRUE(pAosNConfiguration->IsExtraRegErrRetryCntSharedForRegAndSubRequired());
    EXPECT_EQ(0, pAosNConfiguration->GetExtraRegErrFinalType());
    EXPECT_EQ(0, pAosNConfiguration->GetExtraRegErrMaxCount());
    EXPECT_EQ(0, pAosNConfiguration->GetExtraRegErrMinCount());
    EXPECT_EQ(1, pAosNConfiguration->GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached());
    EXPECT_EQ(2, pAosNConfiguration->GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached());
    EXPECT_EQ(CarrierConfig::Assets::ERROR_POLICY_PCSCF_FAILED,
            pAosNConfiguration->GetExtraRegErrPolicy());
    ImsVector<IMS_SINT32>& objErrCode = pAosNConfiguration->GetExtraRegErrCode();
    EXPECT_EQ(1, objErrCode.GetSize());
    EXPECT_EQ(400, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetExtraReregErrCode();
    EXPECT_EQ(1, objErrCode.GetSize());
    EXPECT_EQ(500, objErrCode.GetAt(0));
    ImsVector<IMS_SINT32>& objWaitTime = pAosNConfiguration->GetExtraRegErrWaitTime();
    EXPECT_EQ(30, objWaitTime.GetAt(0));

    // AosNotifyTerminatedForInitRegBundle
    EXPECT_EQ(40, pAosNConfiguration->GetNotifyWaitTime());

    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED &
            (pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED &
            (pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_PROBATION &
            (pAosNConfiguration->GetNotifyEventForInitialRegistration()));
    EXPECT_FALSE(IAosNConfiguration::NOTIFY_TERMINATED_UNREGISTERED &
            (pAosNConfiguration->GetNotifyEventForInitialRegistration()));

    EXPECT_FALSE(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED &
            (pAosNConfiguration->GetNotifyEventForInitialRegWithWaitTime()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED &
            (pAosNConfiguration->GetNotifyEventForInitialRegWithWaitTime()));
    EXPECT_TRUE(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED &
            (pAosNConfiguration->GetNotifyEventForInitialRegWithWaitTime()));

    // AosRegErrCodeWithRaTimeBundle
    EXPECT_FALSE(pAosNConfiguration->IsRegErrCodeWithRetryAfterTimeOnlyDefined());
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetRegErrCodeWithRetryAfterTime();
    EXPECT_EQ(1, objErrCode.GetSize());
    EXPECT_EQ(486, objErrCode.GetAt(0));
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetReregErrCodeWithRetryAfterTime();
    EXPECT_EQ(1, objErrCode.GetSize());

    // AosRegRetryIntervalBundle
    EXPECT_TRUE(pAosNConfiguration->IsRegRetryIntervalsUsedForSub());
    ImsVector<IMS_SINT32>& objRandomInterval = pAosNConfiguration->GetRegRandomRetryIntervals();
    ImsVector<IMS_SINT32>& objInterval = pAosNConfiguration->GetRegRetryIntervals();
    EXPECT_EQ(objRandomInterval.GetSize(), objInterval.GetSize());

    // AosSubErrCodeForInitRegBundle
    EXPECT_EQ(2, pAosNConfiguration->GetRetryCountSubErrorRegRequired());
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetSubErrorRegRequired();
    EXPECT_EQ(2, objErrCode.GetSize());

    // AosSubErrCodeForTerminatedBundle
    EXPECT_EQ(0, pAosNConfiguration->GetRetryCountSubErrorSubTerminated());
    objErrCode.Clear();
    objErrCode = pAosNConfiguration->GetSubErrorSubTerminated();
    EXPECT_EQ(3, objErrCode.GetSize());

    // AosWfcErrMessageBundle
    EXPECT_TRUE(pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::Assets::WFC_ERROR_REG_403));
    EXPECT_TRUE(pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::Assets::WFC_ERROR_REG_500));
    EXPECT_TRUE(pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::Assets::WFC_ERROR_NOT_SUPPORTED_COUNTRY));
    EXPECT_TRUE(pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));
    EXPECT_TRUE(pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::Assets::WFC_ERROR_NOTIFY_TERMINATED));
    EXPECT_TRUE(pAosNConfiguration->IsWfcErrorMessageSupported(
            CarrierConfig::Assets::WFC_ERROR_OTHER_FAILURES));
}