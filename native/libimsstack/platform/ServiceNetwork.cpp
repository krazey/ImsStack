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
#include "ImsMessageDef.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceNetwork.h"
#include "ImsNetworkConnectionState.h"
#include "ImsSocketState.h"
#include "INetworkIpSec.h"
#include "IIpcan.h"
#include "PlatformApi.h"
#include "PlatformFactory.h"

class NetworkServicePrivate
{
public:
    inline NetworkServicePrivate()
        : piIPCAN(IMS_NULL)
        , piNetIPSec(IMS_NULL)
    {}
    inline ~NetworkServicePrivate()
    {
        PlatformFactory::DestroyIpcan(piIPCAN);
        PlatformFactory::DestroyNetworkIpSec(piNetIPSec);
    }

private:
    NetworkServicePrivate(IN const NetworkServicePrivate& objRHS);
    NetworkServicePrivate& operator=(IN const NetworkServicePrivate& objRHS);

public:
    inline IIpcan* GetIpcan()
    {
        if (piIPCAN == IMS_NULL)
        {
            piIPCAN = PlatformFactory::CreateIpcan();
        }

        return piIPCAN;
    }

    inline INetworkIpSec* GetIpSec()
    {
        if (piNetIPSec == IMS_NULL)
        {
            piNetIPSec = PlatformFactory::CreateNetworkIpSec();
        }

        return piNetIPSec;
    }

public:
    IIpcan *piIPCAN;
    INetworkIpSec *piNetIPSec;
};



PRIVATE
NetworkService::NetworkService()
    : pPrivate(new NetworkServicePrivate())
{
}

PRIVATE
NetworkService::~NetworkService()
{
    if (pPrivate != IMS_NULL)
    {
        delete pPrivate;
    }
}

PUBLIC
INetworkConnection* NetworkService::CreateConnection(IN const AString &strProfileName,
        IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    ImsNetworkConnection *pConnection
            = ImsNetworkConnectionState::GetInstance()->LookupHandle(strProfileName, nSlotId);

    if (pConnection != IMS_NULL)
    {
        return pConnection;
    }

    pConnection = PlatformFactory::CreateNetworkConnection(strProfileName, nSlotId);

    IMS_ASSERT(pConnection != IMS_NULL);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!pConnection->Create(strProfileName))
    {
        delete pConnection;
        return IMS_NULL;
    }

    return pConnection;
}

PUBLIC
INetworkConnection* NetworkService::CreateConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    ImsNetworkConnection *pConnection
            = ImsNetworkConnectionState::GetInstance()->LookupHandle(nApnType, nSlotId);

    if (pConnection != IMS_NULL)
    {
        return pConnection;
    }

    pConnection = PlatformFactory::CreateNetworkConnection(nApnType, nSlotId);

    IMS_ASSERT(pConnection != IMS_NULL);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!pConnection->Create(nApnType))
    {
        delete pConnection;
        return IMS_NULL;
    }

    return pConnection;
}

PUBLIC
void NetworkService::DestroyConnection(IN INetworkConnection*& piConnection)
{
    ImsNetworkConnection *pConnection =DYNAMIC_CAST(ImsNetworkConnection*, piConnection);

    if (pConnection != IMS_NULL)
    {
        pConnection->Deactivate();

        delete pConnection;
        piConnection = IMS_NULL;
    }
}

PUBLIC
INetworkConnection* NetworkService::FindConnection(IN const AString &strProfileName,
        IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    return ImsNetworkConnectionState::GetInstance()->LookupHandle(strProfileName, nSlotId);
}

PUBLIC
INetworkConnection* NetworkService::FindConnection(IN IMS_SINT32 nApnType,
        IN IMS_SINT32 nSlotId)
{
    return ImsNetworkConnectionState::GetInstance()->LookupHandle(nApnType, nSlotId);
}

PUBLIC
INetworkConnection* NetworkService::FindConnection(IN const IPAddress &objIPAddress)
{
    return ImsNetworkConnectionState::GetInstance()->LookupHandle(objIPAddress);
}

