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
#ifndef INTERFACE_SIP_SERVER_CONNECTION_LISTENER_H_
#define INTERFACE_SIP_SERVER_CONNECTION_LISTENER_H_

class ISipConnectionNotifier;

/**
 * @brief This class provides a listener interface for an incoming SIP requests.
 *
 * @see ISipConnectionNotifier
 */
class ISipServerConnectionListener
{
public:
    /**
     * @brief This method will notify the listener that a new request is received.
     *
     * This method gives the ISipConnectionNotifier instance.\n
     * The user has to call the ISipConnectionNotifier::AcceptAndOpen() to get
     * the ISipServerConnection object that holds the server transaction and the request received.
     *
     * @param piScn Pointer to ISipConnectionNotifier object carrying ISipServerConnection
     * @param bIsForked Flag to indicate that an incoming request is forked or not
     */
    virtual void ServerConnection_NotifyRequest(IN ISipConnectionNotifier* piScn,
            IN IMS_BOOL bIsForked = IMS_FALSE) = 0;
};

#endif
