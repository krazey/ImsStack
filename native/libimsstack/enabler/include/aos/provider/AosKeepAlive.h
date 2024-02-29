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

#ifndef AOS_KEEP_ALIVE_H_
#define AOS_KEEP_ALIVE_H_

#include "ITimer.h"
#include "ISipKeepAliveHelperListener.h"
#include "IpAddress.h"

class ISipKeepAliveHelper;
class IAosKeepAliveListener;

class AosKeepAlive : public ISipKeepAliveHelperListener, public ITimerListener
{
public:
    explicit AosKeepAlive(IN IMS_SINT32 nSlotId);
    virtual ~AosKeepAlive();

    virtual void SetListener(IN IAosKeepAliveListener* piListener);

    // nRepeatTime is millisecond
    void Start(IN IMS_UINT32 nRepeatTime, IN IMS_BOOL bCheckingPong = IMS_TRUE);
    void Stop();

    void SetTransport(IN const IpAddress& objSourceIpAddress, IN IMS_SINT32 nSourcePort,
            IN const IpAddress& objDestIpAddress, IN IMS_SINT32 nDestPort,
            IN IMS_SINT32 nProtocol = AosKeepAlive::TRANSPORT_UDP);

    enum
    {
        TRANSPORT_ANY = 0,
        TRANSPORT_UDP,
        TRANSPORT_TCP,
        TRANSPORT_TLS,
        TRANSPORT_MAX,
    };

    enum
    {
        TIMER_KEEP_ALIVE = 0,
        TIMER_PONG_WAIT
    };

protected:
    void SendPing();

    void SetCheckingPong(IN IMS_BOOL bCheck);
    IMS_BOOL IsPongChecked() const;

    void ProcessKeepAliveTimerExpired();
    void ProcessPongWaitTimerExpired();

    IMS_BOOL StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    IMS_BOOL StopTimer(IN IMS_UINT32 nType);
    void ClearTimer();

    // ISipKeepAliveHelperListener
    void KeepAliveHelper_PongReceived() override;

    // ITimerListener Interface
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);

protected:
    ISipKeepAliveHelper* m_piKeepAliveHelper;
    IAosKeepAliveListener* m_piListener;
    ITimer* m_piKeepAliveTimer;
    ITimer* m_piPongWaitTimer;
    IMS_UINT32 m_nKeepAliveTime;
    IMS_BOOL m_bIsPongChecked;
    IMS_SINT32 m_nSlotId;
    AString m_strTag;

    static const IMS_UINT32 PONG_WAIT_TIME_MILLIS = 10000;
};

class IAosKeepAliveListener
{
public:
    virtual ~IAosKeepAliveListener(){};

    virtual void KeepAlive_DetectedFlowFailed() = 0;
};

#endif  // AOS_KEEP_ALIVE_H_