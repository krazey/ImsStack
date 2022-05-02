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
#include "Credential.h"
#include "SIPPrivate.h"
#include "SipDebug.h"
#include "SipMethod.h"
#include "SIPUnknownHeaders.h"
#include "SIPMessage.h"
#include "SIPAuHelper.h"
#include "SipConfigProxy.h"
#include "SIPRTConfigUtils.h"
#include "SIPAckPackage.h"
#include "SIPConnectionNotifier.h"
#include "IOnSIPClientConnectionListener.h"
#include "SIPClientConnection.h"

__IMS_TRACE_TAG_SIP__;



PRIVATE GLOBAL
const AString SIPClientConnection::ANONYMOUS_URI("\"Anonymous\" <sip:thisis@anonymous.invalid>");



PUBLIC
SIPClientConnection::SIPClientConnection()
    : SIPConnection()
    , nState(STATE_CREATED)
    , bACKSent(IMS_FALSE)
    , bResubmissionRequestInitialized(IMS_FALSE)
    , strTargetURI(AString::ConstNull())
    , pCTState(IMS_NULL)
    , pAuHelper(IMS_NULL)
    , piListener(IMS_NULL)
    , pTransmissionProxy(IMS_NULL)
{
    pCTState = new SIPClientTransactionState(GetSlotId());

    pCTState->SetListener(this);
    pCTState->SetTransactionListener(this);
    pCTState->SetTransportListener(this);

    pTransmissionProxy = new SIPClientTransmissionProxy();
    pTransmissionProxy->SetListener(this);
    pTransmissionProxy->SetTransactionState(pCTState.Get());
}

PUBLIC
SIPClientConnection::SIPClientConnection(IN CONST AString &strTargetURI_)
    : SIPConnection()
    , nState(STATE_CREATED)
    , bACKSent(IMS_FALSE)
    , bResubmissionRequestInitialized(IMS_FALSE)
    , strTargetURI(strTargetURI_)
    , pCTState(IMS_NULL)
    , pAuHelper(IMS_NULL)
    , piListener(IMS_NULL)
{
    pCTState = new SIPClientTransactionState(GetSlotId());

    pCTState->SetListener(this);
    pCTState->SetTransactionListener(this);
    pCTState->SetTransportListener(this);

    pTransmissionProxy = new SIPClientTransmissionProxy();
    pTransmissionProxy->SetListener(this);
    pTransmissionProxy->SetTransactionState(pCTState.Get());
}

PUBLIC
SIPClientConnection::SIPClientConnection(IN SIPClientTransactionState *pCTState_)
    : SIPConnection()
    , nState(STATE_CREATED)
    , bACKSent(IMS_FALSE)
    , bResubmissionRequestInitialized(IMS_FALSE)
    , strTargetURI(AString::ConstNull())
    , pCTState(pCTState_)
    , pAuHelper(IMS_NULL)
    , piListener(IMS_NULL)
{
    pCTState->SetListener(this);
    pCTState->SetTransactionListener(this);
    pCTState->SetTransportListener(this);

    pTransmissionProxy = new SIPClientTransmissionProxy();
    pTransmissionProxy->SetListener(this);
    pTransmissionProxy->SetTransactionState(pCTState.Get());
}

