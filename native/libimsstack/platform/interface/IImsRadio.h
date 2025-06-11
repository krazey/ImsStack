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
#ifndef INTERFACE_IMS_RADIO_H_
#define INTERFACE_IMS_RADIO_H_

#include "ImsTypeDef.h"

/**
 * @brief Data type for SSAC(Service Specific Access Control)
 *
 * nBarringFactorForVoice/Video
 *     The conditional barring factor as a percentage 0-100, which is the probability
 *     of a random device being barred for the service type.
 *     In case that a factor is 100, the session is exempted from barring.
 * nBarringTimeSecForVoice/Video
 *     The conditional barring time seconds, which is the interval between successive evaluations
 *     for conditional barring based on the barring factor.
 */
struct SsacInfo
{
    IMS_SINT32 nBarringFactorForVoice;
    IMS_SINT32 nBarringTimeSecForVoice;
    IMS_SINT32 nBarringFactorForVideo;
    IMS_SINT32 nBarringTimeSecForVideo;
};

class IImsRadioConnectionListener
{
protected:
    virtual ~IImsRadioConnectionListener() = default;

public:
    /**
     * @brief Notifies the reason of the connection setup corresponding with the IMS traffic type
     *
     * @param nFailureReason The reason of connection failure based on IMS traffic type
     *                       (@ConnectionFailureReason)
     * @param nCauseCode Failure cause code from network or modem specific to the failure
     * @param nWaitTimeMillis Retry wait time provided by network in milliseconds
     */
    virtual void ImsRadio_OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) = 0;

    /**
     * @brief Notifies the preparation of the connection setup corresponding
     *        with the IMS traffic type
     */
    virtual void ImsRadio_OnConnectionSetupPrepared() = 0;

    /// Cause Code
    enum
    {
        CAUSE_CODE_SR_LLF_TIMER_START = 1000
    };
};

class IImsRadioSsacListener
{
protected:
    virtual ~IImsRadioSsacListener() = default;

public:
    /**
     * @brief Notifies that SSAC is changed.
     *
     * @param objSsacInfo SSAC information with barring factor and time based on the service type.
     *
     */
    virtual void ImsRadio_OnSsacChanged(IN const SsacInfo& objSsacInfo) = 0;
};

class IImsRadioTrafficPriorityListener
{
protected:
    virtual ~IImsRadioTrafficPriorityListener() = default;

public:
    /**
     * @brief Notifies that IMS traffic priority is changed for DSDS. If IMS traffic is pending
     *        due to REASON_RF_BUSY or IMS traffic priority. It can be checked again
     *        using IImsRadio#IsImsTrafficAllowed after this is notified.
     */
    virtual void ImsRadio_OnTrafficPriorityChanged() = 0;
};

class IImsRadio
{
protected:
    virtual ~IImsRadio() = default;

public:
    /**
     * @brief Indicates whether Ims traffic is available or not after checking the traffic priority
     *        within the IMS stack for DSDS in advance. There is no interworking with modem.
     *
     * @param nTrafficType The type for IMS traffic (@ImsTrafficType)
     *
     * @return Returns the IMS traffic availability
     */
    virtual IMS_BOOL IsImsTrafficAllowed(IN IMS_UINT32 nTrafficType) = 0;

    /**
     * @brief Indicates NAS and RRC layers of the modem that the upcoming IMS traffic is
     *        for the service mentioned in the ImsTrafficType.
     *
     * @param nTrafficType The type for IMS traffic (@ImsTrafficType)
     * @param nAccessNetworkType The type for radio access network type (@RadioAccessNetworkType)
     * @param nDirection The direction for IMS traffic (@Direction)
     * @param piListener The listener to be added
     */
    virtual void StartImsTraffic(IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nAccessNetworkType,
            IN IMS_UINT32 nDirection, IN IImsRadioConnectionListener* piListener) = 0;

    /**
     * @brief Indicates IMS traffic has been stopped. For all IMS traffic,
     *        notified with StartImsTraffic, IMS service shall notify StopImsTraffic
     *        when it completes the traffic. The reference listener registered
     *        from StartImsTraffic() is removed.
     *
     * @param piListener The listener to be removed
     */
    virtual void StopImsTraffic(IN IImsRadioConnectionListener* piListener) = 0;

