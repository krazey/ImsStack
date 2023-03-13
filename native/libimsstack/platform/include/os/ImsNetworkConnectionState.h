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
#ifndef IMS_NETWORK_CONNECTION_STATE_H_
#define IMS_NETWORK_CONNECTION_STATE_H_

#include "ImsList.h"
#include "ImsNetworkConnection.h"
#include "IpAddress.h"

class IMutex;

GLOBAL void ImsNetworkConnectionState_ExitInstance();
GLOBAL IMS_BOOL ImsNetworkConnectionState_InitInstance();

class ImsNetworkConnectionState
{
private:
    ImsNetworkConnectionState();

public:
    ~ImsNetworkConnectionState();

    ImsNetworkConnectionState(IN const ImsNetworkConnectionState&) = delete;
    ImsNetworkConnectionState& operator=(IN const ImsNetworkConnectionState&) = delete;

public:
    void AttachHandle(IN ImsNetworkConnection* pConnection);
    void DetachAll();
    void DetachHandle(IN const AString& strNetProfile, IN IMS_SINT32 nSlotId);
    ImsNetworkConnection* LookupHandle(IN const AString& strNetProfile, IN IMS_SINT32 nSlotId);
    ImsNetworkConnection* LookupHandle(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    ImsNetworkConnection* LookupHandle(IN const IpAddress& objIpAddr);
    ImsNetworkConnection* LookupHandle(IN IMS_CONNECTION hConnection);
    IMS_BOOL IsHandlePresent(IN IMS_CONNECTION hConnection);
    IMS_BOOL IsEmpty() const;

    IMS_CONNECTION GetAndIncrementHandle(IN IMS_BOOL bMobile = IMS_TRUE);

    static ImsNetworkConnectionState* GetInstance();

private:
    friend void ImsNetworkConnectionState_ExitInstance();
    friend IMS_BOOL ImsNetworkConnectionState_InitInstance();

    IMS_UINT32 GetNextHandle(IN IMS_BOOL bMobile = IMS_TRUE);
    void SetNextHandle(IN IMS_UINT32 nHandle, IN IMS_BOOL bMobile = IMS_TRUE);

private:
    enum
    {
        HANDLE_MOBILE_MIN = 20001,
        HANDLE_MOBILE_MAX = 20020,

        HANDLE_WIFI_MIN = 20101,
        HANDLE_WIFI_MAX = 20120
    };

    IMutex* m_piLock;

    // List of map (IMS_CONNECTION, ImsNetworkConnection)
    // ImsMap objHandle2Object;
    ImsList<ImsNetworkConnection*> m_objNetConnectionList;

    IMS_UINT32 m_nHandleForMobile;
    IMS_UINT32 m_nHandleForWiFi;
};

#endif
