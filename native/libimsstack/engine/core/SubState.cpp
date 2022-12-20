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
#include "TextParser.h"

#include "ISipHeader.h"
#include "ISubscriptionState.h"
#include "Sip.h"
#include "SipDebug.h"
#include "SipHeaderProperty.h"
#include "SipParameter.h"
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
        m_nState(STATE_INIT),
        m_nOperation(NO_OPERATION),
        m_nConfigValue(CONFIG_NONE),
        m_nSubscriptionDuration(NO_EXPIRES),
        m_nSubStateValue(SUB_STATE_INIT),
        m_bSubscriptionDurationUpdated(IMS_FALSE),
        m_bInstantSubscription(IMS_FALSE),
        m_piSipMsg(IMS_NULL)
{
}

PUBLIC VIRTUAL SubState::~SubState()
{
    if (m_piSipMsg != IMS_NULL)
    {
        m_piSipMsg->Destroy();
    }
}

PUBLIC VIRTUAL void SubState::Clear()
{
    // Clear the event package
    m_objEventPackage.SetDuration(NO_EXPIRES);
    m_objEventPackage.SetEventHeader(IMS_NULL);

    m_nState = STATE_INIT;
    m_nOperation = NO_OPERATION;
    m_nSubscriptionDuration = NO_EXPIRES;
    m_nSubStateValue = SUB_STATE_INIT;

    m_bSubscriptionDurationUpdated = IMS_FALSE;
    m_bInstantSubscription = IMS_FALSE;

    if (m_piSipMsg != IMS_NULL)
    {
        m_piSipMsg->Destroy();
        m_piSipMsg = IMS_NULL;
    }
}

PUBLIC
IMS_BOOL SubState::CreateEventPackage(IN const AString& strEvent)
{
    // 1 : Load an event package from configuration

    m_objEventPackage.SetEvent(strEvent);
    return IMS_TRUE;
}

#if 0
PUBLIC
IMS_BOOL SubState::SetHeadersAndBodyParts(IN_OUT ISipMessage *&piSipMsg)
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

    return IMS_TRUE;
}
#endif

PUBLIC
void SubState::SetOperation(IN IMS_SINT32 nOperation)
{
    IMS_TRACE_I("SubState :: %s to %s", OperationToString(m_nOperation),
            OperationToString(nOperation), 0);

    m_nOperation = nOperation;
}

PUBLIC GLOBAL IMS_SINT32 SubState::ExtractExpiresParameter(IN const ISipHeader* piHeader)
{
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

    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nExpires = NO_EXPIRES;

    // 4 Make up for test equipment's fault (e.g. Anite)
    if (pParameter->GetValue().Contains(TextParser::CHAR_DQUOT))
    {
        AString strValue = TextParser::TrimDquot(pParameter->GetValue());
        nExpires = strValue.ToInt32(&bOk);
    }
    else
    {
        nExpires = pParameter->GetValue().ToInt32(&bOk);
    }

    if (!bOk)
    {
        return NO_EXPIRES;
    }

    return nExpires;
}

PUBLIC GLOBAL IMS_SINT32 SubState::ExtractReasonParameter(IN const ISipHeader* piHeader)
{
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
    {
        return REASON_NORESOURCE;
    }
    else if (strReason.EqualsIgnoreCase(STR_REASON_DEACTIVATED))
    {
        return REASON_DEACTIVATED;
    }
    else if (strReason.EqualsIgnoreCase(STR_REASON_PROBATION))
    {
        return REASON_PROBATION;
    }
    else if (strReason.EqualsIgnoreCase(STR_REASON_REJECTED))
    {
        return REASON_REJECTED;
    }
    else if (strReason.EqualsIgnoreCase(STR_REASON_TIMEOUT))
    {
        return REASON_TIMEOUT;
    }
    else if (strReason.EqualsIgnoreCase(STR_REASON_GIVEUP))
    {
        return REASON_GIVEUP;
    }

    return REASON_NONE;
}

PUBLIC GLOBAL IMS_SINT32 SubState::ExtractSubStateValue(IN const ISipHeader* piHeader)
{
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
    nCount = 0;
    return IMS_NULL;
}

PROTECTED
void SubState::SetState(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nState)
{
    AString strCallId = piSipMsg->GetHeader(ISipHeader::CALL_ID);

    (void)strCallId;

    IMS_TRACE_I("SUB_STATE : %s - %s >> %s", SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'),
            StateToString(m_nState), StateToString(nState));

    m_nState = nState;
}

PROTECTED
void SubState::StoreMessage(IN const ISipMessage* piSipMsg)
{
    if (m_piSipMsg != IMS_NULL)
    {
        m_piSipMsg->Destroy();
    }

    m_piSipMsg = piSipMsg->Clone();

    if (m_piSipMsg != IMS_NULL)
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
    }
}

PRIVATE GLOBAL const IMS_CHAR* SubState::OperationToString(IN IMS_SINT32 nOperation)
{
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
