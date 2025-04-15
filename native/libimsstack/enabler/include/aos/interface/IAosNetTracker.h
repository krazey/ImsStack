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
#ifndef INTERFACE_AOS_NET_TRACKER_H_
#define INTERFACE_AOS_NET_TRACKER_H_

#include "ImsTypeDef.h"

class IAosNetTrackerListener;

class IAosNetTracker
{
public:
    virtual ~IAosNetTracker(){};

    virtual IMS_BOOL IsServiceIn(IN IMS_UINT32 nType = TYPE_DEFAULT) = 0;
    virtual IMS_BOOL IsDataIn() = 0;
    virtual IMS_BOOL IsNetworkIn() = 0;
    virtual IMS_BOOL IsEmergencyAttach() = 0;
    virtual IMS_BOOL IsSuspended() = 0;
    virtual IMS_BOOL IsSessionContinuitySupported() = 0;
    virtual IMS_BOOL IsServiceTimerRunning() = 0;
    virtual IMS_BOOL IsImsVoiceCallSupported() = 0;
    virtual IMS_BOOL IsRoaming() = 0;

    // the network type of moblie
    virtual IMS_UINT32 GetMobileChangingNetworkType() = 0;
    virtual IMS_UINT32 GetMobileNetworkType() = 0;
    virtual IMS_UINT32 GetMobileNetworkRegistrationRejectCause() = 0;
    virtual IMS_SINT32 GetMobileVoiceServiceState() = 0;
    virtual IMS_UINT32 GetMobileVoiceNetworkType() = 0;
    /*
        the network type of serive in network
        return : mobile or WLAN
    */
    virtual IMS_UINT32 GetNetworkType() = 0;

    virtual void SetRatGuardTime(IN IMS_UINT32 nGuardTime) = 0;
    virtual void SetSrvOutGuardTime(IN IMS_UINT32 nGuardTime) = 0;
    virtual void SetSrvInGuardTime(IN IMS_UINT32 nGuardTime) = 0;

    virtual void SetListener(IN IAosNetTrackerListener* piListener) = 0;
    virtual void RemoveListener(IN IAosNetTrackerListener* piListener) = 0;

    enum
    {
        TYPE_DEFAULT = 0,
        TYPE_MOBILE,
        TYPE_WLAN
    };

protected:
    friend class AosBuildDirector;
    virtual void Init() = 0;
};
#endif  // INTERFACE_AOS_NET_TRACKER_H_
