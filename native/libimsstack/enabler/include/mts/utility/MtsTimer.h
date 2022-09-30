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

#ifndef MTS_TIMER_H_
#define MTS_TIMER_H_

#include "ITimer.h"
#include "ImsMap.h"
#include "MtsDef.h"

class MtsTimer : public ITimerListener
{
public:
    MtsTimer();
    virtual ~MtsTimer();

public:
    // ITimerListener implementation
    void Timer_TimerExpired(IN ITimer* piExpiredTimer) override;

    void StartTimer(IN MtsTimerType eType, IN IMS_SINT32 nDuration);
    void StopTimer(IN MtsTimerType eType);

    inline void SetListener(IN ITimerListener* piListener) { m_piTimerListener = piListener; }

private:
    ImsMap<MtsTimerType, ITimer*> m_objTimers;
    ITimerListener* m_piTimerListener;
};

#endif
