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
#ifndef INTERFACE_ON_SESSION_EX_LISTENER_H_
#define INTERFACE_ON_SESSION_EX_LISTENER_H_

#include "SessionEx.h"

class VirtualSession;

class IOnSessionExListener
{
public:
    /**
     * @brief Notifies the application that the UPDATE was successfully delivered
     *        on the early state.
     *
     * @param pSessionEx The SessionEx object
     */
    virtual void OnSessionEx_EarlyMediaUpdated(IN SessionEx* pSessionEx) = 0;

    /**
     * @brief Notifies the application that the UPDATE was not successfully delivered
     *        on the early state.
     *
     * @param pSessionEx The SessionEx object
     */
    virtual void OnSessionEx_EarlyMediaUpdateFailed(IN SessionEx* pSessionEx) = 0;

    /**
     * @brief Notifies the application that the acknowledgement of the provisional response
     *        is received.
     *
     * @param pSessionEx The SessionEx object
     */
    virtual void OnSessionEx_EarlyMediaUpdateReceived(IN SessionEx* pSessionEx) = 0;

    /**
     * @brief Notifies the application that the acknowledgement of the provisional response
     *        was successfully delivered.
     *
     * @param pSessionEx The SessionEx object
     */
    virtual void OnSessionEx_PrackDelivered(IN SessionEx* pSessionEx) = 0;

    /**
     * @brief Notifies the application that the acknowledgement of the provisional response
     *        was not successfully delivered.
     *
     * @param pSessionEx The SessionEx object
     */
    virtual void OnSessionEx_PrackDeliveryFailed(IN SessionEx* pSessionEx) = 0;

    /**
     * @brief Notifies the application that the acknowledgement of the provisional response
     *        is received.
     *
     * @param pSessionEx The SessionEx object
     */
    virtual void OnSessionEx_PrackReceived(IN SessionEx* pSessionEx) = 0;

    /**
     * @brief Notifies the application that the RPR delivery is failed (no PRACK received).
     *
     * @param pSessionEx The SessionEx object
     */
    virtual void OnSessionEx_RprDeliveryFailed(IN SessionEx* pSessionEx) = 0;

    /**
     * @brief Notifies the application that the reliable provisional response is received.
     *
     * @param pSessionEx The SessionEx object
     * @param pVirtualSession The VirtualSession object
     * @param nIndex The index of the current response message\n
     *               #Session#INDEX_MOST_RECENT_MESSAGE
     */
    virtual void OnSessionEx_RprReceived(IN SessionEx* pSessionEx,
            IN VirtualSession* pVirtualSession,
            IN IMS_UINT32 nIndex = Session::INDEX_MOST_RECENT_MESSAGE) = 0;
};

#endif
