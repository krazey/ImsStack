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

#include "provider/AosNConfiguration.h"

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
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_CARRIER_VOLTE_AVAILABLE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(
            objCarrierConfig, GetBoolean(CarrierConfig::KEY_CARRIER_VT_AVAILABLE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig, GetBoolean(CarrierConfig::KEY_RTT_SUPPORTED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    // currently it doesn't have a function
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    IMSVector<IMS_SINT32> objCarrierNrAvailabilities;
    objCarrierNrAvailabilities.Clear();
    objCarrierNrAvailabilities.Add(CarrierConfig::Ims::CARRIER_NR_AVAILABILITY_NSA);
    objCarrierNrAvailabilities.Add(CarrierConfig::Ims::CARRIER_NR_AVAILABILITY_SA);
    EXPECT_CALL(
            objCarrierConfig, GetIntArray(CarrierConfig::KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY))
            .WillOnce(Return(objCarrierNrAvailabilities));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::KEY_CARRIER_USSD_METHOD_INT, -1))
            .WillOnce(Return(CarrierConfig::USSD_OVER_IMS_PREFERRED));
    IMSVector<IMS_SINT32> objPcscfDiscoveryMethod;
    objPcscfDiscoveryMethod.Clear();
    objPcscfDiscoveryMethod.Add(0);
    EXPECT_CALL(
            objCarrierConfig, GetIntArray(CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY))
            .WillOnce(Return(objPcscfDiscoveryMethod));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_IMS_SINGLE_REGISTRATION_REQUIRED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT, -1))
            .WillOnce(Return(5060));
    // currently it doesn't have a function
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_KEEP_PDN_UP_IN_NO_VOPS_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_PREFERRED_TRANSPORT_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_TCP));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_IPV4_SIP_MTU_SIZE_CELLULAR_INT, -1))
            .WillOnce(Return(1500));
    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT, -1))
            .WillOnce(Return(1500));

    // currently it doesn't have a function
    IMSVector<IMS_SINT32> objImsPdnEnabledInNoVopsSupport;
    objImsPdnEnabledInNoVopsSupport.Clear();
    objImsPdnEnabledInNoVopsSupport.Add(0);
    objImsPdnEnabledInNoVopsSupport.Add(1);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY))
            .WillOnce(Return(objImsPdnEnabledInNoVopsSupport));

    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    // InitIpsecAlgorithm
    IMSVector<IMS_SINT32> objAuthAlgo;
    objAuthAlgo.Clear();
    objAuthAlgo.Add(0);
    objAuthAlgo.Add(1);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_IPSEC_AUTHENTICATION_ALGORITHMS_INT_ARRAY))
            .WillOnce(Return(objAuthAlgo));

    IMSVector<IMS_SINT32> objEncryAlgo;
    objEncryAlgo.Clear();
    objEncryAlgo.Add(0);
    objEncryAlgo.Add(1);
    objEncryAlgo.Add(2);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_IPSEC_ENCRYPTION_ALGORITHMS_INT_ARRAY))
            .WillOnce(Return(objEncryAlgo));

    EXPECT_CALL(
            objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT, -1))
            .WillOnce(Return(600000));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REGISTRATION_RETRY_BASE_TIMER_MILLIS_INT, -1))
            .WillOnce(Return(30000));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REGISTRATION_RETRY_MAX_TIMER_MILLIS_INT, -1))
            .WillOnce(Return(1800000));
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT, -1))
            .WillOnce(Return(600000));

    IMSVector<IMS_SINT32> objSupportedRats;
    objSupportedRats.Clear();
    objSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
    EXPECT_CALL(objCarrierConfig, GetIntArray(CarrierConfig::Ims::KEY_SUPPORTED_RATS_INT_ARRAY))
            .WillOnce(Return(objSupportedRats))
            .WillOnce(Return(objSupportedRats));

    /// imsvoice.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsVoice::KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    /// imssms.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    IMSVector<IMS_SINT32> objSmsOverImsSupportedRats;
    objSmsOverImsSupportedRats.Clear();
    objSmsOverImsSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objSmsOverImsSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY))
            .WillOnce(Return(objSmsOverImsSupportedRats));

    /// imsemergency.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    IMSVector<IMS_SINT32> objEmergencyOverImsSupportedRats;
    objEmergencyOverImsSupportedRats.Clear();
    objEmergencyOverImsSupportedRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(
                    CarrierConfig::ImsEmergency::KEY_EMERGENCY_OVER_IMS_SUPPORTED_RATS_INT_ARRAY))
            .WillOnce(Return(objEmergencyOverImsSupportedRats));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT, -1))
            .WillOnce(Return(1000));
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT, -1))
            .WillOnce(Return(2000));

    /* carrier_config */
    /// ims.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_REGISTRATION_EVENT_FOR_CAT_REQUIRED_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objCarrierConfig,
            GetBoolean(
                    CarrierConfig::Ims::KEY_UNSUBSCRIBE_REGISTRATION_EVENT_PACKAGE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT, -1))
            .WillOnce(Return(1));

    EXPECT_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_PREFERRED_IMS_DSCP_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::Ims::KEY_REGISTRATION_PREFERRED_ACCESSTYPE_FEATURE_TAG_INT, -1))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED));

    IMSVector<IMS_SINT32> objGeolocationPidfInSipRegisterSupport;
    objGeolocationPidfInSipRegisterSupport.Clear();
    objGeolocationPidfInSipRegisterSupport.Add(
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI);
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY))
            .WillOnce(Return(objGeolocationPidfInSipRegisterSupport));

    IMSVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(0);
    objImsIdentityPriority.Add(1);
    EXPECT_CALL(
            objCarrierConfig, GetIntArray(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY))
            .WillOnce(Return(objImsIdentityPriority));

    IMSVector<IMS_SINT32> objRegistrationPermanentErrorCode;
    objRegistrationPermanentErrorCode.Clear();
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_REGISTRATION_PERMANENT_ERROR_CODE_INT_ARRAY))
            .WillOnce(Return(objRegistrationPermanentErrorCode));

    IMSVector<IMS_SINT32> objUpdateRegistrationWithRatChange;
    objUpdateRegistrationWithRatChange.Clear();
    EXPECT_CALL(objCarrierConfig,
            GetIntArray(CarrierConfig::Ims::KEY_UPDATE_REGISTRATION_WITH_RAT_CHANGE_INT_ARRAY))
            .WillOnce(Return(objUpdateRegistrationWithRatChange));

    /// imsemergency.
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsEmergency::
                               KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_END_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsEmergency::KEY_PREFERRED_EMERGENCY_REGISTRATION_INT, -1))
            .WillOnce(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));

    /// imssms
    EXPECT_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsSms::KEY_SUPPORT_LIMITED_ADMIN_SMS_MODE_BOOL, IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    /// imswfc.
    EXPECT_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsWfc::KEY_REGISTRATION_PRIVATE_HEADER_INT, -1))
            .WillOnce(Return(0));

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