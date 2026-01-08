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
#ifndef OS_TIMER_H_
#define OS_TIMER_H_

#include "ImsTimer.h"

class OsTimer final : public ImsTimer
{
public:
    OsTimer();
    ~OsTimer() override;

    OsTimer(IN const OsTimer&) = delete;
    OsTimer& operator=(IN const OsTimer&) = delete;

public:
    // ITimer class
    IMS_BOOL Equals(IN const ITimer* piTimer) const override;
    IMS_UINTP SetTimer(IN IMS_SINT64 nDuration, IN ITimerListener* piListener) override;
    void KillTimer() override;

    // ImsTimer class
    void Destroy() override;
    inline IMS_UINTP GetTimerId() const override { return m_nTimerId; }
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;

    // For internal uses
    inline IMS_UINT32 GetInternalTimerId() const { return m_nInternalTimerId; }

private:
    IMS_UINTP CreateTimerId() const;

private:
    enum
    {
        STATE_INACTIVE = 0,
        STATE_ACTIVE
    };

    static IMS_UINT32 s_nInternalTimerId;

    IMS_SINT32 m_nState;
    IMS_UINTP m_nTimerId;
    // For internal uses :: compare the internal timer id with lParam
    IMS_UINT32 m_nInternalTimerId;
};

#endif
