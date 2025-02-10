
/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef PASSIVE_TIMER_HOLDER_H_
#define PASSIVE_TIMER_HOLDER_H_

#include "ITimer.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/IPassiveTimerHolder.h"

class IMtcService;
class IPassiveTimerListener;

class PassiveTimerHolder final :
        public IPassiveTimerHolder,
        public IMtcAosStateListener,
        public ITimerListener
{
public:
    PassiveTimerHolder();
    virtual ~PassiveTimerHolder();
    PassiveTimerHolder(IN const PassiveTimerHolder&) = delete;
    PassiveTimerHolder& operator=(IN const PassiveTimerHolder&) = delete;

    void AddTimer(IN IPassiveTimerHolder::Type eType, IN IMS_SINT32 nTimeInMillis,
            IN IMS_BOOL bAllowReset = IMS_FALSE) override;
    void RemoveTimer(IN IPassiveTimerHolder::Type eType) override;
    IMS_BOOL IsActive(IN IPassiveTimerHolder::Type eType) const override;
    void AddListener(IN IPassiveTimerHolder::Type eType,
            IN IPassiveTimerListener* pPassiveTimerListener) override;
    void RemoveListener(IN IPassiveTimerHolder::Type eType,
            IN IPassiveTimerListener* pPassiveTimerListener) override;

    void OnAosStateChanged(IN IMtcService& objMtcService, IN MtcAosState eState,
            IN IMS_UINT32 eAosReason) override;
    inline void OnIpcanChanged(IN IMtcService&, IN IMS_UINT32) override {}

    void Timer_TimerExpired(IN ITimer* piTimer) override;

    void SetNormalService(IN IMtcService* pService);

private:
    struct TimerInfo
    {
    public:
        inline explicit TimerInfo(IN ITimer* piTimer) :
                piTimer(piTimer),
                objListeners(ImsList<IPassiveTimerListener*>()),
                bIsTerminating(IMS_FALSE)
        {
        }

        inline virtual ~TimerInfo() { objListeners.Clear(); }

    private:
        TimerInfo(IN const TimerInfo&) = delete;
        TimerInfo& operator=(IN const TimerInfo&) = delete;

    public:
        void AddListener(IN IPassiveTimerListener* pPassiveTimerListener)
        {
            for (IMS_UINT32 i = 0; i < objListeners.GetSize(); i++)
            {
                if (objListeners.GetAt(i) == pPassiveTimerListener)
                {
                    return;
                }
            }

            objListeners.Append(pPassiveTimerListener);
        }

        void RemoveListener(IN IPassiveTimerListener* pPassiveTimerListener)
        {
            if (bIsTerminating)
            {
                return;
            }

            for (IMS_UINT32 i = 0; i < objListeners.GetSize(); i++)
            {
                if (objListeners.GetAt(i) == pPassiveTimerListener)
                {
                    objListeners.RemoveAt(i);
                    break;
                }
            }
        }

        void SetTerminating() { bIsTerminating = IMS_TRUE; }

        ITimer* piTimer;
        ImsList<IPassiveTimerListener*> objListeners;
        IMS_BOOL bIsTerminating;
    };

    IMS_SLONG GetIndexOfTimerInfo(IN const ITimer* piTimer) const;
    void ReleaseTimerInfo(IN IPassiveTimerHolder::Type eType);
    void ReleaseAllTimerInfo();

    IMtcService* m_piService;
    ImsMap<IPassiveTimerHolder::Type, TimerInfo*> m_objTimerInfoByType;
};

#endif
