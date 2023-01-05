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
#include "IIpcan.h"
#include "ImsMessageDef.h"
#include "ImsNetworkConnectionState.h"
#include "ImsSocketState.h"
#include "INetworkIpSec.h"
#include "PlatformApi.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceThread.h"
#include "SystemConfig.h"

class NetworkServicePrivate
{
public:
    NetworkServicePrivate();
    ~NetworkServicePrivate();

    NetworkServicePrivate(IN const NetworkServicePrivate&) = delete;
    NetworkServicePrivate& operator=(IN const NetworkServicePrivate&) = delete;

public:
    IIpcan* GetIpcan();
    INetworkIpSec* GetIpSec(IN IMS_SINT32 nSlotId);

private:
    IIpcan* m_piIpcan;
    ImsMap<IMS_SINT32, INetworkIpSec*> m_objIpSecs;
};

PUBLIC NetworkServicePrivate::NetworkServicePrivate() :
        m_piIpcan(IMS_NULL)
{
    for (IMS_UINT32 i = 0; i < SystemConfig::GetSupportedSimCount(); ++i)
    {
        m_objIpSecs.Add(i, IMS_NULL);
    }
}

PUBLIC NetworkServicePrivate::~NetworkServicePrivate()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    piOsFactory->DestroyIpcan(m_piIpcan);

    for (IMS_UINT32 i = 0; i < m_objIpSecs.GetSize(); ++i)
    {
        INetworkIpSec* piIpSec = m_objIpSecs.GetValueAt(i);
        piOsFactory->DestroyNetworkIpSec(piIpSec);
    }

    m_objIpSecs.Clear();
}

PUBLIC IIpcan* NetworkServicePrivate::GetIpcan()
{
    if (m_piIpcan == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piIpcan = piOsFactory->CreateIpcan();
    }

    return m_piIpcan;
}

PUBLIC INetworkIpSec* NetworkServicePrivate::GetIpSec(IN IMS_SINT32 nSlotId)
{
    if (nSlotId < 0 || nSlotId >= static_cast<IMS_SINT32>(m_objIpSecs.GetSize()))
    {
        return IMS_NULL;
    }

    INetworkIpSec* piIpSec = m_objIpSecs.GetValue(nSlotId);

    if (piIpSec == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        piIpSec = piOsFactory->CreateNetworkIpSec(nSlotId);

        m_objIpSecs.SetValue(nSlotId, piIpSec);
    }

    return piIpSec;
}

PRIVATE
NetworkService::NetworkService() :
        m_pPrivate(new NetworkServicePrivate())
{
}

PRIVATE
NetworkService::~NetworkService()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
INetworkConnection* NetworkService::CreateConnection(
        IN const AString& strProfileName, IN IMS_SINT32 nSlotId)
{
    ImsNetworkConnection* pConnection =
            ImsNetworkConnectionState::GetInstance()->LookupHandle(strProfileName, nSlotId);

    if (pConnection != IMS_NULL)
    {
        return pConnection;
    }

    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    pConnection = piOsFactory->CreateNetworkConnection(strProfileName, nSlotId);

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
    ImsNetworkConnection* pConnection =
            ImsNetworkConnectionState::GetInstance()->LookupHandle(nApnType, nSlotId);

    if (pConnection != IMS_NULL)
    {
        return pConnection;
    }

    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    pConnection = piOsFactory->CreateNetworkConnection(nApnType, nSlotId);

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
    ImsNetworkConnection* pConnection = DYNAMIC_CAST(ImsNetworkConnection*, piConnection);

    if (pConnection != IMS_NULL)
    {
        pConnection->Deactivate();

        delete pConnection;
        piConnection = IMS_NULL;
    }
}

PUBLIC
INetworkConnection* NetworkService::FindConnection(
        IN const AString& strProfileName, IN IMS_SINT32 nSlotId)
{
    return ImsNetworkConnectionState::GetInstance()->LookupHandle(strProfileName, nSlotId);
}

PUBLIC
INetworkConnection* NetworkService::FindConnection(IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    return ImsNetworkConnectionState::GetInstance()->LookupHandle(nApnType, nSlotId);
}

PUBLIC
INetworkConnection* NetworkService::FindConnection(IN const IPAddress& objIpAddr)
{
    return ImsNetworkConnectionState::GetInstance()->LookupHandle(objIpAddr);
}

