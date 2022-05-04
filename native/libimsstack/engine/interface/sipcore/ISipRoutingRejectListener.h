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
#ifndef INTERFACE_SIP_ROUTING_REJECT_LISTENER_H_
#define INTERFACE_SIP_ROUTING_REJECT_LISTENER_H_

#include "SipStatusCode.h"

class ISipMessage;
class ISipServerConnection;

/**
 * @brief This class provides a listener interface for receiving notifications about
 *        the reject to the incoming SIP request.
 *
 * @see ISipRoutingRejectNotifier, ISipMessage, ISipServerConnection
 */
class ISipRoutingRejectListener
{
public:
    /**
     * @brief Notifies the application that the incoming SIP request will be rejected
     *        in the sipcore layer.
     *
     * At this moment, the application can overwrite the status code of the rejected request.
     *
     * @param piSipMsg SIP message to be rejected
     * @param objStatusCode Status code which will be used for request reject\n
     *                      Application can overwrite the status code to be rejected.
     * @return If the application is changed the status code, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL RoutingReject_NotifyRequest(
            IN ISipMessage* piSipMsg, IN_OUT SipStatusCode& objStatusCode) = 0;

    /**
     * @brief Notifies the application that the incoming SIP request will be rejected
     *        in the sipcore layer.
     *
     * At this moment, the application can overwrite the status code of the rejected request.
     *
     * @param piSsc SIP server connection to be rejected
     * @param objStatusCode Status code which will be used for request reject\n
     *                      Application can overwrite the status code to be rejected.
     * @return If the application is changed the status code, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL RoutingReject_NotifyRequest(
            IN ISipServerConnection* piSsc, IN_OUT SipStatusCode& objStatusCode) = 0;
};

#endif
