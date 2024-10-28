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
#ifndef INTERFACE_AOS_SERVICE_H_
#define INTERFACE_AOS_SERVICE_H_

#include "AString.h"
#include "ImsMap.h"
#include "INativeEnabler.h"

#define TO_UINT32(e) (static_cast<IMS_UINT32>(e))

class IAosRegistrationControlListener;
class IAosServicePhoneListener;
class IAosServiceSettingListener;
class IAosEmergencyListener;

enum class AosReasonCode;
enum class AosNetworkType;
enum class AosCapability;
enum class AosIsimState;
enum class AosPhoneNumberRetryCommand;
enum class AosRegRequestType;
enum class AosPcscfOrder;

class IAosService : public INativeEnabler
{
public:
    virtual void AddListener(IN IAosRegistrationControlListener* piListener) = 0;
    virtual void RemoveListener(IN IAosRegistrationControlListener* piListener) = 0;

    virtual void AddListener(IN IAosServiceSettingListener* piListener) = 0;
    virtual void RemoveListener(IN IAosServiceSettingListener* piListener) = 0;

    virtual void AddListener(IN IAosServicePhoneListener* piListener) = 0;
    virtual void RemoveListener(IN IAosServicePhoneListener* piListener) = 0;

    virtual void AddListener(IN IAosEmergencyListener* piListener) = 0;
    virtual void RemoveListener(IN IAosEmergencyListener* piListener) = 0;

    /**
     * AosService(Java) -> IAosRegistrationControlListener(Native)
     *
     */
    virtual void UpdateSipDelegateRegistration() = 0;
    virtual void TriggerSipDelegateDeregistration() = 0;
    virtual void TriggerFullNetworkRegistration(
            IN IMS_SINT32 nSipCode, IN const AString& sipReason) = 0;
    virtual void NotifyCapabilitiesChanged(
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities) = 0;
    virtual void ControlRegistration(
            IN IMS_SINT32 nRequestType, IN IMS_SINT32 nPcscfOrder, IN IMS_SINT32 nCause) = 0;

    /**
     * AosService(Java) -> IAosServiceSettingListener(Native)
     *
     */
    virtual void NotifyAirplaneSetting(IN IMS_UINT32 nIsOn) = 0;
    virtual void NotifyDataRoamingSetting(IN IMS_UINT32 nIsAllowed) = 0;
    virtual void NotifyMobileDataSetting(IN IMS_UINT32 nIsOn) = 0;
    virtual void NotifyRoamingPreferredVoiceNetwork(IN IMS_UINT32 nState) = 0;
    virtual void NotifyServiceSetting(IN IMS_UINT32 nState, IN IMS_UINT32 nServiceBits) = 0;
    virtual void NotifyTtySetting(IN IMS_UINT32 nIsOn) = 0;
    virtual void NotifyVideoSetting(IN IMS_UINT32 nIsOn) = 0;
    virtual void NotifyVolteSetting(IN IMS_UINT32 nIsOn) = 0;
    virtual void NotifyWfcSetting(IN IMS_UINT32 nIsOn) = 0;

    /**
     * AosService(Java) -> IAosServicePhoneListener(Native)
     *
     */
    virtual void NotifyAosStart() = 0;
    virtual void NotifyIpcanHandoverFailure(
            IN IMS_SINT32 nTargetNetwork, IN IMS_SINT32 nCauseCode) = 0;
    virtual void NotifyIsimState(IN IMS_SINT32 nState) = 0;
    virtual void NotifyLocationInfo(IN IMS_UINT32 nState) = 0;
    virtual void NotifyMobileDataLimit(IN IMS_UINT32 nIsLimited) = 0;
    virtual void NotifyNetworkVideoCapability(IN IMS_UINT32 nIsOn) = 0;
    virtual void NotifyPhoneNumberState(IN IMS_UINT32 nIsRefresh, IN IMS_UINT32 nState) = 0;
    virtual void NotifyPlmnChanged() = 0;
    virtual void NotifyPowerOff() = 0;
    virtual void NotifyPreciseCallState(IN IMS_SINT32 nState) = 0;
    virtual void NotifyCarrierSignalPcoValueChanged(IN IMS_SINT32 nValue) = 0;

