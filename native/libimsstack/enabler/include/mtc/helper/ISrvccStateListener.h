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

#ifndef INTERFACE_SRVCC_STATE_LISTENER_H_
#define INTERFACE_SRVCC_STATE_LISTENER_H_

#include "ImsTypeDef.h"

/**
 * @brief Defines the possible states of SRVCC.
 */
enum class SrvccState
{
    /// SRVCC is not active.
    IDLE = -1,
    /// SRVCC handover has started.
    STARTED,
    /// SRVCC handover completed successfully.
    SUCCEEDED,
    /// SRVCC handover failed.
    FAILED,
    /// SRVCC handover was canceled.
    CANCELED
};

/**
 * @brief Interface for listening to SRVCC state change events.
 *
 * Classes that need to monitor the progress of SRVCC (e.g., to handle media handover or call
 * termination) should implement this interface.
 */
class ISrvccStateListener
{
public:
    virtual ~ISrvccStateListener() {}  // for Unit Test

    /**
     * @brief Notifies that the SRVCC state has been updated.
     *
     * @param eState The new {@link SrvccState}.
     */
    virtual void OnSrvccStateUpdated(IN SrvccState eState) = 0;
};

#endif
