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

#include "ISipHeader.h"
#include "PubState.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"

__IMS_TRACE_TAG_IMS_CORE__;

// clang-format off
PUBLIC GLOBAL
const SipHeaderProperty PubState::RESTRICTED_HEADER_PROPERTIES[] =
{
    // Header name, Header type, Is single header ?
    {IMS_NULL,                      ISipHeader::ACCEPT_CONTACT,        IMS_FALSE},
    {IMS_NULL,                      ISipHeader::AUTHORIZATION,         IMS_FALSE},
    {IMS_NULL,                      ISipHeader::ALLOW,                 IMS_FALSE},
    {IMS_NULL,                      ISipHeader::CALL_ID,               IMS_TRUE },
    {IMS_NULL,                      ISipHeader::CONTACT_ANY,           IMS_FALSE},
    {IMS_NULL,                      ISipHeader::CONTENT_LENGTH,        IMS_TRUE },
    {IMS_NULL,                      ISipHeader::CONTENT_TYPE,          IMS_TRUE },
    {IMS_NULL,                      ISipHeader::CSEQ,                  IMS_TRUE },
    {IMS_NULL,                      ISipHeader::FROM,                  IMS_TRUE },
    {IMS_NULL,                      ISipHeader::MAX_FORWARDS,          IMS_TRUE },
    {IMS_NULL,                      ISipHeader::MIN_EXPIRES,           IMS_TRUE },
    {IMS_NULL,                      ISipHeader::P_ACCESS_NETWORK_INFO, IMS_TRUE },
    {IMS_NULL,                      ISipHeader::P_ASSERTED_IDENTITY,   IMS_FALSE},
    {IMS_NULL,                      ISipHeader::P_PREFERRED_IDENTITY,  IMS_FALSE},
    {IMS_NULL,                      ISipHeader::PROXY_AUTHORIZATION,   IMS_FALSE},
    {IMS_NULL,                      ISipHeader::ROUTE,                 IMS_FALSE},
    {IMS_NULL,                      ISipHeader::TO,                    IMS_TRUE },
    {IMS_NULL,                      ISipHeader::SECURITY_CLIENT,       IMS_FALSE},
    {IMS_NULL,                      ISipHeader::SECURITY_VERIFY,       IMS_FALSE},
    {IMS_NULL,                      ISipHeader::VIA,                   IMS_FALSE},
    // SIP: Content-Length header is handled as unknown header
    {SipHeaderName::CONTENT_LENGTH, ISipHeader::UNKNOWN,               IMS_TRUE },
    {"l",                           ISipHeader::UNKNOWN,               IMS_TRUE }
};
// clang-format on

PUBLIC
PubState::PubState() :
        m_nState(STATE_INIT),
        m_nOperation(NO_OPERATION),
        m_nPublicationDuration(NO_EXPIRES),
        m_strEntityTag(AString::ConstNull()),
        m_piSipMsg(IMS_NULL)
{
}

PUBLIC
PubState::~PubState()
{
    if (m_piSipMsg != IMS_NULL)
    {
        m_piSipMsg->Destroy();
    }
}

PUBLIC
void PubState::Clear()
{
    // Clear the event package
    m_objEventPackage.SetDuration(NO_EXPIRES);
    m_objEventPackage.SetEventHeader(IMS_NULL);

    m_nState = STATE_INIT;
    m_nOperation = NO_OPERATION;
    m_nPublicationDuration = NO_EXPIRES;
    m_strEntityTag = AString::ConstNull();

    if (m_piSipMsg != IMS_NULL)
    {
        m_piSipMsg->Destroy();
        m_piSipMsg = IMS_NULL;
    }
}