    /**
     * Notify an emergency callback mode changed information by AosService (Java)
     * AosService(Java) -> IAosEmergencyListener (Native)
     *
     * @param nType The emergency callback mode type.
     * @param nState The emergency callback mode state.
     * @param nDuration The timer duration(seconds) of emergency callback mode for call or sms.
     */
    virtual void NotifyEmergencyCallbackModeChanged(
            IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_ULONG nDuration) = 0;

    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param nRegType Type of the registration.
     * @param eNetworkType The radio access technology.
     * @param nFeatureTagBits Type of bits an integer.
     * @param objFeatureTags Type of ImsList<AString>&.
     * @see IAosRegistration::IMS_REG_TYPE_XXX
     * @see class AosNetworkType
     * @see class ImsAosFeature
     */
    virtual IMS_BOOL NotifyRegistered(IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType,
            IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags) = 0;

    /**
     * Notify the application that the device is trying to connect to the IMS network.
     *
     * @param nRegType Type of the registration.
     * @param eNetworkType The radio access technology.
     * @param nFeatureTagBits Type of bits an integer.
     * @param objFeatureTags Type of ImsList<AString>&.
     * @see IAosRegistration::IMS_REG_TYPE_XXX
     * @see class NetworkType
     * @see class ImsAosFeature
     */
    virtual IMS_BOOL NotifyRegistering(IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType,
            IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags) = 0;

    /**
     * Notify the application that the device is disconnected from the IMS network.
     *
     * @param nRegType Type of the registration.
     * @param eNetworkType The radio access technology.
     * @param eReason associated with why registration was disconnected.
     * @see IAosRegistration::IMS_REG_TYPE_XXX
     * @see class NetworkType
     * @see class AosReasonCode
     */
    virtual IMS_BOOL NotifyDeregistered(
            IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN AosReasonCode eReason) = 0;

    /**
     * Notify the framework that the handover from the current radio technology to the other
     * technology has failed.
     *
     * @param nRegType Type of the registration.
     * @param eNetworkType The technology that has failed to be changed to.
     * @param eReason Handover failure reason.
     * @see IAosRegistration::IMS_REG_TYPE_XXX
     * @see class AosNetworkType
     * @see class AosReasonCode
     */
    virtual IMS_BOOL NotifyTechnologyChangeFailed(
            IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN AosReasonCode eReason) = 0;

    /**
     * This device's subscriber associated {@link Uri}s have changed, which are used to filter out
     * this device's {@link Uri}s during conference calling.
     *
     * @param objUris is type of const ImsList<AString>&, the network provisioned
     * public user identities.
     */
    virtual IMS_BOOL NotifyAssociatedUriChanged(IN const ImsList<AString>& objUris) = 0;

    /**
     * This method is called when capability update fails after
     * {@link #NotifyCapabilitiesChanged} is called.
     *
     * @param eCapabilities capabilities that failed to update.
     * @param eNetworkType Type of {@link AosNetworkType}.
     * @param eReason Reason for update failure.
     * @see class AosCapability
     * @see class AosNetworkType
     * @see class AosReasonCode
     */
    virtual IMS_BOOL NotifyCapabilitiesUpdateFailed(IN AosCapability eCapabilities,
            IN AosNetworkType eNetworkType, IN AosReasonCode eReason) = 0;

    /**
     * Notify the application that AoS ISIM state.
     * AosService(Native) -> IAosInfoListener(Java)
     *
     * @param nState is type of AosIsimState.
     * @see enum class AosIsimState
     */
    virtual IMS_BOOL NotifyAosIsimState(IN AosIsimState eState) = 0;

    /**
     * Notify the application that Reg event state.
     * AosService(Native) -> AosService(Java)
     *
     * @param nStatusCode is IMS registration status code.
     * @param objImpus is list of IMPUs.
     */
    virtual IMS_BOOL NotifyRegEventState(IN IMS_UINT32 nStatusCode,
            IN const ImsList<AString>& objImpus = ImsList<AString>()) = 0;

    /**
     * Request the application to phone number retry.
     * AosService(Native) -> AosService(Java)
     *
     * @param nCommand is type of AosPhoneNumberRetryCommand.
     * @see enum AosPhoneNumberRetryCommand
     */
    virtual IMS_BOOL RequestPhoneNumberRetry(IN AosPhoneNumberRetryCommand eCommand) = 0;

