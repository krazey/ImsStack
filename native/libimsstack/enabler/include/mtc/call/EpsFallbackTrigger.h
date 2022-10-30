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
#include "call/IMtcCall.h"

class IMtcCallContext;
class MtcConfigurationProxy;
enum class EpsFallbackReason
{
    NO_NETWORK_TRIGGER,
    NO_NETWORK_RESPONSE
};

class EpsFallbackTrigger : public ITimerListener
{
public:
    EpsFallbackTrigger(IN IMtcCallContext& objContext);
    virtual ~EpsFallbackTrigger();
    EpsFallbackTrigger(IN const EpsFallbackTrigger&) = delete;
    EpsFallbackTrigger& operator=(IN const EpsFallbackTrigger&) = delete;

    static IMS_BOOL IsRequired(IN MtcConfigurationProxy& objConfigProxy);
    virtual IMS_BOOL IsVoNr() const;
    virtual void StartWatchdog();
    virtual void OnEpsFallbackCompleted();
    void Timer_TimerExpired(IN ITimer* piTimer) override;
    virtual void TriggerEpsFallback(IN EpsFallbackReason eReason);
    inline virtual IMS_BOOL IsWaitingEpsFallbackForNoResponse() const
    {
        return m_bWaitingEpsFallbackForNoResponse;
    }

private:
    IMS_BOOL IsEpsFallbackTriggeredByNetwork() const;

    IMtcCallContext& m_objContext;
    ITimer* m_piTimerWatchdogWait;
    ITimer* m_piTimerEpsFallbackWait;
    IMS_BOOL m_bWaitingEpsFallbackForNoResponse;
};

#endif
