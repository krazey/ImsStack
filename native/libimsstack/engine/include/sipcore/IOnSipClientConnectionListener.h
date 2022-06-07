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
#ifndef INTERFACE_ON_SIP_CLIENT_CONNECTION_LISTENER_H_
#define INTERFACE_ON_SIP_CLIENT_CONNECTION_LISTENER_H_

class SipClientConnection;

/**
 * @brief The class defines a listener interface for an incoming SIP response.
 *
 * @see SipClientConnection
 */
class IOnSipClientConnectionListener
{
public:
    /**
     * This method gives the SipClientConnection instance, which has received a new SIP response.
     * The application implementing this listener interface has to call
     * SipClientConnection#Receive() to initialize the SipClientConnection object with
     * the new response.
     *
     * @param pScc The SipClientConnection object carrying the response
     */
    virtual void OnClientConnection_NotifyResponse(IN SipClientConnection* pScc) = 0;

    /**
     * This method gives the SipClientConnection instance, which has received a new SIP response.
     * The application implementing this listener interface has to call
     * SipClientConnection#Receive() to initialize the SipClientConnection object with
     * the new response.
     *
     * @param pScc The SipClientConnection object carrying the response
     * @param pForkedScc The SipClientConnection object carrying the forked response
     */
    virtual void OnClientConnection_NotifyForkedResponse(
            IN SipClientConnection* pScc, IN SipClientConnection* pForkedScc) = 0;
};

#endif
