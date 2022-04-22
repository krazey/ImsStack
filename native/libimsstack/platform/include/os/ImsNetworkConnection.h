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
#ifndef IMS_NETWORK_CONNECTION_H_
#define IMS_NETWORK_CONNECTION_H_

#include "ImsNetworkPing.h"
#include "ImsSlot.h"
#include "INetworkConnection.h"

typedef IMS_UINT32    IMS_CONNECTION;

class ImsNetworkConnection
    : public ImsSlot
    , public INetworkConnection
{
public:
    inline ImsNetworkConnection(IN IMS_SINT32 nSlotId)
        : ImsSlot(nSlotId)
    {}
    inline virtual ~ImsNetworkConnection()
    {}

public:
    inline INetworkPing* CreatePing() override
    { return new ImsNetworkPing(); }
    virtual IMS_BOOL Create(IN const AString& strNetProfile) = 0;
    virtual IMS_BOOL Create(IN IMS_SINT32 nApnType) = 0;
    virtual void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) = 0;
    virtual IMS_BOOL Equals(IN const IPAddress& objIpAddr) const = 0;
    virtual IMS_CONNECTION GetHandle() const = 0;
    virtual const AString& GetProfileName() const = 0;
    virtual IMS_SINT32 GetApnType() const = 0;
};

#endif
