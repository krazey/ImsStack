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
#ifndef INTERFACE_ON_CAPABILITIES_LISTENER_H_
#define INTERFACE_ON_CAPABILITIES_LISTENER_H_

#include "ImsTypeDef.h"

class Capabilities;

/**
 * @brief This listener type is used to notify the application about responses
 *        to capability queries.
 */
class IOnCapabilitiesListener
{
protected:
    virtual ~IOnCapabilitiesListener() = default;

public:
    /**
     * @brief Notifies the application that the capability query response from the remote endpoint
     *        was successfully received.
     *
     * @param pCapabilities The concerned Capabilities object
     */
    virtual void OnCapabilities_QueryDelivered(IN Capabilities* pCapabilities) = 0;

    /**
     * @brief Notifies the application that the capability query response from the remote endpoint
     *        was not successfully received.
     *
     * @param pCapabilities The concerned Capabilities object
     */
    virtual void OnCapabilities_QueryDeliveryFailed(IN Capabilities* pCapabilities) = 0;
};

#endif
