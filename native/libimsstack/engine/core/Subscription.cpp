/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100328  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "base/IMS.h"
#include "base/IRefreshListener.h"
#include "Service.h"
#include "Message.h"
// CALLER_PREFERENCE_MANAGER
#include "util/CallerPreferenceManager.h"
#include "util/DialogMethodManager.h"
#include "util/ForkedDialogMethodManager.h"
#include "Capabilities.h"
#include "IOnSubscriptionListener.h"
#include "SubscriberRefreshHelper.h"
#include "SubscriberState.h"
#include "Subscription.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
Subscription::Subscription(IN Service *pService_, IN CONST AString &strEvent_,
        IN IMS_BOOL bImplicitRoutingRequired_ /* = IMS_FALSE */)
    : ServiceMethod(pService_)
    , nState(STATE_INACTIVE)
    , strEvent(strEvent_)
    , nPendingOperation(SubState::NO_OPERATION)
    , piListener(IMS_NULL)
    , pSubState(IMS_NULL)
    , piRefreshListener(IMS_NULL)
    , pRefreshHelper(IMS_NULL)
    , bFlag_SubscriptionInOtherDialog(IMS_FALSE)
    , bFlag_ImplicitRoutingRequired(bImplicitRoutingRequired_)
{
}

PUBLIC VIRTUAL
Subscription::~Subscription()
{
    //---------------------------------------------------------------------------------------------

    CleanupOnDestroy();

    while (!objNotifyMessages.IsEmpty())
    {
        Message *pMessage = objNotifyMessages.GetAt(0);

        if (pMessage != IMS_NULL)
            delete pMessage;

        objNotifyMessages.RemoveAt(0);
    }

    while (!objForkedSubscriptions.IsEmpty())
    {
        Subscription *pSubscription = objForkedSubscriptions.GetAt(0);

        if (pSubscription != IMS_NULL)
            pSubscription->Destroy();

        objForkedSubscriptions.RemoveAt(0);
    }

    if (pRefreshHelper != IMS_NULL)
    {
        delete pRefreshHelper;
        pRefreshHelper = IMS_NULL;
    }

    if (pSubState != IMS_NULL)
    {
        delete pSubState;
        pSubState = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void Subscription::Destroy()
{
    //---------------------------------------------------------------------------------------------

    CleanupOnDestroy();
    ServiceMethod::Destroy();
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PUBLIC VIRTUAL
void Subscription::SetMessageMediator(IN IMessageMediator *piMediator)
{
    //---------------------------------------------------------------------------------------------

    Method::SetMessageMediator(piMediator);

    if (pRefreshHelper != IMS_NULL)
    {
        pRefreshHelper->SetMessageMediator(piMediator);
    }
}

/*

Remarks

*/
PUBLIC
const AString& Subscription::GetEvent() const
{
    //---------------------------------------------------------------------------------------------

    return strEvent;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 Subscription::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Subscription::Poll()
{
    //---------------------------------------------------------------------------------------------

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }
#endif

    if (GetState() != STATE_INACTIVE)
    {
        IMS_TRACE_E(0, "To subscribe(poll) an event, the state MUST be an INACTIVE state",
                0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // if the state is in ACTIVE and refresh is started by the subscription,
    // keep the request and after refresh is completed, try to send a SUBSCRIBE request...

    ISIPClientConnection *piSCC = CreateConnection(SIPMethod(SIPMethod::SUBSCRIBE));

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Event header
    piSIPMsg->SetHeader(ISIPHeader::EVENT, GetEvent());

    // Do something for SubscriberRefreshHelper ???
    // Accept, Allow-Events

    if (!piSIPMsg->IsHeaderPresent(ISIPHeader::EXPIRES_ANY))
    {
        piSIPMsg->SetHeader(ISIPHeader::EXPIRES_ANY, "0");
    }

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::SUBSCRIPTION_POLL, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    pSubState->SetOperation(SubState::OPERATION_FETCH);

    // Update the subscription state
    if (!pSubState->UpdateState(piSIPMsg))
    {
        pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    SetState(STATE_PENDING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void Subscription::SetListener(IN IOnSubscriptionListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Subscription::Subscribe()
{
    //---------------------------------------------------------------------------------------------

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }
#endif

    if ((GetState() != STATE_INACTIVE) && (GetState() != STATE_ACTIVE))
    {
        IMS_TRACE_E(0, "To subscribe an event, the state MUST be an INACTIVE or ACTIVE state",
                0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if (pSubState->IsInstantSubscription())
    {
        IMS_TRACE_E(0, "INVALID OPERATION :: It is for an instant subscription.", 0, 0, 0);
        IMS::SetLastError(IMSError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // if the state is in ACTIVE and refresh is started by the subscription,
    // keep the request and after refresh is completed, try to send a SUBSCRIBE request...
    if (pSubState->GetOperation() == SubState::OPERATION_IMPLICIT_REFRESH)
    {
        nPendingOperation = SubState::OPERATION_REFRESH;

        SetState(STATE_PENDING);

        IMS::SetLastError(IMSError::NO_ERROR);
        return IMS_FAILURE;
    }

    ISIPDialog *piDialog = GetDialog();
    SIPMethod objMethod(SIPMethod::SUBSCRIBE);
    ISIPClientConnection *piSCC;

    if (piDialog == IMS_NULL)
        piSCC = CreateConnection(objMethod);
    else
        piSCC = CreateConnectionL(piDialog, objMethod);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Event header
    piSIPMsg->SetHeader(ISIPHeader::EVENT, GetEvent());

    // Do something for SubscriberRefreshHelper ???
    // Accept, Allow-Events

    if (!piSIPMsg->IsHeaderPresent(ISIPHeader::EXPIRES_ANY))
    {
        // Expires header ?
        AString strExpires;

        if (pSubState->GetEventPackage()->GetDuration() != (-1))
        {
            strExpires.Sprintf("%d", pSubState->GetEventPackage()->GetDuration());
        }
        else
        {
            strExpires.Sprintf("%d", pSubState->GetEventPackage()->GetDefaultDuration());
        }

        piSIPMsg->SetHeader(ISIPHeader::EXPIRES_ANY, strExpires);
    }

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::SUBSCRIPTION_SUBSCRIBE, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    if (GetState() == STATE_INACTIVE)
        pSubState->SetOperation(SubState::OPERATION_CREATE);
    else
        pSubState->SetOperation(SubState::OPERATION_REFRESH);

    // Update the subscription state
    if (!pSubState->UpdateState(piSIPMsg))
    {
        pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    // Update the dialog state for SUBSCRIBE-created dialog (to handle early NOTIFY request)
    if ((GetState() == STATE_INACTIVE) && bFlag_SubscriptionInOtherDialog)
    {
        CheckNCreateDialog(piSCC, IMS_TRUE);
    }

    SetState(STATE_PENDING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT Subscription::Unsubscribe()
{
    //---------------------------------------------------------------------------------------------

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsServiceOpen())
    {
        IMS::SetLastError(IMSError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }
#endif

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "To unsubscribe an event, the state MUST be an ACTIVE state", 0, 0, 0);
        IMS::SetLastError(IMSError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // If the state is in ACTIVE and refresh is started by the subscription,
    // keep the request and after refresh is completed, try to send a SUBSCRIBE request...
    if (pSubState->GetOperation() == SubState::OPERATION_IMPLICIT_REFRESH)
    {
        nPendingOperation = SubState::OPERATION_REMOVE;

        SetState(STATE_PENDING);

        IMS::SetLastError(IMSError::NO_ERROR);
        return IMS_FAILURE;
    }

    SIPMethod objMethod(SIPMethod::SUBSCRIBE);
    ISIPClientConnection *piSCC = CreateConnectionL(GetDialog(), objMethod);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection inside of a dialog failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISIPMessage *piSIPMsg = piSCC->GetMessage();

    // Event header
    piSIPMsg->SetHeader(ISIPHeader::EVENT, GetEvent());

    // Do something for SubscriberRefreshHelper ???
    // Accept, Allow-Events

    // Expires header (overwrites the header field if present)
    piSIPMsg->SetHeader(ISIPHeader::EXPIRES_ANY, "0");

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::SUBSCRIPTION_UNSUBSCRIBE, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    pSubState->SetOperation(SubState::OPERATION_REMOVE);

    // Update the subscription state
    if (!pSubState->UpdateState(piSIPMsg))
    {
        pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    SetState(STATE_PENDING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void Subscription::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    //---------------------------------------------------------------------------------------------

    bFlag_ImplicitRoutingRequired = bFlag;

    // FIXME: If the routing address needs to be provisioned by the application,
    // please add a second argument for it.
}

/*

Remarks

*/
PUBLIC
void Subscription::SetRefreshListener(IN IRefreshListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    piRefreshListener = piListener;
}

/*

Remarks

*/
PUBLIC
void Subscription::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_INACTIVE)
    {
        IMS_TRACE_E(0, "Refresh policy can't be applied in the state (%s)",
                StateToString(GetState()), 0, 0);
        return;
    }

    if (pRefreshHelper == IMS_NULL)
    {
        return;
    }

    switch (nPolicy)
    {
    case REFRESH_POLICY_NO_REFRESH:
        pRefreshHelper->SetPolicy(RefreshHelper::POLICY_NO_REFRESH,
                nCriteriaInterval, nValueEorLT, nValueGT);
        break;

    case REFRESH_POLICY_SPEC:
        pRefreshHelper->SetPolicy(RefreshHelper::POLICY_SPEC,
                nCriteriaInterval, nValueEorLT, nValueGT);
        break;

    case REFRESH_POLICY_REMAIN_TIME:
        pRefreshHelper->SetPolicy(RefreshHelper::POLICY_REMAIN_TIME,
                nCriteriaInterval, nValueEorLT, nValueGT);
        break;

    case REFRESH_POLICY_RATIO:
        pRefreshHelper->SetPolicy(RefreshHelper::POLICY_RATIO,
                nCriteriaInterval, nValueEorLT, nValueGT);
        break;

    default:
        IMS_TRACE_E(0, "Invalid refresh policy (%d)", nPolicy, 0, 0);
        break;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Subscription::DispatchMessage(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
    case AMSG_SUBSCRIPTION_NOTIFY_RECEIVED:
        if (!objNotifyMessages.IsEmpty())
        {
            IMS_BOOL bDestroyNotify = IMS_TRUE;
            Message *pMessage = objNotifyMessages.GetAt(0);

            if (piListener != IMS_NULL)
            {
                piListener->OnSubscription_NotifyReceived(this, pMessage, bDestroyNotify);
            }

            objNotifyMessages.RemoveAt(0);

            if (bDestroyNotify)
            {
                delete pMessage;
            }
        }
        return IMS_TRUE;

    case AMSG_SUBSCRIPTION_STARTED:
        if (piListener != IMS_NULL)
        {
            piListener->OnSubscription_Started(this);
        }
        return IMS_TRUE;

    case AMSG_SUBSCRIPTION_START_FAILED:
        if (piListener != IMS_NULL)
        {
            piListener->OnSubscription_StartFailed(this);
        }
        return IMS_TRUE;

    case AMSG_SUBSCRIPTION_TERMINATED:
        if (piListener != IMS_NULL)
        {
            piListener->OnSubscription_Terminated(this);
        }
        return IMS_TRUE;

    case AMSG_SUBSCRIPTION_FORKED_NOTIFY_RECEIVED:
        if (!objForkedSubscriptions.IsEmpty())
        {
            Subscription *pSubscription = objForkedSubscriptions.GetAt(0);

            objForkedSubscriptions.RemoveAt(0);

            if (piListener != IMS_NULL)
            {
                if (!piListener->OnSubscription_ForkedNotifyReceived(this, pSubscription))
                {
                    pSubscription->Destroy();
                }
            }
            else
            {
                pSubscription->Destroy();
            }
        }
        return IMS_TRUE;

    default:
        break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PROTECTED VIRTUAL
IMS_BOOL Subscription::SendRequestToChallenge(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nOperation = pSubState->GetOperation();

    if (nOperation == SubState::OPERATION_IMPLICIT_REFRESH)
    {
        if (!Method::SendRequestToChallenge(piSCC))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        IMS_SINT32 nServiceMethod = IMessage::SUBSCRIPTION_SUBSCRIBE;

        if (nOperation == SubState::OPERATION_REMOVE)
        {
            nServiceMethod = IMessage::SUBSCRIPTION_UNSUBSCRIBE;
        }
        else if (nOperation == SubState::OPERATION_FETCH)
        {
            nServiceMethod = IMessage::SUBSCRIPTION_POLL;
        }

        // Clear the connection to preserve the SIP connection
        ClearConnection(nServiceMethod);

        if (!SendNUpdateRequestEx(nServiceMethod, piSCC, MESSAGE_CLASS_RESUBMIT))
        {
            // Revert the SIP connection
            UpdateConnection(nServiceMethod, piSCC);
            return IMS_FALSE;
        }
    }

    // Update the subscription state
    if (!pSubState->UpdateState(piSCC->GetMessage()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Subscription::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    //---------------------------------------------------------------------------------------------

    (void) nErrorCode;

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    SetState(STATE_INACTIVE);
    PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);

    pSubState->Clear();
    DestroyDialog();
#endif
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Subscription::InitInstance()
{
    //---------------------------------------------------------------------------------------------

    if (pSubState == IMS_NULL)
    {
        pSubState = new SubscriberState();

        if (pSubState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a subscriber state failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (!pSubState->CreateEventPackage(strEvent))
        {
            IMS_TRACE_E(0, "Creating an event package for an event (%s) failed",
                    strEvent.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        pSubState->SetConfiguration(
                SubState::CONFIG_USE_INITIAL_EXPIRES_ON_NO_EXPIRES_IN_200_OK);
    }

    if (pRefreshHelper == IMS_NULL)
    {
        pRefreshHelper = new SubscriberRefreshHelper(this, pSubState);

        if (pRefreshHelper == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a subscription refresh helper failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
    ForkedDialogMethodManager::GetInstance()->AddMethod(GetName(), this);
    // CALLER_PREFERENCE_MANAGER
    CallerPreferenceManager::GetInstance()->CreatePreferenceWrapper(GetName(),
            AString::ConstNull());
    GetService()->RegisterMethod(this);

    if (GetDialog() != IMS_NULL)
    {
        bFlag_SubscriptionInOtherDialog = IMS_TRUE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Subscription::NotifySIPResponse(IN ISIPClientConnection *piSCC)
{
    ISIPMessage *piSIPMsg = piSCC->GetMessage();
    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("The response is received in the %s", StateToString(GetState()), 0, 0);

    if (!objMethod.Equals(SIPMethod::SUBSCRIBE))
    {
        piSCC->Close();
        return;
    }

    IMS_SINT32 nOperation = pSubState->GetOperation();

    // Update the subscription state
    if (!pSubState->UpdateState(piSIPMsg))
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_START_FAILED, 0, 0);

        CloseConnection();

        if (nOperation == SubState::OPERATION_CREATE)
        {
            pSubState->Clear();

            if (!bFlag_SubscriptionInOtherDialog)
            {
                DestroyDialog();
            }
        }
        return;
    }

    // Handle the response according to the SIP method.
    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    UpdateResponse(piSCC);

    // Handle the response to SUBSCRIBE request ...
    if (SIPStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return;
    }
    else if ((nStatusCode == SIPStatusCode::SC_401)
            || (nStatusCode == SIPStatusCode::SC_407))
    {
        // AUTH_SIP_DIGEST {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piSCC))
        {
            return;
        }
        // }
    }

    // Start or re-start a subscription refresh timer
    pRefreshHelper->UpdateOnMessageReceived(piSCC);

    if (SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        if (GetState() != STATE_PENDING)
        {
            CheckNCreateDialog(piSCC, bFlag_SubscriptionInOtherDialog);
        }
        else
        {
            // RACE_CONDITION: NOTIFY (w/ terminated) & 200 OK to SUBSCRIBE
            CheckNCreateDialog(piSCC, IMS_FALSE, IMS_TRUE);
        }

        // CALLER_PREFERENCE_MANAGER
        if ((nOperation == SubState::OPERATION_CREATE)
                || (nOperation == SubState::OPERATION_REFRESH))
        {
            if (nOperation == SubState::OPERATION_CREATE)
            {
                ISIPDialog *piDialog = GetDialog();

                if ((piDialog != IMS_NULL)
                        && (piDialog->GetState() == ISIPDialog::STATE_CONFIRMED))
                {
                    CallerPreferenceManager::GetInstance()->UpdateDialogId(GetName(),
                            piDialog->GetDialogID());
                }
            }

            IMessage *piMessage = GetPreviousRequest(IMessage::SUBSCRIPTION_SUBSCRIBE);

            if (piMessage != IMS_NULL)
            {
                ISIPMessage *piPreviousSIPMsg = piMessage->GetMessage();

                if (piPreviousSIPMsg != IMS_NULL)
                {
                    CallerPreferenceManager::GetInstance()->UpdateAcceptContacts(
                            GetName(), piPreviousSIPMsg->GetHeaders(ISIPHeader::ACCEPT_CONTACT));
                }
            }
        }

        SetState(STATE_ACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_STARTED, 0, 0);
    }
    else
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_START_FAILED, 0, 0);
    }

    CloseConnection();

    if ((nOperation != SubState::OPERATION_CREATE)
        && (nOperation != SubState::OPERATION_FETCH))
    {
        CheckDialogNCallListener();
    }

    // Initialize all the subscriber's state at this moment,
    // because the application MAY use this subscription
    // to add/fetch/refresh/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        pSubState->Clear();

        if (!bFlag_SubscriptionInOtherDialog)
        {
            DestroyDialog();
        }
    }

    //4 check "nPendingOperation" member field
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Subscription::NotifySIPError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
        IN CONST AString &strMessage)
{
    const SIPMethod &objMethod = piSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    (void) nCode;
    (void) strMessage;

    if (!objMethod.Equals(SIPMethod::SUBSCRIBE))
    {
        piSC->Close();
        return;
    }

    IMS_SINT32 nOperation = pSubState->GetOperation();

    if ((nOperation != SubState::NO_OPERATION)
            && (nOperation != SubState::OPERATION_IMPLICIT_REFRESH))
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_START_FAILED, 0, 0);
    }

    CloseConnection();

    // Initialize all the subscriber's state at this moment,
    // because the application MAY use this subscription
    // to add/fetch/refresh/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        pSubState->Clear();
        DestroyDialog();
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Subscription::Dialog_Compare(IN ISIPServerConnection *piSSC) const
{
    //---------------------------------------------------------------------------------------------

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsServiceOpen())
    {
        return IMS_FALSE;
    }
#endif

    // Filters some method which does not handle in the subscription
    if (!piSSC->GetMethod().Equals(SIPMethod::NOTIFY))
    {
        return IMS_FALSE;
    }

    ISIPDialog *piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        // In case of an early NOTIFY received ...
        if (GetState() == STATE_PENDING)
        {
            IMS_SINT32 nOperation = pSubState->GetOperation();
            ISIPClientConnection *piSCC = IMS_NULL;

            IMS_TRACE_I("Checks if the early NOTIFY is received or not ...", 0, 0, 0);

            if (nOperation == SubState::OPERATION_CREATE)
            {
                piSCC = GetClientConnection(IMessage::SUBSCRIPTION_SUBSCRIBE);
            }
            else if (nOperation == SubState::OPERATION_FETCH)
            {
                piSCC = GetClientConnection(IMessage::SUBSCRIPTION_POLL);
            }

            if (piSCC != IMS_NULL)
            {
                piDialog = piSCC->GetDialog();
            }
        }
    }

    if (piDialog == IMS_NULL)
    {
        IMS_TRACE_D("No dialog exists", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!piDialog->IsSameDialog(piSSC))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Subscription::Dialog_NotifyRequest(IN ISIPServerConnection *piSSC)
{
    const SIPMethod& objMethod = piSSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    if (!objMethod.Equals(SIPMethod::NOTIFY))
    {
        // Handling of OPTIONS request
        if (objMethod.Equals(SIPMethod::OPTIONS))
        {
            if (Capabilities::HandleOPTIONSRequestWithinDialog(
                    GetService(), this, piSSC) != IMS_SUCCESS)
            {
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }

        GetService()->SendResponse(piSSC, SIPStatusCode::SC_405);
        piSSC->Close();
        return IMS_FALSE;
    }

    if (SIPConfigProxy::IsRequestUriValidationRequiredInMidDialog(
            GetSlotId(), GetService()->GetSIPProfile()))
    {
        // Checks if Request-URI is matched or not
        const AString &strRequestURI = piSSC->GetRequestURI();
        SIPAddress objRequestURI(strRequestURI);

        if (!GetService()->ValidateRequestURI(objRequestURI, piSSC->GetDialog(), IMS_TRUE))
        {
            IMS_TRACE_D("Request-URI (%s) in mid-dialog request is not matched",
                    SIPDebug::GetUri1(strRequestURI).GetStr(), 0, 0);

            GetService()->SendResponse(piSSC, SIPStatusCode::SC_404);
            piSSC->Close();
            return IMS_FALSE;
        }
    }

    // Update the dialog info.
    CheckNCreateDialog(piSSC);

    // Update the dialog info.
    if (GetState() != STATE_ACTIVE)
    {
        CheckNCreateDialog(piSSC, bFlag_SubscriptionInOtherDialog);
    }
    else
    {
        CheckNCreateDialog(piSSC);
    }

    // Update the subscription state for NOTIFY request
    if (!pSubState->UpdateState(piSSC->GetMessage()))
    {
        GetService()->SendResponse(piSSC, SIPStatusCode::SC_500);
        piSSC->Close();
        return IMS_FALSE;
    }

    Message *pMessage = Message::CreateReceivedMessage(GetService()->GetAppConfig(),
                            piSSC->GetMessage());

    if (pMessage == IMS_NULL)
    {
        // Internal error ... ???
        if (GetService()->SendResponse(piSSC, SIPStatusCode::SC_500))
        {
            pSubState->UpdateState(piSSC->GetMessage());
            pRefreshHelper->UpdateOnMessageSent(piSSC);
        }

        piSSC->Close();

        return IMS_FALSE;
    }

    // Send a 2xx response to NOTIFY request immediately
    if (GetService()->SendResponse(piSSC, SIPStatusCode::SC_200) == IMS_FALSE)
    {
        delete pMessage;
        piSSC->Close();

        IMS_TRACE_E(0, "Creating & sending the response to NOTIFY request failed", 0, 0, 0);

        return IMS_FALSE;
    }

    // Update the subscription state for NOTIFY response
    if (!pSubState->UpdateState(piSSC->GetMessage()))
    {
        delete pMessage;
        piSSC->Close();

        return IMS_FALSE;
    }

    // Update the subscription refresh timer
    pRefreshHelper->UpdateOnMessageSent(piSSC);

    if (!objNotifyMessages.Append(pMessage))
    {
        delete pMessage;

        IMS_TRACE_E(0, "Queueing NOTIFY message failed", 0, 0, 0);
    }

    piSSC->Close();

    PostMessage(AMSG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, 0);

    if (pSubState->GetState() == SubState::STATE_TERMINATED)
    {
        PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Subscription::ForkedDialog_Compare(IN ISIPDialog *piOrigDialog) const
{
    //---------------------------------------------------------------------------------------------

    if (!IsServiceOpen())
    {
        return IMS_FALSE;
    }

    ISIPDialog *piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piDialog->Equals(piOrigDialog))
    {
        return IMS_FALSE;
    }

    //4 check the flag whether the forked request is allowed to this subscription (event package)

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Subscription::ForkedDialog_NotifyRequest(IN ISIPServerConnection *piSSC)
{
    Subscription *pSubscription = new Subscription(GetService(), strEvent);

    //---------------------------------------------------------------------------------------------

    if (pSubscription == IMS_NULL)
    {
        GetService()->SendResponse(piSSC, SIPStatusCode::SC_500);
        piSSC->Close();

        return IMS_FALSE;
    }

    if (!pSubscription->InitMethod(this, IsMobileOriginated()))
    {
        delete pSubscription;

        GetService()->SendResponse(piSSC, SIPStatusCode::SC_500);
        piSSC->Close();
        return IMS_FALSE;
    }

    // Update the dialog info. enforcelly
    pSubscription->CheckNCreateDialog(piSSC, IMS_TRUE);
    pSubscription->SetState(STATE_ACTIVE);

    // Update the subscription state from the initial SUBSCRIBE message
    pSubscription->pSubState->SetOperation(SubState::OPERATION_CREATE);
    pSubscription->pSubState->UpdateState(pSubState->GetInitialMessage());
    pSubscription->pSubState->SetOperation(SubState::NO_OPERATION);

    if (!objForkedSubscriptions.Append(pSubscription))
    {
        delete pSubscription;

        GetService()->SendResponse(piSSC, SIPStatusCode::SC_500);
        piSSC->Close();
        return IMS_FALSE;
    }

    PostMessage(AMSG_SUBSCRIPTION_FORKED_NOTIFY_RECEIVED, 0, 0);

    if (!pSubscription->Dialog_NotifyRequest(piSSC))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Subscription::Refreshable_RefreshCompleted(IN ISIPClientConnection *piSCC,
        IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ SUBSCRIPTION REFRESH COMPLETED ... Code (%d)", nCode, 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyCompleted(piSCC);
    }

    // In case, the subscription refresh request is successfully done.
    if (nCode == 0)
    {
        if (!pSubState->UpdateState(piSCC->GetMessage()))
        {
            // Set the operation
            pSubState->SetOperation(SubState::NO_OPERATION);

            CheckDialogNCallListener();
            return;
        }

        // AUTH_SIP_DIGEST {
        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        if ((nStatusCode == SIPStatusCode::SC_401)
                || (nStatusCode == SIPStatusCode::SC_407))
        {
            // In case of other method except for REGISTER,
            // the UE only supports the authentication algorithm, MD5
            if (RespondToChallenge(piSCC))
            {
                return;
            }
        }

        ResetChallengeCount(piSCC);
        // }

        CheckDialogNCallListener();
    }
    else if (nCode == SIPStatusCode::SC_408)
    {
        Refreshable_RefreshTerminated();
    }
    // The subscription refresh request is timed out.
    else if (nCode == RefreshHelper::TRANSACTION_TIMEOUT)
    {
        //4 TODO:: what to do ... ? In this moment, do nothing ...
        Refreshable_RefreshTerminated();
    }

    // Set the operation
    pSubState->SetOperation(SubState::NO_OPERATION);
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL Subscription::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ SUBSCRIPTION REFRESH STARTED ... in the %s", StateToString(nState), 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyTimerExpired(bDoImplicitRefresh);
    }

    if (bDoImplicitRefresh && (nState == STATE_ACTIVE))
    {
        // Set the operation
        pSubState->SetOperation(SubState::OPERATION_IMPLICIT_REFRESH);

        // Send a refresh request : SUBSCRIBE
        SIPMethod objMethod(SIPMethod::SUBSCRIBE);
        ISIPClientConnection *piSCC = GetService()->CreateConnection(GetDialog(), objMethod);

        if (piSCC == IMS_NULL)
        {
            pSubState->SetOperation(SubState::NO_OPERATION);

            IMS_TRACE_E(0, "Creating a new SIP connection for a subscription refresh failed",
                    0, 0, 0);
            return IMS_FALSE;
        }

        ISIPMessage *piSIPMsg = piSCC->GetMessage();
        ISIPMessage *piInitialSIPMsg = pSubState->GetInitialMessage();

        if (piInitialSIPMsg != IMS_NULL)
        {
            if (piSIPMsg->CopyHeadersAndBodyParts(piInitialSIPMsg) != IMS_SUCCESS)
            {
                pSubState->SetOperation(SubState::NO_OPERATION);
                piSCC->Close();

                IMS_TRACE_E(0, "Setting SIP headers to refresh SUBSCRIBE request failed",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }

        // IMPLICIT_ROUTING_FOR_MID_DIALOG
        if (bFlag_ImplicitRoutingRequired)
        {
            const AStringArray &objServiceRoutes = GetService()->GetServiceRoutes();

            if (!objServiceRoutes.IsEmpty())
            {
                piSCC->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
            }
        }

        if (pRefreshHelper->SendRefreshRequest(piSCC) != IMS_SUCCESS)
        {
            pSubState->SetOperation(SubState::NO_OPERATION);
            piSCC->Close();

            IMS_TRACE_E(0, "Sending a subscription refresh request failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Update the subscription state
        pSubState->UpdateState(piSIPMsg);

        return IMS_TRUE;
    }

    return (bDoImplicitRefresh == IMS_TRUE) ? IMS_FALSE : IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void Subscription::Refreshable_RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("_____ SUBSCRIPTION REFRESH TERMINATED ...", 0, 0, 0);

    if (piRefreshListener != IMS_NULL)
    {
        piRefreshListener->Refresh_NotifyTerminated();
    }

    if (GetState() == STATE_ACTIVE)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);
    }

    if (GetState() == STATE_INACTIVE)
    {
        pSubState->Clear();
        DestroyDialog();
    }
}

/*

Remarks

*/
PRIVATE
void Subscription::CheckDialogNCallListener()
{
    // Check if the dialog is terminated or not
    ISIPDialog *piDialog = GetDialog();

    //---------------------------------------------------------------------------------------------

    if (piDialog != IMS_NULL)
    {
        if (piDialog->GetState() == ISIPDialog::STATE_TERMINATED)
        {
            PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);
        }
    }
    else
    {
        PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);
    }
}

/*

Remarks

*/
PRIVATE
void Subscription::CleanupOnDestroy()
{
    //---------------------------------------------------------------------------------------------

    if (pRefreshHelper != IMS_NULL)
    {
        if (pRefreshHelper->IsRequestPending())
        {
            pRefreshHelper->AbortConnection();
        }
    }

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    ForkedDialogMethodManager::GetInstance()->RemoveMethod(GetName());

    // CALLER_PREFERENCE_MANAGER
    CallerPreferenceManager::GetInstance()->DestroyPreferenceWrapper(GetName());

    // Clean up the resources
    GetService()->DeregisterMethod(this);
}

/*

Remarks

*/
PRIVATE
void Subscription::CloseConnection()
{
    //---------------------------------------------------------------------------------------------

    switch (pSubState->GetOperation())
    {
    case SubState::OPERATION_CREATE:
    case SubState::OPERATION_REFRESH:
        ServiceMethod::CloseConnection(IMessage::SUBSCRIPTION_SUBSCRIBE);
        break;

    case SubState::OPERATION_FETCH:
        ServiceMethod::CloseConnection(IMessage::SUBSCRIPTION_POLL);
        break;

    case SubState::OPERATION_REMOVE:
        ServiceMethod::CloseConnection(IMessage::SUBSCRIPTION_UNSUBSCRIBE);
        break;

    default:
        // Do nothing ...
        break;
    }

    pSubState->SetOperation(SubState::NO_OPERATION);
}

/*

Remarks

*/
PRIVATE
ISIPClientConnection* Subscription::CreateConnectionL(IN ISIPDialog *piDialog,
        IN CONST SIPMethod &objMethod)
{
    ISIPClientConnection *piSCC = CreateConnection(piDialog, objMethod);

    //---------------------------------------------------------------------------------------------

    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    if (bFlag_ImplicitRoutingRequired && (piSCC != IMS_NULL))
    {
        const AStringArray &objServiceRoutes = GetService()->GetServiceRoutes();

        if (!objServiceRoutes.IsEmpty())
        {
            piSCC->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
        }
    }

    return piSCC;
}

/*

Remarks

*/
PRIVATE
void Subscription::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Subscription :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE
void Subscription::UpdateResponse(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    switch (pSubState->GetOperation())
    {
    case SubState::OPERATION_CREATE:
    case SubState::OPERATION_REFRESH:
        UpdateResponseOnReceived(IMessage::SUBSCRIPTION_SUBSCRIBE, piSCC);
        break;

    case SubState::OPERATION_FETCH:
        UpdateResponseOnReceived(IMessage::SUBSCRIPTION_POLL, piSCC);
        break;

    case SubState::OPERATION_REMOVE:
        UpdateResponseOnReceived(IMessage::SUBSCRIPTION_UNSUBSCRIBE, piSCC);
        break;

    default:
        // Do nothing ...
        break;
    }
}

const IMS_CHAR* Subscription::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
    case STATE_INACTIVE:
        return "STATE_INACTIVE";
    case STATE_PENDING:
        return "STATE_PENDING";
    case STATE_ACTIVE:
        return "STATE_ACTIVE";
    default:
        return "__INVALID__";
    }
}
