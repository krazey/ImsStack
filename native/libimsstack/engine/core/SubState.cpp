/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100412  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"
#include "ISipHeader.h"
#include "Sip.h"
#include "SipDebug.h"
#include "SipParameter.h"
#include "ISubscriptionState.h"
#include "SubState.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL const IMS_CHAR SubState::STR_REASON[] = "reason";
PUBLIC GLOBAL const IMS_CHAR SubState::STR_REASON_DEACTIVATED[] = "deactivated";
PUBLIC GLOBAL const IMS_CHAR SubState::STR_REASON_PROBATION[] = "probation";
PUBLIC GLOBAL const IMS_CHAR SubState::STR_REASON_REJECTED[] = "rejected";
PUBLIC GLOBAL const IMS_CHAR SubState::STR_REASON_TIMEOUT[] = "timeout";
PUBLIC GLOBAL const IMS_CHAR SubState::STR_REASON_GIVEUP[] = "giveup";
PUBLIC GLOBAL const IMS_CHAR SubState::STR_REASON_NORESOURCE[] = "noresource";

PUBLIC
SubState::SubState() :
        nState(STATE_INIT),
        nOperation(NO_OPERATION),
        nConfigValue(CONFIG_NONE),
        nSubscriptionDuration(NO_EXPIRES),
        nSubStateValue(SUB_STATE_INIT),
        bFlag_SubscriptionDurationUpdated(IMS_FALSE),
        bFlag_InstantSubscription(IMS_FALSE),
        piSIPMsg(IMS_NULL)
{
}

PUBLIC VIRTUAL SubState::~SubState()
{
    if (piSIPMsg != IMS_NULL)
    {
        piSIPMsg->Destroy();
    }
}

PUBLIC VIRTUAL void SubState::Clear()
{
    //---------------------------------------------------------------------------------------------

    // Clear the event package
    objEventPackage.SetDuration(NO_EXPIRES);
    objEventPackage.SetEventHeader(IMS_NULL);

    nState = STATE_INIT;
    nOperation = NO_OPERATION;
    nSubscriptionDuration = NO_EXPIRES;
    nSubStateValue = SUB_STATE_INIT;

    bFlag_SubscriptionDurationUpdated = IMS_FALSE;
    bFlag_InstantSubscription = IMS_FALSE;

    if (piSIPMsg != IMS_NULL)
    {
        piSIPMsg->Destroy();
        piSIPMsg = IMS_NULL;
    }
}

PUBLIC
IMS_BOOL SubState::CreateEventPackage(IN CONST AString& strEvent)
{
    //---------------------------------------------------------------------------------------------

    // 1 : Load an event package from configuration

    objEventPackage.SetEvent(strEvent);

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 SubState::GetConfiguration() const
{
    //---------------------------------------------------------------------------------------------

    return nConfigValue;
}

PUBLIC
IMS_SINT32 SubState::GetDuration() const
{
    //---------------------------------------------------------------------------------------------

    return nSubscriptionDuration;
}

PUBLIC
EventPackage* SubState::GetEventPackage()
{
    //---------------------------------------------------------------------------------------------

    return &objEventPackage;
}

PUBLIC
ISipMessage* SubState::GetInitialMessage() const
{
    //---------------------------------------------------------------------------------------------

    return piSIPMsg;
}

PUBLIC
IMS_SINT32 SubState::GetOperation() const
{
    //---------------------------------------------------------------------------------------------

    return nOperation;
}

PUBLIC
IMS_SINT32 SubState::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

PUBLIC
IMS_SINT32 SubState::GetSubState() const
{
    //---------------------------------------------------------------------------------------------

    return nSubStateValue;
}

PUBLIC
IMS_BOOL SubState::IsInstantSubscription() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_InstantSubscription;
}

PUBLIC
IMS_BOOL SubState::IsSubscriptionDurationUpdated() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_SubscriptionDurationUpdated;
}

PUBLIC
IMS_BOOL SubState::IsTerminated() const
{
    //---------------------------------------------------------------------------------------------

    return (GetState() == STATE_TERMINATED);
}

#if 0
PUBLIC
IMS_BOOL SubState::SetHeadersAndBodyParts(IN_OUT ISipMessage *&piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (this->piSIPMsg == IMS_NULL)
    {
        // Nothing to do ...
        return IMS_TRUE;
    }

    if (piSIPMsg->CopyHeadersAndBodyParts(this->piSIPMsg) != IMS_SUCCESS)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
#endif

PUBLIC
void SubState::SetConfiguration(IN IMS_SINT32 nConfigValue)
{
    //---------------------------------------------------------------------------------------------

    this->nConfigValue = nConfigValue;
}

PUBLIC
void SubState::SetOperation(IN IMS_SINT32 nOperation)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("SubState :: %s to %s", OperationToString(this->nOperation),
            OperationToString(nOperation), 0);

    this->nOperation = nOperation;
}

