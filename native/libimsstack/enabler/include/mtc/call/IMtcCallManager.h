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

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

enum class ServiceType;

/**
 * Manages the lifecycle of {@code IMtcCall} objects, providing methods to create, retrieve, and
 * remove them.
 */
class IMtcCallManager
{
public:
    using State = IMtcCall::State;

    virtual ~IMtcCallManager(){};

    /**
     * @brief Creates a new call and starts to manage it.
     *
     * @param eServiceType The service type for the call.
     * @param objCallInfo The initial information for the call.
     * @param strLogTag A log tag for the call.
     * @return A pointer to the created {@code IMtcCall} object. If the service for the given type
     *         is not active, a null call object is returned.
     */
    virtual IMtcCall* CreateCall(
            IN ServiceType eServiceType, IN CallInfo& objCallInfo, IN const AString& strLogTag) = 0;

    /**
     * @brief Removes the call matching the given call key. Does nothing if the call doesn't exist.
     *
     * The user must guarantee that the target call is in TERMINATING state when it's removed.
     *
     * @param nCallKey Key of the call to be removed.
     */
    virtual void RemoveCall(IN CallKey nCallKey) = 0;

    /**
     * @brief Returns a call matching the given call key.
     *
     * @param nCallKey The key of the call to retrieve.
     * @return A pointer to the {@code IMtcCall} object, or a null call object if not found.
     */
    virtual IMtcCall* GetCallByCallKey(IN CallKey nCallKey) = 0;

    /**
     * @brief Returns a list of all calls.
     *
     * The list is sorted in the order in which the calls were created.
     *
     * @return A list of pointers to all managed {@code IMtcCall} objects.
     */
    virtual ImsList<IMtcCall*> GetCalls() = 0;

    /**
     * @brief Returns a list of all calls excluding a given one.
     *
     * The list is sorted in the order in which the calls were created.
     *
     * @param nExcludingCallKey The key of the call to exclude from the list.
     * @return A list of {@code IMtcCall} pointers, excluding the specified call.
     */
    virtual ImsList<IMtcCall*> GetCallsExcluding(IN CallKey nExcludingCallKey) = 0;

    /**
     * @brief Returns a list of calls matching the given session type.
     *
     * The list is sorted in the order in which the calls were created.
     *
     * @param eCallType The type of calls to retrieve.
     * @return A list of {@code IMtcCall} pointers that match the given call type.
     */
    virtual ImsList<IMtcCall*> GetCallsByType(IN CallType eCallType) = 0;

    /**
     * @brief Returns a list of calls matching the given service type.
     *
     * The list is sorted in the order in which the calls were created.
     *
     * @param eServiceType The service type of calls to retrieve.
     * @return A list of {@code IMtcCall} pointers that match the given service type.
     */
    virtual ImsList<IMtcCall*> GetCallsByServiceType(IN ServiceType eServiceType) = 0;

    /**
     * @brief Returns a list of conference calls.
     *
     * The list is sorted in the order in which the calls were created.
     *
     * @return A list of {@code IMtcCall} pointers for calls that are part of a conference.
     */
    virtual ImsList<IMtcCall*> GetCallsInConference() = 0;

    /**
     * @brief Returns a list of calls that are in the specific state.
     *
     * The list is sorted in the order in which the calls were created.
     *
     * @param eState The state of the calls to retrieve.
     * @return A list of {@code IMtcCall} pointers that are in the specified state.
     */
    virtual ImsList<IMtcCall*> GetCallsByState(IN State eState) = 0;

    /**
     * @brief Gets the next available call index.
     *        The index cycles from 1 to a predefined maximum value.
     *
     * @return A next call index.
     */
    virtual IMS_UINT32 GetNextCallIndex() = 0;
};

#endif
