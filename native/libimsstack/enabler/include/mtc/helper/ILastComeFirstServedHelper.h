/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef INTERFACE_LAST_COME_FIRST_SERVED_HELPER_H_
#define INTERFACE_LAST_COME_FIRST_SERVED_HELPER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class ILastComeFirstServedHelper
{
public:
    virtual ~ILastComeFirstServedHelper() = default;

    /**
     * @brief Notifies that the UE receives an incoming call to initiate a timer for pre-alerting
     *        phase. If there is another incoming call that already exists and the timer has
     *        expired, it is rejected and the last incoming call can proceed.
     *
     * @param nIncomingCallKey The key of concerned call.
     */
    virtual void OnCallReceived(IN CallKey nIncomingCallKey) = 0;
};

#endif
