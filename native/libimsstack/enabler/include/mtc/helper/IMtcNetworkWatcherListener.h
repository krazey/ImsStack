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

#ifndef INTERFACE_MTC_NETWORK_WATCHER_LISTENER_H_
#define INTERFACE_MTC_NETWORK_WATCHER_LISTENER_H_

#include "ImsTypeDef.h"

class IMtcNetworkWatcherListener
{
public:
    virtual ~IMtcNetworkWatcherListener() = default;

    /**
     * @brief Notifies the listener that the RAT has changed.
     *
     * @param eRatType The type of the new RAT.
     */
    virtual void OnRatChanged(IN IMS_SINT32 eRatType) = 0;
};

#endif
