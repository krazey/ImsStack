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

#ifndef QOS_TIMER_H_
#define QOS_TIMER_H_

#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "ServiceTimer.h"
#include "precondition/IQosTimerListener.h"
#include "precondition/QosDef.h"

class QosTimer : public ITimerListener
{
public:
    explicit QosTimer(IN IQosTimerListener* pListener);
    virtual ~QosTimer();

private:
    QosTimer(IN const QosTimer& objRHS);
    QosTimer& operator=(IN const QosTimer& objRHS);

public:
    virtual void Timer_TimerExpired(IN ITimer* piExpiredTimer);

    void StartQosTimer(IN QosTimerType eType, IN IMS_SINT32 nDuration);
    void StopQosTimer(IN QosTimerType eType);

    IMS_BOOL IsQosTimerActivated(IN QosTimerType eType);

private:
    ITimer* GetTimer(IN QosTimerType eType);

protected:
    ImsMap<QosTimerType, ITimer*> m_objTimers;
    IQosTimerListener* m_pQosTimerListener;
};
#endif
