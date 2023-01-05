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
#ifndef INTERFACE_AOS_CALL_TRACKER_H_
#define INTERFACE_AOS_CALL_TRACKER_H_

class IAosCallTrackerListener;
enum class CallState;

class IAosCallTracker
{
public:
    virtual ~IAosCallTracker(){};

    virtual IMS_BOOL SetMtcReady() const = 0;

    virtual IMS_BOOL IsCsCallActive() const = 0;
    virtual IMS_BOOL IsNormalCallActive() const = 0;
    virtual IMS_BOOL IsEmergencyCallActive() const = 0;

    // IsVideoCallingActive() returns true when
    // a video calling is in the state of OFFHOOK.
    virtual IMS_BOOL IsVideoCallingActive() const = 0;

    virtual IMS_SINT32 GetSlotId() const = 0;
    virtual CallState GetCallState(IN IMS_UINT32 nType) const = 0;

    virtual void SetCsCallStateWatchMode() = 0;
    virtual void SetActiveCsCallState(IN CallState eActiveCsState) = 0;

    virtual void SetListener(IN IAosCallTrackerListener* piListener) = 0;
    virtual void RemoveListener(IN IAosCallTrackerListener* piListener) = 0;

    enum
    {
        TYPE_CS = 0,
        TYPE_NORMAL,
        TYPE_EMERGENCY
    };
};

enum class CallState
{
    IDLE,
    TERMINATING,
    RINGBACK,
    RINGING,
    ALERTING,
    OFFHOOK
};

#endif  // INTERFACE_AOS_CALL_TRACKER_H_
