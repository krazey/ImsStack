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
#ifndef TEST_NETWORK_SERVICE_H_
#define TEST_NETWORK_SERVICE_H_

#include "MockIIpcan.h"
#include "MockINetworkConnection.h"
#include "MockINetworkIpSec.h"
#include "MockISocket.h"
#include "ServiceNetwork.h"

class TestNetworkService : public NetworkService
{
public:
    inline INetworkConnection* CreateConnection(
            IN const AString& /*strProfileName*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objConnection;
    }
    inline INetworkConnection* CreateConnection(
            IN IMS_SINT32 /*nApnType*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objConnection;
    }
    inline void DestroyConnection(IN INetworkConnection*& /*piConnection*/) override {}
    inline INetworkConnection* FindConnection(
            IN IMS_SINT32 /*nApnType*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objConnection;
    }
    inline INetworkConnection* FindConnection(IN const IPAddress& /*objIpAddr*/) override
    {
        return &m_objConnection;
    }

    inline ISocket* CreateSocket(IN INetworkConnection* /*piConnection*/) override
    {
        return &m_objSocket;
    }
    inline ISocket* CreateSocket(
            IN const IMS_CHAR* /*pszProfileName*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objSocket;
    }

    inline ISocket* CreateSslSocket(
            IN INetworkConnection* /*piConnection*/, IN SslCertificate* /*pCertificate*/) override
    {
        return &m_objSslSocket;
    }
    inline ISocket* CreateSslSocket(IN const IMS_CHAR* /*pszProfileName*/,
            IN SslCertificate* /*pCertificate*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objSslSocket;
    }
    inline void DestroySocket(IN ISocket*& /*piSocket*/) override {}
    inline IMS_BOOL CheckIpAndPortAvailability(IN const IPAddress& /*objIpAddr*/,
            IN IMS_SINT32 /*nPort*/, IN ISocket::SOCKET_ENTYPE /*enType*/) override
    {
        return IMS_TRUE;
    }

    inline IIpcan* GetIpcan() override { return &m_objIpcan; }
    inline INetworkIpSec* GetIpSec() override { return &m_objIpSec; }

private:
    MockINetworkConnection m_objConnection;
    MockISocket m_objSocket;
    MockISocket m_objSslSocket;
    MockIIpcan m_objIpcan;
    MockINetworkIpSec m_objIpSec;
};

#endif
