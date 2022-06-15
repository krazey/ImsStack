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
#ifndef INTERFACE_ON_CORE_SERVICE_LISTENER_H_
#define INTERFACE_ON_CORE_SERVICE_LISTENER_H_

#include "ImsTypeDef.h"

class Capabilities;
class CoreService;
class Message;
class PageMessage;
class ReasonInfo;
class Reference;
class SessionEx;

/**
 * @brief A listener type for receiving notifications on remotely initiated core service methods.
 *
 * @see CoreService
 */
class IOnCoreServiceListener
{
public:
    /**
     * @brief Notifies the application when a page message is received from a remote endpoint.
     *
     * @param pService The concerned Service object
     * @param pMessage The received PageMessage object
     */
    virtual void OnCoreService_PageMessageReceived(
            IN CoreService* pService, IN PageMessage* pMessage) = 0;

    /**
     * @brief Notifies the application when a reference request is received from a remote endpoint.
     *
     * Only references that are created outside of a session are notified in this method.
     *
     * @param pService The concerned Service object
     * @param pReference The received Reference object
     */
    virtual void OnCoreService_ReferenceReceived(
            IN CoreService* pService, IN Reference* pReference) = 0;

    /**
     * @brief Notifies the application when a CoreService is closed.
     *
     * @param pService The concerned Service object
     */
    virtual void OnCoreService_ServiceClosed(
            IN CoreService* pService, IN ReasonInfo* pReasonInfo) = 0;

    /**
     * @brief Notifies the application when a session invitation is received from a remote endpoint.
     *
     * @param pService The concerned Service object
     * @param pSession The received Session object
     */
    virtual void OnCoreService_SessionInvitationReceived(
            IN CoreService* pService, IN SessionEx* pSession) = 0;

    /**
     * @brief Notifies the application when an unsolicited notify is received.
     *
     * @param pService The concerned Service object
     * @param pNotify The received NOTIFY message object
     */
    virtual void OnCoreService_UnsolicitedNotifyReceived(
            IN CoreService* pService, IN Message* pNotify) = 0;

    /**
     * @brief Notifies the application when a capability query is received from a remote endpoint.
     *
     * @param pService The concerned Service object
     * @param pCapabilities The received Capabilities object
     */
    virtual void OnCoreService_CapabilityQueryReceived(
            IN CoreService* pService, IN Capabilities* pCapabilities) = 0;
};

#endif
