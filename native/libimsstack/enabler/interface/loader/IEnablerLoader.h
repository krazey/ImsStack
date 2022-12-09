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
#ifndef INTERFACE_ENABLER_LOADER_H_
#define INTERFACE_ENABLER_LOADER_H_

#include "ImsTypeDef.h"

class IEnablerLoader
{
protected:
    virtual ~IEnablerLoader() = default;

public:
    /**
     * @brief Starts the enablers for the specified slot-id.
     *
     * @param nSlotId The slot-id to be started
     */
    virtual void StartEnabler(IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Stops the enablers for the specified slot-id.
     *
     * @param nSlotId The slot-id to be stopped
     */
    virtual void StopEnabler(IN IMS_SINT32 nSlotId) = 0;
};

#endif
