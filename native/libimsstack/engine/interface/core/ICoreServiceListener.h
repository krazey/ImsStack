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
#ifndef INTERFACE_CORE_SERVICE_LISTENER_H_
#define INTERFACE_CORE_SERVICE_LISTENER_H_

#include "ImsTypeDef.h"

class ICapabilities;
class ICoreService;
class IMessage;
class IPageMessage;
class IReasonInfo;
class IReference;
class ISession;

/**
 * @brief This class provides a listener interface to receive notifications on remotely
 *        initiated core service methods.
 *
 * @see ICoreService
 */
class ICoreServiceListener
{
public:
    /**
     * @brief Notifies the application when a page message is received from a remote endpoint.
     *
     * @param piService Pointer to the concerned IService
     * @param piMessage Pointer to the received IPageMessage
     */
    virtual void CoreService_PageMessageReceived(
            IN ICoreService* piService, IN IPageMessage* piMessage) = 0;

    /**
     * @brief Notifies the application when a reference request is received from a remote endpoint.
     *
     * Only references that are created outside of a session are notified in this method.
     *
     * @param piService Pointer to the concerned IService
     * @param piReference Pointer to the received IReference
     */
    virtual void CoreService_ReferenceReceived(
            IN ICoreService* piService, IN IReference* piReference) = 0;

    /**
     * @brief Notifies the application when a ICoreService is closed.
     *
     * @param piService Pointer to the concerned IService
     * @param piReasonInfo Pointer to IReasonInfo
     */
    virtual void CoreService_ServiceClosed(
            IN ICoreService* piService, IN IReasonInfo* piReasonInfo) = 0;

    /**
     * @brief Notifies the application when a session invitation is received
     *        from a remote endpoint.
     *
     * @param piService Pointer to the concerned IService
     * @param piSession Pointer to the received session invitation
     */
    virtual void CoreService_SessionInvitationReceived(
            IN ICoreService* piService, IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application when an unsolicited notify is received.
     *
     * @param piService Pointer to the concerned IService
     * @param piNotify Pointer to the received NOTIFY message
     */
    virtual void CoreService_UnsolicitedNotifyReceived(
            IN ICoreService* piService, IN IMessage* piNotify) = 0;

    /**
     * @brief Notifies the application when a capability query is received from a remote endpoint.
     *
     * @param piService Pointer to the concerned IService
     * @param piCapabilities Pointer to the received capability query
     */
    virtual void CoreService_CapabilityQueryReceived(
            IN ICoreService* piService, IN ICapabilities* piCapabilities) = 0;
};

#endif
