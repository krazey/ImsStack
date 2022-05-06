/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100424  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "PubState.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL const SIPHeaderProperty PubState::RESTRICTED_HEADER_PROPERTIES[] =
{
    // Header type, Header name, Is single header ?
    {ISipHeader::ACCEPT_CONTACT,        IMS_NULL,                      IMS_FALSE},
    {ISipHeader::AUTHORIZATION,         IMS_NULL,                      IMS_FALSE},
    {ISipHeader::ALLOW,                 IMS_NULL,                      IMS_FALSE},
    {ISipHeader::CALL_ID,               IMS_NULL,                      IMS_TRUE },
    {ISipHeader::CONTACT_ANY,           IMS_NULL,                      IMS_FALSE},
    {ISipHeader::CONTENT_LENGTH,        IMS_NULL,                      IMS_TRUE },
    {ISipHeader::CONTENT_TYPE,          IMS_NULL,                      IMS_TRUE },
    {ISipHeader::CSEQ,                  IMS_NULL,                      IMS_TRUE },
    {ISipHeader::FROM,                  IMS_NULL,                      IMS_TRUE },
    {ISipHeader::MAX_FORWARDS,          IMS_NULL,                      IMS_TRUE },
    {ISipHeader::MIN_EXPIRES,           IMS_NULL,                      IMS_TRUE },
    {ISipHeader::P_ACCESS_NETWORK_INFO, IMS_NULL,                      IMS_TRUE },
    {ISipHeader::P_ASSERTED_IDENTITY,   IMS_NULL,                      IMS_FALSE},
    {ISipHeader::P_PREFERRED_IDENTITY,  IMS_NULL,                      IMS_FALSE},
    {ISipHeader::PROXY_AUTHORIZATION,   IMS_NULL,                      IMS_FALSE},
    {ISipHeader::ROUTE,                 IMS_NULL,                      IMS_FALSE},
    {ISipHeader::TO,                    IMS_NULL,                      IMS_TRUE },
    {ISipHeader::SECURITY_CLIENT,       IMS_NULL,                      IMS_FALSE},
    {ISipHeader::SECURITY_VERIFY,       IMS_NULL,                      IMS_FALSE},
    {ISipHeader::VIA,                   IMS_NULL,                      IMS_FALSE},
    // SIP: Content-Length header is handled as unknown header
    {ISipHeader::UNKNOWN,               SipHeaderName::CONTENT_LENGTH, IMS_TRUE },
    {ISipHeader::UNKNOWN,               "l",                           IMS_TRUE }
};

PUBLIC
PubState::PubState() :
        nState(STATE_INIT),
        nOperation(NO_OPERATION),
        nPublicationDuration(NO_EXPIRES),
        strEntityTag(AString::ConstNull()),
        piSIPMsg(IMS_NULL)
{
}

PUBLIC
PubState::~PubState()
{
    if (piSIPMsg != IMS_NULL)
    {
        piSIPMsg->Destroy();
    }
}

PUBLIC
void PubState::Clear()
{
    //---------------------------------------------------------------------------------------------

    // Clear the event package
    objEventPackage.SetDuration(NO_EXPIRES);
    objEventPackage.SetEventHeader(IMS_NULL);

    nState = STATE_INIT;
    nOperation = NO_OPERATION;
    nPublicationDuration = NO_EXPIRES;
    strEntityTag = AString::ConstNull();

    if (piSIPMsg != IMS_NULL)
    {
        piSIPMsg->Destroy();
        piSIPMsg = IMS_NULL;
    }
}

