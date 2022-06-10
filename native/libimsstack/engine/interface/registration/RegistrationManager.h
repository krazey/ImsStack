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

#include "IRegistration.h"

class IRegistrationListener;
class RegistrationManagerPrivate;

/**
 * @brief This class provides an interface to create/destroy IMS registrations.
 */
class RegistrationManager
{
private:
    RegistrationManager();

public:
    ~RegistrationManager();

    RegistrationManager(IN const RegistrationManager&) = delete;
    RegistrationManager& operator=(IN const RegistrationManager&) = delete;

public:
    /**
     * @brief Creates Registration object for IMS registration.
     *
     * @param nFlowId Registration flow id
     * @param strAor Address-Of-Record(public user identity to be registered)
     * @param bFake Flag to indicate whether it's used for fake or not
     * @param strSubsId Subscriber configuration's identifier as a string
     * @param pProfile SIP profile for this registration
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     * @note MULTI_SUBS, MULTI_REG_SIP_PROFILE
     */
    IMS_BOOL CreateRegistration(IN IMS_UINT32 nFlowId, IN const AString& strAor,
            IN IMS_BOOL bFake = IMS_FALSE, IN const AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Creates Registration object for IMS registration.
     *
     * @param nFlowId Registration flow id
     * @param strAor Address-Of-Record(public user identity to be registered)
     * @param bFake Flag to indicate whether it's used for fake or not
     * @param strSubsId Subscriber configuration's identifier as a string
     * @param pProfile SIP profile for this registration
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     * @note MULTI_SUBS, MULTI_REG_SIP_PROFILE
     */
    IMS_BOOL CreateRegistration(IN IMS_UINT32 nFlowId, IN const SipAddress& objAor,
            IN IMS_BOOL bFake = IMS_FALSE, IN const AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Destroys the specified Registration object.
     *
     * @param piReg Pointer to IRegistration to be destroyed
     * @param bByForce Flag to indicate whether it's aborted locally or not\n
     *                 If it sets to IMS_FALSE, the registration transaction will be preserved
     *                 if possible
     * @note MULTI_SUBS, MULTI_REG_SIP_PROFILE
     */
    void DestroyRegistration(IN IRegistration* piReg, IN IMS_BOOL bByForce = IMS_FALSE);

    /**
     * @brief Checks if "reg" event package subscription is supported or not.
     *
     * @param nSlotId Slot id
     * @param pProfile SIP profile to be checked
     * @note MULTI_SUBS, MULTI_REG_SIP_PROFILE
     */
    IMS_BOOL IsRegSubscriptionSupported(
            IN IMS_SINT32 nSlotId, IN SipProfile* pProfile = IMS_NULL) const;

    /**
     * @brief Gets the Registration object with the specified slot-id and flow id.
     *
     * @param nSlotId Slot id
     * @param nFlowId Registration flow id
     */
    IRegistration* GetRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const;

    /**
     * @brief Gets RegistrationManager's instance.
     */
    static RegistrationManager* GetInstance();

private:
    RegistrationManagerPrivate* m_pRegMngrPrivate;
};

#endif
