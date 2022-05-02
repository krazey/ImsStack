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
#include "IPAddress.h"

class ISIPKeepAliveHelper;
class IAosKeepAliveListener;

class AosKeepAlive
    : public ISIPKeepAliveHelperListener
    , public ITimerListener
{

public:
    AosKeepAlive(IN IMS_SINT32 nSlotId_);
    virtual ~AosKeepAlive();

    virtual void SetListener(IN IAosKeepAliveListener *piListener);

    // nRepeatTime is millisecond
    void Start(IN IMS_UINT32 nRepeatTime, IN IMS_BOOL bCheckingPong = IMS_TRUE);
    void Stop();

    void SetTransport(IN CONST IPAddress &objSourceIPA, IN IMS_SINT32 nSourcePort,
            IN CONST IPAddress &objDestIPA, IN IMS_SINT32 nDestPort,
            IN IMS_SINT32 nProtocol = AosKeepAlive::TRANSPORT_UDP);

    enum
    {
        TRANSPORT_ANY = 0,
        TRANSPORT_UDP,
        TRANSPORT_TCP,
        TRANSPORT_TLS,
        TRANSPORT_MAX,
    };

private:
    void SendPING();

    void SetCheckingPong(IN IMS_BOOL bCheck);
    IMS_BOOL IsPongChecked() const;

    void ProcessKeepAliveTimerExpired();
    void ProcessPongWaitTimerExpired();

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);
    void ClearTimer();

    // ISIPKeepAliveHelperListener
    virtual void KeepAliveHelper_PongReceived();

    // ITimerListener Interface
    virtual void Timer_TimerExpired(IN ITimer *piTimer);

    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);

    enum
    {
        TIMER_KEEP_ALIVE = 0,
        TIMER_PONG_WAT
    };

private:
    ISIPKeepAliveHelper *piKAHelper;
    IAosKeepAliveListener *piListener;
    ITimer *piKeepAliveTimer;
    ITimer *piPongWATTimer;
    IMS_UINT32 nKATime;
    IMS_BOOL bIsPongChecked;
    IMS_SINT32 nSlotId;

    AString strTag;
};

class IAosKeepAliveListener
{
public:
    virtual void KeepAlive_DetectedFlowFailed() = 0;
};

#endif // AOS_KEEP_ALIVE_H_
