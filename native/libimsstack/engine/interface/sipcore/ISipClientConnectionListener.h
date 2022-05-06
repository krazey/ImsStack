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
#ifndef INTERFACE_SIP_CLIENT_CONNECTION_LISTENER_H_
#define INTERFACE_SIP_CLIENT_CONNECTION_LISTENER_H_

class ISipClientConnection;

/**
 * @brief This class provides an interface for an incoming SIP responses.
 *
 * @see ISipClientConnection
 */
class ISipClientConnectionListener
{
public:
    /**
     * @brief This method gives the ISipClientConnection instance, which has received
     *        a new SIP response.
     *
     * The application implementing this listener interface has to call
     * ISipClientConnection::Receive() to initialize the ISipClientConnection object
     * with the new response.
     *
     * @param piScc Pointer to ISipClientConnection object carrying the response
     * @param piForkedSCC Pointer to ISipClientConnection object carrying the forked response
     */
    virtual void ClientConnection_NotifyResponse(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc = IMS_NULL) = 0;
};

#endif
