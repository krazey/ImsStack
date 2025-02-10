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

#ifndef INTERFACE_MTC_CALL_MANAGER_H_
#define INTERFACE_MTC_CALL_MANAGER_H_

#include "IMtcService.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"

// Holds `IMtcCall` objects and provides methods to create, delete and find them.
class IMtcCallManager
{
public:
    using State = IMtcCall::State;

    virtual ~IMtcCallManager(){};

    // Creates a new call and starts to manage it. Returns the created call.

    /**
     * @brief Creates
     *
     * @param eServiceType
     * @param objCallInfo
     * @return
     */
    virtual IMtcCall* CreateCall(IN ServiceType eServiceType, IN CallInfo& objCallInfo) = 0;

    /**
     * @brief Removes the call matching the given call key. Does nothing if the call doesn't exist.
     * Mtc must guarantee that the target MtcCall is in TERMINATING state when it's removed.
     *
     * @param nCallKey Key of the MtcCall to be removed.
     */
    virtual void RemoveCall(IN CallKey nCallKey) = 0;

    // Returns a call matching the given call key.
    // Returns new `UnknownCall` instance if the call doesn't exist.

    /**
     * @brief Gets
     *
     * @param nCallKey
     * @return
     */
    virtual IMtcCall* GetCallByCallKey(IN CallKey nCallKey) = 0;

    // Returns a list of all calls.
    // The list is sorted in the order in which the calls were created.

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ImsList<IMtcCall*> GetCalls() = 0;

    // Returns a list of all calls excluding a given one.
    // The list is sorted in the order in which the calls were created.

    /**
     * @brief Gets
     *
     * @param nExcludingCallKey
     * @return
     */
    virtual ImsList<IMtcCall*> GetCallsExcluding(IN CallKey nExcludingCallKey) = 0;

    // Returns a list of calls matching the given session type.
    // The list is sorted in the order in which the calls were created.

    /**
     * @brief Gets
     *
     * @param eCallType
     * @return
     */
    virtual ImsList<IMtcCall*> GetCallsByType(IN CallType eCallType) = 0;

    // Returns a list of calls matching the given service type.
    // The list is sorted in the order in which the calls were created.

    /**
     * @brief Gets
     *
     * @param eServiceType
     * @return
     */
    virtual ImsList<IMtcCall*> GetCallsByServiceType(IN ServiceType eServiceType) = 0;

    // Returns a list of conference calls.
    // The list is sorted in the order in which the calls were created.

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ImsList<IMtcCall*> GetCallsInConference() = 0;

    // Returns a list of calls that in the specific state.
    // The list is sorted in the order in which the calls were created.

    /**
     * @brief Gets
     *
     * @param eState
     * @return
     */
    virtual ImsList<IMtcCall*> GetCallsByState(IN State eState) = 0;
};

#endif