    /**
     * Request the application to Wifi on or off.
     * AosService(Native) -> AosService(Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual IMS_BOOL RequestWifiService(IN IMS_BOOL bIsOn) = 0;

    /**
     * Returns capabilities.
     *
     * @return Returns capabilities of type ImsMap<IMS_UINT32, IMS_UINT32>&.
     */
    virtual ImsMap<IMS_UINT32, IMS_UINT32>& GetCapabilities() = 0;

    /**
     * Gets the capabilities for the network.
     *
     * @param eNetworkType The radio access technology.
     * @see class AosNetworkType
     */
    virtual IMS_UINT32 GetCapabilitiesForNetwork(AosNetworkType eNetworkType) = 0;

    /**
     * Request the capabilities for the network.
     *
     * @param eNetworkType The radio access technology.
     * @param eCapability The capabilities.
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see class AosNetworkType
     * @see class AosCapability
     */
    virtual IMS_BOOL IsSupportCapabilitiesForNetwork(
            AosNetworkType eNetworkType, AosCapability eCapability) = 0;
};

/**
 * @brief Enum class defining base values for categorizing AosReasonCode.
 *        Each base value represents a distinct category of errors or conditions that can lead to
 *        registration failures.
 */
enum class AosReasonCodeBase
{
    /// General Errors.
    BASE = 0,

    /// Errors requiring special action from the modem.
    BASE_MODEM = 2000,

    /// Errors due to registration common failures.
    BASE_COMMON_OTHER = 4000,

    /// Errors due to registration response 3XX.
    BASE_RESP_3XX = 13000,

    /// Errors due to registration response 4XX.
    BASE_RESP_4XX = 14000,

    /// Errors due to registration response 5XX.
    BASE_RESP_5XX = 15000,

    /// Errors due to registration response 6XX.
    BASE_RESP_6XX = 16000,

    /// Errors due to registration other response.
    BASE_RESP_OTHER = 17000,

    /// Errors due to WFC registration response 3XX.
    BASE_RESP_WFC_3XX = 23000,

    /// Errors due to WFC registration response 4XX.
    BASE_RESP_WFC_4XX = 24000,

    /// Errors due to WFC registration response 5XX.
    BASE_RESP_WFC_5XX = 25000,

    /// Errors due to WFC registration response 6XX.
    BASE_RESP_WFC_6XX = 26000,

    /// Errors due to WFC registration other response.
    BASE_RESP_WFC_OTHER = 27000
};

/**
 * @brief Enum class representing the reasons for registration disconnection.
 *        Each reason belongs to a specific category, indicated by its base value.
 */
enum class AosReasonCode
{

    /**
     * @brief : BASE(0) - General Errors.
     */

    /// The reason for disconnection is unknown or unspecified.
    UNSPECIFIED = TO_UINT32(AosReasonCodeBase::BASE),

    /// A general error occurred during IMS registration.
    REGISTRATION_ERROR = TO_UINT32(AosReasonCodeBase::BASE) + 1,

    /// Service is unavailable because the radio is powered off.
    POWER_OFF = TO_UINT32(AosReasonCodeBase::BASE) + 2,

    /// Service is unavailable due to low battery.
    LOW_BATTERY = TO_UINT32(AosReasonCodeBase::BASE) + 3,

    /// Service is unavailable due to out of service(data service state).
    NETWORK_NO_SERVICE = TO_UINT32(AosReasonCodeBase::BASE) + 4,

    /// Service is unavailable due to no LTE coverage.
    NETWORK_NO_LTE_COVERAGE = TO_UINT32(AosReasonCodeBase::BASE) + 5,

    /// Service is unavailable due to roaming.
    NETWORK_ROAMING = TO_UINT32(AosReasonCodeBase::BASE) + 6,

    /// Service is unavailable because the IP address changed.
    NETWORK_IP_CHANGED = TO_UINT32(AosReasonCodeBase::BASE) + 7,

    /// Service is unavailable for an unspecified reason.
    SERVICE_UNAVAILABLE = TO_UINT32(AosReasonCodeBase::BASE) + 8,

    /// Service is unavailable because IMS is not registered.
    NOT_REGISTERED = TO_UINT32(AosReasonCodeBase::BASE) + 9,

    /// Registration failed due to USIM authentication failure.
    USIM_AUTHENTICATION_FAILURES = TO_UINT32(AosReasonCodeBase::BASE) + 10,

    /**
     * @brief : BASE_MODEM(2000) - Errors requiring special action from the modem.
     */

