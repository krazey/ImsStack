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

#include "ImsMap.h"
#include "MtsDef.h"
#include "ServiceTimer.h"

class MtsTimer : public ITimerListener
{
public:
    MtsTimer();
    virtual ~MtsTimer();

public:
    virtual void Timer_TimerExpired(IN ITimer* piExpiredTimer);

    void StartTimer(IN MtsTimerType eType, IN IMS_SINT32 nDuration);
    void StopTimer(IN MtsTimerType eType);

    inline IMS_BOOL IsScbm() { return m_bIsScbm; }
    inline void SetScbmState(IN IMS_BOOL bIsScbm) { m_bIsScbm = bIsScbm; }

    inline void SetListener(IN ITimerListener* piListener) { m_piTimerListener = piListener; }

private:
    IMSMap<MtsTimerType, ITimer*> m_objTimers;
    ITimerListener* m_piTimerListener;
    IMS_BOOL m_bIsScbm;
};

#endif
