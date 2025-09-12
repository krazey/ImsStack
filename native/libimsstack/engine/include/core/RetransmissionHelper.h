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
#ifndef RETRANSMISSION_HELPER_H_
#define RETRANSMISSION_HELPER_H_

#include "ITimer.h"

class IRetransmissionHelperListener;
class Service;

class RetransmissionHelper : public ITimerListener
{
public:
    explicit RetransmissionHelper(IN Service* pService, IN IMS_BOOL bIntervalCap = IMS_TRUE);
    ~RetransmissionHelper() override;

    RetransmissionHelper(IN const RetransmissionHelper&) = delete;
    RetransmissionHelper& operator=(IN const RetransmissionHelper&) = delete;

public:
    inline void SetListener(IN IRetransmissionHelperListener* piListener)
    {
        m_piListener = piListener;
    }
    void SetMaxDuration(IN IMS_SINT32 nValue);
    IMS_RESULT Start();
    void Stop();

protected:
    void Timer_TimerExpired(IN ITimer* piTimer) override;

public:
    enum
    {
        NOTIFICATION_INTERNAL_ERROR = 0,
        NOTIFICATION_RETRANSMIT = 1,
        NOTIFICATION_TIMER_EXPIRED = 2
    };

private:
    enum
    {
        TIMER_T1 = 2000
    };
    enum
    {
        TIMER_T2 = (8 * TIMER_T1)
    };
    enum
    {
        TIMER_MAX = (64 * TIMER_T1)
    };

    IMS_SINT32 m_nDuration;
    IMS_SINT32 m_nCumulativeDuration;
    IMS_SINT32 m_nMaxDuration;
    IMS_SINT32 m_nIntervalCap;
    ITimer* m_piTimer;
    IRetransmissionHelperListener* m_piListener;
};

#endif
