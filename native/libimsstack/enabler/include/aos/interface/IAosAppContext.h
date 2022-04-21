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
#ifndef INTERFACE_AOS_APP_CONTEXT_H_
#define INTERFACE_AOS_APP_CONTEXT_H_

#include "IMSTypeDef.h"
#include "IMSMap.h"
#include "AString.h"

#include "provider/AosStaticProfile.h"

class IAosHandle;
class IAosApplication;
class IAosConnection;
class IAosRegistration;
class IAosNetTracker;
class IAosBlock;
class IAosSubscriber;
class IAosPcscf;

class IAosAppContext
{
public:
    virtual IMS_SINT32 GetSlotId() const = 0;
    virtual const AString& GetProfileId() const = 0;

    virtual IAosHandle* GetHandle(IN CONST AString& strSrvId) const = 0;
    virtual IAosHandle* GetHandle(IN IMS_UINT32 nServiceType) = 0;
    virtual IMSMap<AString, IAosHandle*>& GetHandles() = 0;

    virtual IAosApplication* GetApp() const = 0;
    virtual IAosConnection* GetConnection() const = 0;
    virtual IAosRegistration* GetRegistration() const = 0;
    virtual IAosNetTracker* GetNetTracker() const = 0;
    virtual IAosBlock* GetBlock() const = 0;
    virtual IAosSubscriber* GetSubscriber() const = 0;
    virtual IAosPcscf* GetPcscf() const = 0;
    virtual AosStaticProfile* GetStaticProfile() const = 0;

private:
    friend class AosBuildDirector;

    virtual void SetSlotId(IN IMS_SINT32 nSlotId) = 0;

    virtual void AddHandle(IN CONST AString& strSrvId, IN IAosHandle* piHandle) = 0;

    virtual void SetApp(IN IAosApplication* piApp) = 0;
    virtual void SetConnection(IN IAosConnection* piConnection) = 0;
    virtual void SetRegistration(IN IAosRegistration* piRegistration) = 0;
    virtual void SetNetTracker(IN IAosNetTracker* piNetTracker) = 0;
    virtual void SetBlock(IN IAosBlock* piBlock) = 0;
    virtual void SetSubscriber(IN IAosSubscriber* piSubscriber) = 0;
    virtual void SetPcscf(IN IAosPcscf* piPcscf) = 0;
};
#endif // INTERFACE_AOS_APP_CONTEXT_H_
