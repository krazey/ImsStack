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

#include "SipDialogInviteUsage.h"
#include "SipPrivate.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP_CORE__;

// clang-format off
PRIVATE GLOBAL const IMS_SINT32
SipDialogInviteUsage::STATE_TABLE[SipDState::STATE_MAX][SipDialogInviteUsage::TRIGGER_MAX] =
{
    // STATE_INIT
    {
        SipDState::STATE_INIT,                 // TRIGGER_INIT
        SipDState::STATE_EARLY,                // TRIGGER_1XX
        SipDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SipDState::STATE_INIT,                 // TRIGGER_NON_2XX
        SipDState::STATE_INIT,                 // TRIGGER_2XX_BYE
        SipDState::STATE_INIT                  // TRIGGER_BYE
    },
    // STATE_TERMINATED
    {
        SipDState::STATE_TERMINATED,           // TRIGGER_INIT
        SipDState::STATE_TERMINATED,           // TRIGGER_1XX
        SipDState::STATE_TERMINATED,           // TRIGGER_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_NON_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_2XX_BYE
        SipDState::STATE_TERMINATED            // TRIGGER_BYE
    },
    // STATE_EARLY
    {
        SipDState::STATE_EARLY,                // TRIGGER_INIT
        SipDState::STATE_EARLY,                // TRIGGER_1XX
        SipDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_NON_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_2XX_BYE
        SipDState::STATE_EARLY                 // TRIGGER_BYE
    },
    // STATE_CONFIRMED
    {
        SipDState::STATE_CONFIRMED,            // TRIGGER_INIT
        SipDState::STATE_CONFIRMED,            // TRIGGER_1XX
        SipDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SipDState::STATE_CONFIRMED,            // TRIGGER_NON_2XX
        SipDState::STATE_TERMINATED,           // TRIGGER_2XX_BYE
        SipDState::STATE_CONFIRMED             // TRIGGER_BYE
    }
};
// clang-format on

PUBLIC VIRTUAL IMS_BOOL SipDialogInviteUsage::CompareTo(IN const SipMessageInfo& objMsgInfo) const
{
    const SipMethod& objMethod = objMsgInfo.GetMethod();

    // CASE : subscribe usage & register usage
    if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER) ||
            objMethod.Equals(SipMethod::NOTIFY) || objMethod.Equals(SipMethod::REGISTER))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_SINT32 SipDialogInviteUsage::GetNextState(
        IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger)
{
    return IsValidTrigger(nTrigger) ? STATE_TABLE[nState][nTrigger] : SipDState::STATE_MAX;
}

PROTECTED VIRTUAL IMS_SINT32 SipDialogInviteUsage::GetActionNTrigger(
        IN const SipMessageInfo& objMsgInfo, OUT IMS_SINT32& nTrigger)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();
    IMS_SINT32 nAction = SipDState::ACTION_TRANSIT_STATE;

    nTrigger = TRIGGER_INIT;

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        if (objMsgInfo.GetMethod().Equals(SipMethod::BYE))
        {
            nTrigger = TRIGGER_BYE;
        }
    }
    else
    {
        const SipMethod& objMethod = objMsgInfo.GetMethod();

        nAction = GetActionForResponse(objMsgInfo);

        IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

        if (SipStatusCode::IsProvisional(nStatusCode))
        {
            nTrigger = TRIGGER_1XX;
        }
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (objMethod.Equals(SipMethod::BYE))
            {
                nAction = SipDState::ACTION_TRANSIT_STATE;
                nTrigger = TRIGGER_2XX_BYE;
            }
            else
            {
                if (!objMethod.Equals(SipMethod::INVITE))
                {
                    nAction = SipDState::ACTION_IGNORE;
                }

                nTrigger = TRIGGER_2XX;
            }
        }
        else
        {
            if (GetState() == SipDState::STATE_EARLY)
            {
                if (objMethod.Equals(SipMethod::INVITE))
                {
                    nTrigger = TRIGGER_NON_2XX;
                }
            }
            else
            {
                // 4 How to handle in case of BYE ???
                if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::BYE))
                {
                    nAction = SipDState::ACTION_IGNORE;
                }

                nTrigger = TRIGGER_NON_2XX;
            }
        }
    }

    return nAction;
}

PROTECTED VIRTUAL const IMS_CHAR* SipDialogInviteUsage::TriggerToString(
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
        case TRIGGER_2XX_BYE:
            return "TRIGGER_2XX_BYE";
        case TRIGGER_BYE:
            return "TRIGGER_BYE";
        default:
            return SipDialogUsage::TriggerToString(nTrigger);
    }
}

PRIVATE GLOBAL IMS_BOOL SipDialogInviteUsage::IsValidTrigger(IN IMS_SINT32 nTrigger)
{
    return (TRIGGER_INIT <= nTrigger) && (nTrigger < TRIGGER_MAX);
}