PUBLIC
IMS_BOOL PubState::CreateEventPackage(IN CONST AString& strEvent)
{
    //---------------------------------------------------------------------------------------------

    // 1 : Load an event package from configuration

    objEventPackage.SetEvent(strEvent);

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 PubState::GetDuration() const
{
    //---------------------------------------------------------------------------------------------

    return nPublicationDuration;
}

PUBLIC
const AString& PubState::GetEntityTag() const
{
    //---------------------------------------------------------------------------------------------

    return strEntityTag;
}

PUBLIC
EventPackage* PubState::GetEventPackage()
{
    //---------------------------------------------------------------------------------------------

    return &objEventPackage;
}

PUBLIC
IMS_SINT32 PubState::GetOperation() const
{
    //---------------------------------------------------------------------------------------------

    return nOperation;
}

PUBLIC
IMS_SINT32 PubState::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

PUBLIC
IMS_BOOL PubState::IsTerminated() const
{
    //---------------------------------------------------------------------------------------------

    return (GetState() == STATE_TERMINATED);
}

PUBLIC
IMS_BOOL PubState::SetHeaders(IN_OUT ISipMessage*& piSIPMsg)
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

    // Set SIP-If-Match header if the entity-tag is present
    if (!strEntityTag.IsNULL())
    {
        if (piSIPMsg->SetHeader(ISipHeader::SIP_IF_MATCH, strEntityTag) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
void PubState::SetOperation(IN IMS_SINT32 nOperation)
{
    //---------------------------------------------------------------------------------------------

    this->nOperation = nOperation;
}

PUBLIC
IMS_BOOL PubState::UpdateState(IN CONST ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piSIPMsg->GetMethod().Equals(SipMethod::PUBLISH))
    {
        return IMS_FALSE;
    }

    // Update the publication state information...

    // On PUBLISH request sent ...
    if (piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST)
    {
        if (!UpdateOnPUBLISHRequest(piSIPMsg))
        {
            IMS_TRACE_E(0, "Updating the publication state on PUBLISH request failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    // On PUBLISH response received ...
    else
    {
        if (!UpdateOnPUBLISHResponse(piSIPMsg))
        {
            IMS_TRACE_E(0, "Updating the publication state on PUBLISH response failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL PubState::UpdateStateOnTxnTimerExpired()
{
    //---------------------------------------------------------------------------------------------

    SetState(STATE_TERMINATED);

    return IMS_TRUE;
}

PRIVATE
void PubState::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("PUB_STATE : %s - %s >> %s", objEventPackage.GetEvent().GetStr(),
            StateToString(this->nState), StateToString(nState));

    this->nState = nState;
}

PRIVATE
void PubState::StoreMessage(IN CONST ISipMessage* piSIPMsg)
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
        IMS_UINT32 nCount =
                sizeof(RESTRICTED_HEADER_PROPERTIES) / sizeof(RESTRICTED_HEADER_PROPERTIES[0]);
        const SIPHeaderProperty* pHeaderProperties = &(RESTRICTED_HEADER_PROPERTIES[0]);

        for (IMS_UINT32 i = 0; i < nCount; ++i)
        {
            const SIPHeaderProperty* pProperty = &(pHeaderProperties[i]);

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

        // Remove all the message body parts if present...
        this->piSIPMsg->RemoveBodyParts();
    }
}

PRIVATE
IMS_BOOL PubState::UpdateOnPUBLISHRequest(IN CONST ISipMessage* piSIPMsg)
{
    AString strHeader;
    ISipHeader* piHeader;
    EventPackage* pEventPackage = GetEventPackage();

    //---------------------------------------------------------------------------------------------

    // Extracts a duartion of publication from Expires header
    if (piSIPMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
    {
        strHeader = piSIPMsg->GetHeader(ISipHeader::EXPIRES_ANY);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EXPIRES_ANY, strHeader);

        if (piHeader != IMS_NULL)
        {
            IMS_SINT32 nPublicationDuration = piHeader->GetValueInt();

            // Stores the Expires value in the initial SUBSCRIBE request...
            if (GetOperation() == OPERATION_CREATE)
            {
                pEventPackage->SetDuration(nPublicationDuration);
            }

            piHeader->Destroy();
        }
    }

    // Extracts an Event header
    if (GetOperation() == OPERATION_CREATE)
    {
        if (!piSIPMsg->IsHeaderPresent(ISipHeader::EVENT))
        {
            IMS_TRACE_E(0, "Mandatory header missing : Event header", 0, 0, 0);
            return IMS_FALSE;
        }

        strHeader = piSIPMsg->GetHeader(ISipHeader::EVENT);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EVENT, strHeader);

        pEventPackage->SetEventHeader(piHeader);
    }

    // Stores an initial/update PUBLISH request for refresh/removal operation
    if (GetOperation() == OPERATION_CREATE || GetOperation() == OPERATION_MODIFY)
    {
        StoreMessage(piSIPMsg);
    }

    SetState(STATE_PENDING);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL PubState::UpdateOnPUBLISHResponse(IN CONST ISipMessage* piSIPMsg)
{
    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return IMS_TRUE;
    }
    else if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        SetState(STATE_ACTIVE);

        // Extracts a duartion of subscription from Expires header
        if (piSIPMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
        {
            AString strHeader = piSIPMsg->GetHeader(ISipHeader::EXPIRES_ANY);
            ISipHeader* piHeader =
                    SipParsingHelper::CreateHeader(ISipHeader::EXPIRES_ANY, strHeader);

            if (piHeader != IMS_NULL)
            {
                nPublicationDuration = piHeader->GetValueInt();

                if (nPublicationDuration < 0)
                {
                    // Transit the state to TERMINATED
                    SetState(STATE_TERMINATED);
                }

                piHeader->Destroy();
            }
        }
        else
        {
            // Transit the state to TERMINATED
            SetState(STATE_TERMINATED);
        }

        // Updates the entity-tag
        strEntityTag = piSIPMsg->GetHeader(ISipHeader::SIP_ETAG);
    }
    else if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        if (GetOperation() == OPERATION_REMOVE)
        {
            // Transit the state to TERMINATED
            SetState(STATE_TERMINATED);
        }

        // Clear the entity-tag
        strEntityTag = AString::ConstNull();

        return IMS_TRUE;
    }
    else
    {
        // Transit the state to TERMINATED
        if (GetOperation() != OPERATION_MODIFY)
        {
            SetState(STATE_TERMINATED);
        }

        // Clear the entity-tag
        strEntityTag = AString::ConstNull();
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL const IMS_CHAR* PubState::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
        case STATE_INIT:
            return "STATE_INIT";
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