PUBLIC
IMS_BOOL PubState::CreateEventPackage(IN const AString& strEvent)
{
    // 1 : Load an event package from configuration

    m_objEventPackage.SetEvent(strEvent);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL PubState::SetHeaders(IN_OUT ISipMessage*& piSipMsg)
{
    if (m_piSipMsg == IMS_NULL)
    {
        // Nothing to do ...
        return IMS_TRUE;
    }

    if (piSipMsg->CopyHeadersAndBodyParts(m_piSipMsg) != IMS_SUCCESS)
    {
        return IMS_FALSE;
    }

    // Set SIP-If-Match header if the entity-tag is present
    if (!m_strEntityTag.IsNULL())
    {
        if (piSipMsg->SetHeader(ISipHeader::SIP_IF_MATCH, m_strEntityTag) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL PubState::UpdateState(IN const ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piSipMsg->GetMethod().Equals(SipMethod::PUBLISH))
    {
        return IMS_FALSE;
    }

    // Update the publication state information...

    // On PUBLISH request sent ...
    if (piSipMsg->GetType() == ISipMessage::TYPE_REQUEST)
    {
        if (!UpdateOnPublishRequest(piSipMsg))
        {
            IMS_TRACE_E(0, "Updating the publication state on PUBLISH request failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    // On PUBLISH response received ...
    else
    {
        UpdateOnPublishResponse(piSipMsg);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL PubState::UpdateStateOnTxnTimerExpired()
{
    SetState(STATE_TERMINATED);
    return IMS_TRUE;
}

PRIVATE
void PubState::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("PUB_STATE : %s - %s >> %s", m_objEventPackage.GetEvent().GetStr(),
            StateToString(m_nState), StateToString(nState));

    m_nState = nState;
}

PRIVATE
void PubState::StoreMessage(IN const ISipMessage* piSipMsg)
{
    if (m_piSipMsg != IMS_NULL)
    {
        m_piSipMsg->Destroy();
    }

    m_piSipMsg = piSipMsg->Clone();

    if (m_piSipMsg != IMS_NULL)
    {
        // Remove an inaccessible headers if present
        IMS_UINT32 nCount =
                sizeof(RESTRICTED_HEADER_PROPERTIES) / sizeof(RESTRICTED_HEADER_PROPERTIES[0]);
        const SipHeaderProperty* pHeaderProperties = &(RESTRICTED_HEADER_PROPERTIES[0]);

        for (IMS_UINT32 i = 0; i < nCount; ++i)
        {
            const SipHeaderProperty* pProperty = &(pHeaderProperties[i]);

            if (pProperty->bSingleHeader)
            {
                if (pProperty->nType != ISipHeader::UNKNOWN)
                {
                    m_piSipMsg->RemoveHeader(pProperty->nType);
                }
                else
                {
                    m_piSipMsg->RemoveHeader(pProperty->nType, pProperty->pszName);
                }
            }
            else
            {
                IMS_SINT32 nHeaderCount;

                if (pProperty->nType != ISipHeader::UNKNOWN)
                {
                    nHeaderCount = m_piSipMsg->GetHeaderCount(pProperty->nType);

                    for (IMS_SINT32 j = 0; j < nHeaderCount; ++j)
                    {
                        m_piSipMsg->RemoveHeader(pProperty->nType);
                    }
                }
                else
                {
                    nHeaderCount = m_piSipMsg->GetHeaderCount(pProperty->nType, pProperty->pszName);

                    for (IMS_SINT32 j = 0; j < nHeaderCount; ++j)
                    {
                        m_piSipMsg->RemoveHeader(pProperty->nType, pProperty->pszName);
                    }
                }
            }
        }

        // Remove all the message body parts if present...
        m_piSipMsg->RemoveBodyParts();
    }
}

PRIVATE
IMS_BOOL PubState::UpdateOnPublishRequest(IN const ISipMessage* piSipMsg)
{
    AString strHeader;
    ISipHeader* piHeader;
    EventPackage* pEventPackage = GetEventPackage();

    // Extracts a duartion of publication from Expires header
    if (piSipMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
    {
        strHeader = piSipMsg->GetHeader(ISipHeader::EXPIRES_ANY);
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
        if (!piSipMsg->IsHeaderPresent(ISipHeader::EVENT))
        {
            IMS_TRACE_E(0, "Mandatory header missing : Event header", 0, 0, 0);
            return IMS_FALSE;
        }

        strHeader = piSipMsg->GetHeader(ISipHeader::EVENT);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EVENT, strHeader);

        pEventPackage->SetEventHeader(piHeader);
    }

    // Stores an initial/update PUBLISH request for refresh/removal operation
    if (GetOperation() == OPERATION_CREATE || GetOperation() == OPERATION_MODIFY)
    {
        StoreMessage(piSipMsg);
    }

    SetState(STATE_PENDING);

    return IMS_TRUE;
}

PRIVATE
void PubState::UpdateOnPublishResponse(IN const ISipMessage* piSipMsg)
{
    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
    }
    else if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        SetState(STATE_ACTIVE);

        // Extracts a duartion of subscription from Expires header
        if (piSipMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
        {
            AString strHeader = piSipMsg->GetHeader(ISipHeader::EXPIRES_ANY);
            ISipHeader* piHeader =
                    SipParsingHelper::CreateHeader(ISipHeader::EXPIRES_ANY, strHeader);

            if (piHeader != IMS_NULL)
            {
                m_nPublicationDuration = piHeader->GetValueInt();

                if (m_nPublicationDuration < 0)
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
        m_strEntityTag = piSipMsg->GetHeader(ISipHeader::SIP_ETAG);
    }
    else if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        if (GetOperation() == OPERATION_REMOVE)
        {
            // Transit the state to TERMINATED
            SetState(STATE_TERMINATED);
        }

        // Clear the entity-tag
        m_strEntityTag = AString::ConstNull();
    }
    else
    {
        // Transit the state to TERMINATED
        if (GetOperation() != OPERATION_MODIFY)
        {
            SetState(STATE_TERMINATED);
        }

        // Clear the entity-tag
        m_strEntityTag = AString::ConstNull();
    }
}

PRIVATE GLOBAL const IMS_CHAR* PubState::StateToString(IN IMS_SINT32 nState)
{
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