PUBLIC GLOBAL IMS_SINT32 SubState::ExtractExpiresParameter(IN CONST ISipHeader* piHeader)
{
    //---------------------------------------------------------------------------------------------

    if (piHeader == IMS_NULL)
    {
        return NO_EXPIRES;
    }

    if (piHeader->GetType() != ISipHeader::SUBSCRIPTION_STATE)
    {
        return NO_EXPIRES;
    }

    const SipParameter* pParameter = piHeader->GetParameter(Sip::STR_EXPIRES);

    if (pParameter == IMS_NULL)
    {
        return NO_EXPIRES;
    }

    IMS_BOOL bOK = IMS_FALSE;
    IMS_SINT32 nExpires = NO_EXPIRES;

    // 4 Make up for test equipment's fault (e.g. Anite)
    if (pParameter->GetValue().Contains(TextParser::CHAR_DQUOT))
    {
        AString strValue = TextParser::TrimDQUOT(pParameter->GetValue());
        nExpires = strValue.ToInt32(&bOK);
    }
    else
    {
        nExpires = pParameter->GetValue().ToInt32(&bOK);
    }

    if (!bOK)
    {
        return NO_EXPIRES;
    }

    return nExpires;
}

PUBLIC GLOBAL IMS_SINT32 SubState::ExtractReasonParameter(IN CONST ISipHeader* piHeader)
{
    //---------------------------------------------------------------------------------------------

    if (piHeader == IMS_NULL)
    {
        return REASON_NONE;
    }

    if (piHeader->GetType() != ISipHeader::SUBSCRIPTION_STATE)
    {
        return REASON_NONE;
    }

    const SipParameter* pParameter = piHeader->GetParameter(STR_REASON);

    if (pParameter == IMS_NULL)
    {
        return REASON_NONE;
    }

    const AString& strReason = pParameter->GetValue();

    if (strReason.EqualsIgnoreCase(STR_REASON_NORESOURCE))
        return REASON_NORESOURCE;
    else if (strReason.EqualsIgnoreCase(STR_REASON_DEACTIVATED))
        return REASON_DEACTIVATED;
    else if (strReason.EqualsIgnoreCase(STR_REASON_PROBATION))
        return REASON_PROBATION;
    else if (strReason.EqualsIgnoreCase(STR_REASON_REJECTED))
        return REASON_REJECTED;
    else if (strReason.EqualsIgnoreCase(STR_REASON_TIMEOUT))
        return REASON_TIMEOUT;
    else if (strReason.EqualsIgnoreCase(STR_REASON_GIVEUP))
        return REASON_GIVEUP;

    return REASON_NONE;
}

PUBLIC GLOBAL IMS_SINT32 SubState::ExtractSubStateValue(IN CONST ISipHeader* piHeader)
{
    //---------------------------------------------------------------------------------------------

    if (piHeader == IMS_NULL)
    {
        return SUB_STATE_INIT;
    }

    if (piHeader->GetType() != ISipHeader::SUBSCRIPTION_STATE)
    {
        return SUB_STATE_INIT;
    }

    const AString& strSubState = piHeader->GetValue();

    if (strSubState.EqualsIgnoreCase(Sip::STR_ACTIVE))
    {
        return SUB_STATE_ACTIVE;
    }
    else if (strSubState.EqualsIgnoreCase(Sip::STR_PENDING))
    {
        return SUB_STATE_PENDING;
    }
    else if (strSubState.EqualsIgnoreCase(Sip::STR_TERMINATED))
    {
        return SUB_STATE_TERMINATED;
    }

    return SUB_STATE_INIT;
}

PUBLIC GLOBAL IMS_SINT32 SubState::GetSubStateFromSubscriptionState(IN IMS_SINT32 nSubState)
{
    //---------------------------------------------------------------------------------------------

    switch (nSubState)
    {
        case ISubscriptionState::STATE_ACTIVE:
            return SUB_STATE_ACTIVE;
        case ISubscriptionState::STATE_PENDING:
            return SUB_STATE_PENDING;
        case ISubscriptionState::STATE_TERMINATED:
            return SUB_STATE_TERMINATED;
        default:
            return SUB_STATE_INIT;
    }
}

