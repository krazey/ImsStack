/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "Connector.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "SipHeaderName.h"
#include "SipDebug.h"
#include "SipStatusCode.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipConfigProxy.h"
#include "base/IMS.h"
#include "ImsIdentity.h"
#include "PAccessNetworkInfoHeader.h"
#include "util/UserAgentHeader.h"
#include "SipTimerValuesHelper.h"
// NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
#include "util/SIPConnectionNotifierManager.h"
#include "util/DialogMethodManager.h"
#include "SubscriberRefreshHelper.h"
#include "SubscriberState.h"
#include "IRegSubscriptionListener.h"
#include "RegInfo.h"
#include "RegInfoManager.h"

#include "RegContact.h"

#include "RegSubscription.h"

__IMS_TRACE_TAG_REG__;

#define REG_SUB_MAKE_LPARAM(HIWORD, LOWORD) (IMS_UINT32)(((HIWORD) << 16) | (LOWORD))

#define REG_SUB_HIWORD(LPARAM) (IMS_UINT16)(((LPARAM) >> 16) & (0xFFFF))

#define REG_SUB_LOWORD(LPARAM) (IMS_UINT16)((LPARAM)&0xFFFF)

PRIVATE GLOBAL const IMS_CHAR RegSubscription::EVENT[] = "reg";
PRIVATE GLOBAL const IMS_CHAR RegSubscription::MEDIA_TYPE[] = "application";
PRIVATE GLOBAL const IMS_CHAR RegSubscription::MEDIA_SUB_TYPE[] = "reginfo+xml";

PUBLIC
RegSubscription::RegSubscription(IN CONST RegKey& objRegKey_, IN RegStateTracker* pRegStateTracker_,
        IN IMS_UINT32 nExpiresValue_ /* = 0 */,
        IN CONST SipTimerValues* pSIPTVs_ /* = IMS_NULL */) :
        Method(),
        nFeatureSet(0),
        nState(STATE_INACTIVE),
        nExpiresValue(DEFAULT_EXPIRES),
        nPendingOperation(SubState::NO_OPERATION),
        piListener(IMS_NULL),
        objRegKey(objRegKey_),
        pRegStateTracker(pRegStateTracker_),
        pSubState(IMS_NULL),
        pRefreshHelper(IMS_NULL),
        piOngoingSCC(IMS_NULL),
        piNextRequest(IMS_NULL),
        piPreviousRequest(IMS_NULL),
        piPreviousResponse(IMS_NULL),
        pSIPTVs(IMS_NULL),
        piReferredSCN(IMS_NULL)
{
    // Default expires value of 'reg' event package will be set to the pre-configured value.
    if (SipConfigProxy::IsRegSubExpiresConfigured(GetSlotId(), pRegStateTracker->GetSIPProfile()))
    {
        nExpiresValue =
                SipConfigProxy::GetRegSubExpires(GetSlotId(), pRegStateTracker->GetSIPProfile());
    }
    else if (nExpiresValue_ != 0)
    {
        nExpiresValue = nExpiresValue_;
    }

    if (pSIPTVs_ != IMS_NULL)
    {
        pSIPTVs = new SipTimerValues(*pSIPTVs_);
    }

    // NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
    if (!pRegStateTracker.IsNull())
    {
        piReferredSCN = SIPConnectionNotifierManager::GetInstance()->GetConnectionNotifier(
                pRegStateTracker->GetIPAddress(), pRegStateTracker->GetPortUS());
    }
}

