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

#include "INetConnection.h"
#include "INetSocket.h"
#include "IMSMSG.h"

class INetIPSec;
class IIPCAN;
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
    INetConnection* CreateConnection(IN const AString &strProfileName,
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
    INetConnection* CreateConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    void DestroyConnection(IN INetConnection *&piConnection);
    INetConnection* FindConnection(IN const AString &strProfileName,
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
    INetConnection* FindConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);
    INetConnection* FindConnection(IN const IPAddress &objIPAddress);

    INetSocket* CreateSocket(IN INetConnection *piConnection);
    INetSocket* CreateSocket(IN const IMS_CHAR *pszProfileName,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    INetSocket* CreateSSLSocket(IN INetConnection *piConnection,
            IN SSLCertificate *pCertificate);
    INetSocket* CreateSSLSocket(IN const IMS_CHAR *pszProfileName,
            IN SSLCertificate *pCertificate, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void DestroySocket(IN INetSocket *&piSocket);

    // To check if the specified IP address & port number can be used to create a socket
    IMS_BOOL CheckIPAndPortAvailability(IN const IPAddress &objIP,
            IN IMS_SINT32 nPort, IN INetSocket::SOCKET_ENTYPE enType);

    IIPCAN* GetIPCAN();
    INetIPSec* GetIPSec();

    void DispatchServiceMessage(IN IMSMSG &objMSG);

    static NetworkService* GetNetworkService();

    // Gets slot-id from the specified network connection
    static IMS_SINT32 GetSlotId(IN const IPAddress& objIPAddress);
    static IMS_SINT32 GetSlotId(IN INetConnection* piConnection);

private:
    NetworkServicePrivate *pPrivate;
};

#endif // SERVICE_IMS_NETWORK_H_
