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

#ifndef MTC_TIMER_WRAPPER_H_
#define MTC_TIMER_WRAPPER_H_

#include "ITimer.h"
#include "ImsList.h"
#include "ServiceTimer.h"

class IMtcTimerListener;

class MtcTimerWrapper : public ITimerListener
{
public:
    MtcTimerWrapper();
    virtual ~MtcTimerWrapper();
    MtcTimerWrapper(IN const MtcTimerWrapper&) = delete;
    MtcTimerWrapper& operator=(IN const MtcTimerWrapper&) = delete;

    // ITimerListener implementation
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    virtual void SetListener(IN IMtcTimerListener* piListener);
    virtual void Start(IN IMS_UINT32 eType, IN IMS_SINT32 nDuration);
    virtual void Stop(IN IMS_UINT32 eType);
    virtual void StopAll();
    virtual IMS_BOOL IsActive(IN IMS_UINT32 eType);

    struct MtcTimer
    {
        explicit MtcTimer(IN IMS_UINT32 eType) :
                eType(eType),
                piTimer(TimerService::GetTimerService()->CreateTimer())
        {
        }
        ~MtcTimer()
        {
            if (piTimer)
            {
                piTimer->KillTimer();
                TimerService::GetTimerService()->DestroyTimer(piTimer);
            }
        }
        MtcTimer(IN const MtcTimer&) = delete;
        MtcTimer& operator=(IN const MtcTimer&) = delete;

        IMS_UINT32 eType;
        ITimer* piTimer;
    };

private:
    void Clear();

    ImsList<MtcTimer*> m_lstTimers;
    IMtcTimerListener* m_piListener;
};

#endif
