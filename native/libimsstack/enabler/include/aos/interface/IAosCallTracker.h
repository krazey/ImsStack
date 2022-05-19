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

class IAosCallTracker
{
public:
    virtual IMS_BOOL IsCsCallActive() const = 0;
    virtual IMS_BOOL IsNormalCallActive() const = 0;
    virtual IMS_BOOL IsEmergencyCallActive() const = 0;

    // IsVideoCallingActive() returns true when
    // a video calling is in the state of OFFHOOK.
    virtual IMS_BOOL IsVideoCallingActive() const = 0;

    virtual IMS_SINT32 GetSlotId() const = 0;
    virtual IMS_UINT32 GetCallState(IN IMS_UINT32 nType) const = 0;
    virtual IMS_UINT32 GetSessionType(IN IMS_UINT32 nType) const = 0;

    virtual void SetCsCallStateWatchMode() = 0;
    virtual void SetActiveCsCallState(IN IMS_UINT32 nActiveCsState) = 0;

    virtual void SetListener(IN IAosCallTrackerListener* piListener) = 0;
    virtual void RemoveListener(IN IAosCallTrackerListener* piListener) = 0;

    enum
    {
        TYPE_CS = 0,
        TYPE_NORMAL,
        TYPE_EMERGENCY
    };

    enum
    {
        STATE_IDLE = 0,
        STATE_TERMINATING = 1,
        STATE_RINGBACK = 2,
        STATE_RINGING = 3,
        STATE_ALERTING = 4,
        STATE_OFFHOOK = 5
    };

    enum
    {
        SESSION_TYPE_NONE = 0x00000000,
        SESSION_TYPE_VOIP = 0x00000001,
        Session_TYPE_VS = 0x00000002,
        SESSION_TYPE_VT = 0x00000004
    };
};
#endif  // INTERFACE_AOS_CALL_TRACKER_H_