    /**
     * @brief Triggers the EPS fallback procedure by UE for the case where the user is trying to
     *        place a voice call in NR network and the voice call is not established
     *        within several seconds.
     *
     * @param nEpsfbReason Specifies the reason that causes EPS fallback (@EpsFallbackReason)
     */
    virtual void TriggerEpsFallback(IN IMS_UINT32 nEpsfbReason) = 0;

    /**
     * @brief Gets SSAC information with barring factor and time based on the service type.
     *
     * @return Returns SSAC information.
     */
    virtual const SsacInfo& GetSsacInfo() const = 0;

    /**
     * @brief Adds the listener to be notified when SSAC is changed
     *
     * @param piListener The listener to be added
     */
    virtual void AddListenerForSsac(IN IImsRadioSsacListener* piListener) = 0;

    /**
     * @brief Removes the listener to be notified when when SSAC is changed
     *
     * @param piListener The listener to be removed
     */
    virtual void RemoveListenerForSsac(IN IImsRadioSsacListener* piListener) = 0;

    /**
     * @brief Adds the listener to be notified when the traffic priority is changed
     *
     * @param piListener The listener to be added
     */
    virtual void AddListenerForTrafficPriority(IN IImsRadioTrafficPriorityListener* piListener) = 0;

    /**
     * @brief Removes the listener to be notified when the traffic priority is changed
     *
     * @param piListener The listener to be removed
     */
    virtual void RemoveListenerForTrafficPriority(
            IN IImsRadioTrafficPriorityListener* piListener) = 0;

    /// ImsTrafficType
    enum
    {
        TRAFFIC_TYPE_EMERGENCY = 1,
        TRAFFIC_TYPE_EMERGENCY_SMS,
        TRAFFIC_TYPE_VOICE,
        TRAFFIC_TYPE_VIDEO,
        TRAFFIC_TYPE_SMS,
        TRAFFIC_TYPE_REGISTRATION,
        TRAFFIC_TYPE_UT_XCAP
    };

    /// RadioAccessNetworkType
    enum
    {
        ACCESS_NETWORK_TYPE_UNKNOWN = 0,
        ACCESS_NETWORK_TYPE_UTRAN,
        ACCESS_NETWORK_TYPE_EUTRAN,
        ACCESS_NETWORK_TYPE_NGRAN,
        ACCESS_NETWORK_TYPE_IWLAN
    };

    /// Direction
    enum
    {
        DIRECTION_MO = 0,
        DIRECTION_MT = 1
    };

    /// ConnectionFailureReason
    enum
    {
        /** Access class check failed */
        REASON_ACCESS_DENIED = 1,
        /** 3GPP Non-access stratum failure */
        REASON_NAS_FAILURE,
        /** Random access failure */
        REASON_RACH_FAILURE,
        /** Radio link failure */
        REASON_RLC_FAILURE,
        /** Radio connection establishment rejected by network */
        REASON_RRC_REJECT,
        /** Radio connection establishment timed out */
        REASON_RRC_TIMEOUT,
        /** Device currently not in service */
        REASON_NO_SERVICE,
        /** The PDN is no more active */
        REASON_PDN_NOT_AVAILABLE,
        /** Radio resource is busy with another subscription */
        REASON_RF_BUSY,
        /** Internal Error */
        REASON_INTERNAL_ERROR
    };

    /// EpsFallbackReason
    enum
    {
        /**
         * If the network only supports the EPS fallback in 5G NR SA for voice calling and the EPS
         * Fallback procedure by the network during the call setup is not triggered, UE initiated
         * fallback will be triggered with this reason. The modem shall locally release the 5G NR
         * SA RRC connection and acquire the LTE network and perform a tracking area update
         * procedure. After the EPS fallback procedure is completed, the call setup for voice will
         * be established if there is no problem.
         */
        EPSFB_REASON_NO_NETWORK_TRIGGER = 1,

        /**
         * If the UE doesn't receive any response for SIP INVITE within a certain timeout in 5G NR
         * SA for MO voice calling, the device determines that voice call is not available in 5G and
         * terminates all active SIP dialogs and SIP requests and enters IMS non-registered state.
         * In that case, UE initiated fallback will be triggered with this reason. The modem shall
         * reset modem's data buffer of IMS PDU to prevent the ghost call. After the EPS fallback
         * procedure is completed, VoLTE call could be tried if there is no problem.
         */
        EPSFB_REASON_NO_NETWORK_RESPONSE
    };
};

#endif
