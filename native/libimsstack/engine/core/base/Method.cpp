/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090729  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"

// IMS_AUTH_SIP_DIGEST
#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"
#include "Credential.h"
#include "ISipGenericChallenge.h"

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "Sip.h"
#include "SipDebug.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "base/IMS.h"
#include "base/SubscriberTracker.h"
#include "base/Method.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
Method::Method()
    : EngineActivity()
    , bFlag_MobileOriginated(IMS_TRUE)
    , pUserAOR(IMS_NULL)
    , pRemoteUserAOR(IMS_NULL)
    , objRemoteUserIds(IMSList<AString>())
    , piDialog(IMS_NULL)
    // AUTH_SIP_DIGEST
    , piAuthChallenge(IMS_NULL)
    , objAuthChallengeMap(IMSMap<IMS_SINT32, IMS_SINT32>())
    // SIP_MESSAGE_MEDIATOR
    , piMessageMediator(IMS_NULL)
{
    // AUTH_SIP_DIGEST {
    objAuthChallengeMap.Add(SIPMethod::BYE, 0);
    objAuthChallengeMap.Add(SIPMethod::CANCEL, 0);
    objAuthChallengeMap.Add(SIPMethod::INVITE, 0);
    objAuthChallengeMap.Add(SIPMethod::OPTIONS, 0);
    objAuthChallengeMap.Add(SIPMethod::PRACK, 0);
    objAuthChallengeMap.Add(SIPMethod::SUBSCRIBE, 0);
    objAuthChallengeMap.Add(SIPMethod::NOTIFY, 0);
    objAuthChallengeMap.Add(SIPMethod::UPDATE, 0);
    objAuthChallengeMap.Add(SIPMethod::MESSAGE, 0);
    objAuthChallengeMap.Add(SIPMethod::REFER, 0);
    objAuthChallengeMap.Add(SIPMethod::PUBLISH, 0);
    objAuthChallengeMap.Add(SIPMethod::INFO, 0);
    // }
}

