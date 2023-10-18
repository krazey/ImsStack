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
#ifndef _UCE_OPTIONS_MANAGER_H_
#define _UCE_OPTIONS_MANAGER_H_

#include "ImsActivityEx.h"
#include "ImsMap.h"

class UceOptions;
class ICoreService;
class ICapabilities;
class IUceJniThread;

class UceOptionsManager : public ImsActivityEx
{
public:
    explicit UceOptionsManager(
            IN const AString& strName, IN ICoreService* piCoreService, IN IMS_SINT32 simSlotId);
    virtual ~UceOptionsManager();

    IMS_BOOL SendOptionsRequest(
            IN IMS_UINT32 nKey, IN const AString& strRemoteURI, IN IMS_UINT32 ownCapabilities);
    IMS_BOOL SendOptionsResponse(IN IMS_UINT32 nKey, IN IMS_UINT32 nResponse,
            IN const AString& reason, IN IMS_UINT32 ownCapabilities);
    IMS_BOOL ReceivedOptions(
            IN const ICoreService* piCoreService, IN ICapabilities* piCapabilities);

    void AoSConnected();
    void AoSDisconnected();
    IMS_BOOL ClosedService();  // core service closed

protected:
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMsg);

private:
    IMS_UINT32 getReceivedKey();
    void SendOptionsReceivedInd(IN IMS_UINT32 nKey, IN AString from, IN IMS_UINT32 capabilities);
    void SendOptionsCommandError(IN IMS_UINT32 nKey, IN IMS_UINT32 code);
    IUceJniThread* GetJniThread();

protected:
    IMS_BOOL m_bAoSConnected;
    ImsMap<IMS_UINT32, UceOptions*> m_objSentUceOptionsMap;
    ImsMap<IMS_UINT32, UceOptions*> m_objReceivedUceOptionsMap;

private:
    IMS_SINT32 m_nSimSlot;
    ICoreService* m_piCoreService;
    IMS_UINT32 m_nReceivedOptionKey;
};
#endif  // _UCE_OPTIONS_MANAGER_H_