PUBLIC VIRTUAL RegSubscription::~RegSubscription()
{
    //---------------------------------------------------------------------------------------------

    RegInfoManager::GetInstance()->DestroyRegInfo(objRegKey);
    objRegKey.Invalidate();

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

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

    if (piOngoingSCC != IMS_NULL)
    {
        piOngoingSCC->Close();
        piOngoingSCC = IMS_NULL;
    }

    ClearNextRequest();
    SetPreviousRequest(IMS_NULL);
    SetPreviousResponse(IMS_NULL);

    if (pSIPTVs != IMS_NULL)
    {
        delete pSIPTVs;
        pSIPTVs = IMS_NULL;
    }

    // NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
    if (piReferredSCN != IMS_NULL)
    {
        SIPConnectionNotifierManager::GetInstance()->ReleaseConnectionNotifier(piReferredSCN);
        piReferredSCN = IMS_NULL;
    }

    IMS_TRACE_D("Destructor :: RegSubscription", 0, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL ISipMessage* RegSubscription::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    if (piNextRequest == IMS_NULL)
    {
        piNextRequest = SipParsingHelper::CreateMessage(ISipMessage::TYPE_REQUEST);
    }

    return piNextRequest;
}

/*

Remarks

*/
PUBLIC VIRTUAL ISipMessage* RegSubscription::GetPreviousRequest() const
{
    //---------------------------------------------------------------------------------------------

    return piPreviousRequest;
}

/*

Remarks

*/
PUBLIC VIRTUAL ISipMessage* RegSubscription::GetPreviousResponse() const
{
    //---------------------------------------------------------------------------------------------

    return piPreviousResponse;
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PUBLIC VIRTUAL void RegSubscription::SetSipMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    SetMessageMediator(piMediator);

    if (pRefreshHelper != IMS_NULL)
    {
        pRefreshHelper->SetMessageMediator(piMediator);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL void RegSubscription::DestroyEx()
{
    //---------------------------------------------------------------------------------------------

    piListener = IMS_NULL;

    if (pRefreshHelper != IMS_NULL)
    {
        if (pRefreshHelper->IsRequestPending())
        {
            pRefreshHelper->AbortConnection();
        }
    }

    if (piOngoingSCC != IMS_NULL)
    {
        piOngoingSCC->Close();
        piOngoingSCC = IMS_NULL;
    }

    RegInfoManager::GetInstance()->DestroyRegInfo(objRegKey);
    objRegKey.Invalidate();

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    // NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
    if (piReferredSCN != IMS_NULL)
    {
        SIPConnectionNotifierManager::GetInstance()->ReleaseConnectionNotifier(piReferredSCN);
        piReferredSCN = IMS_NULL;
    }

    // SIP_MESSAGE_MEDIATOR
    SetSipMessageMediator(IMS_NULL);

    Destroy();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 RegSubscription::DisableFeatures(IN IMS_SINT32 nFeatures)
{
    //---------------------------------------------------------------------------------------------

    nFeatureSet &= (~nFeatures);

    return nFeatureSet;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 RegSubscription::EnableFeatures(IN IMS_SINT32 nFeatures)
{
    //---------------------------------------------------------------------------------------------

    nFeatureSet |= nFeatures;

    return nFeatureSet;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_UINT32 RegSubscription::GetExpires() const
{
    //---------------------------------------------------------------------------------------------

    return nExpiresValue;
}

/*

Remarks

*/
PUBLIC VIRTUAL const IRegInfo* RegSubscription::GetRegInfo() const
{
    //---------------------------------------------------------------------------------------------

    return RegInfoManager::GetInstance()->GetRegInfo(objRegKey);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 RegSubscription::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT RegSubscription::SetContactParameter(
        IN CONST AString& strParameter, IN IMS_SINT32 nOperation /* = 0 (0: ADD, 1: REMOVE) */)
{
    ISipDialog* piDialog = GetDialog();

    //---------------------------------------------------------------------------------------------

    if (piDialog != IMS_NULL)
    {
        IMS_SINT32 nDialogState = piDialog->GetState();

        if (nDialogState == ISipDialog::STATE_CONFIRMED)
        {
            return piDialog->SetContactParameter(strParameter, nOperation);
        }
    }

    return IMS_FAILURE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void RegSubscription::SetExpires(IN IMS_UINT32 nExpires)
{
    //---------------------------------------------------------------------------------------------

    // 140307, hwangoo.park, do not check the current state for refresh failure case
    // if (GetState() != STATE_INACTIVE)
    //    return;

    nExpiresValue = nExpires;

    pSubState->GetEventPackage()->SetDuration(nExpiresValue);
}

/*

Remarks

*/
PUBLIC VIRTUAL void RegSubscription::SetListener(IN IRegSubscriptionListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC VIRTUAL void RegSubscription::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

    if (pRefreshHelper == IMS_NULL)
        return;

    switch (nPolicy)
    {
        case REFRESH_POLICY_NO_REFRESH:
            pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_NO_REFRESH, nCriteriaInterval, nValueEorLT, nValueGT);
            break;

        case REFRESH_POLICY_SPEC:
            pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_SPEC, nCriteriaInterval, nValueEorLT, nValueGT);
            break;

        case REFRESH_POLICY_REMAIN_TIME:
            pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_REMAIN_TIME, nCriteriaInterval, nValueEorLT, nValueGT);
            break;

        case REFRESH_POLICY_RATIO:
            pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_RATIO, nCriteriaInterval, nValueEorLT, nValueGT);
            break;

        default:
            IMS_TRACE_E(0, "Invalid refresh policy (%d)", nPolicy, 0, 0);
            break;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT RegSubscription::Subscribe()
{
    //---------------------------------------------------------------------------------------------

    if ((GetState() != STATE_INACTIVE) && (GetState() != STATE_ACTIVE))
    {
        IMS_TRACE_E(
                0, "To subscribe an event, the state MUST be an INACTIVE or ACTIVE state", 0, 0, 0);
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

    ISipClientConnection* piSCC = CreateConnection(this);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    piSCC->SetListener(this);
    piSCC->SetErrorListener(this);

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    if (!SetHeaders(piSIPMsg))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Copy the headers from the application
    if (!CopyHeadersAndBodyParts(piSIPMsg))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Add "gruu" option tag to Supported header

    // Set Expires header
    if (!piSIPMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
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

        piSIPMsg->SetHeader(ISipHeader::EXPIRES_ANY, strExpires);
    }

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSCC->GetMessage(), MESSAGE_CLASS_NORMAL);

    // Try to send a SUBSCRIBE request to the network
    if (piSCC->Send() != IMS_SUCCESS)
    {
        piSCC->Close();

        IMS_TRACE_E(0, "Sending SUBSCRIBE request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Clear the next request
    ClearNextRequest();

    // Store the ongoing SIP client connection
    SetOngoingConnection(piSCC);

    // Update the last request/response message
    SetPreviousRequest(piSCC->GetMessage());
    SetPreviousResponse(IMS_NULL);

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

    SetState(STATE_PENDING);

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT RegSubscription::Unsubscribe()
{
    //---------------------------------------------------------------------------------------------

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

    ISipClientConnection* piSCC = CreateConnection(this);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection inside of a dialog failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    piSCC->SetListener(this);
    piSCC->SetErrorListener(this);

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    if (!SetHeaders(piSIPMsg))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Copy the headers from the application
    if (!CopyHeadersAndBodyParts(piSIPMsg))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Expires header (overwrites the header field if present)
    piSIPMsg->SetHeader(ISipHeader::EXPIRES_ANY, "0");

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSCC->GetMessage(), MESSAGE_CLASS_NORMAL);

    // Try to send a SUBSCRIBE request to the network
    if (piSCC->Send() != IMS_SUCCESS)
    {
        piSCC->Close();

        IMS_TRACE_E(0, "Sending SUBSCRIBE request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Clear the next request
    ClearNextRequest();

    // Store the ongoing SIP client connection
    SetOngoingConnection(piSCC);

    // Update the last request/response message
    SetPreviousRequest(piSCC->GetMessage());
    SetPreviousResponse(IMS_NULL);

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
PRIVATE VIRTUAL IMS_BOOL RegSubscription::DispatchMessage(IN IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
        case AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED:
            if (piListener != IMS_NULL)
            {
                piListener->RegSubscription_NotifyReceived(LONG_TO_SINT(objMSG.nWparam),
                        REG_SUB_HIWORD(objMSG.nLparam),
                        (REG_SUB_LOWORD(objMSG.nLparam) == 0) ? IMS_FALSE : IMS_TRUE);
            }
            return IMS_TRUE;

        case AMSG_REG_SUBSCRIPTION_STARTED:
            if (piListener != IMS_NULL)
            {
                piListener->RegSubscription_Started();
            }
            return IMS_TRUE;

        case AMSG_REG_SUBSCRIPTION_START_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->RegSubscription_StartFailed(LONG_TO_SINT(objMSG.nWparam));
            }
            return IMS_TRUE;

        case AMSG_REG_SUBSCRIPTION_UPDATED:
            if (piListener != IMS_NULL)
            {
                piListener->RegSubscription_Updated();
            }
            return IMS_TRUE;

        case AMSG_REG_SUBSCRIPTION_UPDATE_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->RegSubscription_UpdateFailed(LONG_TO_SINT(objMSG.nWparam));
            }
            return IMS_TRUE;

        case AMSG_REG_SUBSCRIPTION_REMOVED:
            if (piListener != IMS_NULL)
            {
                piListener->RegSubscription_Removed();
            }
            return IMS_TRUE;

        case AMSG_REG_SUBSCRIPTION_TERMINATED:
            if (piListener != IMS_NULL)
            {
                piListener->RegSubscription_Terminated(LONG_TO_SINT(objMSG.nWparam));
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
PRIVATE VIRTUAL IMS_BOOL RegSubscription::InitInstance()
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

        if (!pSubState->CreateEventPackage(EVENT))
        {
            IMS_TRACE_E(0, "Creating an event package for a 'reg' event failed", 0, 0, 0);
            return IMS_FALSE;
        }

        pSubState->GetEventPackage()->SetDuration(nExpiresValue);
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

    RegInfoManager::GetInstance()->CreateRegInfo(objRegKey);

    RegInfo* pRegInfo = RegInfoManager::GetInstance()->GetRegInfo(objRegKey);

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->AddListener(this);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PRIVATE VIRTUAL IMS_BOOL RegSubscription::SendRequestToChallenge(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    if (piSIPMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "SIP message is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // Set P-Access-Network-Info header
    PAccessNetworkInfoHeader::SetHeader(GetSlotId(), pRegStateTracker->GetIPAddress(),
            pRegStateTracker->GetSIPProfile(), piSIPMsg);

    if (!Method::SendRequestToChallenge(piSCC))
    {
        return IMS_FALSE;
    }

    // Update the last request/response message
    SetPreviousRequest(piSCC->GetMessage());
    SetPreviousResponse(IMS_NULL);

    // Update the subscription state
    if (!pSubState->UpdateState(piSCC->GetMessage()))
    {
        pSubState->SetOperation(SubState::NO_OPERATION);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegSubscription::NotifySIPResponse(IN ISipClientConnection* piSCC)
{
    ISipMessage* piSIPMsg = piSCC->GetMessage();
    const SipMethod& objMethod = piSIPMsg->GetMethod();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ Response (%s) received at the state, %d", objMethod.ToString().GetStr(),
            GetState(), 0);

    if (!objMethod.Equals(SipMethod::SUBSCRIBE))
    {
        piSCC->Close();
        piOngoingSCC = IMS_NULL;
        return;
    }

    // Handle the response according to the SIP method.
    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    // Handle the response to SUBSCRIBE request ...
    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return;
    }

    SetPreviousResponse(piSIPMsg);

    IMS_SINT32 nOperation = pSubState->GetOperation();

    // Update the subscription state
    if (!pSubState->UpdateState(piSIPMsg))
    {
        if ((nOperation == SubState::OPERATION_CREATE) || (nOperation == SubState::OPERATION_FETCH))
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_START_FAILED, REASON_INTERNAL_ERROR, 0);
        }
        else if (nOperation == SubState::OPERATION_REMOVE)
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_REMOVED, REASON_NONE, 0);
        }
        else
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_INTERNAL_ERROR, 0);
        }

        pSubState->SetOperation(SubState::NO_OPERATION);
        piSCC->Close();
        piOngoingSCC = IMS_NULL;
        return;
    }

    // AUTH_SIP_DIGEST {
    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piSCC))
        {
            return;
        }
    }
    // }

    // Start or re-start a subscription refresh timer
    pRefreshHelper->UpdateOnMessageReceived(piSCC);

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // RACE_CONDITION: NOTIFY (w/ terminated) & 200 OK to SUBSCRIBE
        CheckNCreateDialog(piSCC, IMS_FALSE, IMS_TRUE);

        SetState(STATE_ACTIVE);

        if ((nOperation == SubState::OPERATION_CREATE) || (nOperation == SubState::OPERATION_FETCH))
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_STARTED, REASON_NONE, 0);
        }
        else if (nOperation == SubState::OPERATION_REMOVE)
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_REMOVED, REASON_NONE, 0);
        }
        else
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATED, REASON_NONE, 0);
        }
    }
    else
    {
        SetState(STATE_INACTIVE);

        if ((nOperation == SubState::OPERATION_CREATE) || (nOperation == SubState::OPERATION_FETCH))
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_START_FAILED, REASON_STATUS_CODE, 0);
        }
        else if (nOperation == SubState::OPERATION_REMOVE)
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_REMOVED, REASON_NONE, 0);
        }
        else
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_STATUS_CODE, 0);
        }
    }

    pSubState->SetOperation(SubState::NO_OPERATION);
    piSCC->Close();
    piOngoingSCC = IMS_NULL;

    // Check if the dialog is terminated or not
    if (nOperation != SubState::OPERATION_CREATE)
    {
        CheckDialogNCallListener();
    }

    // Initialize all the subscriber's state at this moment,
    // because the application MAY use this subscription
    // to add/fetch/refresh/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        pSubState->Clear();
        DestroyDialog();

        // RESTORE_INITIAL_EXPIRES_VALUE
        pSubState->GetEventPackage()->SetDuration(nExpiresValue);
    }

    // 3 check "nPendingOperation" member field
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegSubscription::NotifySIPError(
        IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    const SipMethod& objMethod = piSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    (void)nCode;
    (void)strMessage;

    if (!objMethod.Equals(SipMethod::SUBSCRIBE))
    {
        piSC->Close();
        piOngoingSCC = IMS_NULL;
        return;
    }

    IMS_SINT32 nOperation = pSubState->GetOperation();

    if ((nOperation == SubState::OPERATION_CREATE) || (nOperation == SubState::OPERATION_FETCH))
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_REG_SUBSCRIPTION_START_FAILED, REASON_TRANSACTION_TIMEOUT, 0);
    }
    else if (nOperation == SubState::OPERATION_REMOVE)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_REG_SUBSCRIPTION_REMOVED, REASON_NONE, 0);
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NONE, 0);
    }
    else if (nOperation == SubState::OPERATION_REFRESH)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_TRANSACTION_TIMEOUT, 0);
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NONE, 0);
    }

    piSC->Close();
    piOngoingSCC = IMS_NULL;

    // Initialize all the subscriber's state at this moment,
    // because the application MAY use this subscription
    // to add/fetch/refresh/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        pSubState->Clear();
        DestroyDialog();

        // RESTORE_INITIAL_EXPIRES_VALUE
        pSubState->GetEventPackage()->SetDuration(nExpiresValue);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegSubscription::Dialog_Compare(IN ISipServerConnection* piSSC) const
{
    //---------------------------------------------------------------------------------------------

    // Filters some method which does not handle in the subscription
    if (!piSSC->GetMethod().Equals(SipMethod::NOTIFY))
    {
        return IMS_FALSE;
    }

    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        // In case of an early NOTIFY received ...
        if (GetState() == STATE_PENDING)
        {
            IMS_SINT32 nOperation = pSubState->GetOperation();

            IMS_TRACE_I("Checks if the early NOTIFY is received or not ...", 0, 0, 0);

            if ((nOperation == SubState::OPERATION_CREATE) ||
                    (nOperation == SubState::OPERATION_FETCH))
            {
                if (piOngoingSCC != IMS_NULL)
                {
                    piDialog = piOngoingSCC->GetDialog();
                }
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
PRIVATE VIRTUAL IMS_BOOL RegSubscription::Dialog_NotifyRequest(IN ISipServerConnection* piSSC)
{
#ifdef __IMS_ASYNC_XML_PARSER__

    IMS_BOOL bDirectNotification = IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    if (!piSSC->GetMethod().Equals(SipMethod::NOTIFY))
    {
        SendResponse(piSSC, SipStatusCode::SC_405);
        piSSC->Close();
        return IMS_FALSE;
    }

    if (SipConfigProxy::IsRequestUriValidationRequiredInMidDialog(
                GetSlotId(), pRegStateTracker->GetSIPProfile()))
    {
        // Checks if Request-URI is matched or not
        const AString& strRequestURI = piSSC->GetRequestUri();
        SipAddress objRequestURI(strRequestURI);

        if (!ValidateRequestURI(objRequestURI, piSSC->GetDialog()))
        {
            IMS_TRACE_D("Request-URI (%s) in mid-dialog request is not matched",
                    SipDebug::GetUri1(strRequestURI).GetStr(), 0, 0);

            SendResponse(piSSC, SipStatusCode::SC_404);
            piSSC->Close();

            // 4 Multiple-Dialog :: Check if the subscription is valid or not
            return IMS_FALSE;
        }
    }

    // Update the dialog info.
    CheckNCreateDialog(piSSC);

    // Update the subscription state for NOTIFY request
    if (!pSubState->UpdateState(piSSC->GetMessage()))
    {
        piSSC->Close();
        return IMS_FALSE;
    }

    ISipMessage* piSIPMsg_NOTIFY = piSSC->GetMessage();

    if (piSIPMsg_NOTIFY == IMS_NULL)
    {
        if (SendResponse(piSSC, SipStatusCode::SC_500))
        {
            pSubState->UpdateState(piSSC->GetMessage());
            pRefreshHelper->UpdateOnMessageSent(piSSC);
        }

        piSSC->Close();
        return IMS_FALSE;
    }

    IMS_UINT16 nReasonParameter = GetReasonParameter(piSIPMsg_NOTIFY);

    IMS_TRACE_D("START - reginfo parsing ...", 0, 0, 0);

    // Parsing 'application/reginfo+xml' using XML parser & update the reginfo.
    IMSList<ISipMessageBodyPart*> objBodyParts = piSIPMsg_NOTIFY->GetBodyParts();

    if (!objBodyParts.IsEmpty())
    {
        AString strContentType;
        AString strType;
        AString strSubType;

        bDirectNotification = IMS_TRUE;

        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);

            if (piBodyPart == IMS_NULL)
                continue;

            strContentType = piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);

            if (!TextParser::ParseMediaType(strContentType, strType, strSubType))
                continue;

            if (!strType.EqualsIgnoreCase(MEDIA_TYPE) ||
                    !strSubType.EqualsIgnoreCase(MEDIA_SUB_TYPE))
            {
                continue;
            }

            AString strRegInfo = piBodyPart->GetContent().ToString();

            strContentType.Prepend("REG_INFO ( ");
            strContentType.Append(" )");

            if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
            {
                IMS_TRACE_XML(strContentType.GetStr(), strRegInfo.GetStr(), strRegInfo.GetLength());
            }

            RegInfoManager::GetInstance()->Update(objRegKey, strRegInfo);

            objNotifications.Append(Notification(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED,
                    pSubState->GetSubState(), REG_SUB_MAKE_LPARAM(nReasonParameter, 1)));

            bDirectNotification = IMS_FALSE;
        }
    }
    else
    {
        IMS_TRACE_D("RegSubscription -> No body parts", 0, 0, 0);

        if (objNotifications.IsEmpty())
        {
            bDirectNotification = IMS_TRUE;
        }
        else
        {
            objNotifications.Append(Notification(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED,
                    pSubState->GetSubState(), REG_SUB_MAKE_LPARAM(nReasonParameter, 0)));
        }
    }

    // Send a 2xx response to NOTIFY request immediately
    if (SendResponse(piSSC, SipStatusCode::SC_200) == IMS_FALSE)
    {
        piSSC->Close();

        IMS_TRACE_E(0, "Creating & sending the response to NOTIFY request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Update the subscription state for NOTIFY response
    if (!pSubState->UpdateState(piSSC->GetMessage()))
    {
        piSSC->Close();
        return IMS_FALSE;
    }

    // Update the subscription refresh timer
    pRefreshHelper->UpdateOnMessageSent(piSSC);

    piSSC->Close();

    if (bDirectNotification)
    {
        PostMessage(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, pSubState->GetSubState(),
                REG_SUB_MAKE_LPARAM(nReasonParameter, 0));
    }

    if (pSubState->GetState() == SubState::STATE_TERMINATED)
    {
        if (objNotifications.IsEmpty())
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NOTIFY_TERMINATED, 0);
        }
        else
        {
            objNotifications.Append(
                    Notification(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NOTIFY_TERMINATED, 0));
        }
    }

#else  // __IMS_ASYNC_XML_PARSER__

    //---------------------------------------------------------------------------------------------

    if (!piSSC->GetMethod().Equals(SipMethod::NOTIFY))
    {
        SendResponse(piSSC, SipStatusCode::SC_405);
        piSSC->Close();
        return IMS_FALSE;
    }

    if (SipConfigProxy::IsRequestUriValidationRequiredInMidDialog(
                GetSlotId(), pRegStateTracker->GetSIPProfile()))
    {
        // Checks if Request-URI is matched or not
        const AString& strRequestURI = piSSC->GetRequestUri();
        SipAddress objRequestURI(strRequestURI);

        if (!ValidateRequestURI(objRequestURI, piSSC->GetDialog()))
        {
            IMS_TRACE_D("Request-URI (%s) in mid-dialog request is not matched",
                    SipDebug::GetUri1(strRequestURI).GetStr(), 0, 0);

            SendResponse(piSSC, SipStatusCode::SC_404);
            piSSC->Close();
            return IMS_FALSE;
        }
    }

    // Update the dialog info.
    CheckNCreateDialog(piSSC);

    // Update the subscription state for NOTIFY request
    if (!pSubState->UpdateState(piSSC->GetMessage()))
    {
        piSSC->Close();
        return IMS_FALSE;
    }

    ISipMessage* piSIPMsg_NOTIFY = piSSC->GetMessage();

    if (piSIPMsg_NOTIFY == IMS_NULL)
    {
        if (SendResponse(piSSC, SipStatusCode::SC_500))
        {
            pSubState->UpdateState(piSSC->GetMessage());
            pRefreshHelper->UpdateOnMessageSent(piSSC);
        }

        piSSC->Close();
        return IMS_FALSE;
    }

    IMS_UINT16 nReasonParameter = GetReasonParameter(piSIPMsg_NOTIFY);

    IMS_TRACE_D("START - reginfo parsing ...", 0, 0, 0);

    // Parsing 'application/reginfo+xml' using XML parser & update the reginfo.
    IMSList<ISipMessageBodyPart*> objBodyParts = piSIPMsg_NOTIFY->GetBodyParts();
    IMS_UINT16 nBodyParts = 0;

    if (!objBodyParts.IsEmpty())
    {
        AString strContentType;
        AString strType;
        AString strSubType;

        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);

            if (piBodyPart == IMS_NULL)
                continue;

            strContentType = piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);

            if (!TextParser::ParseMediaType(strContentType, strType, strSubType))
                continue;

            if (!strType.EqualsIgnoreCase(MEDIA_TYPE) ||
                    !strSubType.EqualsIgnoreCase(MEDIA_SUB_TYPE))
            {
                continue;
            }

            AString strRegInfo = piBodyPart->GetContent().ToString();

            strContentType.Prepend("REG_INFO ( ");
            strContentType.Append(" )");

            if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
            {
                IMS_TRACE_XML(strContentType.GetStr(), strRegInfo.GetStr(), strRegInfo.GetLength());
            }

            RegInfoManager::GetInstance()->Update(objRegKey, strRegInfo);

            nBodyParts = 1;
        }
    }
    else
    {
        IMS_TRACE_D("RegSubscription -> No body parts", 0, 0, 0);
    }

    // Send a 2xx response to NOTIFY request immediately
    if (SendResponse(piSSC, SipStatusCode::SC_200) == IMS_FALSE)
    {
        piSSC->Close();

        IMS_TRACE_E(0, "Creating & sending the response to NOTIFY request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Update the subscription state for NOTIFY response
    if (!pSubState->UpdateState(piSSC->GetMessage()))
    {
        piSSC->Close();
        return IMS_FALSE;
    }

    // Update the subscription refresh timer
    pRefreshHelper->UpdateOnMessageSent(piSSC);

    piSSC->Close();

    PostMessage(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, pSubState->GetSubState(),
            REG_SUB_MAKE_LPARAM(nReasonParameter, nBodyParts));

    if (pSubState->GetState() == SubState::STATE_TERMINATED)
    {
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NOTIFY_TERMINATED, 0);
    }

#endif  // __IMS_ASYNC_XML_PARSER__

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegSubscription::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    (void)piSCC;

    IMS_TRACE_I("___ REGISTRATION SUBSCRIBER REFRESH COMPLETED ... Code (%d)", nCode, 0, 0);

    // In case, the subscription refresh request is successfully done.
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        SetPreviousResponse(piSCC->GetMessage());

        if (!pSubState->UpdateState(piSCC->GetMessage()))
        {
            // Set the operation
            pSubState->SetOperation(SubState::NO_OPERATION);

            // Check if the dialog is terminated or not
            CheckDialogNCallListener();

            return;
        }

        // AUTH_SIP_DIGEST {
        if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
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

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATED, REASON_STATUS_CODE, 0);
        }
        else
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_STATUS_CODE, 0);
        }

        // Check if the dialog is terminated or not
        CheckDialogNCallListener();
    }
    // The subscription refresh request is timed out.
    else if (nCode == RefreshHelper::TRANSACTION_TIMEOUT)
    {
        // 4 what to do ... ? In this moment, do nothing ...
        PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_TRANSACTION_TIMEOUT, 0);

        if (!IsFeatureEnabled(FEATURE_KEEP_ACTIVE_ON_REFRESH_TRANSACTION_TIMEOUT))
        {
            Refreshable_RefreshTerminated();
        }
    }

    // Set the operation
    pSubState->SetOperation(SubState::NO_OPERATION);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegSubscription::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("___ REGISTRATION SUBSCRIPTION REFRESH STARTED ... State(%d)", GetState(), 0, 0);

    if (GetState() == STATE_ACTIVE)
    {
        if (piListener != IMS_NULL)
        {
            piListener->RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);
        }

        if (bDoImplicitRefresh)
        {
            return SubscribeOnImplicitRefresh();
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegSubscription::Refreshable_RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("_____ REGISTRATION SUBSCRIBER REFRESH TERMINATED ...", 0, 0, 0);

    if (GetState() == STATE_ACTIVE)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_REFRESH_TIMEOUT, 0);
    }

    if (GetState() == STATE_INACTIVE)
    {
        pSubState->Clear();
        DestroyDialog();

        // RESTORE_INITIAL_EXPIRES_VALUE
        pSubState->GetEventPackage()->SetDuration(nExpiresValue);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegSubscription::RegInfo_Updated(IN IMS_BOOL bStale /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

#ifdef __IMS_ASYNC_XML_PARSER__

    // Process in order
    if (!objNotifications.IsEmpty())
    {
        const Notification& objNotification = objNotifications.GetAt(0);

        PostMessage(objNotification.nMSG, objNotification.nWParam, objNotification.nLParam);

        objNotifications.RemoveAt(0);
    }

    // Process if the body does not exist
    while (!objNotifications.IsEmpty())
    {
        const Notification& objNotification = objNotifications.GetAt(0);

        if (objNotification.nLParam == 0)
        {
            PostMessage(objNotification.nMSG, objNotification.nWParam, objNotification.nLParam);

            objNotifications.RemoveAt(0);
        }
        else
        {
            break;
        }
    }

#endif  // __IMS_ASYNC_XML_PARSER__

    if (bStale)
    {
        // 4 refresh the subscription
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegSubscription::RegInfo_UpdateFailed()
{
    //---------------------------------------------------------------------------------------------

#ifdef __IMS_ASYNC_XML_PARSER__

    // Process in order;
    // Because of update failure, do not sent any notification, then discard the message.
    if (!objNotifications.IsEmpty())
    {
        objNotifications.RemoveAt(0);
    }

    // Process if the body does not exist
    while (!objNotifications.IsEmpty())
    {
        const Notification& objNotification = objNotifications.GetAt(0);

        if (objNotification.nLParam == 0)
        {
            PostMessage(objNotification.nMSG, objNotification.nWParam, objNotification.nLParam);

            objNotifications.RemoveAt(0);
        }
        else
        {
            break;
        }
    }

#endif  // __IMS_ASYNC_XML_PARSER__
}

/*

Remarks

*/
PRIVATE
void RegSubscription::CheckDialogNCallListener()
{
    // Check if the dialog is terminated or not
    ISipDialog* piDialog = GetDialog();

    //---------------------------------------------------------------------------------------------

    if (piDialog != IMS_NULL)
    {
        if (piDialog->GetState() == ISipDialog::STATE_TERMINATED)
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NONE, 0);
        }
    }
    else
    {
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NONE, 0);
    }
}

