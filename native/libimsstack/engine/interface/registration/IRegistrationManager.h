/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef INTERFACE_REGISTRATION_MANAGER_H_
#define INTERFACE_REGISTRATION_MANAGER_H_

#include "IRegistration.h"

/**
 * @brief An interface for providing the methods to create/destroy IMS registrations and
 *        its related configuration.
 */
class IRegistrationManager
{
protected:
    virtual ~IRegistrationManager() = default;

public:
    /**
     * @brief Creates IRegistration instance for IMS registration.
     *
     * @param nSlotId Slot id
     * @param nFlowId Registration flow id
     * @param strAor Address-Of-Record(public user identity to be registered)
     * @param bFake Flag to indicate whether it's used for fake or not
     * @param strSubsId Subscriber configuration's identifier as a string
     * @param pProfile SIP profile for this registration
     * @return IMS_TRUE if IRegistration is successfully created, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL CreateRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId,
            IN const AString& strAor, IN IMS_BOOL bFake = IMS_FALSE,
            IN const AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pProfile = IMS_NULL) = 0;

    /**
     * @brief Creates IRegistration instance for IMS registration.
     *
     * @param nSlotId Slot id
     * @param nFlowId Registration flow id
     * @param strAor Address-Of-Record(public user identity to be registered)
     * @param bFake Flag to indicate whether it's used for fake or not
     * @param strSubsId Subscriber configuration's identifier as a string
     * @param pProfile SIP profile for this registration
     * @return IMS_TRUE if IRegistration is successfully created, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL CreateRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId,
            IN const SipAddress& objAor, IN IMS_BOOL bFake = IMS_FALSE,
            IN const AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pProfile = IMS_NULL) = 0;

    /**
     * @brief Destroys the specified IRegistration instance.
     *
     * @param piReg Pointer to IRegistration to be destroyed
     * @param bByForce Flag to indicate whether it's aborted locally or not\n
     *                 If it sets to IMS_FALSE, the registration transaction will be preserved
     *                 if possible
     */
    virtual void DestroyRegistration(IN IRegistration* piReg, IN IMS_BOOL bByForce = IMS_FALSE) = 0;

    /**
     * @brief Checks if "reg" event package subscription is supported or not.
     *
     * @param nSlotId Slot id
     * @param pProfile SIP profile to be checked
     */
    virtual IMS_BOOL IsRegSubscriptionSupported(
            IN IMS_SINT32 nSlotId, IN SipProfile* pProfile = IMS_NULL) const = 0;

    /**
     * @brief Gets the Registration object with the specified slot-id and flow id.
     *
     * @param nSlotId Slot id
     * @param nFlowId Registration flow id
     */
    virtual IRegistration* GetRegistration(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) const = 0;
};

#endif
