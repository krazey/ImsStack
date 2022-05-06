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
#ifndef SIP_SOCKET_ADDRESS_H_
#define SIP_SOCKET_ADDRESS_H_

#include "SocketAddress.h"

class SIPSocketAddress
{
public:
    SIPSocketAddress();
    SIPSocketAddress(IN const SIPSocketAddress& other);
    SIPSocketAddress(IN IMS_SINT32 nType, IN IMS_SINT32 nPort, IN const AString& strAddress);
    SIPSocketAddress(IN IMS_SINT32 nType, IN IMS_SINT32 nPort, IN const AString& strAddress,
            IN IMS_BOOL bSecure);
    ~SIPSocketAddress();

public:
    SIPSocketAddress& operator=(IN const SIPSocketAddress& other);

public:
    IMS_BOOL Equals(IN const SIPSocketAddress& other) const;

    inline const IPAddress& GetIPAddress() const { return m_objSockAddr.GetAddress(); }
    inline IMS_SINT32 GetPort() const { return m_objSockAddr.GetPort(); }
    inline IMS_BOOL GetSecure() const { return m_bSecure; }
    inline const SocketAddress& GetSocketAddress() const { return m_objSockAddr; }
    inline IMS_SINT32 GetType() const { return m_nType; }
    inline void SetIPAddress(IN const IPAddress& objIPAddress)
    {
        m_objSockAddr.SetAddress(objIPAddress);
    }
    inline void SetPort(IN IMS_UINT32 nPort) { m_objSockAddr.SetPort(nPort); }
    inline void SetSecure(IN IMS_BOOL bSecure) { m_bSecure = bSecure; }
    inline void SetSocketAddress(IN const SocketAddress& objSockAddr)
    {
        m_objSockAddr = objSockAddr;
    }
    inline void SetType(IN IMS_SINT32 nType) { m_nType = nType; }

public:
    // Types of SIPSocket
    enum
    {
        SOCKET_NONE = 0,

        SOCKET_UDP,
        SOCKET_TCP,
        SOCKET_TCP_CLIENT,
        SOCKET_TCP_CLIENT_BY_PEER,

        SOCKET_MAX
    };

private:
    // Flag for secure socket
    IMS_BOOL m_bSecure;
    // SOCKET_XXX : Type of SIPSocket
    IMS_SINT32 m_nType;
    SocketAddress m_objSockAddr;
};

#endif
