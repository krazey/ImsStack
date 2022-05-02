/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SystemConfig.h"
#include "private/SipConfig.h"
#include "SIPPrivate.h"
#include "SipDebug.h"
#include "SIPManager.h"
#include "SipAddress.h"
#include "SIPFactoryProxy.h"
#include "SIPRTConfigUtils.h"
#include "SIPTransportHelper.h"
#include "SIPSocket.h"
#include "SIPStreamSocket.h"
#include "SIPDialog.h"
#include "SIPDialogImpl.h"
#include "SIPServerConnection.h"
#include "SIPServerConnectionImpl.h"
#include "SIPServerTransactionState.h"
#include "IOnSIPServerConnectionListener.h"
#include "IOnSIPConnectionNotifierErrorListener.h"
#include "SIPConnectionNotifier.h"

__IMS_TRACE_TAG_SIP__;



//3 fix it
PRIVATE GLOBAL
IMS_SINT32* SIPConnectionNotifier::pGlobalSystemPort = IMS_NULL;



PUBLIC
SIPConnectionNotifier::SIPConnectionNotifier(IN IMS_SINT32 nScheme_, IN IMS_SINT32 nPort_,
        IN CONST AString &strParams_, IN IMS_BOOL bSharedMode_ /* = IMS_FALSE */)
    : Connection()
    , nMode(DEDICATED)
    , nScheme(nScheme_)
    , nPort(nPort_)
    , nTransportProtocol(SIP::TRANSPORT_ANY)
    , nTransportExt(SIP::TRANSPORT_EXT_ANY)
    , strType(AString::ConstNull())
    , strFilter(AString::ConstNull())
    , nPortC(SIP::PORT_UNSPECIFIED)
    , pSocket_UDP(IMS_NULL)
    , pSocket_TCP(IMS_NULL)
    , nPortFlowControl(SIP::PORT_UNSPECIFIED)
    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    , pSocket_UDPClient(IMS_NULL)
    , pSocket_TCPClient(IMS_NULL)
    , pSA_FarEnd(IMS_NULL)
    // MULTI_REG_SIP_PROFILE
    , pSIPProfile(IMS_NULL)
    , piListener(IMS_NULL)
    , piErrorListener(IMS_NULL)
{
    if (pGlobalSystemPort == IMS_NULL)
    {
        pGlobalSystemPort = new IMS_SINT32[SystemConfig::GetMaxSimSlot()];

        for (IMS_SINT32 i = 0; i < SystemConfig::GetMaxSimSlot(); ++i)
        {
            pGlobalSystemPort[i] = (i * 2000) + 9000;
        }
    }

    if (bSharedMode_ == IMS_TRUE)
    {
        nMode = SHARED;
    }

    if ((bSharedMode_ == IMS_FALSE) && (nPort == SIP::PORT_UNSPECIFIED))
    {
        nPort = ++pGlobalSystemPort[GetSlotId()];
    }

    ExtractProperties(strParams_);
}

PUBLIC VIRTUAL
SIPConnectionNotifier::~SIPConnectionNotifier()
{
    //---------------------------------------------------------------------------------------------

    if (!objParameters.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
        {
            SIPParameter *pParameter = objParameters.GetAt(i);

            if (pParameter != IMS_NULL)
                delete pParameter;
        }

        objParameters.Clear();
    }

    if (!objTxnStates.IsEmpty())
    {
        objTxnStates.Clear();
    }

    if (!objForkedTxnStates.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objForkedTxnStates.GetSize(); ++i)
        {
            ForkedTxnState *pTxnState = objForkedTxnStates.GetAt(i);

            if (pTxnState != IMS_NULL)
            {
                delete pTxnState;
            }
        }

        objForkedTxnStates.Clear();
    }

    ClearTransportResource();

    IMS_TRACE_D("Destructor :: SIPConnectionNotifier (%s, %d, type: %s)",
            (nMode == SHARED) ? "__SHARED__" : "__DEDICATED__", nPort, strType.GetStr());
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPConnectionNotifier::Close()
{
    //---------------------------------------------------------------------------------------------

    ClearTransportResource();

    SIPManager::GetInstance()->DetachConnectionNotifier(this);

    Connection::Close();
}

