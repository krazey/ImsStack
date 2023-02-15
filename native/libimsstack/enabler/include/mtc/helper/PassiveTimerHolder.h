
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
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/IPassiveTimerHolder.h"

class IMtcService;

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

    void AddTimer(IN IPassiveTimerHolder::Type eType, IN IMS_UINT32 nTimeInMillis) override;
    IMS_BOOL IsActive(IN IPassiveTimerHolder::Type eType) const override;

    void OnAosStateChanged(IN IMtcService& objMtcService, IN MtcAosState eState,
            IN IMS_UINT32 eAosReason) override;
    inline void OnIpcanChanged(IN IMtcService&, IN IMS_UINT32) override {}

    void Timer_TimerExpired(IN ITimer* piTimer) override;

    void SetNormalService(IN IMtcService* pService);

private:
    IMS_SLONG GetIndexOfTimer(IN const ITimer* piTimer);
    void ReleaseTimer(IN ITimer* piTimer);

    IMtcService* m_pService;
    ImsMap<IPassiveTimerHolder::Type, ITimer*> m_objTimers;
};

#endif
