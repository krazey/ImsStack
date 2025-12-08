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

#ifndef INTERFACE_ECT_MANAGER_H_
#define INTERFACE_ECT_MANAGER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class AString;

/**
 * @brief Interface for managing Explicit Call Transfer (ECT) operations.
 *
 * This interface provides the entry point for initiating and querying the state of call transfers.
 */
class IEctManager
{
public:
    /** Defines the possible states of the ECT manager. */
    enum class State
    {
        IDLE,                      // The manager is not currently handling any transfer operation.
        BLIND_TRANSFERRING,        // The manager is currently processing a blind transfer.
        CONSULTATIVE_TRANSFERRING  // The manager is currently processing a consultative transfer.
    };

    virtual ~IEctManager() {}

    /**
     * @brief Initiates a call transfer.
     *
     * This can be a blind or consultative transfer. For a blind transfer, a target number
     * must be provided. For a consultative transfer, the target number should be empty,
     * and there must be another active call to transfer to.
     *
     * @param nCallKey The key of the call to be transferred.
     * @param strNumber The target number for a blind transfer. Should be empty for a
     *                  consultative transfer.
     * @return IMS_RESULT Returns IMS_SUCCESS if the transfer is initiated successfully,
     *                    otherwise an error code.
     */
    virtual IMS_RESULT Transfer(IN CallKey nCallKey, IN const AString& strNumber) = 0;

    /**
     * @brief Gets the current state of the ECT manager.
     *
     * @return State The current state of the manager.
     */
    virtual State GetState() = 0;
};

#endif
