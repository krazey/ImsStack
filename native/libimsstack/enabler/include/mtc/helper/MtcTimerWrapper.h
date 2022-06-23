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

#include "ServiceTimer.h"
#include "ImsList.h"
#include "ITimer.h"

class IMtcTimerListener;

class MtcTimerWrapper final : public ITimerListener
{
public:
    explicit MtcTimerWrapper();
    ~MtcTimerWrapper();
    MtcTimerWrapper(IN const MtcTimerWrapper&) = delete;
    MtcTimerWrapper& operator=(IN const MtcTimerWrapper&) = delete;

    // ITimerListener implementation
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    void SetListener(IN IMtcTimerListener* piListener);
    void Start(IN IMS_UINT32 eType, IN IMS_SINT32 nDuration);
    void Stop(IN IMS_UINT32 eType);
    void StopAll();
    IMS_BOOL IsActive(IN IMS_UINT32 eType);

    struct MtcTimer
    {
        MtcTimer(IN IMS_UINT32 eType) :
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
    // TODO: IMSMap<type, timer> would be better.
    IMSList<MtcTimer*> m_lstTimers;
    IMtcTimerListener* m_piListener;
};

#endif