PUBLIC
ISocket* NetworkService::CreateSocket(IN INetworkConnection* piConnection)
{
    ImsNetworkConnection* pConnection = DYNAMIC_CAST(ImsNetworkConnection*, piConnection);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    ImsSocket* pSocket = piOsFactory->CreateSocket();

    IMS_ASSERT(pSocket != IMS_NULL);

    if (pSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSocket->BindNetworkConnection(pConnection->GetHandle());

    return pSocket;
}

PUBLIC
ISocket* NetworkService::CreateSocket(IN const IMS_CHAR* pszProfileName, IN IMS_SINT32 nSlotId)
{
    ImsNetworkConnection* pConnection =
            ImsNetworkConnectionState::GetInstance()->LookupHandle(pszProfileName, nSlotId);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    ImsSocket* pSocket = piOsFactory->CreateSocket();

    IMS_ASSERT(pSocket != IMS_NULL);

    if (pSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSocket->BindNetworkConnection(pConnection->GetHandle());

    return pSocket;
}

PUBLIC
ISocket* NetworkService::CreateSslSocket(
        IN INetworkConnection* piConnection, IN SslCertificate* pCertificate)
{
    ImsNetworkConnection* pConnection = DYNAMIC_CAST(ImsNetworkConnection*, piConnection);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    ImsSocket* pSocket = piOsFactory->CreateSslSocket(pCertificate);

    IMS_ASSERT(pSocket != IMS_NULL);

    if (pSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSocket->BindNetworkConnection(pConnection->GetHandle());

    return pSocket;
}

PUBLIC
ISocket* NetworkService::CreateSslSocket(
        IN const IMS_CHAR* pszProfileName, IN SslCertificate* pCertificate, IN IMS_SINT32 nSlotId)
{
    ImsNetworkConnection* pConnection =
            ImsNetworkConnectionState::GetInstance()->LookupHandle(pszProfileName, nSlotId);

    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    ImsSocket* pSocket = piOsFactory->CreateSslSocket(pCertificate);

    IMS_ASSERT(pSocket != IMS_NULL);

    if (pSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSocket->BindNetworkConnection(pConnection->GetHandle());

    return pSocket;
}

PUBLIC
void NetworkService::DestroySocket(IN ISocket*& piSocket)
{
    ImsSocket* pSocket = DYNAMIC_CAST(ImsSocket*, piSocket);

    if (pSocket != IMS_NULL)
    {
        pSocket->Destroy();
        piSocket = IMS_NULL;
    }
}

PUBLIC
IMS_BOOL NetworkService::CheckIpAndPortAvailability(
        IN const IPAddress& objIpAddr, IN IMS_SINT32 nPort, IN ISocket::SOCKET_ENTYPE enType)
{
    return PlatformApi::CheckIpAndPortAvailability(objIpAddr, nPort, enType);
}

PUBLIC
IIpcan* NetworkService::GetIpcan()
{
    return m_pPrivate->GetIpcan();
}

PUBLIC
INetworkIpSec* NetworkService::GetIpSec(IN IMS_SINT32 nSlotId)
{
    return m_pPrivate->GetIpSec(nSlotId);
}

PUBLIC
void NetworkService::DispatchServiceMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case IMS_MSG_NETWORK:
        {
            ImsNetworkConnectionState* pState = ImsNetworkConnectionState::GetInstance();
            ImsNetworkConnection* pConnection =
                    pState->LookupHandle(static_cast<IMS_CONNECTION>(objMsg.nLparam));

            if (pConnection != IMS_NULL)
            {
                pConnection->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
            }
            break;
        }
        case IMS_MSG_SOCKET:
        {
            ImsSocketState* pState = ImsSocketState::GetInstance();
            ImsSocket* pSocket = pState->LookupHandle(
                    static_cast<IMS_SOCKET>(IMS_SOCKET_LOWORD(objMsg.nLparam)));

            if (pSocket != IMS_NULL)
            {
                pSocket->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
            }
            break;
        }
        default:
            break;
    }
}

PUBLIC GLOBAL NetworkService* NetworkService::GetNetworkService()
{
    return DYNAMIC_CAST(NetworkService*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_NETWORK));
}

PUBLIC GLOBAL IMS_SINT32 NetworkService::GetSlotId(IN const IPAddress& objIpAddr)
{
    NetworkService* pNetworkService = NetworkService::GetNetworkService();
    return GetSlotId(pNetworkService->FindConnection(objIpAddr));
}

PUBLIC GLOBAL IMS_SINT32 NetworkService::GetSlotId(IN INetworkConnection* piConnection)
{
    ImsNetworkConnection* pConnection = DYNAMIC_CAST(ImsNetworkConnection*, piConnection);
    return (pConnection != IMS_NULL) ? pConnection->GetSlotId() : IMS_SLOT_ANY;
}