/*

Remarks

*/
PUBLIC
ISIPServerConnection* SIPConnectionNotifier::AcceptAndOpen()
{
    //---------------------------------------------------------------------------------------------

    if (objTxnStates.IsEmpty())
    {
        SIPPrivate::SetLastError(SIPError::NO_MESSAGE);
        return IMS_NULL;
    }

    RCPtr<SIPServerTransactionState> pSTState = objTxnStates.GetAt(0);

    if (pSTState.IsNull())
    {
        SIPPrivate::SetLastError(SIPError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    objTxnStates.RemoveAt(0);

    SIPServerConnection *pSSC = new SIPServerConnection(pSTState.Get());

    if (pSSC == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    if (pSSC->InitRequest() != IMS_SUCCESS)
    {
        delete pSSC;

        SIPPrivate::SetLastError(SIPError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    SIPServerConnectionImpl *pSSCImpl = new SIPServerConnectionImpl(pSSC);

    if (pSSCImpl == IMS_NULL)
    {
        delete pSSC;

        SIPPrivate::SetLastError(SIPError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return pSSCImpl;
}

/*

Remarks

*/
PUBLIC
const IPAddress& SIPConnectionNotifier::GetLocalAddress() const
{
    //---------------------------------------------------------------------------------------------

    return objIPA;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPConnectionNotifier::GetLocalPort() const
{
    //---------------------------------------------------------------------------------------------

    return nPort;
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifier::SetListener(IN IOnSIPServerConnectionListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

//// IMS extensions

/*

Remarks

*/
PUBLIC
ISIPServerConnection* SIPConnectionNotifier::AcceptAndOpen(OUT ISIPDialog *&piOrigDialog)
{
    //---------------------------------------------------------------------------------------------

    if (objForkedTxnStates.IsEmpty())
    {
        SIPPrivate::SetLastError(SIPError::NO_MESSAGE);
        return IMS_NULL;
    }

    ForkedTxnState *pForkedTxnState = objForkedTxnStates.GetAt(0);

    if (pForkedTxnState == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    RCPtr<SIPDialogEx> pDialogEx = pForkedTxnState->pDialogEx;
    RCPtr<SIPServerTransactionState> pSTState = pForkedTxnState->pSTState;

    objForkedTxnStates.RemoveAt(0);
    delete pForkedTxnState;

    // Creates a SIP server transaction
    SIPServerConnection *pSSC = new SIPServerConnection(pSTState.Get());

    if (pSSC == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    if (pSSC->InitRequest() != IMS_SUCCESS)
    {
        delete pSSC;

        SIPPrivate::SetLastError(SIPError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    SIPServerConnectionImpl *pSSCImpl = new SIPServerConnectionImpl(pSSC);

    if (pSSCImpl == IMS_NULL)
    {
        delete pSSC;

        SIPPrivate::SetLastError(SIPError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    // Create a SIP dialog
    SIPDialog *pDialog = new SIPDialog(pDialogEx.Get());

    if (pDialog == IMS_NULL)
    {
        delete pSSCImpl;

        SIPPrivate::SetLastError(SIPError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    piOrigDialog = new SIPDialogImpl(pDialog);

    if (piOrigDialog == IMS_NULL)
    {
        delete pDialog;
        delete pSSCImpl;

        SIPPrivate::SetLastError(SIPError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return pSSCImpl;
}

/*

Remarks

*/
PUBLIC
AString SIPConnectionNotifier::GetContactAddress() const
{
    AString strContact(AString::ConstEmpty());

    //---------------------------------------------------------------------------------------------

#ifdef __JSR180_ONLY__

    if (stUserProfile.strDisplayName.GetLength() > 0)
    {
        strContact.Append(stUserProfile.strDisplayName);
        strContact.Append(TextParser::CHAR_SP);
    }

    strContact.Append(TextParser::CHAR_LAQUOT);

    // "sip" & "sips" URI scheme only supports in the spec. (JSR 180 v1.1.0)
    if (nScheme == SIP::URI_SCHEME_SIPS)
        strContact.Append(SIP::STR_SIPS);
    else
        strContact.Append(SIP::STR_SIP);

    strContact.Append(TextParser::CHAR_COLON);

    if (stUserProfile.strUserInfo.GetLength() > 0)
    {
        strContact.Append(stUserProfile.strUserInfo);
        strContact.Append(TextParser::CHAR_AT);
    }

    const IPAddress &objAddress = GetLocalAddress();
    AString strPort;
    strPort.SetNumber(nPort);

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
    if (nTransportProtocol != SIP::TRANSPORT_ANY)
    {
        strContact.Append(TextParser::CHAR_SEMICOLON);
        strContact.Append(SIPAddress::PARAM_TRANSPORT);
        strContact.Append(TextParser::CHAR_EQUAL);

        if (nTransportProtocol == SIPTransportAddress::PROTOCOL_UDP)
            strContact.Append(SIP::STR_UDP);
        else
            strContact.Append(SIP::STR_TCP);
    }

    strContact.Append(TextParser::CHAR_RAQUOT);

    return strContact;

#else

    strContact.Append(TextParser::CHAR_LAQUOT);

    // "sip" & "sips" URI scheme only supports in the spec. (JSR 180 v1.1.0)
    if (nScheme == SIP::URI_SCHEME_SIPS)
        strContact.Append(SIP::STR_SIPS);
    else
        strContact.Append(SIP::STR_SIP);

    strContact.Append(TextParser::CHAR_COLON);

    const IPAddress &objAddress = GetLocalAddress();
    AString strPort;
    strPort.SetNumber(nPort);

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
    if (nTransportProtocol != SIP::TRANSPORT_ANY)
    {
        strContact.Append(TextParser::CHAR_SEMICOLON);
        strContact.Append(SIPAddress::PARAM_TRANSPORT);
        strContact.Append(TextParser::CHAR_EQUAL);

        if (nTransportProtocol == SIPTransportAddress::PROTOCOL_UDP)
            strContact.Append(SIP::STR_UDP);
        else
            strContact.Append(SIP::STR_TCP);
    }

    strContact.Append(TextParser::CHAR_RAQUOT);

    return strContact;

#endif // #ifdef __JSR180_ONLY__
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
SIPProfile* SIPConnectionNotifier::GetSIPProfile() const
{
    //---------------------------------------------------------------------------------------------

    return pSIPProfile.Get();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPConnectionNotifier::IsTransportResourceReserved(
        IN IMS_SINT32 nType/* = TRANSPORT_ALL*/) const
{
    //---------------------------------------------------------------------------------------------

    if (nType == TRANSPORT_SERVER_CONNECTION)
    {
        if (IsTCPConnectionOnlyRequired())
        {
            return IMS_TRUE;
        }

        if ((pSocket_UDP == IMS_NULL)
                || (!IsClientInitiatedConnectionRequired() && (pSocket_TCP == IMS_NULL)))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }
    else if (nType == TRANSPORT_CLIENT_INITIATED_CONNECTION)
    {
        // MULTI_REG_TRANSPORT
        if (IsTCPConnectionOnlyRequired())
        {
            if (pSocket_TCPClient == IMS_NULL)
            {
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }

        if (IsClientInitiatedConnectionRequired() && (pSocket_TCPClient == IMS_NULL))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    // MULTI_REG_TRANSPORT
    if (IsTCPConnectionOnlyRequired())
    {
        if (pSocket_TCPClient == IMS_NULL)
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    if (IsClientInitiatedConnectionRequired() && (pSocket_TCPClient == IMS_NULL))
    {
        return IMS_FALSE;
    }

    if ((pSocket_TCP == IMS_NULL) || (pSocket_UDP == IMS_NULL))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPConnectionNotifier::ReserveTransportResource(IN CONST IPAddress &objIPA,
        IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl)
{
    //---------------------------------------------------------------------------------------------

    // Port (US) will not be updated in here; it will be set when SCN is created...
    (void) nPortS;

    // MULTI_REG_TRANSPORT
    if (IsTCPConnectionOnlyRequired())
    {
        // Update the transport address
        this->objIPA = objIPA;
        this->nPortC = nPortC;

        SIPSocketAddress objSA_FarEnd;

        // Use the unknown far-end socket info. to retrieve the local TCP socket only
        objSA_FarEnd.SetType(SIPSocketAddress::SOCKET_NONE);

        CreateClientInitiatedConnection(nPortC, &objSA_FarEnd);

        IMS_TRACE_D("ConnectionNotifier :: TCP client connection is only required... %p",
                pSocket_TCPClient, 0, 0);

        if (nPort != nPortC)
        {
            IMS_TRACE_D("SCN :: port_us changed (%d >> %d)", nPort, nPortC, 0);
            nPort = nPortC;
        }

        return IMS_SUCCESS;
    }

    // RFC5626_FLOW_CONTROL
    this->nPortFlowControl = nPortFlowControl;

    // Update the transport address
    this->objIPA = objIPA;
    this->nPortC = nPortC;

    SIPTransportHelper *pTransportHelper = GetTransportHelper();
    SIPSocketAddress objSA;

    // Check both transport protocol (UDP/TCP)
    objSA.SetIPAddress(objIPA);
    objSA.SetPort(nPort);

    // RFC5626_FLOW_CONTROL
    if (IsClientInitiatedConnectionRequired())
    {
        SIPSocketAddress objSA_FarEnd;

        // Use the unknown far-end socket info. to retrieve the local TCP socket only
        objSA_FarEnd.SetType(SIPSocketAddress::SOCKET_NONE);

        // It just finds out the existing TCP client socket
        CreateClientInitiatedConnection(this->nPortFlowControl, &objSA_FarEnd);
    }
    else
    {
        objSA.SetType(SIPSocketAddress::SOCKET_TCP);

        // TCP Server Socket
        pSocket_TCP = pTransportHelper->Create(objSA);

        if (pSocket_TCP == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating TCP Server (%s:%d) failed",
                    SIPDebug::GetIP(objIPA), nPort, 0);
            // hwangoo.park, 140210, do not return even though TCP server socket is not created
            // return IMS_FAILURE;
        }
        else
        {
            pSocket_TCP->SetListener(this);
        }
    }

    // UDP Server Socket
    objSA.SetType(SIPSocketAddress::SOCKET_UDP);
    pSocket_UDP = pTransportHelper->Create(objSA);

    if (pSocket_UDP == IMS_NULL)
    {
        ClearTransportResource();

        IMS_TRACE_E(0, "Creating UDP Server (%s:%d) failed", SIPDebug::GetIP(objIPA), nPort, 0);
        return IMS_FAILURE;
    }

    pSocket_UDP->SetListener(this);

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    ControlUDPClientReference(CTRL_CREATE);

    IMS_TRACE_I("SCN :: transport-resources - u=%p, t=%p, u_c=%p",
            pSocket_UDP, pSocket_TCP, pSocket_UDPClient);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPConnectionNotifier::RestoreTransportResource(IN IMS_SINT32 nType,
        IN CONST IPAddress &objPeerIP, IN IMS_SINT32 nPeerPort)
{
    IMS_RESULT nResult = IMS_SUCCESS;

    //---------------------------------------------------------------------------------------------

    if ((nType & TRANSPORT_SERVER_CONNECTION) != 0)
    {
        if (RestoreTransportResourceForServerConnection() != IMS_SUCCESS)
        {
            nResult = IMS_FAILURE;
        }
    }

    if ((nType & TRANSPORT_CLIENT_INITIATED_CONNECTION) != 0)
    {
        if (RestoreTransportResourceForClientInitiatedConnection(
                objPeerIP, nPeerPort) != IMS_SUCCESS)
        {
            nResult = IMS_FAILURE;
        }
    }

    return nResult;
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifier::SetErrorListener(IN IOnSIPConnectionNotifierErrorListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piErrorListener = piListener;
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifier::SetFilter(IN CONST AString &strFilter)
{
    //---------------------------------------------------------------------------------------------

    this->strFilter = strFilter;
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifier::SetFromAndContact(IN CONST AString &strFrom,
        IN CONST AString &strDisplayName, IN CONST AString &strUserInfo)
{
    //---------------------------------------------------------------------------------------------

#ifdef __JSR180_ONLY__
    stUserProfile.strFrom = strFrom;
    stUserProfile.strDisplayName = strDisplayName;
    stUserProfile.strUserInfo = strUserInfo;
#else
    (void) strFrom;
    (void) strDisplayName;
    (void) strUserInfo;
#endif
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
void SIPConnectionNotifier::SetSIPProfile(IN SIPProfile *pProfile)
{
    //---------------------------------------------------------------------------------------------

    pSIPProfile = pProfile;
}

/*

Remarks
 RFC5626_FLOW_CONTROL
*/
PUBLIC
void SIPConnectionNotifier::UpdatePortFlowControl(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    if (nPort != nPortFlowControl)
    {
        IMS_TRACE_D("SCN :: port_flow_control changed (%d >> %d)", nPortFlowControl, nPort, 0);

        if (IsClientInitiatedConnectionRequired())
        {
            DestroyClientInitiatedConnection(nPortFlowControl);
        }

        nPortFlowControl = nPort;

        if (SIP::IsPortSpecified(nPort))
        {
            // First, find out the existing socket
            if (!CreateClientInitiatedConnection(nPort, IMS_NULL))
            {
                // Second, create a new socket using the existing host information
                if (CreateClientInitiatedConnection(nPort, pSA_FarEnd))
                {
                    ConnectClientInitiatedConnection();
                }
            }
        }
    }
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifier::UpdatePortUC(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    if (nPort != nPortC)
    {
        IMS_TRACE_D("SCN :: port_uc changed (%d >> %d)", nPortC, nPort, 0);

        if (IsTCPConnectionOnlyRequired())
        {
            DestroyClientInitiatedConnection(nPortC);

            if (SIP::IsPortSpecified(nPort))
            {
                CreateClientInitiatedConnection(nPort, IMS_NULL);

                if (this->nPort != nPort)
                {
                    IMS_TRACE_D("SCN :: port_us changed (%d >> %d)", this->nPort, nPort, 0);
                    this->nPort = nPort;
                }
            }
        }

        nPortC = nPort;

        // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
        ControlUDPClientReference(CTRL_DESTROY);
        ControlUDPClientReference(CTRL_CREATE);
    }
}

// IMS extensions

/*

Remarks

*/
PUBLIC
AString SIPConnectionNotifier::GetUserIdentity() const
{
    AString strUserId;

    //---------------------------------------------------------------------------------------------

#ifdef __JSR180_ONLY__

    strUserId = stUserProfile.strDisplayName;

    if (stUserProfile.strDisplayName.GetLength() > 0)
        strUserId = stUserProfile.strDisplayName + TextParser::CHAR_SP;
    else
        strUserId = "";

    if (stUserProfile.strFrom.GetIndexOf(TextParser::CHAR_LAQUOT) != AString::NPOS)
    {
        strUserId += stUserProfile.strFrom;
    }
    else
    {
        strUserId += TextParser::CHAR_LAQUOT;
        strUserId += stUserProfile.strFrom;
        strUserId += TextParser::CHAR_RAQUOT;
    }

    return strUserId;

#else

    strUserId = "\"Anonymous\" <sip:anonymous@anonymous.invalid>";

    return strUserId;

#endif // #ifdef __JSR180_ONLY__
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPConnectionNotifier::IsSameConnectionNotifier(IN CONST SIPTransportAddress &objTA) const
{
    //---------------------------------------------------------------------------------------------

    if (GetLocalAddress().Equals(objTA.GetIPAddress()))
    {
        if (nPort == objTA.GetPort())
        {
            // Shall we check the transport protocol type ???
            return IMS_TRUE;
        }
        // RFC5626_FLOW_CONTROL
        else if (IsClientInitiatedConnectionRequired()
                && (nPortFlowControl == objTA.GetPort()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPConnectionNotifier::ServerTransactionState_ForkedRequestReceived(
        IN SIPServerTransactionState *pSTState, IN SIPDialogEx *pOrigDialogEx)
{
    //---------------------------------------------------------------------------------------------

    if (pSTState == IMS_NULL)
        return;

    objForkedTxnStates.Append(new ForkedTxnState(pOrigDialogEx, pSTState));

    if (piListener != IMS_NULL)
    {
        piListener->OnServerConnection_NotifyForkedRequest(this);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPConnectionNotifier::ServerTransactionState_RequestCreated(
        IN SIPServerTransactionState *pSTState)
{
    //---------------------------------------------------------------------------------------------

    if (pSTState == IMS_NULL)
        return;

    // Set a default Contact information
    pSTState->SetDefaultContact(GetContactAddress());

    // MULTI_REG_SIP_PROFILE
    pSTState->SetSIPProfile(pSIPProfile.Get());

    // Update the transport information
    // RFC5626_FLOW_CONTROL
    // MULTI_REG_TRANSPORT
    pSTState->SetTransportTuple(objIPA, nPort, nPortC, nPortFlowControl, nTransportExt);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPConnectionNotifier::ServerTransactionState_RequestReceived(
        IN SIPServerTransactionState *pSTState)
{
    //---------------------------------------------------------------------------------------------

    if (pSTState == IMS_NULL)
        return;

    objTxnStates.Append(pSTState);

    if (piListener != IMS_NULL)
    {
        piListener->OnServerConnection_NotifyRequest(this);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifier::Socket_NotifyError(IN SIPSocket *pSocket, IN IMS_SINT32 nErrorCode)
{
    //---------------------------------------------------------------------------------------------

    if ((nErrorCode == SIPSocket::ERROR_CLOSED)
            || (nErrorCode == SIPSocket::ERROR_DATA_CONNECTION_LOST))
    {
        //// Listening channel only waits for the local resource release case...
    }
    else
    {
        IMS_TRACE_D("ConnectionNotifier :: Socket(%p), ErrorCode(%d)", pSocket, nErrorCode, 0);
        return;
    }

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    if (pSocket == pSocket_UDPClient)
    {
        IMS_TRACE_D("UDP Client :: NotifyError (%d)", nErrorCode, 0, 0);
        ControlUDPClientReference(CTRL_DESTROY);
        return;
    }

    if (pSocket_TCPClient == pSocket)
    {
        IMS_TRACE_D("TCP Client :: NotifyError (%d)", nErrorCode, 0, 0);

        if (piErrorListener != IMS_NULL)
        {
            piErrorListener->OnConnectionNotifierError_NotifyError(this,
                    TRANSPORT_ERROR_TCP_CLIENT, "TCP client connection is lost");
        }

        if (IsTCPConnectionOnlyRequired())
        {
            DestroyClientInitiatedConnection(nPortC);
        }
        else if (IsClientInitiatedConnectionRequired())
        {
            DestroyClientInitiatedConnection(nPortFlowControl);
        }
        else
        {
            DestroyClientInitiatedConnection(SIP::PORT_UNSPECIFIED);
        }
        return;
    }

    if ((pSocket_TCP == IMS_NULL) && (pSocket_UDP == IMS_NULL))
    {
        return;
    }

    SIPTransportHelper *pTransportHelper = GetTransportHelper();

    if (pSocket == pSocket_TCP)
    {
        IMS_TRACE_D("TCP Server :: NotifyError (%d)", nErrorCode, 0, 0);

        pTransportHelper->Destroy(pSocket_TCP, this);
        pSocket_TCP = IMS_NULL;

        if (piErrorListener != IMS_NULL)
        {
            piErrorListener->OnConnectionNotifierError_NotifyError(this,
                    TRANSPORT_ERROR_TCP_SERVER, "TCP server connection is lost");
        }
        return;
    }

    if (pSocket == pSocket_UDP)
    {
        IMS_TRACE_D("UDP Server :: NotifyError (%d)", nErrorCode, 0, 0);

        pTransportHelper->Destroy(pSocket_UDP, this);
        pSocket_UDP = IMS_NULL;

        if (piErrorListener != IMS_NULL)
        {
            piErrorListener->OnConnectionNotifierError_NotifyError(this,
                    TRANSPORT_ERROR_UDP_SERVER, "UDP server connection is lost");
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifier::Socket_SendEnabled(IN SIPSocket *pSocket)
{
    //---------------------------------------------------------------------------------------------

    if (pSocket == pSocket_UDP)
    {
        IMS_TRACE_I("UDP Server :: SendEnabled", 0, 0, 0);
    }

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    if (pSocket == pSocket_UDPClient)
    {
        IMS_TRACE_I("UDP Client :: SendEnabled", 0, 0, 0);
    }
}

/*

Remarks

*/
PRIVATE
void SIPConnectionNotifier::ClearTransportResource()
{
    SIPTransportHelper *pTransportHelper = GetTransportHelper();

    //---------------------------------------------------------------------------------------------

    if (pSocket_UDP != IMS_NULL)
    {
        pTransportHelper->Destroy(pSocket_UDP, this);
        pSocket_UDP = IMS_NULL;
    }

    if (pSocket_TCP != IMS_NULL)
    {
        pTransportHelper->Destroy(pSocket_TCP, this);
        pSocket_TCP = IMS_NULL;
    }

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    ControlUDPClientReference(CTRL_DESTROY);

    if (IsTCPConnectionOnlyRequired())
    {
        DestroyClientInitiatedConnection(nPortC);
    }
    else if (IsClientInitiatedConnectionRequired())
    {
        DestroyClientInitiatedConnection(nPortFlowControl);
    }
    else
    {
        DestroyClientInitiatedConnection(SIP::PORT_UNSPECIFIED);
    }

    if (pSA_FarEnd != IMS_NULL)
    {
        delete pSA_FarEnd;
        pSA_FarEnd = IMS_NULL;
    }
}

/*

Remarks
 FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
*/
PRIVATE
void SIPConnectionNotifier::ControlUDPClientReference(IN IMS_SINT32 nControl)
{
    if (nControl == CTRL_CREATE)
    {
        if (SIP::IsPortSpecified(nPortC) && (nPort != nPortC) && IsIPSecRequired())
        {
            SIPSocketAddress objSA;

            objSA.SetIPAddress(objIPA);
            objSA.SetPort(nPortC);
            objSA.SetType(SIPSocketAddress::SOCKET_UDP);

            pSocket_UDPClient = GetTransportHelper()->Create(objSA);

            if (pSocket_UDPClient != IMS_NULL)
            {
                pSocket_UDPClient->SetListener(this);

                IMS_TRACE_D("UDP client (Ref. :: %s-%d) is created",
                        SIPDebug::GetIP(objIPA), nPortC, 0);
            }
        }
    }
    else if (nControl == CTRL_DESTROY)
    {
        if (pSocket_UDPClient != IMS_NULL)
        {
            GetTransportHelper()->Destroy(pSocket_UDPClient, this);
            pSocket_UDPClient = IMS_NULL;
        }
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPConnectionNotifier::CreateClientInitiatedConnection(IN IMS_SINT32 nPort,
        IN SIPSocketAddress *pFarEnd)
{
    SIPTransportHelper *pTransportHelper = GetTransportHelper();
    SIPSocketAddress objSA;
    IMS_BOOL bPeerNameRequired = IMS_TRUE;

    objSA.SetIPAddress(objIPA);
    objSA.SetPort(nPort);
    objSA.SetType(SIPSocketAddress::SOCKET_TCP_CLIENT);

    if (pFarEnd == IMS_NULL)
    {
        pSocket_TCPClient = pTransportHelper->Open(objSA);
    }
    else
    {
        // Use the unknown far-end socket info. to retrieve the local TCP socket
        if (pFarEnd->GetType() == SIPSocketAddress::SOCKET_NONE)
        {
            pSocket_TCPClient = pTransportHelper->OpenStreamSocket(objSA, *pFarEnd);
        }
        else
        {
            bPeerNameRequired = IMS_FALSE;
            pSocket_TCPClient = pTransportHelper->CreateStreamSocket(objSA, *pFarEnd);
        }
    }

    if (pSocket_TCPClient == IMS_NULL)
    {
        IMS_TRACE_I("TCP client socket(%s,%d) can't be opened or created",
                SIPDebug::GetIP(objIPA), nPort, 0);
        return IMS_FALSE;
    }

    pSocket_TCPClient->SetListener(this);

    IPAddress objPeerIP;
    IMS_UINT32 nPeerPort = 0;

    if (bPeerNameRequired)
    {
        pSocket_TCPClient->GetPeerName(objPeerIP, nPeerPort);
    }
    else
    {
        objPeerIP = pFarEnd->GetIPAddress();
        nPeerPort = pFarEnd->GetPort();
    }

    if (pSA_FarEnd == IMS_NULL)
    {
        pSA_FarEnd = new SIPSocketAddress();
    }

    if (pSA_FarEnd != IMS_NULL)
    {
        pSA_FarEnd->SetType(SIPSocketAddress::SOCKET_TCP_CLIENT);
        pSA_FarEnd->SetIPAddress(objPeerIP);
        pSA_FarEnd->SetPort(nPeerPort);
    }

    SIPStreamSocket *pStreamSocket = DYNAMIC_CAST(SIPStreamSocket*, pSocket_TCPClient);

    if (pStreamSocket != IMS_NULL)
    {
        pStreamSocket->SetKeepAlivePolicy(SipConfig::TcpTimerValues::PERMANENT);
    }

    pTransportHelper->AttachClientInitiatedConnection(pSocket_TCPClient);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPConnectionNotifier::ConnectClientInitiatedConnection()
{
    if (pSocket_TCPClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "TCP client socket is null", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!pSocket_TCPClient->Connect())
    {
        // LOG_EXCLUDING_SERVER_INFO
        if (pSA_FarEnd != IMS_NULL)
        {
            IMS_TRACE_E(0, "Connecting TCP client socket failed; (peer=%s,%d)",
                    SIPRTConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId()) ? \
                        "xxx" : SIPDebug::GetIP(pSA_FarEnd->GetIPAddress()),
                    pSA_FarEnd->GetPort(), 0);
        }
        else
        {
            IMS_TRACE_E(0, "Connecting TCP client socket failed", 0, 0, 0);
        }

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void SIPConnectionNotifier::DestroyClientInitiatedConnection(IN IMS_SINT32 nPort)
{
    if (pSocket_TCPClient != IMS_NULL)
    {
        SIPTransportHelper *pTransportHelper = GetTransportHelper();

        pTransportHelper->DetachClientInitiatedConnection(pSocket_TCPClient);
        pTransportHelper->Destroy(pSocket_TCPClient, this);

        if ((pSA_FarEnd != IMS_NULL) && SIP::IsPortSpecified(nPort))
        {
            SIPSocketAddress objSA;

            objSA.SetType(SIPSocketAddress::SOCKET_TCP_CLIENT);
            objSA.SetPort(nPort);
            objSA.SetIPAddress(objIPA);

            pTransportHelper->DestroyStreamSocket(objSA, *pSA_FarEnd);
        }

        pSocket_TCPClient = IMS_NULL;
    }
}

/*

Remarks

*/
PRIVATE
void SIPConnectionNotifier::ExtractProperties(IN CONST AString &strParams)
{
    AString strTmp = strParams.Trim();

    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    if (strTmp.GetLength() > 0)
    {
        IMS_TRACE_D("SCN :: params=%s", strTmp.GetStr(), 0, 0);
    }

    objParameters = SIPStack::ExtractParameters(strTmp, TextParser::CHAR_SEMICOLON);

    // MULTI_REG_TRANSPORT
    if (!objParameters.IsEmpty())
    {
        const AString TRANSPORT_EXT(SIP::STR_TRANSPORT_EXT);
        for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
        {
            const SIPParameter *pParameter = objParameters.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                if (pParameter->GetName().EqualsIgnoreCase(TRANSPORT_EXT))
                {
                    IMS_BOOL bOK = IMS_FALSE;

                    nTransportExt = pParameter->GetValue().ToInt32(&bOK);

                    if (!bOK)
                    {
                        nTransportExt = SIP::TRANSPORT_EXT_ANY;
                    }

                    IMS_TRACE_D("SCN :: transportProtocolExt=%02X", nTransportExt, 0, 0);

                    objParameters.RemoveAt(i);
                    break;
                }
            }
        }
    }

    if (objParameters.IsEmpty())
    {
        IMS_TRACE_I("SIPConnectionNotifier - NO PARAMETERS IN NAME (open())", 0, 0, 0);
        return;
    }

    const SIPParameter *pParameter = objParameters.GetAt(0);

    if (pParameter == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::LIST_OPERATION_FAILED);
        return; // throw exception
    }

    IMS_UINT32 i;

    // Look up "type" & "transport" parameters
    for (i = 0; i < objParameters.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParameters.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            if (pParameter->GetName().EqualsIgnoreCase(SIP::STR_TYPE))
            {
                strType = pParameter->GetValue();
                objParameters.RemoveAt(i);
                break;
            }
        }
    }

    if ((strType.GetLength() == 0) && (nMode == SHARED))
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return; // throw exception
    }

    for (i = 0; i < objParameters.GetSize(); ++i)
    {
        const SIPParameter *pParameter = objParameters.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            if (pParameter->GetName().EqualsIgnoreCase(SIPAddress::PARAM_TRANSPORT))
            {
                const AString &strTransport = pParameter->GetValue();

                if (strTransport.EqualsIgnoreCase(SIP::STR_UDP))
                    nTransportProtocol = SIPTransportAddress::PROTOCOL_UDP;
                else if (strTransport.EqualsIgnoreCase(SIP::STR_TCP))
                    nTransportProtocol = SIPTransportAddress::PROTOCOL_TCP;
                else
                {
                    // Both transport scheme MUST be handled.
                }

                objParameters.RemoveAt(i);
                break;
            }
        }
    }
}

/*

Remarks
 RFC5626_FLOW_CONTROL
*/
PRIVATE
SIPTransportHelper* SIPConnectionNotifier::GetTransportHelper() const
{
    return SIPFactoryProxy::GetInstance()->GetTransportHelper(GetSlotId());
}

/*

Remarks
 RFC5626_FLOW_CONTROL
*/
PRIVATE
IMS_BOOL SIPConnectionNotifier::IsClientInitiatedConnectionRequired() const
{
    //---------------------------------------------------------------------------------------------

    return SIP::IsPortSpecified(nPortFlowControl);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPConnectionNotifier::IsIPSecRequired() const
{
    return ((nTransportExt & SIP::TRANSPORT_EXT_IPSEC) != 0);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPConnectionNotifier::IsTCPConnectionOnlyRequired() const
{
    return ((nTransportExt & SIP::TRANSPORT_EXT_TCP_ONLY) != 0);
}

/*

Remarks

*/
PRIVATE
IMS_RESULT SIPConnectionNotifier::RestoreTransportResourceForClientInitiatedConnection(
        IN CONST IPAddress &objPeerIP, IN IMS_SINT32 nPeerPort)
{
    IMS_BOOL bRestorationRequired = IMS_FALSE;
    IMS_SINT32 nClientPort = SIP::PORT_UNSPECIFIED;

    //---------------------------------------------------------------------------------------------

    // MULTI_REG_TRANSPORT
    if (pSocket_TCPClient == IMS_NULL)
    {
        bRestorationRequired = IMS_TRUE;

        if (IsTCPConnectionOnlyRequired())
        {
            nClientPort = nPortC;
        }
        else if (IsClientInitiatedConnectionRequired())
        {
            nClientPort = nPortFlowControl;
        }
    }

    if (bRestorationRequired && (nClientPort != SIP::PORT_UNSPECIFIED))
    {
        IMS_TRACE_I("RestoreTransportResourceForClientInitiatedConnection", 0, 0, 0);

        SIPSocketAddress objSA_FarEnd;

        objSA_FarEnd.SetIPAddress(objPeerIP);
        objSA_FarEnd.SetPort(nPeerPort);
        objSA_FarEnd.SetType(SIPSocketAddress::SOCKET_TCP_CLIENT);

        if (!CreateClientInitiatedConnection(nClientPort, &objSA_FarEnd))
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

/*

Remarks

*/
PRIVATE
IMS_RESULT SIPConnectionNotifier::RestoreTransportResourceForServerConnection()
{
    //---------------------------------------------------------------------------------------------

    if ((pSocket_TCP != IMS_NULL) && (pSocket_UDP != IMS_NULL))
    {
        return IMS_SUCCESS;
    }

    IMS_TRACE_I("RestoreTransportResourceForServerConnection", 0, 0, 0);

    SIPTransportHelper *pTransportHelper = GetTransportHelper();
    IMS_RESULT nResult = IMS_SUCCESS;
    SIPSocketAddress objSA;

    // Check both transport protocol (UDP/TCP)
    objSA.SetIPAddress(objIPA);
    objSA.SetPort(nPort);

    // TCP Server Socket
    if (!IsClientInitiatedConnectionRequired() && (pSocket_TCP == IMS_NULL))
    {
        objSA.SetType(SIPSocketAddress::SOCKET_TCP);

        pSocket_TCP = pTransportHelper->Create(objSA);

        if (pSocket_TCP == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating TCP Server (%s:%d) failed",
                    SIPDebug::GetIP(objIPA), nPort, 0);

            nResult = IMS_FAILURE;
        }
        else
        {
            pSocket_TCP->SetListener(this);
        }
    }

    // UDP Server Socket
    if (pSocket_UDP == IMS_NULL)
    {
        objSA.SetType(SIPSocketAddress::SOCKET_UDP);

        pSocket_UDP = pTransportHelper->Create(objSA);

        if (pSocket_UDP == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating UDP Server (%s:%d) failed",
                    SIPDebug::GetIP(objIPA), nPort, 0);

            nResult = IMS_FAILURE;
        }
        else
        {
            pSocket_UDP->SetListener(this);

            // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
            ControlUDPClientReference(CTRL_DESTROY);
            ControlUDPClientReference(CTRL_CREATE);
        }
    }

    return nResult;
}
