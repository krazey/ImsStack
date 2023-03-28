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
#ifndef INTERFACE_AOS_CONNECTION_H_
#define INTERFACE_AOS_CONNECTION_H_

#include "ImsTypeDef.h"
#include "AStringArray.h"
#include "IpAddress.h"

class IAosConnectionListener;

class IAosConnection
{
public:
    virtual ~IAosConnection(){};

    virtual IMS_BOOL Activate() = 0;
    virtual void Deactivate() = 0;
    virtual IMS_BOOL IsActivationRequested() = 0;
    virtual IMS_UINT32 GetState() = 0;

    virtual IMS_SINT32 GetConnectionType() = 0;

    virtual void SetListener(IN IAosConnectionListener* piListener) = 0;
    virtual void RemoveListener(IN IAosConnectionListener* piListener) = 0;

    // INetworkConnection Util
    virtual IMS_SINT32 GetMtu() = 0;
    virtual const IpAddress& GetLocalAddress(IN IMS_SINT32 nIpVersion = 0) = 0;
    virtual const AStringArray& GetPcscfAddress(IN IMS_SINT32 nIpVersion = 0) = 0;
    virtual IMS_SINT32 GetHostByName(IN const AString& strHostName, OUT ImsList<IpAddress>& objIps,
            IN IMS_SINT32 nIpVersion = 0) = 0;
    virtual const AString& GetIfaceName() = 0;
    virtual IMS_BOOL IsEpdgEnabled() = 0;
    virtual IMS_SINT32 GetIpcanCategory() = 0;
    virtual IMS_BOOL IsLimitedServicePcoValue() = 0;
    virtual IMS_SINT32 GetCarrierSignalPcoValue() = 0;
    virtual void SetCarrierSignalPcoValue(IN IMS_SINT32 nValue) = 0;

    enum
    {
        STATE_IDLE = 0,
        STATE_ACTIVE,
        STATE_ACTIVATING
    };

    enum
    {
        IP_VERSION_4 = 4,
        IP_VERSION_6 = 6,
        IP_VERSION_46 = 46,
        IP_VERSION_64 = 64
    };
};
#endif  // INTERFACE_AOS_CONNECTION_H_
