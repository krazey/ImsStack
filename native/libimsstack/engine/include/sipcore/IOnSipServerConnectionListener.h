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
#ifndef INTERFACE_ON_SIP_SERVER_CONNECTION_LISTENER_H_
#define INTERFACE_ON_SIP_SERVER_CONNECTION_LISTENER_H_

class SipConnectionNotifier;

/**
 * @brief This class defines a listener interface for an incoming SIP requests.
 *
 * @see SipConnectionNotifier
 */
class IOnSipServerConnectionListener
{
public:
    /**
     * This method will notify the listener that a new request is received. This method gives the
     * SipConnectionNotifier instance. The user has to call
     * the SipConnectionNotifier#AcceptAndOpen() to get the SipServerConnection object
     * that holds the server transaction and the request received.
     *
     * @param pScn The SipConnectionNotifier object carrying SipServerConnection
     */
    virtual void OnServerConnection_NotifyRequest(IN SipConnectionNotifier* pScn) = 0;

    /**
     * This method will notify the listener that a new request is received. This method gives the
     * SipConnectionNotifier instance. The user has to call
     * the SipConnectionNotifier#AcceptAndOpen() to get the SipServerConnection object
     * that holds the server transaction and the request received.
     *
     * @param pScn The SipConnectionNotifier object carrying SipServerConnection
     */
    virtual void OnServerConnection_NotifyForkedRequest(IN SipConnectionNotifier* pScn) = 0;
};

#endif
