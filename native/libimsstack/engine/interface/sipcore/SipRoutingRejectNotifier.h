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
#ifndef SIP_ROUTING_REJECT_NOTIFIER_H_
#define SIP_ROUTING_REJECT_NOTIFIER_H_

#include "ImsList.h"
#include "ISipRoutingRejectNotifier.h"
#include "SipStatusCode.h"

class ISipMessage;
class ISipServerConnection;

class SipRoutingRejectNotifier : public ISipRoutingRejectNotifier
{
public:
    inline SipRoutingRejectNotifier() {}
    inline virtual ~SipRoutingRejectNotifier() {}

    SipRoutingRejectNotifier(IN const SipRoutingRejectNotifier&) = delete;
    SipRoutingRejectNotifier& operator=(IN const SipRoutingRejectNotifier&) = delete;

public:
    /**
     * Checks if the SIP routing reject notification is required or not.
     *
     * @return IMS_TRUE if the notification is required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsNotificationRequired() const { return !m_objListeners.IsEmpty(); }

    /**
     * Notifies the applications that the incoming SIP request will be rejected by the engine.
     *
     * @param piSipMsg SIP message to be rejected
     * @param objStatusCode Status code which will be used for request reject
     */
    void NotifyRequestReject(IN ISipMessage* piSipMsg, IN_OUT SipStatusCode& objStatusCode);

    /**
     * Notifies the applications that the incoming SIP request will be rejected by the engine.
     *
     * @param piSsc SIP server connection to be rejected
     * @param objStatusCode Status code which will be used for request reject
     */
    void NotifyRequestReject(IN ISipServerConnection* piSsc, IN_OUT SipStatusCode& objStatusCode);

private:
    // ISipRoutingRejectNotifier class
    void AddListener(IN ISipRoutingRejectListener* piListener) override;
    void RemoveListener(IN ISipRoutingRejectListener* piListener) override;

private:
    ImsList<ISipRoutingRejectListener*> m_objListeners;
};

#endif
