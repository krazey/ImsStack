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

#include "ISipHeader.h"
#include "SipDialogSubscribeUsage.h"
#include "SipPrivate.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP__;

// clang-format off
PRIVATE GLOBAL const IMS_SINT32
SipDialogSubscribeUsage::STATE_TABLE[SipDState::STATE_MAX][SipDialogSubscribeUsage::TRIGGER_MAX] =
{
    // STATE_INIT
    {
        SipDState::STATE_INIT,                 // TRIGGER_INIT
        SipDState::STATE_EARLY,                // TRIGGER_1XX
        SipDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SipDState::STATE_INIT,                 // TRIGGER_NON_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_XXX_NOTIFY_TERMINATED
        SipDState::STATE_CONFIRMED             // TRIGGER_NOTIFY
    },
    // STATE_TERMINATED
    {
        SipDState::STATE_TERMINATED,           // TRIGGER_INIT
        SipDState::STATE_TERMINATED,           // TRIGGER_1XX
        SipDState::STATE_TERMINATED,           // TRIGGER_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_NON_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_XXX_NOTIFY_TERMINATED
        SipDState::STATE_TERMINATED            // TRIGGER_NOTIFY
    },
    // STATE_EARLY
    {
        SipDState::STATE_EARLY,                // TRIGGER_INIT
        SipDState::STATE_EARLY,                // TRIGGER_1XX
        SipDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_NON_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_XXX_NOTIFY_TERMINATED
        SipDState::STATE_CONFIRMED             // TRIGGER_NOTIFY
    },
    // STATE_CONFIRMED
    {
        SipDState::STATE_CONFIRMED,            // TRIGGER_INIT
        SipDState::STATE_CONFIRMED,            // TRIGGER_1XX
        SipDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SipDState::STATE_CONFIRMED,            // TRIGGER_NON_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_XXX_NOTIFY_TERMINATED
        SipDState::STATE_CONFIRMED             // TRIGGER_NOTIFY
    }
};
// clang-format on

PUBLIC
SipDialogSubscribeUsage::SipDialogSubscribeUsage(IN SipDialogBase* pDialog) :
        SipDialogUsage(SipDialogUsage::TYPE_SUBSCRIBE, pDialog),
        m_nSubState(SUB_STATE_INIT),
        m_nMethod(METHOD_SUBSCRIBE),
        m_strEvent(AString::ConstNull()),
        m_strEventId(AString::ConstNull()),
        m_nCSeqForNotifyWithTerminated(SipPrivate::INVALID_SEQ_NUM)
{
}

PUBLIC
SipDialogSubscribeUsage::SipDialogSubscribeUsage(IN const SipDialogSubscribeUsage& other) :
        SipDialogUsage(other),
        m_nSubState(other.m_nSubState),
        m_nMethod(other.m_nMethod),
        m_strEvent(other.m_strEvent),
        m_strEventId(other.m_strEventId),
        m_nCSeqForNotifyWithTerminated(other.m_nCSeqForNotifyWithTerminated)
{
}

