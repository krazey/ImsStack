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
#ifndef AOS_TRANSACTION_H_
#define AOS_TRANSACTION_H_

#include "AString.h"
#include "IImsRadio.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ITimer.h"

#include "interface/IAosTransaction.h"

class IAosTrafficListener
{
public:
    virtual ~IAosTrafficListener(){};

    virtual void Traffic_OnConnectionFailed(IN IMS_UINT32 nType, IN IMS_UINT32 nFailureReason,
            IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis) = 0;

    virtual void Traffic_OnConnectionSetupPrepared(IN IMS_UINT32 nType) = 0;
};

class AosTraffic : public IImsRadioConnectionListener
{
public:
    AosTraffic(IN IMS_UINT32 nType, IN IAosTrafficListener* piTrafficListener) :
            m_nType(nType),
            m_piTrafficListener(piTrafficListener)
    {
    }

    virtual ~AosTraffic() {}

    // IImsRadioConnectionListener
    inline void ImsRadio_OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override
    {
        if (m_piTrafficListener != IMS_NULL)
        {
            m_piTrafficListener->Traffic_OnConnectionFailed(
                    m_nType, nFailureReason, nCauseCode, nWaitTimeMillis);
        }
    }

    inline void ImsRadio_OnConnectionSetupPrepared() override
    {
        if (m_piTrafficListener != IMS_NULL)
        {
            m_piTrafficListener->Traffic_OnConnectionSetupPrepared(m_nType);
        }
    }

private:
    IMS_UINT32 m_nType;
    IAosTrafficListener* m_piTrafficListener;
};

class AosTransaction :
        public IAosTransaction,
        public IAosTrafficListener,
        public ITimerListener,
        public IImsRadioTrafficPriorityListener
{
public:
    explicit AosTransaction(IN IMS_SINT32 nSlotId);
    virtual ~AosTransaction();

    void SetListener(IN IMS_UINT32 nType, IN IAosTransactionListener* piListener) override;
    void RemoveListener(IN IMS_UINT32 nType, IN IAosTransactionListener* piListener) override;

    IMS_BOOL IsTransactionAllowed(IN IMS_UINT32 nType) override;

    IMS_BOOL StartTraffic(IN IMS_UINT32 nType, IN IMS_UINT32 nRadioType) override;
    void StartEmergencyTraffic(IN IMS_UINT32 nRadioType) override;
    void StopTraffic(IN IMS_UINT32 nType) override;
    void StopEmergencyTraffic() override;

    void SetWlan(IN IMS_BOOL bEnabled) override;

protected:
    IMS_BOOL IsResponseWaiting(IN IMS_UINT32 nType) const;
    IMS_BOOL IsStarted() const;
    IMS_BOOL IsStarted(IN IMS_UINT32 nType) const;
    IMS_BOOL IsStartUpdated() const;
    IMS_BOOL IsTimerRunning() const;
    IMS_BOOL IsTrafficResponseWaiting() const;

    // ITimerListener Interface
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // For Unit Test
    inline ImsMap<IMS_UINT32, ImsList<IAosTransactionListener*>> GetListeners()
    {
        return m_objListeners;
    }

private:
    IMS_UINT32 GetAccessNetworkType(IN IMS_UINT32 nRadioType);

    void NotifyConnectionFailed(IN IN IMS_UINT32 nType, IN IMS_UINT32 nFailureReason,
            IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis);
    void NotifyConnectionSetupPrepared(IN IMS_UINT32 nType);
    void NotifyTrafficPriorityChanged(IN IMS_UINT32 nType);

    void Start(IN IMS_UINT32 nType);
    void Stop(IN IMS_UINT32 nType);

    void AddForWaitingResponse(IN IMS_UINT32 nType);
    void RemoveForWaitingResponse(IN IMS_UINT32 nType);

    void StartTimer(IN IMS_UINT32 nDuration);
    void StopTimer();
    void ProcessTimerExpired();

    // IAosTrafficListener
    void Traffic_OnConnectionFailed(IN IMS_UINT32 nType, IN IMS_UINT32 nFailureReason,
            IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis) override;

    void Traffic_OnConnectionSetupPrepared(IN IMS_UINT32 nType) override;

    // IImsRadioTrafficPriorityListener
    void ImsRadio_OnTrafficPriorityChanged() override;

protected:
    IImsRadio* m_piImsRadio;
    ITimer* m_piStopTimer;

private:
    IMS_BOOL m_bIsEmergencyStartUpdated;
    IMS_BOOL m_bIsStartUpdated;
    IMS_BOOL m_bIsTrafficResponseWaiting;

    IMS_UINT32 m_nResponseWaitingTraffics;
    IMS_SINT32 m_nSlotId;
    IMS_UINT32 m_nStartType;
    IMS_UINT32 m_nTraffics;

    ImsMap<IMS_UINT32, ImsList<IAosTransactionListener*>> m_objListeners;
    ImsMap<IMS_UINT32, AosTraffic*> m_objTraffics;

    AString m_strTag;

    static const IMS_UINT32 TIME_STOP_DELAY = 1000;
};

#endif
