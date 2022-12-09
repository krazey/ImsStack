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
#ifndef INTERFACE_SIP_CONNECTION_FACTORY_LISTENER_H_
#define INTERFACE_SIP_CONNECTION_FACTORY_LISTENER_H_

#include "ImsTypeDef.h"

class ISipConnectionFactory;
class ISipServerConnection;

/**
 * @brief This class provides a listener interface to monitor an incoming SIP transaction.
 *
 * @see ISipServerConnection
 */
class ISipConnectionFactoryListener
{
protected:
    virtual ~ISipConnectionFactoryListener() = default;

public:
    /**
     * @brief Notifies the application that a new incoming request inside of SIP dialog
     *        is received.
     *
     * @param piScFactory Pointer to ISipConnectionFactory
     * @param piSsc Pointer to ISipServerConnection; for incoming SIP request
     */
    virtual void ConnectionFactory_NotifyRequest(
            IN ISipConnectionFactory* piScFactory, IN ISipServerConnection* piSsc) = 0;
};

#endif