PUBLIC VIRTUAL
SIPClientConnection::~SIPClientConnection()
{
    //---------------------------------------------------------------------------------------------

    if (nState == STATE_PROCEEDING)
    {
        pCTState->Abort();
    }

    // FORKED_RESPONSE
    const SIPMethod &objMethod = SIPConnection::GetMethod();

    if (objMethod.Equals(SIPMethod::INVITE) || objMethod.Equals(SIPMethod::ACK))
    {
        pCTState->RemoveForkedTransaction();
    }

    if (pAuHelper != IMS_NULL)
        delete pAuHelper;

    if (!objResponseMessages.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objResponseMessages.GetSize(); ++i)
        {
            SIPMessage *pResponseMessage = objResponseMessages.GetAt(i);

            if (pResponseMessage != IMS_NULL)
                delete pResponseMessage;
        }

        objResponseMessages.Clear();
    }

    pCTState->SetListener(IMS_NULL);
    pCTState->SetTransactionListener(IMS_NULL);
    pCTState->SetTransportListener(IMS_NULL);

    if (pTransmissionProxy != IMS_NULL)
    {
        delete pTransmissionProxy;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPClientConnection::Close()
{
    //---------------------------------------------------------------------------------------------

    if (pTransmissionProxy != IMS_NULL)
    {
        pTransmissionProxy->Abort();
        delete pTransmissionProxy;
        pTransmissionProxy = IMS_NULL;
    }

    if (nState == STATE_PROCEEDING)
    {
        pCTState->Abort();
    }

    // FORKED_RESPONSE
    const SIPMethod &objMethod = SIPConnection::GetMethod();

    if (objMethod.Equals(SIPMethod::INVITE) || objMethod.Equals(SIPMethod::ACK))
    {
        pCTState->RemoveForkedTransaction();
    }

    SetState(STATE_TERMINATED);

    SIPConnection::Close();
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_RESULT SIPClientConnection::AddHeader(IN CONST AString &strName, IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_INITIALIZED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SIPConnection::AddHeader(strName, strValue);
}

/*

Remarks

*/
PUBLIC VIRTUAL
AString SIPClientConnection::GetHeader(IN CONST AString &strName, IN IMS_SINT32 nIndex /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    // Message is not initialized or the connection is closed
    if ((nState == STATE_CREATED)
            || (nState == STATE_TERMINATED))
    {
        SIPPrivate::SetLastError(SIPError::NO_ERROR);
        return AString::ConstNull();
    }

    return SIPConnection::GetHeader(strName, nIndex);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMSList<AString> SIPClientConnection::GetHeaders(IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------

    // Message is not initialized or the connection is closed
    if ((nState == STATE_CREATED)
            || (nState == STATE_TERMINATED))
    {
        SIPPrivate::SetLastError(SIPError::NO_ERROR);
        return IMSList<AString>();
    }

    return SIPConnection::GetHeaders(strName);
}

/*

Remarks

*/
PUBLIC VIRTUAL
const SIPMethod& SIPClientConnection::GetMethod() const
{
    //---------------------------------------------------------------------------------------------

    // Message is not initialized or the connection is closed
    if ((nState == STATE_CREATED)
            || (nState == STATE_TERMINATED))
    {
        return SIPMethod::INVALID_METHOD;
    }

    return SIPConnection::GetMethod();
}

/*

Remarks

*/
PUBLIC VIRTUAL
const AString& SIPClientConnection::GetReasonPhrase() const
{
    //---------------------------------------------------------------------------------------------

    // Status code is available if the state is in PROCEEDING, UNAUTHORIZED, and COMPLETED
    if ((nState != STATE_PROCEEDING)
            && (nState != STATE_UNAUTHORIZED)
            && (nState != STATE_COMPLETED))
    {
        return AString::ConstNull();
    }

    return SIPConnection::GetReasonPhrase();
}

/*

Remarks

*/
PUBLIC VIRTUAL
const AString& SIPClientConnection::GetRequestURI() const
{
    //---------------------------------------------------------------------------------------------

    // Message is not initialized or the connection is closed
    if ((nState == STATE_CREATED)
            || (nState == STATE_TERMINATED))
    {
        return AString::ConstNull();
    }

    return SIPConnection::GetRequestURI();
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SIPClientConnection::GetStatusCode() const
{
    //---------------------------------------------------------------------------------------------

    // Status code is available if the state is in PROCEEDING, UNAUTHORIZED, and COMPLETED
    if ((nState != STATE_PROCEEDING)
            && (nState != STATE_UNAUTHORIZED)
            && (nState != STATE_COMPLETED))
    {
        return 0;
    }

    return SIPConnection::GetStatusCode();
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_RESULT SIPClientConnection::RemoveHeader(IN CONST AString &strName)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_INITIALIZED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SIPConnection::RemoveHeader(strName);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_RESULT SIPClientConnection::Send()
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_INITIALIZED)
            && (nState != STATE_UNAUTHORIZED))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    pTransmissionProxy->SetTimerValues(GetTransactionTimerValues());

    if (nState == STATE_UNAUTHORIZED)
    {
        if (SendWithCredentials() != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "SendWithCredentials() failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }
    else
    {
        // Throw exception - INVALID_MESSAGE if the message format was invalid
        if (!pMessage->FormMessage())
        {
            SIPPrivate::SetLastError(SIPError::INVALID_MESSAGE);
            return IMS_FAILURE;
        }

        // Update the Routing information (Determining the target destination)
        if (!pCTState->UpdateRouteDetails(GetMethod()))
        {
            SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        // Set a Authorization / Proxy-Authorization in the initial request
        if (pAuHelper != IMS_NULL)
        {
            SipMessage *pstMessage = pMessage->GetMessage();

            if (!pAuHelper->FormCredentials(pstMessage))
            {
                SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
                return IMS_FAILURE;
            }

            // If the authentication challenge is present in the INITIALIZED,
            // then removes all the credentials after generating the response
            // to an authentication challenge.
            if (pAuHelper->IsChallengePresent())
            {
                IMS_TRACE_D("Authorization/Proxy-Authorization in the initial request; " \
                        "The credentials will be removed...", 0, 0, 0);

                pAuHelper->RemoveAllCredentials();
            }
        }

        if (!pCTState->FormMessage())
        {
            IMS_TRACE_E(0, "FormMessage() failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        IMS_RESULT nResult = pTransmissionProxy->Send();

        if (nResult == SIPClientTransmissionProxy::RESULT_NOK)
        {
            IMS_TRACE_E(0, "Send() failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        // Update the state
        if (pMessage->GetMethod().Equals(SIPMethod::ACK))
        {
            bACKSent = IMS_TRUE;
        }

        if (nResult == SIPClientTransmissionProxy::RESULT_OK)
        {
            // Update a dialog info...
            if (SIPDialogBase::IsDialogCreatable(GetMethod()))
            {
                if (pDialog != IMS_NULL)
                {
                    pDialog->UpdateDialog(pCTState->GetDialog());
                }
            }
        }
    }

    // Update the state
    if (SIPConnection::GetMethod().Equals(SIPMethod::ACK))
    {
        SetState(STATE_COMPLETED);
    }
    else
    {
        SetState(STATE_PROCEEDING);
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    //// DEBUG
    if (!SIPConnection::GetMethod().Equals(SIPMethod::REGISTER))
    {
        SIPDebug::Send(GetSlotId(), SIPDebug::MSG_REQ,
                SIPDebug::DIR_OUT, SIPConnection::GetMethod().ToInt());
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_RESULT SIPClientConnection::SetHeader(IN CONST AString &strName, IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_INITIALIZED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SIPConnection::SetHeader(strName, strValue);
}

/*

Remarks

*/
PUBLIC VIRTUAL
const ByteArray& SIPClientConnection::GetContent() const
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_PROCEEDING)
            && (nState != STATE_COMPLETED))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return ByteArray::ConstNull();
    }

    return SIPConnection::GetContent();
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_RESULT SIPClientConnection::SetContent(IN CONST ByteArray &objContent)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_INITIALIZED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    return SIPConnection::SetContent(objContent);
}

// IMS extensions
/*

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SIPClientConnection::GetHeaderCount(IN CONST AString &strName) const
{
    //---------------------------------------------------------------------------------------------

    // Message is not initialized or the connection is closed
    if ((nState == STATE_CREATED)
            || (nState == STATE_TERMINATED))
    {
        return 0;
    }

    return SIPConnection::GetHeaderCount(strName);
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC VIRTUAL
void SIPClientConnection::SetSIPProfile(IN SIPProfile *pProfile)
{
    //---------------------------------------------------------------------------------------------

    if (!pCTState.IsNull())
    {
        pCTState->SetSIPProfile(pProfile);
    }
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::InitAck()
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_COMPLETED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    // If ACK request is already sent, throw INVALID_OPERATION
    if (bACKSent)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // If the method of this transaction is not INVITE, throw INVALID_OPERATION
    if (!SIPConnection::GetMethod().Equals(SIPMethod::INVITE))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    IMS_SINT32 nStatusCode = GetStatusCode();

    if ((nStatusCode < SIPStatusCode::SC_200)
            || (nStatusCode >= SIPStatusCode::SC_300))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    InitMessage();

    pCTState->UpdateMessage(pMessage->GetMessage());

    // Method
    pMessage->SetMethod(SIPMethod(SIPMethod::ACK));

    if (!pCTState->InitRequest(GetMethod(), IMS_NULL))
    {
        SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Update the Request-URI if there are any changes
    //if (pCTState->IsTargetUpdated())
    pMessage->UpdateRequestURI();

    SetState(STATE_INITIALIZED);

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
SIPClientConnection* SIPClientConnection::InitCancel()
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_PROCEEDING)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_NULL;
    }

    if (!SIPConnection::GetMethod().Equals(SIPMethod::INVITE))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_OPERATION);
        return IMS_NULL;
    }

    SIPClientConnection *pCANCEL = new SIPClientConnection(strTargetURI);

    if (pCANCEL == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::CONNECTION_NOT_FOUND);
        return IMS_NULL;
    }

    pCANCEL->InitMessage();

    pCANCEL->pCTState->UpdateMessage(pCANCEL->pMessage->GetMessage());

    if (!pCANCEL->pCTState->InitCancel(pCTState.Get()))
    {
        pCANCEL->Close();

        SIPPrivate::SetLastError(SIPError::CONNECTION_NOT_FOUND);
        return IMS_NULL;
    }

    pCANCEL->pMessage->SetMethod(SIPMethod(SIPMethod::CANCEL));

    // Update the Request-URI if there are any changes
    //if (pCANCEL->pCTState->IsTargetUpdated())
    pCANCEL->pMessage->UpdateRequestURI();

    pCANCEL->SetState(STATE_INITIALIZED);

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return pCANCEL;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::InitRequest(IN CONST AString &strMethod,
        IN SIPConnectionNotifier *pSCN)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_CREATED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    SIPMethod objMethod(strMethod);

    if (objMethod.Equals(SIPMethod::ACK)
            || objMethod.Equals(SIPMethod::CANCEL))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if ((objMethod.Equals(SIPMethod::BYE))
            || (objMethod.Equals(SIPMethod::NOTIFY))
            || (objMethod.Equals(SIPMethod::PRACK))
            || (objMethod.Equals(SIPMethod::UPDATE)))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    /*
     * The following headers are set by the system :
     *        To
     *        From
     *        CSeq
     *        Call-ID
     *        Max-Forwards
     *        Via
     *        Contact
     */

    InitMessage();

    pCTState->UpdateMessage(pMessage->GetMessage());

    if (!pCTState->InitRequest(objMethod))
    {
        SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Method
    pMessage->SetMethod(objMethod);

    // Request-URI
    pMessage->SetRequestURI(strTargetURI);

    // From & Contact header
    // If SCN is not shared, From & Contact will be set by the application (J180 Engine)
    // From header
    if (pSCN != IMS_NULL)
        pMessage->SetHeader(ISIPHeader::FROM, pSCN->GetUserIdentity());
    else
        pMessage->SetHeader(ISIPHeader::FROM, ANONYMOUS_URI);

    // To header
    pMessage->SetHeader(ISIPHeader::TO, strTargetURI);

    // Call-ID : after setting PD connection ??? ---> It will be set in the transport layer

    // Contact header if SCN exists
    if (objMethod.Equals(SIPMethod::REGISTER)
            || objMethod.Equals(SIPMethod::INVITE)
            || objMethod.Equals(SIPMethod::SUBSCRIBE)
            || objMethod.Equals(SIPMethod::REFER))
    {
        if (pSCN != IMS_NULL)
            pMessage->SetHeader(ISIPHeader::CONTACT_NORMAL, pSCN->GetContactAddress());
    }

    // Create a dialog if the dialog can be created
    if (SIPDialogBase::IsDialogCreatable(objMethod))
    {
        pDialog = new SIPDialog(pCTState->GetDialog());
    }

    SetState(STATE_INITIALIZED);

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::Receive(IN IMS_SLONG /* nTimeout = 0 */)
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_PROCEEDING)
            && (nState != STATE_COMPLETED))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if (objResponseMessages.IsEmpty())
    {
        SIPPrivate::SetLastError(SIPError::NO_MESSAGE);
        return IMS_FAILURE;
    }

    SIPMessage *pSIPMsg = objResponseMessages.GetAt(0);

    if (pSIPMsg == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::NO_MESSAGE);
        return IMS_FAILURE;
    }

    // Initialize SIP connection with new response message
    InitMessage(pSIPMsg, SIPMessage::TYPE_RESPONSE);
    pCTState->UpdateMessage(pSIPMsg->GetMessage());

    objResponseMessages.RemoveAt(0);

    IMS_SINT32 nStatusCode = SIPConnection::GetStatusCode();

    if ((nStatusCode == SIPStatusCode::SC_401)
            || (nStatusCode == SIPStatusCode::SC_407))
    {
        SetState(STATE_UNAUTHORIZED);
    }
    else if ((nStatusCode >= SIPStatusCode::SC_100)
            && (nStatusCode < SIPStatusCode::SC_200))
    {
        SetState(STATE_PROCEEDING);
    }
    else
    {
        SetState(STATE_COMPLETED);
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::SetCredentials(IN IMSList<Credential> &objCredentials)
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_INITIALIZED)
            && (nState != STATE_UNAUTHORIZED))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if (objCredentials.IsEmpty())
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (pAuHelper == IMS_NULL)
    {
        pAuHelper = new SIPAuHelper();
    }

    if (pAuHelper == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    for (IMS_UINT32 i = 0; i < objCredentials.GetSize(); ++i)
    {
        if (!pAuHelper->AddCredential(objCredentials.GetAt(i)))
        {
            SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
            return IMS_FAILURE;
        }
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::SetCredentials(IN CONST Credential &objCredential)
{
    //---------------------------------------------------------------------------------------------

    if ((nState != STATE_INITIALIZED)
            && (nState != STATE_UNAUTHORIZED))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    IMS_BOOL bPasswordLogging = IMS_TRUE;

    if (SIPRTConfigUtils::IsMessageHiddenInLog(GetSlotId()))
    {
        bPasswordLogging = IMS_FALSE;
    }
    else if ((objCredential.GetType() == Credential::TYPE_AKAv1_MD5)
            || (objCredential.GetType() == Credential::TYPE_AKAv2_MD5))
    {
        bPasswordLogging = IMS_FALSE;
    }

    // LOG_EXCLUDING_SERVER_INFO
    IMS_TRACE_D("Credential :: UserName (%s), Password (%s), Realm (%s)",
            SIPDebug::GetCharA1(objCredential.GetUsername().GetStr(), 6),
            bPasswordLogging ? objCredential.GetPassword().GetStr() : "xxx",
            SIPDebug::GetCharA2(objCredential.GetRealm().GetStr(), 4));

    if ((objCredential.GetType() == Credential::TYPE_AKAv1_MD5)
            || (objCredential.GetType() == Credential::TYPE_AKAv2_MD5))
    {
        // In case of AKA authentication, the password can be an empty (AUTS/MAC failure)
        if ((objCredential.GetUsername().GetLength() == 0)
                || (objCredential.GetRealm().GetLength() == 0))
        {
            SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
            return IMS_FAILURE;
        }
    }
    else
    {
        if ((objCredential.GetUsername().GetLength() == 0)
                || (objCredential.GetPassword().GetLength() == 0)
                || (objCredential.GetRealm().GetLength() == 0))
        {
            SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
            return IMS_FAILURE;
        }
    }

    if (pAuHelper == IMS_NULL)
    {
        pAuHelper = new SIPAuHelper();
    }

    if (pAuHelper == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (!pAuHelper->AddCredential(objCredential))
    {
        SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void SIPClientConnection::SetListener(IN IOnSIPClientConnectionListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::SetRequestURI(IN CONST AString &strURI)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_INITIALIZED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    strTargetURI = strURI;

    return pMessage->SetRequestURI(strURI);
}

/*

Remarks

*/
PUBLIC
ISIPGenericChallenge* SIPClientConnection::GetAuthenticationChallenge(
        IN IMS_SINT32 nIndex /* = 0 */) const
{
    //---------------------------------------------------------------------------------------------

    if (pAuHelper == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pAuHelper->GetChallenge(nIndex);
}

/*

Remarks

*/
PUBLIC
ISIPAckPackage* SIPClientConnection::GrabAck()
{
    //---------------------------------------------------------------------------------------------

    if (pDialog == IMS_NULL)
    {
        return IMS_NULL;
    }

    SIPAckPackage *pAckPackage = SIPAckPackage::CreateAckPackage(pDialog->GetCallId());

    if (pAckPackage != IMS_NULL)
    {
        IMS_BOOL bSipConfigRequired = IMS_TRUE;
        IMS_SINT32 nAliveInterval = 2000 * 64;
        SIPTimerValues *pTVs = GetTransactionTimerValues();

        if (pTVs != IMS_NULL)
        {
            IMS_SINT32 nTempTV = pTVs->GetValue(SIPTimerValues::TV_TIMER_H);

            if (nTempTV > 0)
            {
                bSipConfigRequired = IMS_FALSE;
                nAliveInterval = nTempTV;
            }
            else
            {
                nTempTV = pTVs->GetValue(SIPTimerValues::TIMER_T1);

                if (nTempTV > 0)
                {
                    bSipConfigRequired = IMS_FALSE;
                    nAliveInterval = nTempTV * 64;
                }
            }
        }

        if (bSipConfigRequired)
        {
            IMS_SINT32 nTV_T1 = SIPConfigProxy::GetTimerValueT1(
                    GetSlotId(), pCTState->GetSIPProfile());

            if (nTV_T1 > 0)
            {
                nAliveInterval = nTV_T1 * 64;
            }
        }

        pAckPackage->AddAck(pCTState.Get(), nAliveInterval);
    }

    return pAckPackage;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::InitResubmissionRequest()
{
    //---------------------------------------------------------------------------------------------

    if (bResubmissionRequestInitialized)
    {
        IMS_TRACE_E(0, "Resubmission request message is already initialized", 0, 0, 0);
        return IMS_SUCCESS;
    }

    if (nState != STATE_UNAUTHORIZED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    // Form a new request w/ credentials and sent it to the network.
    SIPMessage *pSIPMsg = new SIPMessage(pCTState->GetLastMessage());

    InitMessage(pSIPMsg, SIPMessage::TYPE_REQUEST);

    pCTState->UpdateMessage(pSIPMsg->GetMessage());

    bResubmissionRequestInitialized = IMS_TRUE;

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void SIPClientConnection::RemoveAllChallenges()
{
    //---------------------------------------------------------------------------------------------

    if (pAuHelper == IMS_NULL)
        return;

    pAuHelper->RemoveAllChallenges();
}

/*

Remarks

*/
PUBLIC
void SIPClientConnection::RemoveAllCredentials()
{
    //---------------------------------------------------------------------------------------------

    if (pAuHelper == IMS_NULL)
        return;

    pAuHelper->RemoveAllCredentials();
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::SetAuthenticationChallenge(IN ISIPGenericChallenge *piChallenge)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_INITIALIZED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    if (piChallenge == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (pAuHelper == IMS_NULL)
    {
        pAuHelper = new SIPAuHelper();

        if (pAuHelper == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating SIPAuHelper failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (!pAuHelper->AddChallenge(piChallenge))
    {
        IMS_TRACE_E(0, "Adding an authentication challenge failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void SIPClientConnection::SetExtensionTokenForViaBranch(IN CONST AString &strToken)
{
    //---------------------------------------------------------------------------------------------

    pCTState->SetExtensionTokenForViaBranch(strToken);
}

/*

Remarks

*/
PUBLIC
void SIPClientConnection::SetImplicitRouteHeader(IN CONST AString &strRouteHeader)
{
    //---------------------------------------------------------------------------------------------

    pCTState->SetImplicitRouteHeader(strRouteHeader);
}

/*

Remarks
 RFC5626_FLOW_CONTROL, MULTI_REG_TRANSPORT
*/
PUBLIC
void SIPClientConnection::SetTransportTuple(IN CONST IPAddress &objIPA,
        IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFC /* = 0xFFFF */,
        IN IMS_SINT32 nTransportExt /* = 0 (ANY) */)
{
    //---------------------------------------------------------------------------------------------

    pCTState->SetTransportTuple(objIPA, nPortS, nPortC, nPortFC, nTransportExt);
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::InitDialogRequest(IN CONST SIPMethod &objMethod,
        IN SIPDialogEx *pDialogEx)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_CREATED)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_STATE);
        return IMS_FAILURE;
    }

    InitMessage();

    pCTState->UpdateMessage(pMessage->GetMessage());

    if (!pCTState->InitRequest(objMethod, pDialogEx))
        return IMS_FAILURE;

    pMessage->SetMethod(objMethod);
    pMessage->UpdateRequestURI();

    // Create a dialog
    pDialog = new SIPDialog(pDialogEx);

    if (pDialog == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::NO_MEMORY);
        return IMS_FAILURE;
    }

    SetState(STATE_INITIALIZED);

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnection::SendWithCredentials()
{
    //---------------------------------------------------------------------------------------------

    if (pAuHelper == IMS_NULL)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // Check later; if the credentials is not present,
    // it needs to be sent with "anonymous" & empty password.
    if (!pAuHelper->IsCredentialPresent())
    {
        SIPPrivate::SetLastError(SIPError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    if (!bResubmissionRequestInitialized)
    {
        if (InitResubmissionRequest() != IMS_SUCCESS)
        {
            SIPPrivate::SetLastError(SIPError::INVALID_OPERATION);
            return IMS_FAILURE;
        }
    }

    SipMessage *pstMessage = pMessage->GetMessage();

    if (!pAuHelper->FormCredentials(pstMessage))
    {
        SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (!pMessage->FormMessageOnChallenge())
    {
        SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (!pCTState->FormMessageForResubmissionRequest())
    {
        SIPPrivate::SetLastError(SIPError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    IMS_RESULT nResult = pTransmissionProxy->SendWithCredentials();

    if (nResult == SIPClientTransmissionProxy::RESULT_NOK)
    {
        SIPPrivate::SetLastError(SIPError::TRANSPORT_ERROR);
        return IMS_FAILURE;
    }

    if (nResult == SIPClientTransmissionProxy::RESULT_OK)
    {
        // Update a dialog info...
        if (SIPDialogBase::IsDialogCreatable(GetMethod()))
        {
            if (pDialog != IMS_NULL)
            {
                pDialog->UpdateDialog(pCTState->GetDialog());
            }
        }
    }

    bResubmissionRequestInitialized = IMS_FALSE;

    SIPPrivate::SetLastError(SIPError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPClientConnection::ClientTransactionState_ForkedResponseReceived(
        IN SIPClientTransactionState *pCTState)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "No listener", 0, 0, 0);
        return;
    }

    SIPClientConnection *pSCC = new SIPClientConnection(pCTState);

    IMS_TRACE_D("SCC :: Handling a forked response ...", 0, 0, 0);

    if (pSCC == IMS_NULL)
    {
        return;
    }

    // Copy the transaction timer values
    SIPTimerValues *pTVs = GetTransactionTimerValues();

    if (pTVs != IMS_NULL)
    {
        pSCC->SetTransactionTimerValues(*pTVs);
    }

    pSCC->strTargetURI = strTargetURI;

    SipMessage *pstMessage = pCTState->GetMessage();
    SIPMethod objMethod = SIPStack::GetMethod(pstMessage);

    if (SIPDialogBase::IsDialogCreatable(objMethod))
    {
        pSCC->pDialog = new SIPDialog(pCTState->GetDialog());
    }

    // Set a SIP message from the previous request message
    SIPMessage *pSIPMsg = new SIPMessage(pCTState->GetLastMessage());

    if (pSIPMsg == IMS_NULL)
    {
        delete pSCC;
        return;
    }

    pSCC->InitMessage(pSIPMsg);

    // Append the response message
    pSIPMsg = new SIPMessage(pstMessage);

    if (pSIPMsg == IMS_NULL)
    {
        delete pSCC;
        return;
    }

    if (!pSCC->objResponseMessages.Append(pSIPMsg))
    {
        delete pSIPMsg;
        delete pSCC;
        return;
    }

    pSCC->SetState(STATE_PROCEEDING);

    //// DEBUG
    if (!SIPConnection::GetMethod().Equals(SIPMethod::REGISTER))
    {
        SIPDebug::Send(GetSlotId(), SIPDebug::MSG_RSP, SIPDebug::DIR_IN,
                SIPConnection::GetMethod().ToInt(), pSIPMsg->GetStatusCode());
    }

    // Notify the forked response to the application
    piListener->OnClientConnection_NotifyForkedResponse(this, pSCC);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPClientConnection::ClientTransactionState_ResponseReceived(IN SipMessage *pstMessage)
{
    IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

    //---------------------------------------------------------------------------------------------

    //// DEBUG
    if (!SIPConnection::GetMethod().Equals(SIPMethod::REGISTER))
    {
        SIPDebug::Send(GetSlotId(), SIPDebug::MSG_RSP,
                SIPDebug::DIR_IN, SIPConnection::GetMethod().ToInt(), nStatusCode);
    }

    if ((nStatusCode == SIPStatusCode::SC_401) || (nStatusCode == SIPStatusCode::SC_407))
    {
        if (pAuHelper != IMS_NULL)
        {
            pAuHelper->SetChallenges(pstMessage);

            if (pAuHelper->IsCredentialPresent())
            {
                IMS_TRACE_D("___ AUTHENTICATION IS PROCESSED BY SIP STACK ...", 0, 0, 0);

                SetState(STATE_UNAUTHORIZED);

                if (Send() != IMS_SUCCESS)
                {
                    NotifyError(SIPError::AUTHENTICATION_FAILED, AString("Authentication failed"));
                }
                return;
            }
        }
        else
        {
            IMS_TRACE_D("Authentication helper does not exist...", 0, 0, 0);

            pAuHelper = new SIPAuHelper();

            if (pAuHelper == IMS_NULL)
            {
                NotifyError(SIPError::AUTHENTICATION_FAILED, AString("Authentication failed"));
                return;
            }

            pAuHelper->SetChallenges(pstMessage);
        }
    }

    SIPMessage *pSIPMsg = new SIPMessage(pstMessage);

    if (pSIPMsg == IMS_NULL)
    {
        return;
    }

    if (!objResponseMessages.Append(pSIPMsg))
    {
        delete pSIPMsg;
        return;
    }

    // Notify the response to the application
    if (piListener != IMS_NULL)
    {
        piListener->OnClientConnection_NotifyResponse(this);
    }
}

/*

Remarks
 SIP_TRANSPORT_ERROR_REPORT_ON_TXN
*/
PROTECTED VIRTUAL
IMS_BOOL SIPClientConnection::IsTransportErrorReportRequired(IN IMS_SINT32 nCode,
        IN CONST AString &strMessage) const
{
    //---------------------------------------------------------------------------------------------

    if (nCode == SIPError::TRANSPORT_ERROR)
    {
        AString strTECode;

        strTECode.Sprintf("%d", SIPError::TRANSPORT_E_CODE_104);

        // SIPError::TRANSPORT_E_CODE_104
        if (strMessage.StartsWith(strTECode))
        {
            if ((nState == STATE_PROCEEDING)
                    && SIPConfigProxy::IsTransportErrorReportOnTxnRequired(
                            GetSlotId(), pCTState->GetSIPProfile()))
            {
                // Notifies "Socket is closed by peer"(104) error
                // if the SIP transaction is in progress
                return IMS_TRUE;
            }
        }
    }

    return SIPConnection::IsTransportErrorReportRequired(nCode, strMessage);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPClientConnection::ClientTransmission_NotifyError(
        IN IMS_SINT32 nCode, IN CONST AString &strMessage)
{
    //---------------------------------------------------------------------------------------------

    if (pTransmissionProxy == IMS_NULL)
    {
        return;
    }

    TransportError_NotifyError(nCode, strMessage);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPClientConnection::ClientTransmission_TransmissionCompleted()
{
    //---------------------------------------------------------------------------------------------

    if (pTransmissionProxy == IMS_NULL)
    {
        return;
    }

    // Update a dialog info...
    if (SIPDialogBase::IsDialogCreatable(GetMethod()))
    {
        if (pDialog != IMS_NULL)
        {
            pDialog->UpdateDialog(pCTState->GetDialog());
        }
    }
}

/*

Remarks

*/
PRIVATE
void SIPClientConnection::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("SCC :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE GLOBAL
const IMS_CHAR* SIPClientConnection::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
    case STATE_CREATED:
        return "STATE_CREATED";
    case STATE_INITIALIZED:
        return "STATE_INITIALIZED";
    case STATE_PROCEEDING:
        return "STATE_PROCEEDING";
    case STATE_COMPLETED:
        return "STATE_COMPLETED";
    case STATE_UNAUTHORIZED:
        return "STATE_UNAUTHORIZED";
    case STATE_TERMINATED:
        return "STATE_TERMINATED";
    default:
        return "__INVALID__";
    }
}
