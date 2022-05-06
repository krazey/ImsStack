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
#ifndef PLATFORM_API_H_
#define PLATFORM_API_H_

#include "ISocket.h"

class PlatformApi
{
public:
    // Checks if socket bind information is used or not.
    static IMS_BOOL CheckIpAndPortAvailability(
            IN const IPAddress& objIp, IN IMS_SINT32 nPort, IN ISocket::SOCKET_ENTYPE enType);
    // Sets IMS debug flag
    static void SetDebugOn(IN IMS_BOOL bDebugOn);

    // Methods for IMS private property
    static AString GetPrivateProperty(
            IN IMS_BOOL bPersistent, IN const AString& strKey, IN IMS_SINT32 nSlotId);
    static void SetPrivateProperty(IN IMS_BOOL bPersistent, IN const AString& strKey,
            IN const AString& strValue, IN IMS_SINT32 nSlotId);
};

#endif