    /// Indicates that the IMS registration is failed with fatal error such as 403 or 404 on all
    /// P-CSCF addresses. The radio shall block the current PLMN or disable the RAT.
    PLMN_BLOCK = TO_UINT32(AosReasonCodeBase::BASE_MODEM),

    /// Indicates that the IMS registration on current PLMN failed multiple times. The radio shall
    /// block the current PLMN or disable the RAT during the time based on carrier requirement.
    PLMN_BLOCK_WITH_TIMEOUT = TO_UINT32(AosReasonCodeBase::BASE_MODEM) + 1,

    /// The current RAT was blocked because registration failed for all P-CSCFs.
    RAT_BLOCK = TO_UINT32(AosReasonCodeBase::BASE_MODEM) + 2,

    /// Clears blocks for all RATs.
    CLEAR_RAT_BLOCKS = TO_UINT32(AosReasonCodeBase::BASE_MODEM) + 3,

    /**
     * @brief : BASE_RESP_4XX(14000) - Errors due to registration response 4XX.
     */

    /// Registration failed due to a 403 Forbidden response from the network.
    REG_RESP_403 = TO_UINT32(AosReasonCodeBase::BASE_RESP_4XX) + 403,

    /**
     * @brief : BASE_RESP_OTHER(17000) - Errors due to registration other response.
     */

    /// No response received for the registration request, TCP connection failure or timeout.
    REG_RESP_NETWORK_TIMEOUT = TO_UINT32(AosReasonCodeBase::BASE_RESP_OTHER),

    /**
     * @brief : BASE_RESP_WFC_4XX(24000) - Errors due to WFC registration response 4XX.
     */

    /// WFC registration failed due to a 403 Forbidden response (excluding unsupported country).
    WFC_REG_RESP_403 = TO_UINT32(AosReasonCodeBase::BASE_RESP_WFC_4XX) + 403,

    /**
     * @brief : BASE_RESP_WFC_5XX(25000) - Errors due to WFC registration response 5XX.
     */

    /// WFC registration failed due to a 500 Internal Server Error response.
    WFC_REG_RESP_500 = TO_UINT32(AosReasonCodeBase::BASE_RESP_WFC_5XX) + 500,

    /**
     * @brief : BASE_RESP_WFC_OTHER(27000) - Errors due to WFC registration other response.
     */

    /// WFC registration failed due to a 403 Forbidden response with unsupported country.
    WFC_REG_RESP_403_NOT_SUPPORTED_COUNTRY = TO_UINT32(AosReasonCodeBase::BASE_RESP_WFC_OTHER),

    /// WFC registration failed due to other unspecified reasons.
    WFC_REG_RESP_OTHER_FAILURES = TO_UINT32(AosReasonCodeBase::BASE_RESP_WFC_OTHER) + 1,

    /// WFC registration failed due to a 403 Forbidden response during subscription.
    WFC_SUB_RESP_403 = TO_UINT32(AosReasonCodeBase::BASE_RESP_WFC_OTHER) + 2,

    /// WFC registration terminated due to a Notify Terminate message.
    WFC_SUB_NOTIFY_TERMINATED = TO_UINT32(AosReasonCodeBase::BASE_RESP_WFC_OTHER) + 3
};

/**
 * Network Type
 */
enum class AosNetworkType
{
    NONE = -1,
    LTE = 0,
    IWLAN = 1,
    CROSS_SIM = 2,
    NR = 3,
    UTRAN = 4
};

/**
 * Capability
 */
enum class AosCapability
{
    NONE = 0,
    VOICE = 1 << 0,
    VIDEO = 1 << 1,
    UT = 1 << 2,
    SMS = 1 << 3,
    CALL_COMPOSER = 1 << 4,
    CALL_COMPOSER_BUSINESS_ONLY = 1 << 5,
    OPTIONS_UCE = 1 << 6,
    PRESENCE_UCE = 1 << 7,

    // Internal capabilities
    TEXT = 1 << 11
};

/**
 * ISIM State
 */
enum class AosIsimState
{
    INVALID = 0,
    VALID = 1,
    REFRESH_STARTED = 2,
    REFRESH_COMPLETE = 3
};

/**
 * Phone number retry request command
 */
enum class AosPhoneNumberRetryCommand
{
    INITIAL = 0,
    REFRESH = 1,
    CLEAR = 2
};

#endif  // INTERFACE_AOS_SERVICE_H_
