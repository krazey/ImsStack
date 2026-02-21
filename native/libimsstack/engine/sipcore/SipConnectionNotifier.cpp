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
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

#include "private/SipConfig.h"

#include "IOnSipConnectionNotifierErrorListener.h"
#include "IOnSipServerConnectionListener.h"
#include "SipAddress.h"
#include "SipConnectionNotifier.h"
#include "SipDebug.h"
#include "SipDialog.h"
#include "SipDialogImpl.h"
#include "SipError.h"
#include "SipFactoryProxy.h"
#include "SipManager.h"
#include "SipParameter.h"
#include "SipPrivate.h"
#include "SipRtConfigUtils.h"
#include "SipServerConnection.h"
#include "SipServerConnectionImpl.h"
#include "SipSocket.h"
#include "SipStack.h"
#include "SipStreamSocket.h"
#include "SipTransportHelper.h"

__IMS_TRACE_TAG_SIP_CORE__;

// 3 fix it
PRIVATE GLOBAL IMS_SINT32* SipConnectionNotifier::s_pGlobalSystemPort = IMS_NULL;

PUBLIC
SipConnectionNotifier::SipConnectionNotifier(IN IMS_SINT32 nScheme, IN IMS_SINT32 nPort,
        IN const AString& strParams, IN IMS_BOOL bSharedMode /*= IMS_FALSE*/) :
        Connection(),
        m_nMode((bSharedMode == IMS_TRUE) ? SHARED : DEDICATED),
        m_nScheme(nScheme),
        m_nPort(nPort),
        m_nTransportProtocol(Sip::TRANSPORT_ANY),
        m_nTransportExt(Sip::TRANSPORT_EXT_ANY),
        m_strType(AString::ConstNull()),
        m_strFilter(AString::ConstNull()),
        m_nPortC(Sip::PORT_UNSPECIFIED),
        m_pSocketUdp(IMS_NULL),
        m_pSocketTcp(IMS_NULL),
        m_nPortFlowControl(Sip::PORT_UNSPECIFIED),
        // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
        m_pSocketUdpClient(IMS_NULL),
        m_pSocketTcpClient(IMS_NULL),
        m_pSockAddrFarEnd(IMS_NULL),
        m_pSipProfile(IMS_NULL),
        m_piListener(IMS_NULL),
        m_piErrorListener(IMS_NULL)
{
    if (s_pGlobalSystemPort == IMS_NULL)
    {
        s_pGlobalSystemPort = new IMS_SINT32[SystemConfig::GetSupportedSimCount()];

        for (IMS_SINT32 i = 0; i < SystemConfig::GetSupportedSimCount(); ++i)
        {
            s_pGlobalSystemPort[i] = (i * 2000) + 9000;
        }
    }

    if ((bSharedMode == IMS_FALSE) && (nPort == Sip::PORT_UNSPECIFIED))
    {
        m_nPort = ++s_pGlobalSystemPort[GetSlotId()];
    }

    ExtractProperties(strParams);
}

PUBLIC VIRTUAL SipConnectionNotifier::~SipConnectionNotifier()
{
    if (!m_objParameters.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objParameters.GetSize(); ++i)
        {
            SipParameter* pParameter = m_objParameters.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                delete pParameter;
            }
        }

        m_objParameters.Clear();
    }

    if (!m_objTxnStates.IsEmpty())
    {
        m_objTxnStates.Clear();
    }

    if (!m_objForkedTxnStates.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objForkedTxnStates.GetSize(); ++i)
        {
            ForkedTxnState* pTxnState = m_objForkedTxnStates.GetAt(i);

            if (pTxnState != IMS_NULL)
            {
                delete pTxnState;
            }
        }

        m_objForkedTxnStates.Clear();
    }

    ClearTransportResource();

    IMS_TRACE_D("dtor: SipConnectionNotifier(%s|%d|%s)",
            (m_nMode == SHARED) ? "__SHARED__" : "__DEDICATED__", m_nPort, m_strType.GetStr());
}

PUBLIC VIRTUAL void SipConnectionNotifier::Close()
{
    ClearTransportResource();

    SipManager::GetInstance()->DetachConnectionNotifier(this);

    Connection::Close();
}

