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
#ifndef INTERFACE_REGISTRATION_EX_H_
#define INTERFACE_REGISTRATION_EX_H_

#include "IRegistration.h"
#include "ISipConnectionNotifierErrorListener.h"

class RegInfo;
class RegObserver;
class RegStateTracker;

class IRegistrationEx : public IRegistration, public ISipConnectionNotifierErrorListener
{
protected:
    ~IRegistrationEx() override = default;

public:
    /**
     * @brief Adds the observer to get the registration state transition.
     *
     * @param pObserver The observer to monitor the registration
     */
    virtual void AddObserver(IN RegObserver* pObserver) = 0;

    /**
     * @brief Removes the observer to get the registration state transition.
     *
     * @param pObserver The observer that was previously added
     */
    virtual void RemoveObserver(IN RegObserver* pObserver) = 0;

    /**
     * @brief Adds the reference for SIP connection notifier's error listener.
     *
     * @return The current reference count after adding.
     */
    virtual IMS_SINT32 AddReferenceForScnErrorListener() = 0;

    /**
     * Removes the reference for SIP connection notifier's error listener.
     *
     * @return The current reference count after removing.
     */
    virtual IMS_SINT32 RemoveReferenceForScnErrorListener() = 0;

    /**
     * @brief Returns the reg info of this registration.
     *
     * @return The RegInfo ("reg" event package info.) instance.
     */
    virtual const RegInfo* GetRegInfo() const = 0;

    /**
     * @brief Returns the registration state tracker of this registration.
     *
     * @return The RegStateTracker instance.
     */
    virtual const RegStateTracker* GetStateTracker() const = 0;

    /**
     * @brief Notifies the registration when the caller capability is changed to refresh
     *        the IMS registration if the device is already registered to the IMS network.
     */
    virtual void NotifyCallerCapabilityChanged() = 0;

    /**
     * @brief Checks whether the restoration of active bindings is enabled or not.
     *
     * @return IMS_TRUE if the restoration of active bindings is enabled, IMS_FALSE otherwise.
     * @note REG_RESTORATION_FOR_ACTIVE_BINDING
     */
    virtual IMS_BOOL IsActiveBindingsRestorationEnabled() const = 0;

    /**
     * @brief Checks if this registration is for emergency or not.
     *
     * @return true if this registration is for emergency, false otherwise.
     */
    virtual IMS_BOOL IsEmergencyRegistration() const = 0;

public:
    /// Update states for registration binding
    enum
    {
        BINDING_REGISTERING,
        BINDING_DEREGISTERING,
        BINDING_RESULT_OK,
        BINDING_RESULT_NOK,
        BINDING_RESTORE,
        /// REG_RESTORATION_FOR_ACTIVE_BINDING
        BINDING_RESTORE_ACTIVE_BINDINGS,
        BINDING_DESTROY_CONTACT,
        BINDING_DESTROY
    };
};

#endif
