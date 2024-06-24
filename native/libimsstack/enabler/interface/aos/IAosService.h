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
    virtual ~IAosService(){};

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
    virtual void NotifyEmcCallbackModeChanged(
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
     * @param nCauseCode Handover failure cause.
     * @see IAosRegistration::IMS_REG_TYPE_XXX
     * @see class AosNetworkType
     * @see class android.telephony.DataFailCause
     */
    virtual IMS_BOOL NotifyTechnologyChangeFailed(
            IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN IMS_SINT32 nCauseCode) = 0;

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
 * reasons associated with why registration was disconnected
 */
enum class AosReasonCode
{
    /**
     * The Reason is unspecified.
     */
    UNSPECIFIED = 0,
    /**
     * Indicates that the IMS registration is failed with fatal error such as 403 or 404
     * on all P-CSCF addresses. The radio shall block the current PLMN or disable
     * the RAT
     */
    PLMN_BLOCK = 1,
    /**
     * Indicates that the IMS registration on current PLMN failed multiple times.
     * The radio shall block the current PLMN or disable the RAT during the time
     * based on carrier requirement
     */
    PLMN_BLOCK_WITH_TIMEOUT = 2,
    /**
     * IMS Registration error code
     */
    REGISTRATION_ERROR = 3,
    /**
     * WFC Registration error code if the network returns 403 Forbidden for Register.
     * The 403 Forbidden case due to non-support for other countries are not included.
     */
    REGISTRATION_ERROR_WFC_REG_403 = 4,
    /**
     * WFC Registration error code if the network returns 500 Internal server error for Register.
     */
    REGISTRATION_ERROR_WFC_REG_500 = 5,
    /**
     * WFC Registration error code if the network returns 403 Forbidden with a different country for
     * register.
     */
    REGISTRATION_ERROR_WFC_NOT_SUPPORTED_COUNTRY = 6,
    /**
     * WFC Registration error code if the network returns 403 response for Subscribe.
     */
    REGISTRATION_ERROR_WFC_SUB_403 = 7,
    /**
     * WFC Registration error code if the network returns Notify Terminate message.
     */
    REGISTRATION_ERROR_WFC_NOTIFY_TERMINATED = 8,
    /**
     * WFC Registration error code for all other failures.
     */
    REGISTRATION_ERROR_WFC_OTHER_FAILURES = 9,
    /**
     * Registration error code for USIM authentication failures.
     */
    REGISTRATION_ERROR_USIM_AUTHENTICATION_FAILURES = 10,
    /**
     * Service unavailable; radio power off
     */
    LOCAL_POWER_OFF = 11,
    /**
     * Service unavailable; low battery
     */
    LOCAL_LOW_BATTERY = 12,
    /**
     * Service unavailable; out of service (data service state)
     */
    LOCAL_NETWORK_NO_SERVICE = 13,
    /**
     * Service unavailable; no LTE coverage
     * (VoLTE is not supported even though IMS is registered)
     */
    LOCAL_NETWORK_NO_LTE_COVERAGE = 14,
    /**
     * Service unavailable; located in roaming area
     */
    LOCAL_NETWORK_ROAMING = 15,
    /**
     * Service unavailable; IP changed
     */
    NETWORK_IP_CHANGED = 16,
    /**
     * Service unavailable; for an unspecified reason
     */
    LOCAL_SERVICE_UNAVAILABLE = 17,
    /**
     * Service unavailable; IMS is not registered
     */
    LOCAL_NOT_REGISTERED = 18,
    /**
     * The current RAT was blocked because registration failed for all P-CSCFs.
     */
    RAT_BLOCK = 19,
    /**
     * Clears blocks for all RATs.
     */
    CLEAR_RAT_BLOCKS = 20
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
    OPTIONS_UCE = 1 << 5,
    PRESENCE_UCE = 1 << 6
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
