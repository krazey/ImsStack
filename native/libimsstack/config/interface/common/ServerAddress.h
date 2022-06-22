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
#ifndef SERVER_ADDRESS_H_
#define SERVER_ADDRESS_H_

#include "AString.h"

class ServerAddress
{
public:
    inline ServerAddress(IN const AString& strAddress, IN IMS_SINT32 nPort = PORT_UNSPECIFIED) :
            m_strAddress(strAddress),
            m_nPort(nPort)
    {
    }
    inline ~ServerAddress() {}

    ServerAddress(IN const ServerAddress&) = delete;
    ServerAddress& operator=(IN const ServerAddress&) = delete;

public:
    /**
     * @brief Returns the address (FQDN or IP address) of the IMS server.
     *
     * @return The address of IMS server.
     */
    inline const AString& GetAddress() const { return m_strAddress; }

    /**
     * @brief Returns the port number of the IMS server.
     *
     * @return The port number.
     */
    inline IMS_SINT32 GetPort() const { return m_nPort; }

private:
    /**
     * @brief Sets the address (FQDN or IP address) of the IMS server.
     *
     * @param strAddress The address of IMS server (FQDN or IP-based string)
     */
    inline void SetAddress(IN const AString& strAddress) { m_strAddress = strAddress; }

    /**
     * @brief Sets the port number of the IMS server.
     *
     * @param nPort The port number
     */
    inline void SetPort(IN IMS_SINT32 nPort) { m_nPort = nPort; }

public:
    enum
    {
        PORT_UNSPECIFIED = (-1)
    };

private:
    friend class SubscriberConfig;

    AString m_strAddress;
    IMS_SINT32 m_nPort;
};

#endif
