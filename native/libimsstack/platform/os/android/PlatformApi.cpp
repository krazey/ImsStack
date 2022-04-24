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
#include "OsDb.h"
#include "OsUtil.h"
#include "PlatformApi.h"
#include "network/OsSocket.h"
#include "system-intf/System.h"

PUBLIC GLOBAL
IMS_BOOL PlatformApi::CheckIpAndPortAvailability(IN const IPAddress& objIp,
        IN IMS_SINT32 nPort, IN ISocket::SOCKET_ENTYPE enType)
{
    return OsSocket::CheckIpAndPortAvailability(objIp, nPort, enType);
}

PUBLIC GLOBAL
IMS_BOOL PlatformApi::CheckIfDatabaseExists(IN const AString& strDb)
{
    return OsDb::Exists(strDb);
}

PUBLIC GLOBAL
void PlatformApi::SetDebugOn(IN IMS_BOOL bDebugOn)
{
    OsUtil::GetInstance()->SetDebugOn(bDebugOn);
}

// Methods for IMS private property
PUBLIC GLOBAL
AString PlatformApi::GetPrivateProperty(IN IMS_BOOL bPersistent,
        IN const AString& strKey, IN IMS_SINT32 nSlotId)
{
    return System::GetInstance()->GetPrivateProperty(bPersistent, strKey, nSlotId);
}

PUBLIC GLOBAL
void PlatformApi::SetPrivateProperty(IN IMS_BOOL bPersistent,
        IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId)
{
    System::GetInstance()->SetPrivateProperty(bPersistent, strKey, strValue, nSlotId);
}
