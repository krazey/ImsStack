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
#ifndef INTERFACE_AOS_SERVICE_PHONE_LISTENER_H_
#define INTERFACE_AOS_SERVICE_PHONE_LISTENER_H_

enum class IsimState;
enum class LocationInfo;
enum class PhoneNumberState;
enum class PreciseCallState;

class IAosServicePhoneListener
{
public:
    virtual ~IAosServicePhoneListener(){};

    /**
     * Called to notify the start of AoS Service.
     * Called by AosService (Java)
     *
     */
    virtual void ServicePhone_AosStart() = 0;

    /**
     * Called to notify the failure of IPCAN Handover.
     * Called by AosService (Java)
     *
     * @param nTargetNetwork is the technology that has failed to be changed to.
     * @param nCauseCode is the handover failure cause
     * @see {@link IIpcan#CATEGORY_MOBILE} and {@link IIpcan#CATEGORY_WLAN}
     * @see class {@link android.telephony.DataFailCause}
     */
    virtual void ServicePhone_notifyIpcanHandoverFailure(
            IN IMS_SINT32 nTargetNetwork, IN IMS_SINT32 nCauseCode) = 0;

    /**
     * Called to notify the change of ISIM state.
     * Called by AosService (Java).
     *
     * @param nState is type of IsimState.
     * @see enum class {@link #IsimState}
     */
    virtual void ServicePhone_IsimStateChanged(IN IsimState eState) = 0;

    /**
     * Called to notify the change of location information.
     * Called by AosService (Java)
     *
     * @param nState is type of LocationInfo.
     * @see enum class {@link #LocationInfo}
     */
    virtual void ServicePhone_LocationInfoChanged(IN LocationInfo eState) = 0;

    /**
     * Called to notify the change of mobile data limit.
     * Called by AosService (Java).
     *
     * @param bIsLimited {@code IMS_TRUE} if limited, {@code IMS_FALSE} if not limited.
     */
    virtual void ServicePhone_MobileDataLimitChanged(IN IMS_BOOL bIsLimited) = 0;

    /**
     * Called to notify the change of network capability.
     * Called by AosService (Java)
     *
     * @param bIsOn {@code IMS_TRUE} if on, {@code IMS_FALSE} if off.
     */
    virtual void ServicePhone_NetworkVideoCapabilityChanged(IN IMS_BOOL bIsOn) = 0;

    /**
     * Called to notify the change of phone number state.
     * Called by AosService (Java)
     *
     * @param bIsRefresh {@code IMS_TRUE} if refresh action, {@code IMS_FALSE} if initial action.
     * @param nState is type of PhoneNumberState.
     * @see enum class {@link #PhoneNumberState}
     */
    virtual void ServicePhone_PhoneNumberStateChanged(
            IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState) = 0;

    /**
     * Called to notify the change of PLMN.
     * Called by AosService (Java).
     *
     */
    virtual void ServicePhone_PlmnChanged() = 0;

    /**
     * Called to notify the power off.
     * Called by AosService (Java).
     *
     */
    virtual void ServicePhone_PowerOff() = 0;

    /**
     * Called to notify the change of precise call state.
     * Called by AosService (Java).
     *
     * @param nState is type of PreciseCallState.
     * @see enum class {@link #PreciseCallState}
     */
    virtual void ServicePhone_PreciseCallStateChanged(IN PreciseCallState eState) = 0;

    /**
     * Called to notify the change of carrier signal PCO value.
     * Called by AosService (Java).
     *
     * @param nValue is value of carrier signal PCO.
     */
    virtual void ServicePhone_PcoValueChanged(IN IMS_SINT32 nValue) = 0;
};

/**
 * ISIM State
 */
enum class IsimState
{
    UNKNOWN = -1,
    NOT_PRESENT = 0,
    NOT_READY = 1,
    LOADED = 2,
    REFRESH_STARTED = 3,
    REFRESH_COMPLETED = 4,
    REMOVED = 5
};

/**
 * Location Information
 */
enum class LocationInfo
{
    FIXED = 1,
    COUNTRY_CHANGED = 2,
    CHANGED = 3,
    AVAILABLE = 4
};

/**
 * PhoneNumber state
 */
enum class PhoneNumberState
{
    SIM_LOADED = 0,
    RETRY_SUCCESS = 1,
    RETRY_FAILURE = 2
};

/**
 * Precise call state
 */
enum class PreciseCallState
{
    NOT_VALID = -1,
    IDLE = 0,
    ACTIVE = 1,
    HOLDING = 2,
    DIALING = 3,
    ALERTING = 4,
    INCOMING = 5,
    WAITING = 6,
    DISCONNECTED = 7,
    DISCONNECTING = 8
};

class AosServicePhoneListener : public IAosServicePhoneListener
{
public:
    inline void ServicePhone_AosStart() override{};
    inline void ServicePhone_notifyIpcanHandoverFailure(
            IN IMS_SINT32 /*nTargetNetwork*/, IN IMS_SINT32 /*nCauseCode*/) override
    {
    }
    inline void ServicePhone_IsimStateChanged(IN IsimState /*eState*/) override{};
    inline void ServicePhone_LocationInfoChanged(IN LocationInfo /*eState*/) override{};
    inline void ServicePhone_MobileDataLimitChanged(IN IMS_BOOL /*bIsLimited*/) override{};
    inline void ServicePhone_NetworkVideoCapabilityChanged(IN IMS_BOOL /*bIsOn*/) override{};
    inline void ServicePhone_PhoneNumberStateChanged(
            IN IMS_BOOL /*bIsRefresh*/, IN PhoneNumberState /*eState*/) override{};
    inline void ServicePhone_PlmnChanged() override{};
    inline void ServicePhone_PowerOff() override{};
    inline void ServicePhone_PreciseCallStateChanged(IN PreciseCallState /*eState*/) override{};
    inline void ServicePhone_PcoValueChanged(IN IMS_SINT32 /*nValue*/) override{};
};

#endif  // INTERFACE_AOS_SERVICE_PHONE_LISTENER_H_