PUBLIC VIRTUAL
Method::~Method()
{
    if (pUserAOR != IMS_NULL)
        delete pUserAOR;

    if (pRemoteUserAOR != IMS_NULL)
        delete pRemoteUserAOR;

    DestroyDialog();

    if (piAuthChallenge != IMS_NULL)
    {
        piAuthChallenge->Destroy();
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Method::Destroy()
{
    //---------------------------------------------------------------------------------------------

    PostMessage(AMSG_DESTROY, 0, 0);
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PUBLIC VIRTUAL
void Method::SetMessageMediator(IN IMessageMediator *piMediator)
{
    //---------------------------------------------------------------------------------------------

    piMessageMediator = piMediator;
}

/*
 When any error occurs in the Service, the Service notifies the error to the specific Method.

Remarks

*/
PUBLIC VIRTUAL
void Method::Exception_NotifyError(IN IMS_SINT32 /* nErrorCode */)
{
    //---------------------------------------------------------------------------------------------

    // The subclass MUST implement this method if the error needs to be handled.

}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL Method::SetReferredMessageListener(IN IReferredMessageListener * /* piListener */)
{
    //---------------------------------------------------------------------------------------------

    // The subclass MUST implement this method if the referred message needs to be handled.

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Method::Equals(IN CONST Method *pMethod) const
{
    //---------------------------------------------------------------------------------------------

    if (pMethod == IMS_NULL)
        return IMS_FALSE;

    if (!GetName().Equals(pMethod->GetName()))
        return IMS_FALSE;

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Method::InitMethod(IN CONST AString &strFrom, IN CONST AString &strTo,
        IN CONST SIPAddress &objUserAOR, IN IMS_BOOL bMobileOriginated /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    bFlag_MobileOriginated = bMobileOriginated;

    // From header
    if (strFrom.IsNULL())
    {
        pUserAOR = new SIPAddress(objUserAOR);

        if (pUserAOR == IMS_NULL)
        {
            IMS::SetLastError(IMSError::NO_MEMORY);
            return IMS_FALSE;
        }
    }
    else
    {
        pUserAOR = new SIPAddress();

        if (pUserAOR == IMS_NULL)
        {
            IMS::SetLastError(IMSError::NO_MEMORY);
            return IMS_FALSE;
        }

        if (!pUserAOR->Create(strFrom))
        {
            IMS::SetLastError(IMSError::PARSING_ERROR);
            return IMS_FALSE;
        }
    }

    // To header
    if (strTo.IsNULL())
    {
        pRemoteUserAOR = new SIPAddress(objUserAOR);

        if (pRemoteUserAOR == IMS_NULL)
        {
            IMS::SetLastError(IMSError::NO_MEMORY);
            return IMS_FALSE;
        }
    }
    else
    {
        pRemoteUserAOR = new SIPAddress();

        if (pRemoteUserAOR == IMS_NULL)
        {
            IMS::SetLastError(IMSError::NO_MEMORY);
            return IMS_FALSE;
        }

        if (!pRemoteUserAOR->Create(strTo))
        {
            IMS::SetLastError(IMSError::PARSING_ERROR);
            return IMS_FALSE;
        }
    }

    return InitInstance();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Method::InitMethod(IN CONST Method *pMethod,
        IN IMS_BOOL bMobileOriginated /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    if (pMethod == IMS_NULL)
    {
        return IMS_FALSE;
    }

    bFlag_MobileOriginated = bMobileOriginated;

    // From header
    pUserAOR = new SIPAddress(*(pMethod->pUserAOR));

    if (pUserAOR == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_FALSE;
    }

    // To header
    pRemoteUserAOR = new SIPAddress(*(pMethod->pRemoteUserAOR));

    if (pRemoteUserAOR == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_FALSE;
    }

    // To send/receive a request inside of a dialog
    if (pMethod->piDialog != IMS_NULL)
    {
        if (piDialog != IMS_NULL)
        {
            piDialog->Destroy();
        }

        piDialog = pMethod->piDialog->Clone();
    }

    return InitInstance();
}

/*

Remarks

*/
PUBLIC
ISIPDialog* Method::GetDialog() const
{
    //---------------------------------------------------------------------------------------------

    return piDialog;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Method::ServerConnection_NotifyRequest(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    if (piSSC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("Method - \"%s\" REQUEST RECEIVED", piSSC->GetMethod().ToString().GetStr(), 0, 0);

    piSSC->SetErrorListener(this);

    // Update a remote user identities
    UpdateRemoteUserIds(piSSC);

    return NotifySIPRequest(piSSC);
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Method::InitInstance()
{
    //---------------------------------------------------------------------------------------------

    // The subclass MUST implement this method if an additional initialization work needs.

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Method::NotifySIPRequest(IN ISIPServerConnection * /* piSSC */)
{
    //---------------------------------------------------------------------------------------------

    // The subclass MUST implement this method if an incoming SIP request needs to be handled.

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Method::NotifySIPForkedResponse(IN ISIPClientConnection *piSCC,
        IN ISIPClientConnection *piForkedSCC)
{
    //---------------------------------------------------------------------------------------------

    (void) piSCC;

    // The subclass MUST implement this method if a SIP forked response needs to be handled.

    IMS_TRACE_E(0, "SIP forked response (%s) is not handled",
        piForkedSCC->GetMethod().ToString().GetStr(), 0, 0);

    piForkedSCC->Close();

    return IMS_FALSE;
}

/*

Remarks
 MULTI_SUBS
*/
PROTECTED VIRTUAL
const AString& Method::GetSubscriberId() const
{
    //---------------------------------------------------------------------------------------------

    return AString::ConstNull();
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PROTECTED VIRTUAL
IMS_BOOL Method::SendRequestToChallenge(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    // The subclass MAY insert the specific headers for each SIP method

    // SIP_MESSAGE_MEDIATOR
    (void) AdjustMessage(piSCC->GetMessage(), MESSAGE_CLASS_RESUBMIT);

    if (piSCC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending the resubmission request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*
 This method is invoked before sending the SIP message to the network
to adjust the specified message.

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PROTECTED
IMS_RESULT Method::AdjustMessage(IN_OUT ISIPMessage *piSIPMsg,
        IN IMS_SINT32 nMessage /* = MESSAGE_CLASS_NORMAL */)
{
    if (piMessageMediator == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return piMessageMediator->MessageMediator_AdjustMessage(piSIPMsg, nMessage);
}

/*

Remarks

*/
PROTECTED
void Method::CheckNCreateDialog(IN ISIPConnection *piSC, IN IMS_BOOL bDestroy /* = IMS_FALSE */,
        IN IMS_BOOL bTerminatedDialogRequired /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (bDestroy)
    {
        DestroyDialog();
    }

    if (piDialog == IMS_NULL)
    {
        ISIPDialog *piTmpDlg = piSC->GetDialog();

        if ((piTmpDlg->GetState() == ISIPDialog::STATE_EARLY)
                || (piTmpDlg->GetState() == ISIPDialog::STATE_CONFIRMED))
        {
            piDialog = DYNAMIC_CAST(ISIPDialog*, piTmpDlg->Clone());

            IMS_TRACE_D("___ DIALOG IS CREATED BY %s ___",
                    piSC->GetMethod().ToString().GetStr(), 0, 0);
        }
        else if (bTerminatedDialogRequired
                && (piTmpDlg->GetState() == ISIPDialog::STATE_TERMINATED))
        {
            piDialog = DYNAMIC_CAST(ISIPDialog*, piTmpDlg->Clone());

            IMS_TRACE_D("___ DIALOG IS CREATED BY %s IN TERMINATED STATE ___",
                    piSC->GetMethod().ToString().GetStr(), 0, 0);
        }
    }
}

/*

Remarks

*/
PROTECTED
void Method::DestroyDialog()
{
    //---------------------------------------------------------------------------------------------

    if (piDialog != IMS_NULL)
    {
        piDialog->Destroy();
        piDialog = IMS_NULL;
    }
}

/*

Remarks

*/
PROTECTED
const SIPAddress* Method::GetUserAOR() const
{
    //---------------------------------------------------------------------------------------------

    return pUserAOR;
}

/*

Remarks

*/
PROTECTED
const SIPAddress* Method::GetRemoteUserAOR() const
{
    //---------------------------------------------------------------------------------------------

    return pRemoteUserAOR;
}

/*

Remarks

*/
PROTECTED
const IMSList<AString>& Method::GetRemoteUserIds() const
{
    //---------------------------------------------------------------------------------------------

    return objRemoteUserIds;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Method::HandleAllSIPResponse(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    do
    {
        if (piSCC->Receive() != IMS_SUCCESS)
        {
            if (SIPError::GetLastError() == SIPError::NO_MESSAGE)
            {
                IMS_TRACE_I("No more messages in this client transaction", 0, 0, 0);
                break;
            }

            IMS_TRACE_E(0, "Receive() :: SIPError (%d)", SIPError::GetLastError(), 0, 0);
            return IMS_FALSE;
        }

        ISIPMessage *piSIPMsg = piSCC->GetMessage();

        if (piSIPMsg == IMS_NULL)
        {
            IMS_TRACE_E(0, "SIP message does not exist", 0, 0, 0);
            return IMS_FALSE;
        }

        // Parse the message body if it is a multipart body
        if (!SIPParsingHelper::CreateMessageBodyParts(piSIPMsg))
        {
            IMS_TRACE_E(0, "Parsing a message body part failed", 0, 0, 0);

            Error_NotifyError(piSCC,
                    SIPError::PARSING_ERROR, AString("Parsing Error :: message body part"));
            return IMS_FALSE;
        }

        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        IMS_TRACE_I("___ %d response to %s request is received ...",
            nStatusCode, piSIPMsg->GetMethod().ToString().GetStr(), 0);

        // Update the remote user identities if present
        if (SIPStatusCode::IsProvisional(nStatusCode)
                || SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            UpdateRemoteUserIds(piSCC);
        }

        // All the subclass of this class MUST implement the below method.
        NotifySIPResponse(piSCC);

        if (nStatusCode >= SIPStatusCode::SC_200)
        {
            // After the final response is received and handled,
            // the SCC does not contain any response messages in the queue.
            break;
        }
    } while (IMS_TRUE);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Method::IsMobileOriginated() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_MobileOriginated;
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PROTECTED
void Method::ResetChallengeCount(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "SCC is null", 0, 0, 0);
        return;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    if (piSIPMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "No SIP message", 0, 0, 0);
        return;
    }

    if (SIPStatusCode::IsFinalSuccess(piSIPMsg->GetStatusCode()))
    {
        const SIPMethod &objMethod = piSIPMsg->GetMethod();
        IMS_SLONG nIndex = objAuthChallengeMap.GetIndexOfKey(objMethod.ToInt());

        if (nIndex >= 0)
        {
            IMS_SINT32 &nAuthChallengeCount = objAuthChallengeMap.GetValueAt(nIndex);

            if (nAuthChallengeCount > 0)
            {
                IMS_TRACE_I("Authentication challenge count (%d) will be reset",
                        nAuthChallengeCount, 0, 0);
                nAuthChallengeCount = 0;
            }
        }
    }
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Method::RespondToChallenge(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "SCC is null", 0, 0, 0);
        return IMS_FALSE;
    }

    const SIPMethod &objMethod = piSCC->GetMethod();
    IMS_SLONG nIndex = objAuthChallengeMap.GetIndexOfKey(objMethod.ToInt());

    // If any needs, we MAY change the scheme to compare with the configuration value
    // of each services.
    if (nIndex >= 0)
    {
        IMS_SINT32 nAuthChallengeCount = objAuthChallengeMap.GetValueAt(nIndex);

        if (nAuthChallengeCount >= MAX_CHALLENGE_COUNT)
        {
            IMS_TRACE_I("Authentication challenge count (%d) is over", nAuthChallengeCount, 0, 0);
            return IMS_FALSE;
        }
    }

    ISIPGenericChallenge *piChallenge = piSCC->GetAuthenticationChallenge();

    if (piChallenge == IMS_NULL)
    {
        IMS_TRACE_E(0, "No authentication challenge", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nAlgorithm = Credential::TranslateAlgorithm(piChallenge->GetAlgorithm());

    if (nAlgorithm != Credential::TYPE_MD5)
    {
        IMS_TRACE_E(0, "Authentication algorithm (%s) is not supported for non-REGISTER request",
                piChallenge->GetAlgorithm().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    const SubscriberConfig *pSubscriberConfig = IMS_NULL;
    const AString& strSubsId = GetSubscriberId();
    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    if (strSubsId.GetLength() > 0)
    {
        pSubscriberConfig = pConfigMngr->GetSubscriberConfig(strSubsId, GetSlotId());
    }
    else
    {
        const AString &strId = SubscriberTracker::GetInstance()->GetSubscriberId(
                GetSlotId(), GetUserAOR());
        pSubscriberConfig = pConfigMngr->GetSubscriberConfig(strId, GetSlotId());
    }

    if (pSubscriberConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "SubscriberConfig is null; subsId=%s", strSubsId.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    Credential objCredential = pSubscriberConfig->GetCredential();

    objCredential.SetType(nAlgorithm);

    // If the username field is empty, then sets it to the private user identity.
    if (objCredential.GetUsername().GetLength() == 0)
    {
        objCredential.SetUsername(pSubscriberConfig->GetPrivateUserId());
    }

    // Overwrite the realm parameter if it required
    if (pSubscriberConfig->IsAuthRealmLenient()
            && (!objCredential.GetRealm().Equals(piChallenge->GetRealm())))
    {
        IMS_TRACE_D("auth_realm_leniency is true; %s -> %s",
                SIPDebug::GetCharA1(objCredential.GetRealm().GetStr(), 4),
                SIPDebug::GetCharA2(piChallenge->GetRealm().GetStr(), 4), 0);

        objCredential.SetRealm(piChallenge->GetRealm());
    }

    IMS_TRACE_D("Respond to the authentication challenge ...", 0, 0, 0);

    if (piSCC->InitResubmissionRequest() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Initializing resubmission request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Authentication algorithm will be MD5
    if (piSCC->SetCredentials(objCredential) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting a credential information failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!SendRequestToChallenge(piSCC))
    {
        // Remove the previous credential information for re-authentication
        piSCC->RemoveAllCredentials();

        IMS_TRACE_E(0, "Sending the resubmission request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Updates the authentication challenge information ...
    if (piAuthChallenge != IMS_NULL)
    {
        piAuthChallenge->Destroy();
        piAuthChallenge = IMS_NULL;
    }

    piChallenge = piSCC->GetAuthenticationChallenge();

    if (piChallenge != IMS_NULL)
    {
        piAuthChallenge = piChallenge->Clone();

        IMS_TRACE_D("Authentication challenge is updated...", 0, 0, 0);
    }

    // Remove the previous credential information for re-authentication
    piSCC->RemoveAllCredentials();

    // Increments the authentication challenge
    if (nIndex >= 0)
    {
        IMS_SINT32 &nAuthChallengeCount = objAuthChallengeMap.GetValueAt(nIndex);

        ++nAuthChallengeCount;

        IMS_TRACE_I("Authentication challenge :: %d >> %d ",
                nAuthChallengeCount - 1, nAuthChallengeCount, 0);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL Method::SetChallengeNCredentials(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "SCC is null", 0, 0, 0);
        return IMS_FALSE;
    }

    if (piAuthChallenge == IMS_NULL)
    {
        IMS_TRACE_D("No authentication challenge", 0, 0, 0);
        return IMS_TRUE;
    }

    const AString &strId = SubscriberTracker::GetInstance()->GetSubscriberId(
            GetSlotId(), GetUserAOR());
    const SubscriberConfig *pSubscriberConfig
            = ConfigurationManager::GetInstance()->GetSubscriberConfig(strId);

    if (pSubscriberConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "SubscriberConfig is null", 0, 0, 0);
        return IMS_FALSE;
    }

    Credential objCredential = pSubscriberConfig->GetCredential();

    objCredential.SetType(Credential::TYPE_MD5);

    // If the username field is empty, then sets it to the private user identity.
    if (objCredential.GetUsername().GetLength() == 0)
    {
        objCredential.SetUsername(pSubscriberConfig->GetPrivateUserId());
    }

    // Overwrite the realm parameter if it required
    if (pSubscriberConfig->IsAuthRealmLenient()
            && (!objCredential.GetRealm().Equals(piAuthChallenge->GetRealm())))
    {
        IMS_TRACE_D("auth_realm_leniency is true; %s -> %s",
                SIPDebug::GetCharA1(objCredential.GetRealm().GetStr(), 4),
                SIPDebug::GetCharA2(piAuthChallenge->GetRealm().GetStr(), 4), 0);

        objCredential.SetRealm(piAuthChallenge->GetRealm());
    }

    // Authentication algorithm will be MD5
    if (piSCC->SetCredentials(objCredential) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting a credential information failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Updates the nonce count (usage will be increased)
    piAuthChallenge->IncreaseNonceCount();

    // Set a previous authentication challenge
    piSCC->SetAuthenticationChallenge(piAuthChallenge);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
void Method::UpdateRemoteUserIds(IN ISIPConnection *piSC)
{
    ISIPMessage *piSIPMsg = (piSC != IMS_NULL) ? piSC->GetMessage() : IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return;
    }

    if (objRemoteUserIds.IsEmpty())
    {
        objRemoteUserIds = piSIPMsg->GetHeaders(ISIPHeader::P_ASSERTED_IDENTITY);
    }
    else
    {
        IMSList<AString> objLatestPAIDs = piSIPMsg->GetHeaders(ISIPHeader::P_ASSERTED_IDENTITY);

        if (!objLatestPAIDs.IsEmpty())
        {
            objRemoteUserIds.Clear();
            objRemoteUserIds = objLatestPAIDs;
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void Method::ClientConnection_NotifyResponse(IN ISIPClientConnection *piSCC,
        IN ISIPClientConnection *piForkedSCC /* = IMS_NULL */)
{
    //---------------------------------------------------------------------------------------------

    if (piForkedSCC != IMS_NULL)
    {
        if (!NotifySIPForkedResponse(piSCC, piForkedSCC))
        {
            IMS_TRACE_E(0, "Handling a forked response failed", 0, 0, 0);
        }

        return;
    }

    if (piSCC->Receive() != IMS_SUCCESS)
    {
        if (SIPError::GetLastError() == SIPError::NO_MESSAGE)
        {
            IMS_TRACE_I("No more messages in this client transaction", 0, 0, 0);
        }

        IMS_TRACE_E(0, "Receive() :: SIPError (%d)", SIPError::GetLastError(), 0, 0);
        return;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    if (piSIPMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "No SIP message", 0, 0, 0);
        return;
    }

    // Parse the message body if it is a multipart body
    if (!SIPParsingHelper::CreateMessageBodyParts(piSIPMsg))
    {
        IMS_TRACE_E(0, "Parsing a message body part failed", 0, 0, 0);

        Error_NotifyError(piSCC,
                SIPError::PARSING_ERROR, AString("Parsing Error :: message body part"));
        return;
    }

    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    IMS_TRACE_I("___ %d response to %s request is received ...",
            nStatusCode, piSIPMsg->GetMethod().ToString().GetStr(), 0);

    // Update the remote user identities if present
    if (SIPStatusCode::IsProvisional(nStatusCode)
            || SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        UpdateRemoteUserIds(piSCC);
    }

    // AUTH_SIP_DIGEST {
    // RESET the authentication challenge count
    ResetChallengeCount(piSCC);
    // }

    // Handle the SIP response.
    // All the subclass of this class MUST implement the below method.
    NotifySIPResponse(piSCC);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void Method::Error_NotifyError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
        IN CONST AString &strMessage)
{

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Error_NotifyError - %s : %d : %s",
            piSC->GetMethod().ToString().GetStr(), nCode, strMessage.GetStr());

    // SIPError::TRANSACTION_TIMER_EXPIRED
    // SIPError::AUTHENTICATION_FAILED
    // SIPError::PARSING_ERROR

    // AUTH_SIP_DIGEST {
    if (piAuthChallenge != IMS_NULL)
    {
        piAuthChallenge->Destroy();
        piAuthChallenge = IMS_NULL;
    }
    // }

    // Handle the transaction failure.
    // All the subclass of this class MUST implement the below method.
    NotifySIPError(piSC, nCode, strMessage);
}

/*

Remarks

*/
PUBLIC
Method::SCCListener::SCCListener()
{
    IMS_TRACE_D("Constructor :: SCCListener", 0, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL
Method::SCCListener::~SCCListener()
{
    IMS_TRACE_D("Destructor :: SCCListener", 0, 0, 0);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Method::SCCListener::Error_NotifyError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
        IN CONST AString &strMessage)
{
    //---------------------------------------------------------------------------------------------

    if (piSC == IMS_NULL)
        return;

    IMS_TRACE_I("SCCListener :: Error_NotifyError - %s : %d : %s",
            piSC->GetMethod().ToString().GetStr(), nCode, strMessage.GetStr());

    piSC->Close();

    delete this;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Method::SCCListener::ClientConnection_NotifyResponse(IN ISIPClientConnection *piSCC,
        IN ISIPClientConnection *piForkedSCC /* = IMS_NULL */)
{
    //---------------------------------------------------------------------------------------------

    if (piForkedSCC != IMS_NULL)
    {
        piForkedSCC->Close();
    }

    if (piSCC == IMS_NULL)
        return;

    if (piSCC->Receive() != IMS_SUCCESS)
    {
        if (SIPError::GetLastError() == SIPError::NO_MESSAGE)
        {
            IMS_TRACE_I("No more messages in this client transaction", 0, 0, 0);
        }

        IMS_TRACE_E(0, "Receive() :: SIPError (%d)", SIPError::GetLastError(), 0, 0);

        piSCC->Close();

        delete this;
        return;
    }

    IMS_TRACE_I("SCCListener :: ___ %d response to %s request is received ...",
            piSCC->GetStatusCode(), piSCC->GetMethod().ToString().GetStr(), 0);

    piSCC->Close();

    delete this;
}
