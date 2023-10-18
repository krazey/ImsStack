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
#ifndef INTERFACE_SIPCONTROLLER_SERVICE_H_
#define INTERFACE_SIPCONTROLLER_SERVICE_H_

#include "INativeEnabler.h"

class AString;

class ISipControllerService : public INativeEnabler
{
public:
    virtual ~ISipControllerService(){};

    /**
     * Called by the framework to request that the ImsService perform the network registration.
     *
     * @param nParam This is assigned Sip Delegate Feature tags
     */
    virtual void UpdateDelegateRegistration(IN IMS_UINTP nParam) = 0;

    /**
     * Called by the framework to request that the ImsService perform the network
     * deregistration.
     *
     */
    virtual void TriggerDelegateDeregistration(void) = 0;

    /**
     * Ready to send or receive the sip message
     *
     * @param threadName This name is the thread name to deliver when the message is received
     */
    virtual void OpenMessageTracker(IN const AString& threadName) = 0;

    /**
     * Called this method when a remote RCS application wishes to send a new outgoing
     * SIP message.
     *
     * @param nParam Sip Message
     */
    virtual void SendMessage(IN IMS_UINTP nParam) = 0;

    /**
     * The remote application has received the SIP message and is processing it.
     *
     * @param nParam viaTransactionId and error reason
     */
    virtual void NotifyMessageReceiveError(IN IMS_UINTP nParam) = 0;

    /**
     * The remote IMS application has closed a SIP session and the routing resources associated
     * with the SIP session using the provided Call-ID may now be cleaned up.
     * @param callId The call-ID header value associated with the ongoing SIP Session that the
     * framework is requesting be cleaned up.
     */
    virtual void CloseSession(IN const AString& strCallId) = 0;
};
#endif  // INTERFACE_SIPCONTROLLER_SERVICE_H_
