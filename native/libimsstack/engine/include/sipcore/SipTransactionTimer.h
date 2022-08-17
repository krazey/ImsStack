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
#ifndef SIP_TRANSACTION_TIMER_H_
#define SIP_TRANSACTION_TIMER_H_

#include "ITimer.h"

class SipTransactionTimer : public ITimerListener
{
public:
    SipTransactionTimer(IN SipTimeoutData* pData, IN SipTimerCallback pfnTimerCallback);
    virtual ~SipTransactionTimer();

    SipTransactionTimer(IN const SipTransactionTimer&) = delete;
    SipTransactionTimer& operator=(IN const SipTransactionTimer&) = delete;

public:
    IMS_BOOL Start(IN IMS_SINT32 nDuration);
    void Stop(OUT SipTimeoutData*& pData);

    static void FreeTimer(IN void* pvTimerHandle);
    static void TimerExpired(IN IMS_SINT32 enTimerType);

private:
    // ITimerListener
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

private:
    ITimer* m_piTimer;
    SipTimeoutData* m_pData;
    SipTimerCallback m_pfnTimerCallback;
};

#endif
