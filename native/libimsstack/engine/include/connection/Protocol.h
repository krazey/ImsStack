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
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "AString.h"

class IConnection;

class Protocol
{
public:
    Protocol() = default;
    virtual ~Protocol() = 0;

    Protocol(IN const Protocol&) = delete;
    Protocol& operator=(IN const Protocol&) = delete;

public:
    /**
     * @brief Creates and opens a Connection; SipConnection, Service, and so on.
     *
     * The creation of Connections is performed dynamically by looking up a protocol implementation
     * class whose name is formed from the platform name (read from a system property) and
     * the protocol name of the requested connection (extracted from the parameter string supplied
     * by the application programmer).
     *
     * The last error can be as follows:
     *   ILLEGAL_ARGUMENT,
     *   CONNECTION_NOT_FOUND
     *
     * @param strName The connection name with the scheme
     * @return An IConnection instance.
     */
    inline virtual IConnection* OpenPrim(IN const AString& /*strName*/) { return IMS_NULL; }

    /**
     * @brief Creates and opens a Connection; SipConnection, Service, and so on.
     *
     * The creation of Connections is performed dynamically by looking up a protocol implementation
     * class whose name is formed from the platform name (read from a system property) and
     * the protocol name of the requested connection (extracted from the parameter string supplied
     * by the application programmer).
     *
     * The last error can be as follows:
     *   ILLEGAL_ARGUMENT,
     *   CONNECTION_NOT_FOUND
     *
     * @param strScheme The connection scheme (i.e. "imscore", "sip")
     * @param strTarget The host information for this connection
     * @param strParams The additional parameters for this connection
     * @return An IConnection instance.
     */
    inline virtual IConnection* OpenPrim(IN const AString& /*strScheme*/,
            IN const AString& /*strTarget*/, IN const AString& /*strParams*/)
    {
        return IMS_NULL;
    }

    static void ParseName(IN const AString& strName, OUT AString& strScheme, OUT AString& strTarget,
            OUT AString& strParams);
};

#endif