PUBLIC VIRTUAL IMS_BOOL SipDialogSubscribeUsage::InitDialogUsage(
        IN const SipMessageInfo& objMsgInfo)
{
    // For a forked NOTIFY request, checking to NOTIFY method is added
    if (!objMsgInfo.GetMethod().Equals(SipMethod::SUBSCRIBE) &&
            !objMsgInfo.GetMethod().Equals(SipMethod::REFER) &&
            !objMsgInfo.GetMethod().Equals(SipMethod::NOTIFY))
    {
        IMS_TRACE_E(0, "OPERATION NOT ALLOWED : Can't create a subscribe dialog usage for %s",
                objMsgInfo.GetMethod().ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (objMsgInfo.GetMethod().Equals(SipMethod::REFER))
    {
        m_nMethod = METHOD_REFER;
        m_strEvent = Sip::STR_REFER;
        // MULTIPLE_REFER
        m_strEventId.SetNumber(SipStack::GetCSeqNumber(objMsgInfo.GetMessage()));
    }
    else
    {
        // Get Event header & set this info.
        if (!SipStack::GetEventHeader(objMsgInfo.GetMessage(), m_strEvent, m_strEventId))
        {
            IMS_TRACE_E(0, "OPERATION FAILED : Getting Event Header failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return SipDialogUsage::InitDialogUsage(objMsgInfo);
}

PUBLIC VIRTUAL IMS_BOOL SipDialogSubscribeUsage::CompareTo(
        IN const SipMessageInfo& objMsgInfo) const
{
    const SipMethod& objMethod = objMsgInfo.GetMethod();

    if ((objMethod.Equals(SipMethod::REFER) && (m_nMethod != METHOD_REFER)) ||
            (objMethod.Equals(SipMethod::SUBSCRIBE) && (m_nMethod != METHOD_SUBSCRIBE)))
    {
        return IMS_FALSE;
    }

    AString strTmpEvent;
    AString strTmpEventId;

    if (!SipStack::GetEventHeader(objMsgInfo.GetMessage(), strTmpEvent, strTmpEventId))
    {
        IMS_TRACE_E(0, "NOT FOUND : Getting Event header failed (%s)",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (!m_strEvent.Equals(strTmpEvent))
    {
        IMS_TRACE_I("Event (%s , %s) is not matched", m_strEvent.GetStr(), strTmpEvent.GetStr(), 0);
        return IMS_FALSE;
    }

    // MULTIPLE_REFER
    if ((m_nMethod == METHOD_REFER) && objMsgInfo.GetMethod().Equals(SipMethod::REFER) &&
            (objMsgInfo.GetDirection() == SipMessageInfo::DIRECTION_OUTGOING) &&
            (strTmpEventId.GetLength() == 0))
    {
        strTmpEventId.SetNumber(SipStack::GetCSeqNumber(objMsgInfo.GetMessage()));
    }

    if (!m_strEventId.Equals(strTmpEventId))
    {
        // MULTIPLE_REFER
        if (m_nMethod == METHOD_REFER)
        {
            if (strTmpEventId.GetLength() == 0)
            {
                // There is no id parameter in NOTIFY request of the implicit subscription,
                // so, ignore the comparison result.
                IMS_TRACE_I("Event id is not used for 'refer' event package", 0, 0, 0);
                return IMS_TRUE;
            }
            else if (strTmpEventId.Equals("0"))
            {
                IMS_TRACE_I("Event id is not valid. Skip validation check.", 0, 0, 0);
                return IMS_TRUE;
            }
        }

        IMS_TRACE_I("Event id (%s , %s) is not matched", m_strEventId.GetStr(),
                strTmpEventId.GetStr(), 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipDialogSubscribeUsage::Equals(IN SipDialogUsage* pDUsage) const
{
    SipDialogSubscribeUsage* pDSubscribeUsage = DYNAMIC_CAST(SipDialogSubscribeUsage*, pDUsage);

    if (pDSubscribeUsage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Check if the shared dialog state equals or not
    if (!SipDialogUsage::Equals(pDSubscribeUsage))
    {
        return IMS_FALSE;
    }

    // Check if the subscribe usage info. equals or not
    if (m_nMethod != pDSubscribeUsage->m_nMethod)
    {
        return IMS_FALSE;
    }

    if (!m_strEvent.Equals(pDSubscribeUsage->m_strEvent))
    {
        return IMS_FALSE;
    }

    if (!m_strEventId.Equals(pDSubscribeUsage->m_strEventId))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SipDialogSubscribeUsage::ToString() const
{
    AString strValue;

    strValue.Sprintf("DialogUsage: [%s|%s|%s]", (m_nMethod == METHOD_REFER) ? "REFER" : "SUBSCRIBE",
            m_strEvent.GetStr(), m_strEventId.GetStr());

    return strValue;
}

PUBLIC VIRTUAL IMS_SINT32 SipDialogSubscribeUsage::UpdateUsageDetails(
        IN const SipMessageInfo& objMsgInfo)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();
    const SipMethod& objMethod = objMsgInfo.GetMethod();

    // Update Subscription-State header
    if (SipStack::IsRequestMessage(pSipMsg) && objMethod.Equals(SipMethod::NOTIFY))
    {
        AString strSubState;

        if (!SipStack::GetSubscriptionStateHeader(objMsgInfo.GetMessage(), strSubState))
        {
            IMS_TRACE_E(0, "Getting Subscription-State header failed", 0, 0, 0);
            return SipPrivate::MESSAGE_FAILED;
        }

        if (strSubState.EqualsIgnoreCase(Sip::STR_ACTIVE))
        {
            m_nSubState = SUB_STATE_ACTIVE;
        }
        else if (strSubState.EqualsIgnoreCase(Sip::STR_PENDING))
        {
            m_nSubState = SUB_STATE_PENDING;
        }
        else if (strSubState.EqualsIgnoreCase(Sip::STR_TERMINATED))
        {
            m_nSubState = SUB_STATE_TERMINATED;
            m_nCSeqForNotifyWithTerminated = SipStack::GetCSeqNumber(objMsgInfo.GetMessage());
        }
    }

    // Update the shared dialog states
    return SipDialogUsage::UpdateUsageDetails(objMsgInfo);
}

PUBLIC
IMS_BOOL SipDialogSubscribeUsage::InitDialogUsage(IN const SipMethod& objMethod)
{
    if (!objMethod.Equals(SipMethod::SUBSCRIBE) && !objMethod.Equals(SipMethod::REFER) &&
            !objMethod.Equals(SipMethod::NOTIFY))
    {
        IMS_TRACE_E(0, "OPERATION NOT ALLOWED : Can't create a subscribe dialog usage for %s",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (objMethod.Equals(SipMethod::REFER))
    {
        m_nMethod = METHOD_REFER;
        m_strEvent = Sip::STR_REFER;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_SINT32 SipDialogSubscribeUsage::GetNextState(
        IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger)
{
    if ((nTrigger < TRIGGER_INIT) || (nTrigger >= TRIGGER_MAX))
    {
        return SipDState::STATE_MAX;
    }

    // cppcheck-suppress arrayIndexOutOfBoundsCond
    return STATE_TABLE[nState][nTrigger];
}

PROTECTED VIRTUAL IMS_SINT32 SipDialogSubscribeUsage::GetActionNTrigger(
        IN const SipMessageInfo& objMsgInfo, OUT IMS_SINT32& nTrigger)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();
    IMS_SINT32 nAction = SipDState::ACTION_TRANSIT_STATE;

    nTrigger = TRIGGER_INIT;

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        if (objMsgInfo.GetMethod().Equals(SipMethod::NOTIFY))
        {
            if ((m_nSubState == SUB_STATE_ACTIVE) || (m_nSubState == SUB_STATE_PENDING) ||
                    (m_nSubState == SUB_STATE_TERMINATED))
            {
                nTrigger = TRIGGER_NOTIFY;
            }
        }
    }
    else
    {
        nAction = GetActionForResponse(objMsgInfo);

        IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

        if (SipStatusCode::IsProvisional(nStatusCode))
        {
            nTrigger = TRIGGER_1XX;
        }
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (m_nSubState == SUB_STATE_TERMINATED)
            {
                IMS_UINT32 nCSeq = SipStack::GetCSeqNumber(objMsgInfo.GetMessage());

                if ((m_nCSeqForNotifyWithTerminated == SipPrivate::INVALID_SEQ_NUM) ||
                        (objMsgInfo.GetMethod().Equals(SipMethod::NOTIFY) &&
                                (m_nCSeqForNotifyWithTerminated == nCSeq)))
                {
                    nAction = SipDState::ACTION_TRANSIT_STATE;
                    nTrigger = TRIGGER_XXX_NOTIFY_TERMINATED;
                }
                else
                {
                    nTrigger = TRIGGER_2XX;
                }
            }
            else
            {
                nTrigger = TRIGGER_2XX;
            }
        }
        else
        {
            // 4 How to handle in case of NOTIFY with TERMINATED ???

            // A NOTIFY request is considered failed if the response times out,
            // or a non-200 class response code is received which has no "Retry-After" header
            // and no implied further action which can be taken to retry the request
            // (e.g. 401 Unauthorized).
            //
            // If the NOTIFY request fails (as defined above) due to a timeout condition,
            // and the subscription was installed using a soft-state mechanism (such as SUBSCRIBE),
            // the notifier SHOULD remove the subscription.
            //
            // If the NOTIFY request fails (as defined above) due to an error response,
            // and the subscription was installed using a soft-state mechanism,
            // the notifier MUST remove the corresponding subscription.
            //
            // If a NOTIFY request receives a 481 response,
            // the notifier MUST remove the corresponding subscription even if such subscription
            // was installed by non-SUBSCRIBE means (such as an administrative interface).
            nTrigger = TRIGGER_NON_2XX;

            if (m_nSubState == SUB_STATE_TERMINATED)
            {
                IMS_UINT32 nCSeq = SipStack::GetCSeqNumber(objMsgInfo.GetMessage());

                if ((m_nCSeqForNotifyWithTerminated == SipPrivate::INVALID_SEQ_NUM) ||
                        (objMsgInfo.GetMethod().Equals(SipMethod::NOTIFY) &&
                                (m_nCSeqForNotifyWithTerminated == nCSeq)))
                {
                    nAction = SipDState::ACTION_TRANSIT_STATE;
                    nTrigger = TRIGGER_XXX_NOTIFY_TERMINATED;
                }
            }
            else
            {
                if ((nStatusCode != SipStatusCode::SC_401) &&
                        (nStatusCode != SipStatusCode::SC_407))
                {
                    // No "Retry-After" header... ???
                }
            }
        }
    }

    return nAction;
}

PROTECTED VIRTUAL const IMS_CHAR* SipDialogSubscribeUsage::TriggerToString(
        IN IMS_SINT32 nTrigger) const
{
    switch (nTrigger)
    {
        case TRIGGER_1XX:
            return "TRIGGER_1XX";
        case TRIGGER_2XX:
            return "TRIGGER_2XX";
        case TRIGGER_NON_2XX:
            return "TRIGGER_NON_2XX";
        case TRIGGER_XXX_NOTIFY_TERMINATED:
            return "TRIGGER_XXX_NOTIFY_TERMINATED";
        case TRIGGER_NOTIFY:
            return "TRIGGER_NOTIFY";
        default:
            return SipDialogUsage::TriggerToString(nTrigger);
    }
}
