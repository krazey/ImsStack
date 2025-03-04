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

#ifndef INTERFACE_MTC_AOS_STATE_LISTENER_H_
#define INTERFACE_MTC_AOS_STATE_LISTENER_H_

#include "ImsTypeDef.h"

class IMtcService;

enum class MtcAosState
{
    CONNECTED,
    SUSPENDED,
    DISCONNECTING,
    DISCONNECTED,
};

class IMtcAosStateListener
{
public:
    virtual ~IMtcAosStateListener() {}

    /**
     * @brief Notifies
     *
     * @param objMtcService
     * @param eState
     * @param eAosReason
     */
    virtual void OnAosStateChanged(
            IN IMtcService& objMtcService, IN MtcAosState eState, IN IMS_UINT32 eAosReason) = 0;
};

#endif
