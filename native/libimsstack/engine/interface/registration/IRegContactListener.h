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
#ifndef INTERFACE_REG_CONTACT_LISTENER_H_
#define INTERFACE_REG_CONTACT_LISTENER_H_

#include "AString.h"

/**
 * @brief This class provides an interface to monitor the contact binding of IMS registration.
 */
class IRegContactListener
{
public:
    /**
     * @brief Notifies the application that the specified service is added to this contact.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     */
    virtual void RegContact_BindingAdded(
            IN const AString& strAppId, IN const AString& strServiceId) = 0;

    /**
     * @brief Notifies the application that the specified service is removed from this contact.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     */
    virtual void RegContact_BindingRemoved(
            IN const AString& strAppId, IN const AString& strServiceId) = 0;

    /**
     * @brief Notifies the application that the contact is in ACTIVE or just transit to ACTIVE
     *        state.
     */
    virtual void RegContact_OnActive() = 0;

    /**
     * @brief Notifies the application that the contact transits to ACTIVE state.
     */
    virtual void RegContact_OnTerminated() = 0;
};

#endif
