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
#ifndef INTERFACE_CAPABILITIES_LISTENER_H_
#define INTERFACE_CAPABILITIES_LISTENER_H_

#include "ImsTypeDef.h"

class ICapabilities;

/**
 * @brief This class provides a listener interface to notify the application about responses
 *        to capability queries.
 *
 * @see ICapabilities
 */
class ICapabilitiesListener
{
protected:
    virtual ~ICapabilitiesListener() = default;

public:
    /**
     * @brief Notifies the application that the capability query response
     *        from the remote endpoint was successfully received.
     *
     * @param piCapabilities Pointer to ICapabilities to be notified
     */
    virtual void CapabilityQueryDelivered(IN ICapabilities* piCapabilities) = 0;

    /**
     * @brief Notifies the application that the capability query response
     *        from the remote endpoint was not successfully received.
     *
     * @param piCapabilities Pointer to ICapabilities to be notified
     */
    virtual void CapabilityQueryDeliveryFailed(IN ICapabilities* piCapabilities) = 0;
};

#endif
