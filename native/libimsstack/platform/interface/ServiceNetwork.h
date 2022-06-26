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
#ifndef SERVICE_NETWORK_H_
#define SERVICE_NETWORK_H_

#include "ImsMessage.h"
#include "INetworkConnection.h"
#include "ISocket.h"

class IIpcan;
class INetworkIpSec;
class NetworkServicePrivate;
class SslCertificate;

class NetworkService
{
private:
    NetworkService();
    ~NetworkService();

public:
    NetworkService(IN const NetworkService&) = delete;
    NetworkService& operator=(IN const NetworkService&) = delete;

public:
    INetworkConnection* CreateConnection(IN const AString& strProfileName, IN IMS_SINT32 nSlotId);
    /**
     * @brief Creates a data connection with the specified APN type.
     *
     * @param nApnType The APN type\n
     *                 #NetworkPolicy#APN_IMS\n
     *                 #NetworkPolicy#APN_EMERGENCY\n
     *                 #NetworkPolicy#APN_INTERNET\n
     *                 #NetworkPolicy#APN_WIFI
     * @param nSlotId The slot id
     * @return An instance of data connection.
     */
    INetworkConnection* CreateConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    void DestroyConnection(IN INetworkConnection*& piConnection);
    inline INetworkConnection* FindConnection(IN const AString& strProfileName)
            __IMS_DEPRECATED__("Use FindConnection(IMS_SINT32,IMS_SINT32) instead")
    {
        return FindConnection(strProfileName, IMS_SLOT_0);
    }
    INetworkConnection* FindConnection(IN const AString& strProfileName, IN IMS_SINT32 nSlotId)
            __IMS_DEPRECATED__("Use FindConnection(IMS_SINT32,IMS_SINT32) instead");

    /**
     * @brief Finds a data connection with the specified APN type.
     *
     * @param nApnType The APN type\n
     *                 #NetworkPolicy#APN_IMS\n
     *                 #NetworkPolicy#APN_EMERGENCY\n
     *                 #NetworkPolicy#APN_INTERNET\n
     *                 #NetworkPolicy#APN_WIFI
     * @param nSlotId The slot id
     * @return An instance of data connection.
     */
    INetworkConnection* FindConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    INetworkConnection* FindConnection(IN const IPAddress& objIpAddr);

    ISocket* CreateSocket(IN INetworkConnection* piConnection);
    ISocket* CreateSocket(IN const IMS_CHAR* pszProfileName, IN IMS_SINT32 nSlotId);

    ISocket* CreateSslSocket(IN INetworkConnection* piConnection, IN SslCertificate* pCertificate);
    ISocket* CreateSslSocket(IN const IMS_CHAR* pszProfileName, IN SslCertificate* pCertificate,
            IN IMS_SINT32 nSlotId);
    void DestroySocket(IN ISocket*& piSocket);

    // To check if the specified IP address & port number can be used to create a socket
    IMS_BOOL CheckIpAndPortAvailability(
            IN const IPAddress& objIpAddr, IN IMS_SINT32 nPort, IN ISocket::SOCKET_ENTYPE enType);

    IIpcan* GetIpcan();
    INetworkIpSec* GetIpSec();

    void DispatchServiceMessage(IN ImsMessage& objMsg);

    static NetworkService* GetNetworkService();

    // Gets slot-id from the specified network connection
    static IMS_SINT32 GetSlotId(IN const IPAddress& objIpAddr);
    static IMS_SINT32 GetSlotId(IN INetworkConnection* piConnection);

private:
    NetworkServicePrivate* m_pPrivate;
};

#endif