/*

Remarks

*/
PRIVATE
void RegSubscription::ClearNextRequest()
{
    //---------------------------------------------------------------------------------------------

    if (piNextRequest != IMS_NULL)
    {
        piNextRequest->Destroy();
        piNextRequest = IMS_NULL;
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegSubscription::CopyHeadersAndBodyParts(IN_OUT ISipMessage*& piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    // Copy the headers from the application
    if (piNextRequest != IMS_NULL)
    {
        IMS_BOOL bGRUURequired = IMS_FALSE;
        const AString strGRUU(Sip::STR_GRUU);

        bGRUURequired = piSIPMsg->IsOptionSupported(strGRUU);

        // Set the headers and body parts if application already sets
        if (piSIPMsg->CopyHeadersAndBodyParts(piNextRequest) != IMS_SUCCESS)
        {
            IMS::SetLastError(IMSError::GENERAL_ERROR);
            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (bGRUURequired && !piSIPMsg->IsOptionSupported(strGRUU))
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegSubscription::IsFeatureEnabled(IN IMS_SINT32 nFeature) const
{
    //---------------------------------------------------------------------------------------------

    return (nFeatureSet & nFeature) != 0;
}

/*

Remarks

*/
PRIVATE
void RegSubscription::SetOngoingConnection(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piOngoingSCC != IMS_NULL)
    {
        piOngoingSCC->Close();
        piOngoingSCC = IMS_NULL;
    }

    if (piSCC != IMS_NULL)
    {
        piOngoingSCC = piSCC;
    }
}

/*

Remarks

*/
PRIVATE
void RegSubscription::SetPreviousRequest(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piPreviousRequest != IMS_NULL)
    {
        piPreviousRequest->Destroy();
        piPreviousRequest = IMS_NULL;
    }

    if (piSIPMsg != IMS_NULL)
    {
        piPreviousRequest = piSIPMsg->Clone();
    }
}

/*

Remarks

*/
PRIVATE
void RegSubscription::SetPreviousResponse(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piPreviousResponse != IMS_NULL)
    {
        piPreviousResponse->Destroy();
        piPreviousResponse = IMS_NULL;
    }

    if (piSIPMsg != IMS_NULL)
    {
        piPreviousResponse = piSIPMsg->Clone();
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegSubscription::SendResponse(IN ISipServerConnection* piSSC, IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    if (piSSC->InitResponse(nStatusCode) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Initializing a SIP response failed - SipError(%d)",
                SipError::GetLastError(), 0, 0);

        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    ISipMessage* piSIPMsg = piSSC->GetMessage();

    // Sets Allow header fields
    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        IMS_BOOL bIsContactGRUU = IMS_FALSE;

        // Set Contact headers
        if (!SetContactHeader(piSIPMsg, bIsContactGRUU))
        {
            IMS::SetLastError(IMSError::GENERAL_ERROR);
            return IMS_FALSE;
        }

        if (bIsContactGRUU)
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_GRUU);
        }

        // Do not P-Preferred-Identity header in case of NOTIFY

        const AStringArray& objMethods =
                SipConfigProxy::GetRegAllowMethods(GetSlotId(), pRegStateTracker->GetSIPProfile());

        for (IMS_SINT32 j = 0; j < objMethods.GetCount(); ++j)
        {
            if (piSIPMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(j)) != IMS_SUCCESS)
            {
                IMS::SetLastError(IMSError::GENERAL_ERROR);
                return IMS_FALSE;
            }
        }
    }

    // Sets P-Access-Network-Info header field
    PAccessNetworkInfoHeader::SetHeader(GetSlotId(), pRegStateTracker->GetIPAddress(),
            pRegStateTracker->GetSIPProfile(), piSIPMsg);

    // Sets Server header field - User-Agent ?
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), pRegStateTracker->GetSIPProfile()))
    {
        if (SipConfigProxy::IsUserAgentSetByContext(GetSlotId(), pRegStateTracker->GetSIPProfile()))
        {
            UserAgentHeader::SetHeader(SipHeaderName::SERVER, pRegStateTracker->GetSIPProfile(),
                    AString::ConstNull(), pRegStateTracker->GetIPAddress(), GetSlotId(), piSIPMsg);
        }
        else
        {
            UserAgentHeader::SetHeader(SipHeaderName::USER_AGENT, pRegStateTracker->GetSIPProfile(),
                    AString::ConstNull(), pRegStateTracker->GetIPAddress(), GetSlotId(), piSIPMsg);
        }
    }

    // Sets the SIP transaction timers
    piSSC->SetTransactionTimerValues(
            SipTimerValuesHelper::GetValues(GetSlotId(), pRegStateTracker->GetSIPProfile()));

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSSC->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

    if (piSSC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending SIP response failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegSubscription::SetContactHeader(
        IN_OUT ISipMessage*& piSIPMsg, OUT IMS_BOOL& bIsContactGRUU)
{
    const RegContact* pRegContact = pRegStateTracker->GetPreferredContact();
    AString strContact;

    //---------------------------------------------------------------------------------------------

    bIsContactGRUU = IMS_FALSE;

    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), pRegStateTracker->GetSIPProfile()))
    {
        if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), pRegStateTracker->GetSIPProfile()))
        {
            SipAddress objContact;

            if (pRegContact != IMS_NULL)
            {
                // 4 Consider the Privacy information (temp-gruu)
                const SipAddress* pPubGRUU = pRegContact->GetPublicGRUU();

                if (pPubGRUU != IMS_NULL)
                {
                    bIsContactGRUU = IMS_TRUE;
                    objContact = *pPubGRUU;
                }
                else
                {
                    const SipAddress* pContact =
                            pRegStateTracker->GetContactAddressForOutgoingMessage();

                    objContact = (pContact != IMS_NULL) ? *pContact
                                                        : pRegStateTracker->GetContactAddress();
                }
            }
            else
            {
                const SipAddress* pContact =
                        pRegStateTracker->GetContactAddressForOutgoingMessage();

                objContact =
                        (pContact != IMS_NULL) ? *pContact : pRegStateTracker->GetContactAddress();
            }

            if (SipConfigProxy::IsMultipleRegConfigured(
                        GetSlotId(), pRegStateTracker->GetSIPProfile()))
            {
                objContact.AddParameter(Sip::STR_OB, AString::ConstNull());
            }

            strContact = objContact.ToString();
        }
        else
        {
            if (pRegContact != IMS_NULL)
            {
                // 4 Consider the Privacy information (temp-gruu)
                const SipAddress* pPubGRUU = pRegContact->GetPublicGRUU();

                if (pPubGRUU != IMS_NULL)
                {
                    bIsContactGRUU = IMS_TRUE;
                    strContact = pPubGRUU->ToString();
                }
                else
                {
                    const SipAddress* pContact =
                            pRegStateTracker->GetContactAddressForOutgoingMessage();

                    strContact = (pContact != IMS_NULL)
                            ? pContact->ToString()
                            : pRegStateTracker->GetContactAddress().ToString();
                }
            }
            else
            {
                const SipAddress* pContact =
                        pRegStateTracker->GetContactAddressForOutgoingMessage();

                strContact = (pContact != IMS_NULL)
                        ? pContact->ToString()
                        : pRegStateTracker->GetContactAddress().ToString();
            }
        }
    }
    else
    {
        if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), pRegStateTracker->GetSIPProfile()))
        {
            const SipAddress* pContact = pRegStateTracker->GetContactAddressForOutgoingMessage();
            SipAddress objContact =
                    (pContact != IMS_NULL) ? *pContact : pRegStateTracker->GetContactAddress();

            if (SipConfigProxy::IsMultipleRegConfigured(
                        GetSlotId(), pRegStateTracker->GetSIPProfile()))
            {
                objContact.AddParameter(Sip::STR_OB, AString::ConstNull());
            }

            strContact = objContact.ToString();
        }
        else
        {
            const SipAddress* pContact = pRegStateTracker->GetContactAddressForOutgoingMessage();

            strContact = (pContact != IMS_NULL) ? pContact->ToString()
                                                : pRegStateTracker->GetContactAddress().ToString();
        }
    }

    IMS_BOOL bDeviceIdRequired = IMS_FALSE;
    IMS_BOOL bIsWithinTrustDomain = pRegStateTracker->IsWithinTrustDomain(GetSlotId());

    if (!bIsContactGRUU && bIsWithinTrustDomain)
    {
        bDeviceIdRequired = IMS_TRUE;
    }

    // FIXME: it is not required, if not necessary, remove it later.
    if (!bDeviceIdRequired && !bIsContactGRUU)
    {
        if (SipConfigProxy::IsGruuConfigured(GetSlotId(), pRegStateTracker->GetSIPProfile()) &&
                bIsWithinTrustDomain)
        {
            bDeviceIdRequired = IMS_TRUE;
        }
    }

    IMS_BOOL bInstanceParameterIncluded = IMS_FALSE;

    if (bDeviceIdRequired && (pRegContact != IMS_NULL))
    {
        // Add the '+sip.instance' parameter
        const SipParameter* pParameter = pRegContact->GetInstanceParameter();

        if (pParameter != IMS_NULL)
        {
            bInstanceParameterIncluded = IMS_TRUE;

            strContact.Append(TextParser::CHAR_SEMICOLON);
            strContact.Append(pParameter->ToString());
        }
    }

    if (piSIPMsg->GetMethod().Equals(SipMethod::NOTIFY))
    {
        ISipDialog* piDialog = GetDialog();
        const ISipHeader* piHeader =
                (piDialog != IMS_NULL) ? piDialog->GetContactHeader() : IMS_NULL;

        if (piHeader != IMS_NULL)
        {
            const IMSList<SipParameter*>& objParams = piHeader->GetParameters();

            for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
            {
                const SipParameter* pParameter = objParams.GetAt(i);

                if (pParameter == IMS_NULL)
                {
                    continue;
                }

                if (bInstanceParameterIncluded)
                {
                    if (pParameter->GetName().Equals("+sip.instance"))
                    {
                        continue;
                    }
                }

                strContact.Append(TextParser::CHAR_SEMICOLON);
                strContact.Append(pParameter->ToString());
            }
        }
    }

    if (piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContact) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegSubscription::SetHeaders(IN_OUT ISipMessage*& piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (piSIPMsg->GetMethod().Equals(SipMethod::SUBSCRIBE))
    {
        // Event header
        piSIPMsg->SetHeader(ISipHeader::EVENT, EVENT);

        // Add a User-Agent if configurable
        if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), pRegStateTracker->GetSIPProfile()))
        {
            UserAgentHeader::SetHeader(SipHeaderName::USER_AGENT, pRegStateTracker->GetSIPProfile(),
                    AString::ConstNull(), pRegStateTracker->GetIPAddress(), GetSlotId(), piSIPMsg);
        }

        IMS_BOOL bIsContactGRUU = IMS_FALSE;

        if (GetState() == STATE_INACTIVE)
        {
            // Sets Contact headers
            if (!SetContactHeader(piSIPMsg, bIsContactGRUU))
            {
                return IMS_FALSE;
            }

            // Sets Route headers
            const AStringArray& objServiceRoutes = pRegStateTracker->GetServiceRoutes();

            for (IMS_SINT32 i = 0; i < objServiceRoutes.GetCount(); ++i)
            {
                if (piSIPMsg->AddHeader(ISipHeader::ROUTE, objServiceRoutes.GetElementAt(i)) !=
                        IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Setting Route header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
        else
        {
            ISipDialog* piDialog = GetDialog();
            const ISipHeader* piContactHeader =
                    (piDialog != IMS_NULL) ? piDialog->GetContactHeader() : IMS_NULL;
            const SipAddress* pContact =
                    (piContactHeader != IMS_NULL) ? piContactHeader->GetSipAddress() : IMS_NULL;

            if (pContact != IMS_NULL)
            {
                bIsContactGRUU = (pContact->GetParameter("gr") != IMS_NULL);
            }
        }

        const AString strGRUU(Sip::STR_GRUU);

        if (bIsContactGRUU && !piSIPMsg->IsOptionSupported(strGRUU))
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void RegSubscription::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I(
            "RegSubscription :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegSubscription::SubscribeOnImplicitRefresh()
{
    //---------------------------------------------------------------------------------------------

    // Set the operation
    pSubState->SetOperation(SubState::OPERATION_IMPLICIT_REFRESH);

    // Send a refresh request : SUBSCRIBE
    ISipClientConnection* piSCC = CreateConnection(this);

    if (piSCC == IMS_NULL)
    {
        pSubState->SetOperation(SubState::NO_OPERATION);

        IMS_TRACE_E(0, "Creating a new SIP connection for a 'reg' subscription refresh failed", 0,
                0, 0);
        return IMS_FALSE;
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();
    ISipMessage* piInitialSIPMsg = pSubState->GetInitialMessage();

    if (piInitialSIPMsg != IMS_NULL)
    {
        if (piSIPMsg->CopyHeadersAndBodyParts(piInitialSIPMsg) != IMS_SUCCESS)
        {
            pSubState->SetOperation(SubState::NO_OPERATION);
            piSCC->Close();

            IMS_TRACE_E(0, "Setting SIP headers to refresh SUBSCRIBE request failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    if (pRefreshHelper->SendRefreshRequest(piSCC) != IMS_SUCCESS)
    {
        pSubState->SetOperation(SubState::NO_OPERATION);
        piSCC->Close();

        IMS_TRACE_E(0, "Sending a subscription refresh request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Clear the next request
    ClearNextRequest();

    // Update the last request/response message
    SetPreviousRequest(piSCC->GetMessage());
    SetPreviousResponse(IMS_NULL);

    // Update the subscription state
    if (!pSubState->UpdateState(piSCC->GetMessage()))
    {
        pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
// IMS_REQUEST_URI_VALIDATION_IN_MID_DIALOG
PRIVATE
IMS_BOOL RegSubscription::ValidateRequestURI(
        IN CONST SipAddress& objRequestURI, IN ISipDialog* piDialog) const
{
    // Checks GRUU identities if it is supported
    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), pRegStateTracker->GetSIPProfile()))
    {
        static const AString GR("gr");
        const SipParameter* pParamGR = objRequestURI.GetParameter(GR);

        if (pParamGR != IMS_NULL)
        {
            if (pParamGR->GetValue().GetLength() > 0)
            {
                // If the pub-gruu is available, then compare it with the Request-URI
                const SipAddress* pPubGRUU =
                        pRegStateTracker->GetPreferredContact()->GetPublicGRUU();

                if (pPubGRUU != IMS_NULL)
                {
                    if (pPubGRUU->Equals(objRequestURI))
                    {
                        IMS_TRACE_D("pub-gruu is matched", 0, 0, 0);
                        return IMS_TRUE;
                    }
                }
            }
            else
            {
                // 4 List ?
                //  If the temp-gruu is available, then compare it with the Request-URI
                const SipAddress* pTempGRUU =
                        pRegStateTracker->GetPreferredContact()->GetTemporaryGRUU();

                if (pTempGRUU != IMS_NULL)
                {
                    if (pTempGRUU->Equals(objRequestURI))
                    {
                        IMS_TRACE_D("temp-gruu is matched", 0, 0, 0);
                        return IMS_TRUE;
                    }
                }
            }

            IMS_TRACE_D(
                    "Request-URI contains a 'gr' parameter & GRUU ids are not matched", 0, 0, 0);

            return IMS_FALSE;
        }
    }

    // Checks if the contact address matches or not.
    const SipAddress* pContact = IMS_NULL;

    if (piDialog != IMS_NULL)
    {
        const ISipHeader* piContactHeader = piDialog->GetContactHeader();

        if (piContactHeader != IMS_NULL)
        {
            pContact = piContactHeader->GetSipAddress();
        }
    }

    if (pContact == IMS_NULL)
    {
        IMS_TRACE_D("Contact header is not present in the dialog; "
                    "use the default contact address...",
                0, 0, 0);

        pContact = pRegStateTracker->GetContactAddressForOutgoingMessage();

        if (pContact == IMS_NULL)
        {
            const SipAddress& objContactURI = pRegStateTracker->GetContactAddress();
            pContact = &objContactURI;
        }
    }

    if (pContact->Equals(objRequestURI))
    {
        return IMS_TRUE;
    }

    IMS_TRACE_D("Request-URI in mid-dialog request is not matched", 0, 0, 0);

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE GLOBAL ISipClientConnection* RegSubscription::CreateConnection(IN RegSubscription* pRegSub)
{
    AString strAOR = pRegSub->GetUserAOR()->ToString();
    RegStateTracker* pStateTracker = pRegSub->pRegStateTracker.Get();
    ISipClientConnection* piSCC = IMS_NULL;
    IMS_BOOL bOverwriteTarget = IMS_FALSE;
    IMS_SINT32 nTransportExt = pStateTracker->GetTransportExt();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("CreateConnection - To (%s), Method (SUBSCRIBE)",
            SipDebug::GetUri1(strAOR).GetStr(), 0, 0);

    if (pRegSub->GetState() == STATE_ACTIVE)
    {
        ISipDialog* piDialog = pRegSub->GetDialog();

        piSCC = piDialog->GetNewClientConnection(SipMethod::ToName(SipMethod::SUBSCRIBE));

        // AUTH_SIP_DIGEST {
        // Sets an authentication challenge & credentials
        pRegSub->SetChallengeNCredentials(piSCC);
        // }
    }
    else
    {
        if (!pRegSub->GetUserAOR()->IsSchemeSip() && !pRegSub->GetUserAOR()->IsSchemeSips())
        {
            bOverwriteTarget = IMS_TRUE;
            strAOR = ImsIdentity::GetAnonymousUserId();
        }

        piSCC = DYNAMIC_CAST(ISipClientConnection*, Connector::Open(strAOR));

        if (pRegSub->IsFeatureEnabled(
                    IRegSubscription::FEATURE_USE_TCP_TRANSPORT_ON_INITIAL_SUBSCRIPTION))
        {
            nTransportExt |= Sip::TRANSPORT_EXT_TCP;
            IMS_TRACE_D("RegSubscription :: TCP transport protocol will be used", 0, 0, 0);
        }
    }

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a SIP client connection failed", 0, 0, 0);
        return IMS_NULL;
    }

    // In case of a multiple PDConnections & SIPConnectionNotifiers,
    // we needs to have a policy of which network will have a preference between the connections.

    // MULTI_REG_SIP_PROFILE
    piSCC->SetSipProfile(pStateTracker->GetSIPProfile());

    // Sets the transport tuples
    // RFC5626_FLOW_CONTROL
    // MULTI_REG_TRANSPORT
    piSCC->SetTransportTuple(pStateTracker->GetIPAddress(), pStateTracker->GetPortUS(),
            pStateTracker->GetPortUC(), pStateTracker->GetPortFlowControl(), nTransportExt);

    // Sets the SIP transaction timers
    if (pRegSub->pSIPTVs != IMS_NULL)
    {
        piSCC->SetTransactionTimerValues(*(pRegSub->pSIPTVs));
    }
    else
    {
        piSCC->SetTransactionTimerValues(SipTimerValuesHelper::GetValues(
                pRegSub->GetSlotId(), pStateTracker->GetSIPProfile()));
    }

    if (pRegSub->GetState() != STATE_ACTIVE)
    {
        if (piSCC->InitRequest(SipMethod::ToName(SipMethod::SUBSCRIBE), IMS_NULL) != IMS_SUCCESS)
        {
            piSCC->Close();
            return IMS_NULL;
        }

        // Re-write the Request-URI & To header if the URI is not "sip" or "sips" URI
        if (bOverwriteTarget)
        {
            strAOR = pRegSub->GetUserAOR()->ToString();

            if (piSCC->SetRequestUri(strAOR) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Request-URI failed", 0, 0, 0);

                piSCC->Close();
                return IMS_NULL;
            }

            // Sets To header field
            if (piSCC->GetMessage()->SetHeader(ISipHeader::TO, strAOR) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting To header failed", 0, 0, 0);

                piSCC->Close();
                return IMS_NULL;
            }
        }

        // From
        if (piSCC->GetMessage()->SetHeader(ISipHeader::FROM, strAOR) != IMS_SUCCESS)
        {
            piSCC->Close();
            return IMS_NULL;
        }
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    // Sets Accept header field
    AString strAccept;

    strAccept.Append(MEDIA_TYPE);
    strAccept.Append('/');
    strAccept.Append(MEDIA_SUB_TYPE);

    if (piSIPMsg->SetHeader(ISipHeader::ACCEPT, strAccept) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Adding Accept header failed", 0, 0, 0);

        piSCC->Close();
        return IMS_NULL;
    }

    // Sets P-Preferred-Identity header fields
    // hwangoo.park, 130514, do not add P-Preferred-Identity header
    if (SipConfigProxy::IsPPreferredIdInRegSubRequired(
                pRegSub->GetSlotId(), pStateTracker->GetSIPProfile()))
    {
        if (piSIPMsg->SetHeader(ISipHeader::P_PREFERRED_IDENTITY, strAOR) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding P-Preferred-Identity header failed", 0, 0, 0);

            piSCC->Close();
            return IMS_NULL;
        }
    }

    // Add an allowed/supported methods for this UA
    const AStringArray& objMethods = SipConfigProxy::GetRegAllowMethods(
            pRegSub->GetSlotId(), pStateTracker->GetSIPProfile());

    for (IMS_SINT32 i = 0; i < objMethods.GetCount(); ++i)
    {
        if (piSIPMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(i)) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Allow header failed", 0, 0, 0);

            piSCC->Close();
            return IMS_NULL;
        }
    }

    // IPSEC {
    {
        // RFC 3329 - SIP Security Agreement :: Sets Security-Client / Security-Verify headers
        // Do not add Security-Client headers
#if 0
        const AStringArray &objSecurityClients = pStateTracker->GetSecurityClients();

        for (IMS_SINT32 i = 0; i < objSecurityClients.GetCount(); ++i)
        {
            const AString &strHeader = objSecurityClients.GetElementAt(i);

            if (piSIPMsg->AddHeader(ISipHeader::SECURITY_CLIENT, strHeader) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding Security-Client header failed", 0, 0, 0);

                piSCC->Close();
                return IMS_NULL;
            }
        }
#endif

        const AStringArray& objSecurityVerifys = pStateTracker->GetSecurityVerifys();

        for (IMS_SINT32 i = 0; i < objSecurityVerifys.GetCount(); ++i)
        {
            const AString& strHeader = objSecurityVerifys.GetElementAt(i);

            if (piSIPMsg->AddHeader(ISipHeader::SECURITY_VERIFY, strHeader) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding Security-Verify header failed", 0, 0, 0);

                piSCC->Close();
                return IMS_NULL;
            }
        }

        if (!objSecurityVerifys.IsEmpty())
        {
            piSIPMsg->AddHeader(ISipHeader::REQUIRE, Sip::STR_SEC_AGREE);
            piSIPMsg->AddHeader(
                    ISipHeader::UNKNOWN, Sip::STR_SEC_AGREE, SipHeaderName::PROXY_REQUIRE);
        }
    }
    // }

    // Set P-Access-Network-Info header
    PAccessNetworkInfoHeader::SetHeader(pRegSub->GetSlotId(), pStateTracker->GetIPAddress(),
            pStateTracker->GetSIPProfile(), piSIPMsg);

    return piSCC;
}

/*

Remarks

*/
PRIVATE GLOBAL IMS_UINT16 RegSubscription::GetReasonParameter(IN ISipMessage* piMessage)
{
    AString strSubState = piMessage->GetHeader(ISipHeader::SUBSCRIPTION_STATE);
    ISipHeader* piHeader =
            SipParsingHelper::CreateHeader(ISipHeader::SUBSCRIPTION_STATE, strSubState);
    IMS_UINT16 nReason = IRegSubscription::PARAM_REASON_NONE;
    ;

    //---------------------------------------------------------------------------------------------

    if (piHeader == IMS_NULL)
    {
        return nReason;
    }

    nReason = SubState::ExtractReasonParameter(piHeader);

    if (nReason == SubState::REASON_NORESOURCE)
    {
        nReason = IRegSubscription::PARAM_REASON_NORESOURCE;
    }
    else if (nReason == SubState::REASON_DEACTIVATED)
    {
        nReason = IRegSubscription::PARAM_REASON_DEACTIVATED;
    }
    else if (nReason == SubState::REASON_PROBATION)
    {
        nReason = IRegSubscription::PARAM_REASON_PROBATION;
    }
    else if (nReason == SubState::REASON_REJECTED)
    {
        nReason = IRegSubscription::PARAM_REASON_REJECTED;
    }
    else if (nReason == SubState::REASON_TIMEOUT)
    {
        nReason = IRegSubscription::PARAM_REASON_TIMEOUT;
    }
    else if (nReason == SubState::REASON_GIVEUP)
    {
        nReason = IRegSubscription::PARAM_REASON_GIVEUP;
    }

    piHeader->Destroy();

    return nReason;
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* RegSubscription::StateToString(IN IMS_SINT32 nState)
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