PUBLIC
ISocket* NetworkService::CreateSocket(IN INetworkConnection *piConnection)
{
    ImsNetworkConnection *pConnection = DYNAMIC_CAST(ImsNetworkConnection*, piConnection);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    ImsSocket *pSocket = PlatformFactory::CreateSocket();

    IMS_ASSERT(pSocket != IMS_NULL);

    if (pSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSocket->BindNetworkConnection(pConnection->GetHandle());

    return pSocket;
}

PUBLIC
ISocket* NetworkService::CreateSocket(IN const IMS_CHAR *pszProfileName,
        IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    ImsNetworkConnection *pConnection
            = ImsNetworkConnectionState::GetInstance()->LookupHandle(pszProfileName, nSlotId);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    ImsSocket *pSocket = PlatformFactory::CreateSocket();

    IMS_ASSERT(pSocket != IMS_NULL);

    if (pSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSocket->BindNetworkConnection(pConnection->GetHandle());

    return pSocket;
}

PUBLIC
ISocket* NetworkService::CreateSslSocket(IN INetworkConnection *piConnection,
        IN SSLCertificate *pCertificate)
{
    ImsNetworkConnection *pConnection = DYNAMIC_CAST(ImsNetworkConnection*, piConnection);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    ImsSocket *pSocket = PlatformFactory::CreateSslSocket(pCertificate);

    IMS_ASSERT(pSocket != IMS_NULL);

    if (pSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSocket->BindNetworkConnection(pConnection->GetHandle());

    return pSocket;
}

PUBLIC
ISocket* NetworkService::CreateSslSocket(IN const IMS_CHAR *pszProfileName,
        IN SSLCertificate *pCertificate, IN IMS_SINT32 nSlotId/* = IMS_SLOT_0*/)
{
    ImsNetworkConnection *pConnection
            = ImsNetworkConnectionState::GetInstance()->LookupHandle(pszProfileName, nSlotId);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    ImsSocket *pSocket = PlatformFactory::CreateSslSocket(pCertificate);

    IMS_ASSERT(pSocket != IMS_NULL);

    if (pSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSocket->BindNetworkConnection(pConnection->GetHandle());

    return pSocket;
}

PUBLIC
void NetworkService::DestroySocket(IN ISocket *&piSocket)
{
    ImsSocket *pSocket = DYNAMIC_CAST(ImsSocket*, piSocket);

    if (pSocket != IMS_NULL)
    {
        pSocket->Destroy();
        piSocket = IMS_NULL;
    }
}

PUBLIC
IMS_BOOL NetworkService::CheckIpAndPortAvailability(IN const IPAddress &objIP,
        IN IMS_SINT32 nPort, IN ISocket::SOCKET_ENTYPE enType)
{
    return PlatformApi::CheckIpAndPortAvailability(objIP, nPort, enType);
}

PUBLIC
IIpcan* NetworkService::GetIpcan()
{
    return pPrivate->GetIpcan();
}

PUBLIC
INetworkIpSec* NetworkService::GetIpSec()
{
    return pPrivate->GetIpSec();
}

PUBLIC
void NetworkService::DispatchServiceMessage(IN ImsMessage &objMSG)
{
    switch (objMSG.GetName())
    {
    case IMS_MSG_NETWORK:
    {
        ImsNetworkConnectionState *pState = ImsNetworkConnectionState::GetInstance();
        ImsNetworkConnection *pConnection
                = pState->LookupHandle(static_cast<IMS_CONNECTION>(objMSG.nLparam));

        if (pConnection != IMS_NULL)
        {
            pConnection->DispatchServiceMessage(objMSG.nWparam, objMSG.nLparam);
        }
    }
    break;

    case IMS_MSG_SOCKET:
    {
        ImsSocketState *pState = ImsSocketState::GetInstance();
        ImsSocket *pSocket = pState->LookupHandle(
                static_cast<IMS_SOCKET>(IMS_SOCKET_LOWORD(objMSG.nLparam)));

        if (pSocket != IMS_NULL)
        {
            pSocket->DispatchServiceMessage(objMSG.nWparam, objMSG.nLparam);
        }
    }
    break;

    default:
    break;
    }
}

PUBLIC GLOBAL
NetworkService* NetworkService::GetNetworkService()
{
    static NetworkService *pNetworkService = IMS_NULL;

    if (pNetworkService == IMS_NULL)
    {
        pNetworkService = new NetworkService();
    }

    return pNetworkService;
}

PUBLIC GLOBAL
IMS_SINT32 NetworkService::GetSlotId(IN const IPAddress& objIPAddress)
{
    NetworkService* pNetworkService = NetworkService::GetNetworkService();
    return GetSlotId(pNetworkService->FindConnection(objIPAddress));
}

PUBLIC GLOBAL
IMS_SINT32 NetworkService::GetSlotId(IN INetworkConnection* piConnection)
{
    ImsNetworkConnection* pConnection = DYNAMIC_CAST(ImsNetworkConnection*, piConnection);
    return (pConnection != IMS_NULL) ? pConnection->GetSlotId() : IMS_SLOT_ANY;
}
