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

#ifndef EPS_FALLBACK_TRIGGER_H_
#define EPS_FALLBACK_TRIGGER_H_

#include "ITimer.h"
#include "ImsTypeDef.h"

class IMtcCallContext;
class MtcConfigurationProxy;
struct CallReasonInfo;

enum class EpsFallbackReason
{
    NONE,

    /**
     * EPS fallback was not triggered by the network.
     * Example: A watchdog timer (e.g., Verizon's) expired.
     * No data buffer flush is performed by the modem.
     */
    NO_NETWORK_TRIGGER,

    /**
     * No response was received from the network.
     * Data buffer flush is performed by the modem.
     */
    NO_NETWORK_RESPONSE,

    /**
     * No response was received from the network, and it requests IMS registration to AoS.
     * Data buffer flush is performed by the modem.
     */
    NO_NETWORK_RESPONSE_REQUIRING_REG,

    /**
     * The call was blocked due to a radio check failure.
     * Example: OnConnectionFailed() by AC Barring or RRC Reject.
     * Data buffer flush is performed by the modem.
     */
    RADIO_CHECK_BLOCK,

    /**
     * The call was explicitly rejected by the network with a specific status code.
     * See {@CarrierConfig::ImsVoice::START_ERROR_ACTION_TRIGGER_EPSFB}.
     * No data buffer flush is performed by the modem.
     */
    FAILURE_RESPONSE,
};

class EpsFallbackTrigger : public ITimerListener
{
public:
    explicit EpsFallbackTrigger(IN IMtcCallContext& objContext);
    virtual ~EpsFallbackTrigger() override;
    EpsFallbackTrigger(IN const EpsFallbackTrigger&) = delete;
    EpsFallbackTrigger& operator=(IN const EpsFallbackTrigger&) = delete;

    static IMS_BOOL ShouldTriggerByReasonInfo(
            IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason);
    static IMS_BOOL ShouldTriggerByWatchdogTimer(IN IMtcCallContext& objContext);
    static IMS_BOOL ShouldTriggerByMoRequestTimeout(IN IMtcCallContext& objContext);
    static IMS_BOOL IsEpsFbAvailable(IN IMtcCallContext& objContext);

    virtual void StartWatchdog();
    virtual void OnEpsFallbackCompleted();
    void Timer_TimerExpired(IN ITimer* piTimer) override;
    virtual void TriggerEpsFallback(IN EpsFallbackReason eReason);

    inline virtual EpsFallbackReason GetTriggerReason() const { return m_eTriggerReason; }
    inline virtual IMS_BOOL IsWaitingEpsFallback() const
    {
        return m_eTriggerReason == EpsFallbackReason::NO_NETWORK_RESPONSE ||
                m_eTriggerReason == EpsFallbackReason::FAILURE_RESPONSE;
    }
    inline virtual IMS_BOOL IsWaitingRegistration() const
    {
        return m_eTriggerReason == EpsFallbackReason::NO_NETWORK_RESPONSE_REQUIRING_REG ||
                m_eTriggerReason == EpsFallbackReason::RADIO_CHECK_BLOCK;
    }

private:
    IMS_BOOL IsEpsFallbackTriggeredByNetwork() const;

    IMtcCallContext& m_objContext;
    ITimer* m_piTimerWatchdogWait;
    ITimer* m_piTimerEpsFallbackWait;
    EpsFallbackReason m_eTriggerReason;
    static const IMS_UINT32 EPS_FALLBACK_COMPLETE_TIMEOUT = 20000;
};

#endif
