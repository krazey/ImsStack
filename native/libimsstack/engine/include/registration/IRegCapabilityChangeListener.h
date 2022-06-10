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
#ifndef INTERFACE_REG_CAPABILITY_CHANGE_LISTENER_H_
#define INTERFACE_REG_CAPABILITY_CHANGE_LISTENER_H_

#include "AString.h"

class IRegCapabilityChangeListener
{
public:
    /**
     * @brief Notifies the application that the registration binding is updated by adding a service.
     *
     * @param strAppId An IMS application identifier
     * @param strServiceId An IMS service identifier
     */
    virtual void RegCapabilityChange_ServiceAdded(
            IN const AString& strAppId, IN const AString& strServiceId) = 0;

    /**
     * @brief Notifies the application that the registration binding is updated
     *        by removing a service.
     *
     * @param strAppId An IMS application identifier
     * @param strServiceId An IMS service identifier\
     */
    virtual void RegCapabilityChange_ServiceRemoved(
            IN const AString& strAppId, IN const AString& strServiceId) = 0;
};

#endif
