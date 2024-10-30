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
#ifndef INTERFACE_AOS_APPLICATION_H_
#define INTERFACE_AOS_APPLICATION_H_

#include "ImsTypeDef.h"

/**
 * @brief This class provides the interface for Always On Service.
 *
 * It controls the overall operation of the AoS module and sends the information
 * suitable for the necessary module and triggers the required operations.
 *
 */

class IAosApplication
{
public:
    virtual ~IAosApplication(){};

    virtual void Reconfig() = 0;

    virtual IMS_BOOL RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0) = 0;

    virtual const AString& GetActivityName() = 0;
    virtual void GetProperty(
            IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue) = 0;
    virtual IMS_UINT32 GetAppState() = 0;
    virtual IMS_UINT32 GetOffReason() = 0;

    virtual IMS_BOOL IsActivated() = 0;
    virtual IMS_BOOL IsOn() = 0;

    virtual void SetActivation(IN IMS_BOOL bActivation) = 0;
    virtual void NotifyEpsFallbackCallState(IN IMS_UINT32 nState) = 0;
    virtual void NotifyPublishState(IN IMS_BOOL bStart) = 0;

    /// GetState
    enum
    {
        STATE_NOTREADY = 0,
        STATE_READY,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_UPDATING,
        STATE_DISCONNECTING
    };

    /// App_StateChanged
    enum
    {
        APP_DISCONNECTED = 0,
        APP_CONNECTED,
        APP_UPDATING,
        APP_DISCONNECTING
    };

    /// Property
    enum
    {
        PROPERTY_REGISTERED_RAT = 0
    };

    /// Command
    enum
    {
        CMD_ECALL_INIT = 100,
        CMD_ESMS_INIT
    };

protected:
    friend class AosBuildDirector;
    friend class AosAppContext;
    virtual void Init() = 0;
    virtual void CleanUp() = 0;
};
#endif  // INTERFACE_AOS_APPLICATION_H_
