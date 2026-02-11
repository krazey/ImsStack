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
#include "SipDState.h"
#include "SipDialogUsage.h"
#include "SipFeatures.h"
#include "SipMessageInfo.h"
#include "SipPrivate.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC VIRTUAL SipDialogUsage::~SipDialogUsage()
{
    IMS_TRACE_D("dtor: SipDialogUsage(%s)",
            (m_nType == TYPE_EPHEMERAL) ? "EPHEMERAL"
                                        : ((m_nType == TYPE_INVITE) ? "INVITE" : "SUBSCRIBE"),
            0, 0);
}

PUBLIC VIRTUAL IMS_BOOL SipDialogUsage::InitDialogUsage(IN const SipMessageInfo& /* objMsgInfo */)
{
    if (m_nType != TYPE_EPHEMERAL)
    {
        if (!m_pDialogBase->OnInit())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipDialogUsage::Equals(IN SipDialogUsage* pDUsage) const
{
    if (pDUsage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nType != pDUsage->m_nType)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SipDialogUsage::ToString() const
{
    AString strValue;

    strValue.Sprintf("DialogUsage: [%s]",
            (m_nType == TYPE_INVITE) ? "Invite"
                                     : ((m_nType == TYPE_SUBSCRIBE) ? "Subscribe" : "Ephemeral"));

    return strValue;
}

PUBLIC VIRTUAL IMS_SINT32 SipDialogUsage::UpdateUsageDetails(IN const SipMessageInfo& objMsgInfo)
{
    IMS_SINT32 nTrigger = SipDState::TRIGGER_INIT;
    IMS_SINT32 nAction = GetActionNTrigger(objMsgInfo, nTrigger);

    // If a dialog usage needs to be destroyed, then remove it and update the dialog details.
    if (m_nType != TYPE_EPHEMERAL)
    {
        if (IsUsageTerminated(m_pDialogBase->GetState(), nTrigger) ||
                (nAction == SipDState::ACTION_DESTROY_USAGE) ||
                (nAction == SipDState::ACTION_DESTROY_DIALOG))
        {
            m_pDialogBase->OnTerminated();
        }
    }

    IMS_TRACE_D(
            "UpdateDialogDetails: %s|%s", ActionToString(nAction), TriggerToString(nTrigger), 0);

    return m_pDialogBase->OnUpdateDialogDetails(objMsgInfo, m_nType, nAction, nTrigger);
}

PROTECTED VIRTUAL IMS_SINT32 SipDialogUsage::GetActionNTrigger(
        IN const SipMessageInfo& /* objMsgInfo */, OUT IMS_SINT32& nTrigger)
{
    nTrigger = SipDState::TRIGGER_INIT;

    return SipDState::ACTION_IGNORE;
}

PROTECTED
IMS_SINT32 SipDialogUsage::GetActionForResponse(IN const SipMessageInfo& objMsgInfo)
{
    // Base class's method takes an action by the response message only.
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();

    if (!SipStack::IsRequestMessage(pSipMsg))
    {
        IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);
        const SipDialogState* pDialogState = m_pDialogBase->GetDialogState();

        switch (nStatusCode)
        {
            // Impacts on a dialog usage (-> destroys a dialog usage)
            case SipStatusCode::SC_481:
                if (objMsgInfo.IsOutgoingMessage() &&
                        objMsgInfo.GetMethod().Equals(SipMethod::PRACK))
                {
                    IMS_TRACE_D("481-PRACK is ignored on dialog transition", 0, 0, 0);
                    return SipDState::ACTION_IGNORE;
                }

                return SipDState::ACTION_DESTROY_USAGE;

            case SipStatusCode::SC_405:  // FALL-THROUGH
            case SipStatusCode::SC_501:
                if (m_pDialogBase->GetState() == SipDState::STATE_CONFIRMED)
                {
                    const SipMethod& objMethod = objMsgInfo.GetMethod();

                    if (m_nType == TYPE_INVITE)
                    {
                        if (!objMethod.Equals(SipMethod::INVITE) &&
                                !objMethod.Equals(SipMethod::UPDATE) &&
                                !objMethod.Equals(SipMethod::PRACK) &&
                                !objMethod.Equals(SipMethod::BYE))
                        {
                            return SipDState::ACTION_IGNORE;
                        }
                    }
                    else if (m_nType == TYPE_SUBSCRIBE)
                    {
                        if (!objMethod.Equals(SipMethod::SUBSCRIBE) &&
                                !objMethod.Equals(SipMethod::NOTIFY))
                        {
                            return SipDState::ACTION_IGNORE;
                        }
                    }
                }

                if (SipFeatures::IsMultipleDialogUsagesRequiredForNonSharedDialog(
                            objMsgInfo.GetSlotId()) ||
                        ((pDialogState != IMS_NULL) && pDialogState->HasMultipleDialogUsages()))
                {
                    return SipDState::ACTION_DESTROY_USAGE;
                }

                if (m_pDialogBase->GetState() != SipDState::STATE_CONFIRMED)
                {
                    return SipDState::ACTION_TRANSIT_STATE;
                }
                break;

            case SipStatusCode::SC_489:
                if ((m_nType != TYPE_SUBSCRIBE) &&
                        (m_pDialogBase->GetState() == SipDState::STATE_CONFIRMED))
                {
                    return SipDState::ACTION_IGNORE;
                }

                return SipDState::ACTION_DESTROY_USAGE;

            case SipStatusCode::SC_480:
                if (m_pDialogBase->GetState() == SipDState::STATE_CONFIRMED)
                {
                    if (SipStack::IsHeaderPresent(pSipMsg, ISipHeader::RETRY_AFTER_ANY))
                    {
                        return SipDState::ACTION_IGNORE;
                    }
                }

                if (SipFeatures::IsMultipleDialogUsagesRequiredForNonSharedDialog(
                            objMsgInfo.GetSlotId()) ||
                        ((pDialogState != IMS_NULL) && pDialogState->HasMultipleDialogUsages()))
                {
                    return SipDState::ACTION_DESTROY_USAGE;
                }

                if (m_pDialogBase->GetState() != SipDState::STATE_CONFIRMED)
                {
                    return SipDState::ACTION_TRANSIT_STATE;
                }
                break;

                // Impacts on a dialog (-> destroys a dialog)
            case SipStatusCode::SC_404:  // FALL-THROUGH
            case SipStatusCode::SC_410:  // FALL-THROUGH
            case SipStatusCode::SC_416:  // FALL-THROUGH
            case SipStatusCode::SC_482:  // FALL-THROUGH
            case SipStatusCode::SC_483:  // FALL-THROUGH
            case SipStatusCode::SC_484:  // FALL-THROUGH
            case SipStatusCode::SC_485:  // FALL-THROUGH
            case SipStatusCode::SC_502:  // FALL-THROUGH
            case SipStatusCode::SC_604:
                // Should 502 be excluded if the dialog usage is for subscription? (RFC 6665)
                if (SipFeatures::IsMultipleDialogUsagesRequiredForNonSharedDialog(
                            objMsgInfo.GetSlotId()) ||
                        ((pDialogState != IMS_NULL) && pDialogState->HasMultipleDialogUsages()))
                {
                    return SipDState::ACTION_DESTROY_DIALOG;
                }

                if (m_pDialogBase->GetState() != SipDState::STATE_CONFIRMED)
                {
                    return SipDState::ACTION_TRANSIT_STATE;
                }
                break;

                // Impacts on transaction only
            default:
                // In this case, the state will not be changed.
                if (m_pDialogBase->GetState() == SipDState::STATE_CONFIRMED)
                {
                    if (nStatusCode == SipStatusCode::SC_408)
                    {
                        // Case1) TYPE_INVITE
                        //      It should be verified in the commercial networks.
                        //      return SipDState::ACTION_DESTROY_USAGE;
                        // Case2) TYPE_SUBSCRIBE
                        //      If re-SUBSCRIBE is timed out (Timer F),
                        //      it does not terminate the dialog usage.
                        //      if (objMsgInfo.GetMethod().Equals(SipMethod::NOTIFY))
                        //      {
                        //          return SipDState::ACTION_DESTROY_USAGE;
                        //      }
                    }
                    break;
                }

                return SipDState::ACTION_TRANSIT_STATE;
        }
    }

    return SipDState::ACTION_IGNORE;
}

PROTECTED
IMS_SINT32 SipDialogUsage::GetState() const
{
    if (m_pDialogBase == IMS_NULL)
    {
        return SipDState::STATE_INIT;
    }

    return m_pDialogBase->GetState();
}

PROTECTED VIRTUAL const IMS_CHAR* SipDialogUsage::TriggerToString(IN IMS_SINT32 nTrigger) const
{
    switch (nTrigger)
    {
        case SipDState::TRIGGER_INIT:
            return "TRIGGER_INIT";
        default:
            return "__INVALID__";
    }
}

PROTECTED GLOBAL const IMS_CHAR* SipDialogUsage::ActionToString(IN IMS_SINT32 nAction)
{
    switch (nAction)
    {
        case SipDState::ACTION_IGNORE:
            return "ACTION_IGNORE";
        case SipDState::ACTION_TRANSIT_STATE:
            return "ACTION_TRANSIT_STATE";
        case SipDState::ACTION_DESTROY_USAGE:
            return "ACTION_DESTROY_USAGE";
        case SipDState::ACTION_DESTROY_DIALOG:
            return "ACTION_DESTROY_DIALOG";
        default:
            return "__INVALID__";
    }
}
