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
#include "IMSMap.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosServiceSettingListener.h"

class IAosRegistrationControlListener;
class JniAosService;

enum class AosReasonCode;
enum class AosNetworkType;
enum class AosCapability;
enum class AosIsimState;
enum class AosRegEvent;
enum class AosPhoneNumberRetryCommand;
enum class AosRegRequestType;
enum class AosPcscfOrder;

class IAosService
{
public:
    virtual void SetJniAosService(IN JniAosService* pJniAosService) = 0;

    virtual void AddListener(IN IAosRegistrationControlListener* piListener) = 0;
    virtual void RemoveListener(IN IAosRegistrationControlListener* piListener) = 0;

    virtual void AddListener(IN IAosServiceSettingListener* piListener) = 0;
    virtual void RemoveListener(IN IAosServiceSettingListener* piListener) = 0;

    virtual void AddListener(IN IAosServicePhoneListener* piListener) = 0;
    virtual void RemoveListener(IN IAosServicePhoneListener* piListener) = 0;

    /**
     * AosService(Java) -> IAosRegistrationControlListener(Native)
     *
     */
    virtual void UpdateSipDelegateRegistration() = 0;
    virtual void TriggerSipDelegateDeregistration() = 0;
    virtual void TriggerFullNetworkRegistration(
            IN IMS_SINT32 nSipCode, IN const AString& sipReason) = 0;
    virtual void NotifyCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objCapabilities) = 0;
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
    virtual void NotifyAosStart();
    virtual void NotifyIpcanHandoverFailure(
            IN IMS_SINT32 nTargetNetwork, IN IMS_SINT32 nCauseCode) = 0;
    virtual void NotifyIsimState(IN IMS_UINT32 nState) = 0;
    virtual void NotifyLocationInfo(IN IMS_UINT32 nState) = 0;
    virtual void NotifyMobileDataLimit(IN IMS_UINT32 nIsLimited) = 0;
    virtual void NotifyNetworkVideoCapability(IN IMS_UINT32 nIsOn) = 0;
    virtual void NotifyPhoneNumberState(IN IMS_UINT32 nIsRefresh, IN IMS_UINT32 nState) = 0;
    virtual void NotifyPlmnChanged() = 0;
    virtual void NotifyPowerOff() = 0;
    virtual void NotifyPreciseCallState(IN IMS_SINT32 nState) = 0;

    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param eNetworkType The radio access technology.
     * @param nFeatureTagBits Type of bits an integer.
     * @param objFeatureTags Type of IMSList<AString>&.
     * @see class ImsAosFeature
     * @see class AosNetworkType
     */
    virtual void NotifyRegistered(IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
            IN const IMSList<AString>& objFeatureTags) = 0;

    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param eNetworkType The radio access technology.
     * @param nFeatureTagBits Type of bits an integer.
     * @param objFeatureTags Type of IMSList<AString>&.
     * @see class ImsAosFeature
     * @see class NetworkType
     */
    virtual void NotifyRegistering(IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
            IN const IMSList<AString>& objFeatureTags) = 0;

    /**
     * Notify the application that the device is disconnected from the IMS network.
     *
     * @param eReason associated with why registration was disconnected.
     * @see class AosReasonCode
     */
    virtual void NotifyDeregistered(IN AosReasonCode eReason) = 0;

    /**
     * Notify the framework that the handover from the current radio technology to the other
     * technology has failed.
     *
     * @param eNetworkType The technology that has failed to be changed to.
     * @param nCauseCode Handover failure cause.
     * @see class AosNetworkType
     * @see class android.telephony.DataFailCause
     */
    virtual void NotifyTechnologyChangeFailed(
            IN AosNetworkType eNetworkType, IN IMS_SINT32 nCauseCode) = 0;

    /**
     * This device's subscriber associated {@link Uri}s have changed, which are used to filter out
     * this device's {@link Uri}s during conference calling.
     *
     * @param objUris is type of const IMSList<AString>&, the network provisioned
     * public user identities.
     */
    virtual void NotifyAssociatedUriChanged(IN const IMSList<AString>& objUris) = 0;

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
    virtual void NotifyCapabilitiesUpdateFailed(IN AosCapability eCapabilities,
            IN AosNetworkType eNetworkType, IN AosReasonCode eReason) = 0;

    /**
     * Notify the application that AoS ISIM state.
     * AosService(Native) -> IAosInfoListener(Java)
     *
     * @param nState is type of AosIsimState.
     * @see enum class AosIsimState
     */
    virtual void NotifyAosIsimState(IN AosIsimState eState) = 0;

    /**
     * Notify the application that Reg event state.
     * AosService(Native) -> AosService(Java)
     *
     * @param nState is type of AosRegEvent.
     * @see enum class AosRegEvent
     */
    virtual void NotifyRegEventState(IN AosRegEvent eState) = 0;

    /**
     * Request the application to phone number retry.
     * AosService(Native) -> AosService(Java)
     *
     * @param nCommand is type of AosPhoneNumberRetryCommand.
     * @see enum AosPhoneNumberRetryCommand
     */
    virtual void RequestPhoneNumberRetry(IN AosPhoneNumberRetryCommand eCommand) = 0;

    /**
     * Request the application to Wifi on or off.
     * AosService(Native) -> AosService(Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual void RequestWifiService(IN IMS_BOOL bIsOn) = 0;

    /**
     * Returns capabilities.
     *
     * @return Returns capabilities of type IMSMap<IMS_UINT32, IMS_UINT32>&.
     */
    virtual IMSMap<IMS_UINT32, IMS_UINT32>& GetCapabilities();

    /**
     * Gets the capabilities for the network.
     *
     * @param eNetworkType The radio access technology.
     * @see class AosNetworkType
     */
    virtual IMS_UINT32 GetCapabilitiesForNetwork(AosNetworkType eNetworkType);

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
            AosNetworkType eNetworkType, AosCapability eCapability);
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
     * IMS Registration error code
     */
    REGISTRATION_ERROR = 1000,
    /**
     * Indicates the registration attempt on IWLAN failed due to IKEv2 authetication failure
     * during tunnel establishment.
     */
    IKEV2_AUTH_FAILURE = 1408,
    /**
     * Call/IMS registration failed/dropped because of a RLF
     */
    RADIO_LINK_FAILURE = 1506,
    /**
     * Call/IMS registration failed/dropped because of radio link lost
     */
    RADIO_LINK_LOST = 1507,
    /**
     * The call Call/IMS registration failed because of a radio uplink issue
     */
    RADIO_UPLINK_FAILURE = 1508
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
 * Reg event state
 */
enum class AosRegEvent
{
    ACTIVE = 1,
    INVALID = 2
};

/**
 * Phone number retry request command
 */
enum class AosPhoneNumberRetryCommand
{
    INITIAL = 0,
    REFRESH = 1,
    CLEAR = 2,
};

#endif  // INTERFACE_AOS_SERVICE_H_
