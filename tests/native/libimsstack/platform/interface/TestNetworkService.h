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
    inline TestNetworkService() :
            NetworkService(),
            m_piConnection(&m_objConnection),
            m_piSocket(&m_objSocket),
            m_piSslSocket(&m_objSslSocket),
            m_piIpcan(&m_objIpcan),
            m_piIpSec(&m_objIpSec)
    {
    }

    inline INetworkConnection* CreateConnection(
            IN const AString& /*strProfileName*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piConnection;
    }
    inline INetworkConnection* CreateConnection(
            IN IMS_SINT32 /*nApnType*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piConnection;
    }
    inline void DestroyConnection(IN INetworkConnection*& /*piConnection*/) override {}
    inline INetworkConnection* FindConnection(
            IN IMS_SINT32 /*nApnType*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piConnection;
    }
    inline INetworkConnection* FindConnection(IN const IPAddress& /*objIpAddr*/) override
    {
        return m_piConnection;
    }

    inline ISocket* CreateSocket(IN INetworkConnection* /*piConnection*/) override
    {
        return m_piSocket;
    }
    inline ISocket* CreateSocket(
            IN const IMS_CHAR* /*pszProfileName*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piSocket;
    }

    inline ISocket* CreateSslSocket(
            IN INetworkConnection* /*piConnection*/, IN SslCertificate* /*pCertificate*/) override
    {
        return m_piSslSocket;
    }
    inline ISocket* CreateSslSocket(IN const IMS_CHAR* /*pszProfileName*/,
            IN SslCertificate* /*pCertificate*/, IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piSslSocket;
    }
    inline void DestroySocket(IN ISocket*& /*piSocket*/) override {}
    inline IMS_BOOL CheckIpAndPortAvailability(IN const IPAddress& /*objIpAddr*/,
            IN IMS_SINT32 /*nPort*/, IN ISocket::SOCKET_ENTYPE /*enType*/) override
    {
        return IMS_TRUE;
    }

    inline IIpcan* GetIpcan() override { return m_piIpcan; }
    inline INetworkIpSec* GetIpSec(IN IMS_SINT32 /*nSlotId*/) override { return m_piIpSec; }

    inline MockINetworkConnection& GetMockConnection() { return m_objConnection; }
    inline MockISocket& GetMockSocket() { return m_objSocket; }
    inline MockISocket& GetMockSslSocket() { return m_objSslSocket; }
    inline MockIIpcan& GetMockIpcan() { return m_objIpcan; }
    inline MockINetworkIpSec& GetMockIpSec() { return m_objIpSec; }
    inline void SetConnection(IN INetworkConnection* piConnection)
    {
        m_piConnection = piConnection;
    }
    inline void SetSocket(IN ISocket* piSocket) { m_piSocket = piSocket; }
    inline void SetSslSocket(IN ISocket* piSslSocket) { m_piSslSocket = piSslSocket; }
    inline void SetIpcan(IN IIpcan* piIpcan) { m_piIpcan = piIpcan; }
    inline void SetIpSec(IN INetworkIpSec* piIpSec) { m_piIpSec = piIpSec; }

private:
    MockINetworkConnection m_objConnection;
    MockISocket m_objSocket;
    MockISocket m_objSslSocket;
    MockIIpcan m_objIpcan;
    MockINetworkIpSec m_objIpSec;

    INetworkConnection* m_piConnection;
    ISocket* m_piSocket;
    ISocket* m_piSslSocket;
    IIpcan* m_piIpcan;
    INetworkIpSec* m_piIpSec;
};

#endif
