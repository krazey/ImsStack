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
#ifndef SERVICE_IMS_NETWORK_H_
#define SERVICE_IMS_NETWORK_H_

#include "INetworkConnection.h"
#include "ISocket.h"
#include "ImsMessage.h"

class INetworkIpSec;
class IIpcan;
class SSLCertificate;
class NetworkServicePrivate;

class NetworkService
{
private:
    NetworkService();
    ~NetworkService();

    NetworkService(IN const NetworkService& objRHS);
    NetworkService& operator=(IN const NetworkService& objRHS);

public:
    INetworkConnection* CreateConnection(IN const AString &strProfileName,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);
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
    INetworkConnection* FindConnection(IN const AString &strProfileName,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);

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
    INetworkConnection* FindConnection(IN const IPAddress &objIPAddress);

    ISocket* CreateSocket(IN INetworkConnection* piConnection);
    ISocket* CreateSocket(IN const IMS_CHAR *pszProfileName,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    ISocket* CreateSslSocket(IN INetworkConnection* piConnection,
            IN SSLCertificate *pCertificate);
    ISocket* CreateSslSocket(IN const IMS_CHAR *pszProfileName,
            IN SSLCertificate *pCertificate, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void DestroySocket(IN ISocket *&piSocket);

    // To check if the specified IP address & port number can be used to create a socket
    IMS_BOOL CheckIpAndPortAvailability(IN const IPAddress &objIP,
            IN IMS_SINT32 nPort, IN ISocket::SOCKET_ENTYPE enType);

    IIpcan* GetIpcan();
    INetworkIpSec* GetIpSec();

    void DispatchServiceMessage(IN ImsMessage &objMSG);

    static NetworkService* GetNetworkService();

    // Gets slot-id from the specified network connection
    static IMS_SINT32 GetSlotId(IN const IPAddress& objIPAddress);
    static IMS_SINT32 GetSlotId(IN INetworkConnection* piConnection);

private:
    NetworkServicePrivate *pPrivate;
};

#endif // SERVICE_IMS_NETWORK_H_
