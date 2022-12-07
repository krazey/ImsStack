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
#ifndef AOS_APP_CONTEXT_H_
#define AOS_APP_CONTEXT_H_

#include "interface/IAosAppContext.h"

class AosAppContext : public IAosAppContext
{
public:
    explicit AosAppContext(IN AosStaticProfile* pProfile);
    virtual ~AosAppContext();

private:
    AosAppContext(IN const AosAppContext& objRHS);
    AosAppContext& operator=(IN const AosAppContext& objRHS);

public:
    virtual IMS_SINT32 GetSlotId() const;

    virtual const AString& GetProfileId() const;

    virtual IAosHandle* GetHandle(IN const AString& strSrvId) const;
    virtual IAosHandle* GetHandle(IN IMS_UINT32 nServiceType);
    virtual IMSMap<AString, IAosHandle*>& GetHandles();

    virtual IAosApplication* GetApp() const;
    virtual IAosConnection* GetConnection() const;
    virtual IAosRegistration* GetRegistration() const;
    virtual IAosNetTracker* GetNetTracker() const;
    virtual IAosBlock* GetBlock() const;
    virtual IAosSubscriber* GetSubscriber() const;
    virtual IAosPcscf* GetPcscf() const;
    virtual AosStaticProfile* GetStaticProfile() const;

private:
    virtual void SetSlotId(IN IMS_SINT32 nSlotId);

    virtual void AddHandle(IN const AString& strSrvId, IN IAosHandle* piHandle);

    virtual void SetApp(IN IAosApplication* piApp);
    virtual void SetConnection(IN IAosConnection* piConnection);
    virtual void SetRegistration(IN IAosRegistration* piRegistration);
    virtual void SetNetTracker(IN IAosNetTracker* piNetTracker);
    virtual void SetBlock(IN IAosBlock* piBlock);
    virtual void SetSubscriber(IN IAosSubscriber* piSubscriber);
    virtual void SetPcscf(IN IAosPcscf* piPcscf);

private:
    IMS_SINT32 m_nSlotId;

    AosStaticProfile* m_pStaticProfile;

    IMSMap<AString, IAosHandle*> objAosHandles;

    IAosApplication* m_piApp;
    IAosConnection* m_piConnection;
    IAosRegistration* m_piRegistration;
    IAosNetTracker* m_piNetTracker;
    IAosBlock* m_piBlock;
    IAosSubscriber* m_piSubscriber;
    IAosPcscf* m_piPcscf;
};
#endif  // AOS_APP_CONTEXT_H_
