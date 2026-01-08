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
#ifndef REGISTRATION_MANAGER_H_
#define REGISTRATION_MANAGER_H_

#include "ImsMap.h"

#include "IRegistrationManager.h"
#include "RegKey.h"

class IMutex;

/**
 * @brief A class for managing the IMS registrations and providing its related functions.
 */
class RegistrationManager : public IRegistrationManager
{
public:
    RegistrationManager();
    ~RegistrationManager() override;

public:
    RegistrationManager(IN const RegistrationManager&) = delete;
    RegistrationManager& operator=(IN const RegistrationManager&) = delete;

public:
    IMS_BOOL CreateRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId,
            IN const AString& strAor, IN IMS_BOOL bEmergency, IN IMS_BOOL bFake = IMS_FALSE,
            IN const AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pProfile = IMS_NULL) override;
    IMS_BOOL CreateRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId,
            IN const SipAddress& objAor, IN IMS_BOOL bEmergency, IN IMS_BOOL bFake = IMS_FALSE,
            IN const AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pProfile = IMS_NULL) override;
    void DestroyRegistration(IN IRegistration* piReg, IN IMS_BOOL bByForce = IMS_FALSE) override;
    IMS_BOOL IsRegSubscriptionSupported(
            IN IMS_SINT32 nSlotId, IN SipProfile* pProfile = IMS_NULL) const override;
    IRegistration* GetRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const override;

private:
    void ClearRegistrations();

private:
    IMutex* m_piLock;
    // List of registration (bindings): < AOR (IMPU) + Contacts >
    ImsMap<RegKey, IRegistration*> m_objRegistrations;
};

#endif
