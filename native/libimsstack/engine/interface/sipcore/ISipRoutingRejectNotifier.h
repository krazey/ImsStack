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
#ifndef INTERFACE_SIP_ROUTING_REJECT_NOTIFIER_H_
#define INTERFACE_SIP_ROUTING_REJECT_NOTIFIER_H_

#include "ImsTypeDef.h"

class ISipRoutingRejectListener;

/**
 * @brief This class provides an interface to monitor SIP reject scenario
 *        of an incoming SIP request by sipcore engine.
 *
 * @see ISipRoutingRejectListener
 */
class ISipRoutingRejectNotifier
{
protected:
    virtual ~ISipRoutingRejectNotifier() = default;

public:
    /**
     * @brief Adds a listener to monitor the incoming SIP request to be just rejected by engine.
     *
     * @param piListener Listener to be added
     */
    virtual void AddListener(IN ISipRoutingRejectListener* piListener) = 0;

    /**
     * @brief Removes a listener to monitor the incoming SIP request to be just rejected by engine.
     *
     * @param piListener Listener to be removed
     */
    virtual void RemoveListener(IN ISipRoutingRejectListener* piListener) = 0;
};

#endif
