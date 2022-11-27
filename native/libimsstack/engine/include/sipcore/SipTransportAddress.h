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
#ifndef SIP_TRANSPORT_ADDRESS_H_
#define SIP_TRANSPORT_ADDRESS_H_

#include "IpAddress.h"

class SipTransportAddress
{
public:
    SipTransportAddress();
    SipTransportAddress(IN const SipTransportAddress& other);
    SipTransportAddress(IN IMS_SINT32 nProtocol, IN IMS_SINT32 nPort, IN const AString& strAddress);
    ~SipTransportAddress();

public:
    SipTransportAddress& operator=(IN const SipTransportAddress& other);

public:
    IMS_BOOL Equals(IN const SipTransportAddress& other) const;

    inline const IPAddress& GetIpAddress() const { return m_objIpAddr; }
    inline IMS_SINT32 GetPort() const { return m_nPort; }
    inline IMS_SINT32 GetProtocol() const { return m_nProtocol; }
    inline void SetIpAddress(IN const IPAddress& objIpAddr) { m_objIpAddr = objIpAddr; }
    inline void SetPort(IN IMS_UINT32 nPort) { m_nPort = nPort; }
    inline void SetProtocol(IN IMS_SINT32 nProtocol) { m_nProtocol = nProtocol; }

public:
    /// Types of the transport supported by SIP stack
    enum
    {
        PROTOCOL_ANY = 0,

        PROTOCOL_UDP,
        PROTOCOL_TCP,
        PROTOCOL_TLS,

        PROTOCOL_MAX
    };

private:
    // TRANSPORT_UDP, ... in SIP class or TYPE_UDP, ... in SipSocket class
    IMS_SINT32 m_nProtocol;
    // Port; If -1, the default port number will be selected according to the transport protocol
    IMS_SINT32 m_nPort;
    // IP address (null-terminating dotted numeric IP address)
    IPAddress m_objIpAddr;
};

#endif
