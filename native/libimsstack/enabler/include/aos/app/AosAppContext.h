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
    IMS_SINT32 GetSlotId() const override;

    const AString& GetProfileId() const override;

    IAosHandle* GetHandle(IN const AString& strSrvId) const override;
    IAosHandle* GetHandle(IN IMS_UINT32 nServiceType) override;
    ImsMap<AString, IAosHandle*>& GetHandles() override;

    IAosApplication* GetApp() const override;
    IAosConnection* GetConnection() const override;
    IAosRegistration* GetRegistration() const override;
    IAosNetTracker* GetNetTracker() const override;
    IAosBlock* GetBlock() const override;
    IAosSubscriber* GetSubscriber() const override;
    IAosPcscf* GetPcscf() const override;
    AosStaticProfile* GetStaticProfile() const override;

private:
    void SetSlotId(IN IMS_SINT32 nSlotId) override;

    void AddHandle(IN const AString& strSrvId, IN IAosHandle* piHandle) override;

    void SetApp(IN IAosApplication* piApp) override;
    void SetConnection(IN IAosConnection* piConnection) override;
    void SetRegistration(IN IAosRegistration* piRegistration) override;
    void SetNetTracker(IN IAosNetTracker* piNetTracker) override;
    void SetBlock(IN IAosBlock* piBlock) override;
    void SetSubscriber(IN IAosSubscriber* piSubscriber) override;
    void SetPcscf(IN IAosPcscf* piPcscf) override;

private:
    IMS_SINT32 m_nSlotId;

    AosStaticProfile* m_pStaticProfile;

    ImsMap<AString, IAosHandle*> objAosHandles;

    IAosApplication* m_piApp;
    IAosConnection* m_piConnection;
    IAosRegistration* m_piRegistration;
    IAosNetTracker* m_piNetTracker;
    IAosBlock* m_piBlock;
    IAosSubscriber* m_piSubscriber;
    IAosPcscf* m_piPcscf;
};
#endif  // AOS_APP_CONTEXT_H_
