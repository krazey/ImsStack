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
#ifndef INTERFACE_SYSTEM_CONFIG_LISTENER_H_
#define INTERFACE_SYSTEM_CONFIG_LISTENER_H_

#include "ImsTypeDef.h"

class ISystemConfigListener
{
public:
    /**
     * @brief Notifies the applications when system configuration is changed.
     *
     * @param nEvent An event to indicate which configuration is changed
     * @param nSlotId A slot id for the following events.\n
     *                #SystemConfig#EVENT_SYSTEM_CONFIGURATION_CHANGED\n
     *                #SystemConfig#EVENT_SUBSCRIPTION_CHANGED\n
     *                If nSlotId is IMS_SLOT_ANY, then the application considers that
     *                all (SIM slots) the configurations are changed.
     */
    virtual void SystemConfig_ConfigurationChanged(
            IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId = IMS_SLOT_ANY) = 0;
};

#endif