PUBLIC
ISipServerConnection* SipConnectionNotifier::AcceptAndOpen()
{
    if (m_objTxnStates.IsEmpty())
    {
        SipPrivate::SetLastError(SipError::NO_MESSAGE);
        return IMS_NULL;
    }

    RcPtr<SipServerTransactionState> pStState = m_objTxnStates.GetAt(0);

    if (pStState.IsNull())
    {
        SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    m_objTxnStates.RemoveAt(0);

    SipServerConnection* pSsc = new SipServerConnection(pStState.Get());

    if (pSsc == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    if (pSsc->InitRequest() != IMS_SUCCESS)
    {
        delete pSsc;

        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    SipServerConnectionImpl* pSscImpl = new SipServerConnectionImpl(pSsc);

    if (pSscImpl == IMS_NULL)
    {
        delete pSsc;

        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return pSscImpl;
}

PUBLIC
ISipServerConnection* SipConnectionNotifier::AcceptAndOpen(OUT ISipDialog*& piOrigDialog)
{
    if (m_objForkedTxnStates.IsEmpty())
    {
        SipPrivate::SetLastError(SipError::NO_MESSAGE);
        return IMS_NULL;
    }

    ForkedTxnState* pForkedTxnState = m_objForkedTxnStates.GetAt(0);

    if (pForkedTxnState == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    RcPtr<SipDialogEx> pDialogEx = pForkedTxnState->pDialogEx;
    RcPtr<SipServerTransactionState> pStState = pForkedTxnState->pStState;

    m_objForkedTxnStates.RemoveAt(0);
    delete pForkedTxnState;

    // Creates a SIP server transaction
    SipServerConnection* pSsc = new SipServerConnection(pStState.Get());

    if (pSsc == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    if (pSsc->InitRequest() != IMS_SUCCESS)
    {
        delete pSsc;

        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    SipServerConnectionImpl* pSscImpl = new SipServerConnectionImpl(pSsc);

    if (pSscImpl == IMS_NULL)
    {
        delete pSsc;

        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    // Create a SIP dialog
    SipDialog* pDialog = new SipDialog(pDialogEx.Get());

    if (pDialog == IMS_NULL)
    {
        delete pSscImpl;

        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    piOrigDialog = new SipDialogImpl(pDialog);

    if (piOrigDialog == IMS_NULL)
    {
        delete pDialog;
        delete pSscImpl;

        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return pSscImpl;
}

PUBLIC
AString SipConnectionNotifier::GetContactAddress() const
{
    AString strContact(AString::ConstEmpty());

    strContact.Append(TextParser::CHAR_LAQUOT);

    // "sip" & "sips" URI scheme only supports in the spec. (JSR 180 v1.1.0)
    if (m_nScheme == Sip::URI_SCHEME_SIPS)
    {
        strContact.Append(Sip::STR_SIPS);
    }
    else
    {
        strContact.Append(Sip::STR_SIP);
    }

    strContact.Append(TextParser::CHAR_COLON);

    const IpAddress& objAddress = GetLocalAddress();
    AString strPort;
    strPort.SetNumber(m_nPort);

    if (objAddress.IsIPv4Address())
    {
        strContact.Append(objAddress.ToString());
    }
    else
    {
        strContact.Append(TextParser::CHAR_LSBRACKET);
        strContact.Append(objAddress.ToString());
        strContact.Append(TextParser::CHAR_RSBRACKET);
    }

    strContact.Append(TextParser::CHAR_COLON);
    strContact.Append(strPort);

    // Set transport protocol if the type is present in Connection String.
    if (m_nTransportProtocol != Sip::TRANSPORT_ANY)
    {
        strContact.Append(TextParser::CHAR_SEMICOLON);
        strContact.Append(SipAddress::PARAM_TRANSPORT);
        strContact.Append(TextParser::CHAR_EQUAL);

        if (m_nTransportProtocol == SipTransportAddress::PROTOCOL_UDP)
        {
            strContact.Append(Sip::STR_UDP);
        }
        else
        {
            strContact.Append(Sip::STR_TCP);
        }
    }

    strContact.Append(TextParser::CHAR_RAQUOT);

    return strContact;
}

PUBLIC
IMS_BOOL SipConnectionNotifier::IsTransportResourceReserved(
        IN IMS_SINT32 nType /*= TRANSPORT_ALL*/) const
{
    if (nType == TRANSPORT_SERVER_CONNECTION)
    {
        if (IsTcpConnectionOnlyRequired())
        {
            return IMS_TRUE;
        }

        if ((m_pSocketUdp == IMS_NULL) ||
                (!IsClientInitiatedConnectionRequired() && (m_pSocketTcp == IMS_NULL)))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }
    else if (nType == TRANSPORT_CLIENT_INITIATED_CONNECTION)
    {
        // MULTI_REG_TRANSPORT
        if (IsTcpConnectionOnlyRequired())
        {
            if (m_pSocketTcpClient == IMS_NULL)
            {
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }

        if (IsClientInitiatedConnectionRequired() && (m_pSocketTcpClient == IMS_NULL))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    // MULTI_REG_TRANSPORT
    if (IsTcpConnectionOnlyRequired())
    {
        if (m_pSocketTcpClient == IMS_NULL)
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    if (IsClientInitiatedConnectionRequired() && (m_pSocketTcpClient == IMS_NULL))
    {
        return IMS_FALSE;
    }

    if ((m_pSocketTcp == IMS_NULL) || (m_pSocketUdp == IMS_NULL))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_RESULT SipConnectionNotifier::ReserveTransportResource(IN const IpAddress& objIp,
        IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl)
{
    // Port (US) will not be updated in here; it will be set when SCN is created.
    (void)nPortS;

    // MULTI_REG_TRANSPORT
    if (IsTcpConnectionOnlyRequired())
    {
        // Update the transport address
        m_objIpAddr = objIp;
        m_nPortC = nPortC;

        SipSocketAddress objFarEnd;

        // Use the unknown far-end socket info. to retrieve the local TCP socket only
        objFarEnd.SetType(SipSocketAddress::SOCKET_NONE);

        CreateClientInitiatedConnection(nPortC, &objFarEnd);

        IMS_TRACE_D("SCN: TCP client(%p) connection only required", m_pSocketTcpClient, 0, 0);

        if (m_nPort != nPortC)
        {
            IMS_TRACE_D("SCN: port_us changed(%d >> %d)", m_nPort, nPortC, 0);
            m_nPort = nPortC;
        }

        return IMS_SUCCESS;
    }

    // RFC5626_FLOW_CONTROL
    m_nPortFlowControl = nPortFlowControl;

    // Update the transport address
    m_objIpAddr = objIp;
    m_nPortC = nPortC;

    SipTransportHelper* pTransportHelper = GetTransportHelper();
    SipSocketAddress objNearEnd;

    // Check both transport protocol (UDP/TCP)
    objNearEnd.SetIpAddress(objIp);
    objNearEnd.SetPort(m_nPort);

    // RFC5626_FLOW_CONTROL
    if (IsClientInitiatedConnectionRequired())
    {
        SipSocketAddress objFarEnd;

        // Use the unknown far-end socket info. to retrieve the local TCP socket only
        objFarEnd.SetType(SipSocketAddress::SOCKET_NONE);

        // It just finds out the existing TCP client socket
        CreateClientInitiatedConnection(m_nPortFlowControl, &objFarEnd);
    }
    else
    {
        objNearEnd.SetType(SipSocketAddress::SOCKET_TCP);

        // TCP Server Socket
        m_pSocketTcp = pTransportHelper->Create(objNearEnd);

        if (m_pSocketTcp == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating TCP Server(%s:%d) failed", SipDebug::GetIp(objIp), m_nPort, 0);
            // Do not return even though TCP server socket is not created
            // return IMS_FAILURE;
        }
        else
        {
            m_pSocketTcp->SetListener(this);
        }
    }

    // UDP Server Socket
    objNearEnd.SetType(SipSocketAddress::SOCKET_UDP);
    m_pSocketUdp = pTransportHelper->Create(objNearEnd);

    if (m_pSocketUdp == IMS_NULL)
    {
        ClearTransportResource();

        IMS_TRACE_E(0, "Creating UDP Server(%s:%d) failed", SipDebug::GetIp(objIp), m_nPort, 0);
        return IMS_FAILURE;
    }

    m_pSocketUdp->SetListener(this);

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    ControlUdpClientReference(CTRL_CREATE);

    IMS_TRACE_I("SCN: transport-resources - u=%p, t=%p, u_c=%p", m_pSocketUdp, m_pSocketTcp,
            m_pSocketUdpClient);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipConnectionNotifier::RestoreTransportResource(
        IN IMS_SINT32 nType, IN const IpAddress& objPeerIp, IN IMS_SINT32 nPeerPort)
{
    IMS_RESULT nResult = IMS_SUCCESS;

    if ((nType & TRANSPORT_SERVER_CONNECTION) != 0)
    {
        if (RestoreTransportResourceForServerConnection() != IMS_SUCCESS)
        {
            nResult = IMS_FAILURE;
        }
    }

    if ((nType & TRANSPORT_CLIENT_INITIATED_CONNECTION) != 0)
    {
        if (RestoreTransportResourceForClientInitiatedConnection(objPeerIp, nPeerPort) !=
                IMS_SUCCESS)
        {
            nResult = IMS_FAILURE;
        }
    }

    return nResult;
}

PUBLIC
void SipConnectionNotifier::SetFromAndContact(
        IN const AString& strFrom, IN const AString& strDisplayName, IN const AString& strUserInfo)
{
    (void)strFrom;
    (void)strDisplayName;
    (void)strUserInfo;
}

PUBLIC
void SipConnectionNotifier::UpdatePortFlowControl(IN IMS_SINT32 nPort)
{
    if (nPort != m_nPortFlowControl)
    {
        IMS_TRACE_D("SCN: port_flow_control changed(%d >> %d)", m_nPortFlowControl, nPort, 0);

        if (IsClientInitiatedConnectionRequired())
        {
            DestroyClientInitiatedConnection(m_nPortFlowControl);
        }

        m_nPortFlowControl = nPort;

        if (Sip::IsPortSpecified(nPort))
        {
            // First, find out the existing socket
            if (!CreateClientInitiatedConnection(nPort, IMS_NULL))
            {
                // Second, create a new socket using the existing host information
                if (CreateClientInitiatedConnection(nPort, m_pSockAddrFarEnd))
                {
                    ConnectClientInitiatedConnection();
                }
            }
        }
    }
}

PUBLIC
void SipConnectionNotifier::UpdatePortUc(IN IMS_SINT32 nPort)
{
    if (nPort != m_nPortC)
    {
        IMS_TRACE_D("SCN: port_uc changed (%d >> %d)", m_nPortC, nPort, 0);

        if (IsTcpConnectionOnlyRequired())
        {
            DestroyClientInitiatedConnection(m_nPortC);

            if (Sip::IsPortSpecified(nPort))
            {
                CreateClientInitiatedConnection(nPort, IMS_NULL);

                if (m_nPort != nPort)
                {
                    IMS_TRACE_D("SCN: port_us changed (%d >> %d)", m_nPort, nPort, 0);
                    m_nPort = nPort;
                }
            }
        }

        m_nPortC = nPort;

        // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
        ControlUdpClientReference(CTRL_DESTROY);
        ControlUdpClientReference(CTRL_CREATE);
    }
}

PUBLIC
IMS_BOOL SipConnectionNotifier::IsSameConnectionNotifier(
        IN const SipTransportAddress& objTAddr) const
{
    if (GetLocalAddress().Equals(objTAddr.GetIpAddress()))
    {
        // RFC5626_FLOW_CONTROL
        if ((m_nPort == objTAddr.GetPort()) ||
                (IsClientInitiatedConnectionRequired() && m_nPortFlowControl == objTAddr.GetPort()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void SipConnectionNotifier::ServerTransactionState_ForkedRequestReceived(
        IN SipServerTransactionState* pStState, IN SipDialogEx* pOrigDialogEx)
{
    if (pStState == IMS_NULL)
    {
        return;
    }

    m_objForkedTxnStates.Append(new ForkedTxnState(pOrigDialogEx, pStState));

    if (m_piListener != IMS_NULL)
    {
        m_piListener->OnServerConnection_NotifyForkedRequest(this);
    }
}

PROTECTED VIRTUAL void SipConnectionNotifier::ServerTransactionState_RequestCreated(
        IN SipServerTransactionState* pStState)
{
    if (pStState == IMS_NULL)
    {
        return;
    }

    // Set a default Contact information
    pStState->SetDefaultContact(GetContactAddress());

    // MULTI_REG_SIP_PROFILE
    pStState->SetSipProfile(m_pSipProfile.Get());

    // Update the transport information
    // RFC5626_FLOW_CONTROL
    // MULTI_REG_TRANSPORT
    pStState->SetTransportTuple(
            m_objIpAddr, m_nPort, m_nPortC, m_nPortFlowControl, m_nTransportExt);
}

PROTECTED VIRTUAL void SipConnectionNotifier::ServerTransactionState_RequestReceived(
        IN SipServerTransactionState* pStState)
{
    if (pStState == IMS_NULL)
    {
        return;
    }

    m_objTxnStates.Append(pStState);

    if (m_piListener != IMS_NULL)
    {
        m_piListener->OnServerConnection_NotifyRequest(this);
    }
}

PRIVATE VIRTUAL void SipConnectionNotifier::Socket_NotifyError(
        IN SipSocket* pSocket, IN IMS_SINT32 nErrorCode)
{
    if ((nErrorCode == SipSocket::ERROR_CLOSED) ||
            (nErrorCode == SipSocket::ERROR_DATA_CONNECTION_LOST))
    {
        //// Listening channel only waits for the local resource release case.
    }
    else
    {
        IMS_TRACE_D("SCN: Socket(%p) error=%d", pSocket, nErrorCode, 0);
        return;
    }

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    if (pSocket == m_pSocketUdpClient)
    {
        IMS_TRACE_D("UDP Client: error=%d", nErrorCode, 0, 0);
        ControlUdpClientReference(CTRL_DESTROY);
        return;
    }

    if (m_pSocketTcpClient == pSocket)
    {
        IMS_TRACE_D("TCP Client: error=%d", nErrorCode, 0, 0);

        if (m_piErrorListener != IMS_NULL)
        {
            m_piErrorListener->OnConnectionNotifierError_NotifyError(
                    this, TRANSPORT_ERROR_TCP_CLIENT, "TCP client connection is lost");
        }

        if (IsTcpConnectionOnlyRequired())
        {
            DestroyClientInitiatedConnection(m_nPortC);
        }
        else if (IsClientInitiatedConnectionRequired())
        {
            DestroyClientInitiatedConnection(m_nPortFlowControl);
        }
        else
        {
            DestroyClientInitiatedConnection(Sip::PORT_UNSPECIFIED);
        }
        return;
    }

    if ((m_pSocketTcp == IMS_NULL) && (m_pSocketUdp == IMS_NULL))
    {
        return;
    }

    SipTransportHelper* pTransportHelper = GetTransportHelper();

    if (pSocket == m_pSocketTcp)
    {
        IMS_TRACE_D("TCP Server: error=%d", nErrorCode, 0, 0);

        pTransportHelper->Destroy(m_pSocketTcp, this);
        m_pSocketTcp = IMS_NULL;

        if (m_piErrorListener != IMS_NULL)
        {
            m_piErrorListener->OnConnectionNotifierError_NotifyError(
                    this, TRANSPORT_ERROR_TCP_SERVER, "TCP server connection is lost");
        }
        return;
    }

    if (pSocket == m_pSocketUdp)
    {
        IMS_TRACE_D("UDP Server: error=%d", nErrorCode, 0, 0);

        pTransportHelper->Destroy(m_pSocketUdp, this);
        m_pSocketUdp = IMS_NULL;

        if (m_piErrorListener != IMS_NULL)
        {
            m_piErrorListener->OnConnectionNotifierError_NotifyError(
                    this, TRANSPORT_ERROR_UDP_SERVER, "UDP server connection is lost");
        }
    }
}

PRIVATE VIRTUAL void SipConnectionNotifier::Socket_SendEnabled(IN SipSocket* pSocket)
{
    if (pSocket == m_pSocketUdp)
    {
        IMS_TRACE_I("UDP Server: SendEnabled", 0, 0, 0);
    }

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    if (pSocket == m_pSocketUdpClient)
    {
        IMS_TRACE_I("UDP Client: SendEnabled", 0, 0, 0);
    }
}

PRIVATE
void SipConnectionNotifier::ClearTransportResource()
{
    SipTransportHelper* pTransportHelper = GetTransportHelper();

    if (m_pSocketUdp != IMS_NULL)
    {
        pTransportHelper->Destroy(m_pSocketUdp, this);
        m_pSocketUdp = IMS_NULL;
    }

    if (m_pSocketTcp != IMS_NULL)
    {
        pTransportHelper->Destroy(m_pSocketTcp, this);
        m_pSocketTcp = IMS_NULL;
    }

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    ControlUdpClientReference(CTRL_DESTROY);

    if (IsTcpConnectionOnlyRequired())
    {
        DestroyClientInitiatedConnection(m_nPortC);
    }
    else if (IsClientInitiatedConnectionRequired())
    {
        DestroyClientInitiatedConnection(m_nPortFlowControl);
    }
    else
    {
        DestroyClientInitiatedConnection(Sip::PORT_UNSPECIFIED);
    }

    if (m_pSockAddrFarEnd != IMS_NULL)
    {
        delete m_pSockAddrFarEnd;
        m_pSockAddrFarEnd = IMS_NULL;
    }
}

// FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
PRIVATE
void SipConnectionNotifier::ControlUdpClientReference(IN IMS_SINT32 nControl)
{
    if (nControl == CTRL_CREATE)
    {
        if (Sip::IsPortSpecified(m_nPortC) && (m_nPort != m_nPortC) && IsIpSecRequired())
        {
            SipSocketAddress objNearEnd;

            objNearEnd.SetIpAddress(m_objIpAddr);
            objNearEnd.SetPort(m_nPortC);
            objNearEnd.SetType(SipSocketAddress::SOCKET_UDP);

            m_pSocketUdpClient = GetTransportHelper()->Create(objNearEnd);

            if (m_pSocketUdpClient != IMS_NULL)
            {
                m_pSocketUdpClient->SetListener(this);

                IMS_TRACE_D("UDP client(%s|%d) created", SipDebug::GetIp(m_objIpAddr), m_nPortC, 0);
            }
        }
    }
    else if (nControl == CTRL_DESTROY)
    {
        if (m_pSocketUdpClient != IMS_NULL)
        {
            GetTransportHelper()->Destroy(m_pSocketUdpClient, this);
            m_pSocketUdpClient = IMS_NULL;
        }
    }
}

PRIVATE
IMS_BOOL SipConnectionNotifier::CreateClientInitiatedConnection(
        IN IMS_SINT32 nPort, IN const SipSocketAddress* pFarEnd)
{
    SipTransportHelper* pTransportHelper = GetTransportHelper();
    SipSocketAddress objNearEnd;
    IMS_BOOL bPeerNameRequired = IMS_TRUE;

    objNearEnd.SetIpAddress(m_objIpAddr);
    objNearEnd.SetPort(nPort);
    objNearEnd.SetType(SipSocketAddress::SOCKET_TCP_CLIENT);

    if (pFarEnd == IMS_NULL)
    {
        m_pSocketTcpClient = pTransportHelper->Open(objNearEnd);
    }
    else
    {
        // Use the unknown far-end socket info. to retrieve the local TCP socket
        if (pFarEnd->GetType() == SipSocketAddress::SOCKET_NONE)
        {
            m_pSocketTcpClient = pTransportHelper->OpenStreamSocket(objNearEnd, *pFarEnd);
        }
        else
        {
            bPeerNameRequired = IMS_FALSE;
            m_pSocketTcpClient = pTransportHelper->CreateStreamSocket(objNearEnd, *pFarEnd);
        }
    }

    if (m_pSocketTcpClient == IMS_NULL)
    {
        IMS_TRACE_I("TCP client socket(%s,%d) can't be opened or created",
                SipDebug::GetIp(m_objIpAddr), m_nPort, 0);
        return IMS_FALSE;
    }

    m_pSocketTcpClient->SetListener(this);

    IpAddress objPeerIp;
    IMS_UINT32 nPeerPort = 0;

    if (bPeerNameRequired)
    {
        m_pSocketTcpClient->GetPeerName(objPeerIp, nPeerPort);
    }
    else
    {
        objPeerIp = pFarEnd->GetIpAddress();
        nPeerPort = pFarEnd->GetPort();
    }

    if (m_pSockAddrFarEnd == IMS_NULL)
    {
        m_pSockAddrFarEnd = new SipSocketAddress();
    }

    if (m_pSockAddrFarEnd != IMS_NULL)
    {
        m_pSockAddrFarEnd->SetType(SipSocketAddress::SOCKET_TCP_CLIENT);
        m_pSockAddrFarEnd->SetIpAddress(objPeerIp);
        m_pSockAddrFarEnd->SetPort(nPeerPort);
    }

    SipStreamSocket* pStreamSocket = DYNAMIC_CAST(SipStreamSocket*, m_pSocketTcpClient);

    if (pStreamSocket != IMS_NULL)
    {
        pStreamSocket->SetKeepAlivePolicy(SipConfig::TcpTimerValues::PERMANENT);
    }

    pTransportHelper->AttachClientInitiatedConnection(m_pSocketTcpClient);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipConnectionNotifier::ConnectClientInitiatedConnection()
{
    if (m_pSocketTcpClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "TCP client socket is null", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!m_pSocketTcpClient->Connect())
    {
        // LOG_EXCLUDING_SERVER_INFO
        if (m_pSockAddrFarEnd != IMS_NULL)
        {
            IMS_TRACE_E(0, "Connecting TCP client socket failed; (peer=%s|%d)",
                    SipRtConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId())
                            ? "***"
                            : SipDebug::GetIp(m_pSockAddrFarEnd->GetIpAddress()),
                    m_pSockAddrFarEnd->GetPort(), 0);
        }
        else
        {
            IMS_TRACE_E(0, "Connecting TCP client socket failed", 0, 0, 0);
        }

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void SipConnectionNotifier::DestroyClientInitiatedConnection(IN IMS_SINT32 nPort)
{
    if (m_pSocketTcpClient != IMS_NULL)
    {
        SipTransportHelper* pTransportHelper = GetTransportHelper();

        pTransportHelper->DetachClientInitiatedConnection(m_pSocketTcpClient);
        pTransportHelper->Destroy(m_pSocketTcpClient, this);

        if ((m_pSockAddrFarEnd != IMS_NULL) && Sip::IsPortSpecified(nPort))
        {
            SipSocketAddress objNearEnd;

            objNearEnd.SetType(SipSocketAddress::SOCKET_TCP_CLIENT);
            objNearEnd.SetPort(nPort);
            objNearEnd.SetIpAddress(m_objIpAddr);

            pTransportHelper->DestroyStreamSocket(objNearEnd, *m_pSockAddrFarEnd);
        }

        m_pSocketTcpClient = IMS_NULL;
    }
}

PRIVATE
void SipConnectionNotifier::ExtractProperties(IN const AString& strParams)
{
    SipPrivate::SetLastError(SipError::NO_ERROR);

    AString strTmp = strParams.Trim();

    if (strTmp.GetLength() > 0)
    {
        IMS_TRACE_D("SCN: params=%s", strTmp.GetStr(), 0, 0);
    }

    m_objParameters = SipStack::ExtractParameters(strTmp, TextParser::CHAR_SEMICOLON);

    // MULTI_REG_TRANSPORT
    if (!m_objParameters.IsEmpty())
    {
        const AString TRANSPORT_EXT(Sip::STR_TRANSPORT_EXT);
        for (IMS_UINT32 i = 0; i < m_objParameters.GetSize(); ++i)
        {
            const SipParameter* pParameter = m_objParameters.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                if (pParameter->GetName().EqualsIgnoreCase(TRANSPORT_EXT))
                {
                    IMS_BOOL bOk = IMS_FALSE;

                    m_nTransportExt = pParameter->GetValue().ToInt32(&bOk);

                    if (!bOk)
                    {
                        m_nTransportExt = Sip::TRANSPORT_EXT_ANY;
                    }

                    IMS_TRACE_D("SCN: transportProtocolExt=%02X", m_nTransportExt, 0, 0);

                    m_objParameters.RemoveAt(i);
                    break;
                }
            }
        }
    }

    if (m_objParameters.IsEmpty())
    {
        IMS_TRACE_I("SCN: NO PARAMETERS IN NAME(open)", 0, 0, 0);
        return;
    }

    const SipParameter* pParameter = m_objParameters.GetAt(0);

    if (pParameter == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
        return;  // throw exception
    }

    IMS_UINT32 i;

    // Look up "type" & "transport" parameters
    for (i = 0; i < m_objParameters.GetSize(); ++i)
    {
        pParameter = m_objParameters.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            if (pParameter->GetName().EqualsIgnoreCase(Sip::STR_TYPE))
            {
                m_strType = pParameter->GetValue();
                m_objParameters.RemoveAt(i);
                break;
            }
        }
    }

    if ((m_strType.GetLength() == 0) && (m_nMode == SHARED))
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return;  // throw exception
    }

    for (i = 0; i < m_objParameters.GetSize(); ++i)
    {
        pParameter = m_objParameters.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            if (pParameter->GetName().EqualsIgnoreCase(SipAddress::PARAM_TRANSPORT))
            {
                const AString& strTransport = pParameter->GetValue();

                if (strTransport.EqualsIgnoreCase(Sip::STR_UDP))
                {
                    m_nTransportProtocol = SipTransportAddress::PROTOCOL_UDP;
                }
                else if (strTransport.EqualsIgnoreCase(Sip::STR_TCP))
                {
                    m_nTransportProtocol = SipTransportAddress::PROTOCOL_TCP;
                }
                else
                {
                    // Both transport scheme MUST be handled.
                }

                m_objParameters.RemoveAt(i);
                break;
            }
        }
    }
}

PRIVATE
SipTransportHelper* SipConnectionNotifier::GetTransportHelper() const
{
    return SipFactoryProxy::GetInstance()->GetTransportHelper(GetSlotId());
}

PRIVATE
IMS_RESULT SipConnectionNotifier::RestoreTransportResourceForClientInitiatedConnection(
        IN const IpAddress& objPeerIp, IN IMS_SINT32 nPeerPort)
{
    IMS_BOOL bRestorationRequired = IMS_FALSE;
    IMS_SINT32 nClientPort = Sip::PORT_UNSPECIFIED;

    // MULTI_REG_TRANSPORT
    if (m_pSocketTcpClient == IMS_NULL)
    {
        bRestorationRequired = IMS_TRUE;

        if (IsTcpConnectionOnlyRequired())
        {
            nClientPort = m_nPortC;
        }
        else if (IsClientInitiatedConnectionRequired())
        {
            nClientPort = m_nPortFlowControl;
        }
    }

    if (bRestorationRequired && (nClientPort != Sip::PORT_UNSPECIFIED))
    {
        IMS_TRACE_I("RestoreTransportResourceForClientInitiatedConnection", 0, 0, 0);

        SipSocketAddress objFarEnd;

        objFarEnd.SetIpAddress(objPeerIp);
        objFarEnd.SetPort(nPeerPort);
        objFarEnd.SetType(SipSocketAddress::SOCKET_TCP_CLIENT);

        if (!CreateClientInitiatedConnection(nClientPort, &objFarEnd))
        {
            return IMS_FAILURE;
        }

        if (!ConnectClientInitiatedConnection())
        {
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT SipConnectionNotifier::RestoreTransportResourceForServerConnection()
{
    if ((m_pSocketTcp != IMS_NULL) && (m_pSocketUdp != IMS_NULL))
    {
        return IMS_SUCCESS;
    }

    IMS_TRACE_I("RestoreTransportResourceForServerConnection", 0, 0, 0);

    SipTransportHelper* pTransportHelper = GetTransportHelper();
    IMS_RESULT nResult = IMS_SUCCESS;
    SipSocketAddress objNearEnd;

    // Check both transport protocol (UDP/TCP)
    objNearEnd.SetIpAddress(m_objIpAddr);
    objNearEnd.SetPort(m_nPort);

    // TCP Server Socket
    if (!IsClientInitiatedConnectionRequired() && (m_pSocketTcp == IMS_NULL))
    {
        objNearEnd.SetType(SipSocketAddress::SOCKET_TCP);

        m_pSocketTcp = pTransportHelper->Create(objNearEnd);

        if (m_pSocketTcp == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating TCP Server (%s:%d) failed", SipDebug::GetIp(m_objIpAddr),
                    m_nPort, 0);

            nResult = IMS_FAILURE;
        }
        else
        {
            m_pSocketTcp->SetListener(this);
        }
    }

    // UDP Server Socket
    if (m_pSocketUdp == IMS_NULL)
    {
        objNearEnd.SetType(SipSocketAddress::SOCKET_UDP);

        m_pSocketUdp = pTransportHelper->Create(objNearEnd);

        if (m_pSocketUdp == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating UDP Server (%s:%d) failed", SipDebug::GetIp(m_objIpAddr),
                    m_nPort, 0);

            nResult = IMS_FAILURE;
        }
        else
        {
            m_pSocketUdp->SetListener(this);

            // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
            ControlUdpClientReference(CTRL_DESTROY);
            ControlUdpClientReference(CTRL_CREATE);
        }
    }

    return nResult;
}