PUBLIC GLOBAL IMS_SINT32 SubState::GetReasonFromSubscriptionState(IN IMS_SINT32 nReason)
{
    //---------------------------------------------------------------------------------------------

    switch (nReason)
    {
        case ISubscriptionState::REASON_DEACTIVATED:
            return REASON_DEACTIVATED;
        case ISubscriptionState::REASON_PROBATION:
            return REASON_PROBATION;
        case ISubscriptionState::REASON_REJECTED:
            return REASON_REJECTED;
        case ISubscriptionState::REASON_TIMEOUT:
            return REASON_TIMEOUT;
        case ISubscriptionState::REASON_GIVEUP:
            return REASON_GIVEUP;
        case ISubscriptionState::REASON_NORESOURCE:
            return REASON_NORESOURCE;
        default:
            return REASON_NONE;
    }
}

PROTECTED VIRTUAL const SipHeaderProperty* SubState::GetRestrictedHeaders(
        OUT IMS_UINT32& nCount) const
{
    //---------------------------------------------------------------------------------------------

    nCount = 0;

    return IMS_NULL;
}

PROTECTED
void SubState::SetDuration(IN IMS_SINT32 nDuration)
{
    //---------------------------------------------------------------------------------------------

    nSubscriptionDuration = nDuration;
}

PROTECTED
void SubState::SetDurationUpdated(IN IMS_BOOL bDurationUpdated)
{
    //---------------------------------------------------------------------------------------------

    bFlag_SubscriptionDurationUpdated = bDurationUpdated;
}

PROTECTED
void SubState::SetInstantSubscription(IN IMS_BOOL bInstantSubscription)
{
    //---------------------------------------------------------------------------------------------

    bFlag_InstantSubscription = bInstantSubscription;
}

PROTECTED
void SubState::SetState(IN CONST ISipMessage* piSIPMsg, IN IMS_SINT32 nState)
{
    AString strCallId = piSIPMsg->GetHeader(ISipHeader::CALL_ID);

    //-----------------------------------------------------------------------------------------

    (void)strCallId;

    IMS_TRACE_I("SUB_STATE : %s - %s >> %s", SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'),
            StateToString(this->nState), StateToString(nState));

    this->nState = nState;
}

PROTECTED
void SubState::SetSubState(IN IMS_SINT32 nSubState)
{
    //---------------------------------------------------------------------------------------------

    nSubStateValue = nSubState;
}

PROTECTED
void SubState::StoreMessage(IN CONST ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (this->piSIPMsg != IMS_NULL)
    {
        this->piSIPMsg->Destroy();
    }

    this->piSIPMsg = piSIPMsg->Clone();

    if (this->piSIPMsg != IMS_NULL)
    {
        // Remove an inaccessible headers if present
        IMS_UINT32 nCount = 0;

        const SipHeaderProperty* pHeaderProperties = GetRestrictedHeaders(nCount);

        for (IMS_UINT32 i = 0; i < nCount; ++i)
        {
            const SipHeaderProperty* pProperty = &(pHeaderProperties[i]);

            if (pProperty->bSingleHeader)
            {
                if (pProperty->nType != ISipHeader::UNKNOWN)
                    this->piSIPMsg->RemoveHeader(pProperty->nType);
                else
                    this->piSIPMsg->RemoveHeader(pProperty->nType, pProperty->pszName);
            }
            else
            {
                IMS_SINT32 nHeaderCount;

                if (pProperty->nType != ISipHeader::UNKNOWN)
                {
                    nHeaderCount = this->piSIPMsg->GetHeaderCount(pProperty->nType);

                    for (IMS_SINT32 j = 0; j < nHeaderCount; ++j)
                    {
                        this->piSIPMsg->RemoveHeader(pProperty->nType);
                    }
                }
                else
                {
                    nHeaderCount =
                            this->piSIPMsg->GetHeaderCount(pProperty->nType, pProperty->pszName);

                    for (IMS_SINT32 j = 0; j < nHeaderCount; ++j)
                    {
                        this->piSIPMsg->RemoveHeader(pProperty->nType, pProperty->pszName);
                    }
                }
            }
        }
    }
}

PRIVATE GLOBAL const IMS_CHAR* SubState::OperationToString(IN IMS_SINT32 nOperation)
{
    //---------------------------------------------------------------------------------------------

    switch (nOperation)
    {
        case NO_OPERATION:
            return "NO_OPERATION";
        case OPERATION_CREATE:
            return "OPERATION_CREATE";
        case OPERATION_REFRESH:
            return "OPERATION_REFRESH";
        case OPERATION_FETCH:
            return "OPERATION_FETCH";
        case OPERATION_REMOVE:
            return "OPERATION_REMOVE";
        case OPERATION_IMPLICIT_REFRESH:
            return "OPERATION_IMPLICIT_REFRESH";
        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* SubState::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
        case STATE_INIT:
            return "STATE_INIT";
        case STATE_SUBSCRIBING:
            return "STATE_SUBSCRIBING";
        case STATE_PENDING:
            return "STATE_PENDING";
        case STATE_ACTIVE:
            return "STATE_ACTIVE";
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}
