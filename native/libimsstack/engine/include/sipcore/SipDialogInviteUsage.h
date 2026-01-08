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
#ifndef SIP_DIALOG_INVITE_USAGE_H_
#define SIP_DIALOG_INVITE_USAGE_H_

#include "SipDialogUsage.h"

/**
 * @brief This class defines an INVITE dialog usage.
 *
 * Created by:
 *  1) Non-100 provisional responses to INVITE
 *  2) 200 response to INVITE
 * Destroyed by:
 *  1) 200 responses to BYE
 *  2) Certain failure responses to INVITE, UPDATE, PRACK, INFO or BYE
 *  3) Anything that destroys a dialog and all its usages
 */
class SipDialogInviteUsage : public SipDialogUsage
{
public:
    inline explicit SipDialogInviteUsage(IN SipDialogBase* pDialog) :
            SipDialogUsage(SipDialogUsage::TYPE_INVITE, pDialog)
    {
    }
    inline SipDialogInviteUsage(IN const SipDialogInviteUsage& other) :
            SipDialogUsage(other)
    {
    }
    ~SipDialogInviteUsage() override = default;

    SipDialogInviteUsage() = delete;
    SipDialogInviteUsage& operator=(IN const SipDialogInviteUsage&) = delete;

public:
    // SipDialogUsage class
    inline SipDialogUsage* Clone() const override { return new SipDialogInviteUsage(*this); }
    IMS_BOOL CompareTo(IN const SipMessageInfo& objMsgInfo) const override;

    static IMS_SINT32 GetNextState(IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger);

protected:
    IMS_SINT32 GetActionNTrigger(
            IN const SipMessageInfo& objMsgInfo, OUT IMS_SINT32& nTrigger) override;
    inline IMS_BOOL IsUsageTerminated(IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger) const override
    {
        return (GetNextState(nState, nTrigger) == SipDState::STATE_TERMINATED) ? IMS_TRUE
                                                                               : IMS_FALSE;
    }

    const IMS_CHAR* TriggerToString(IN IMS_SINT32 nTrigger) const override;

private:
    static IMS_BOOL IsValidTrigger(IN IMS_SINT32 nTrigger);

private:
    /// INVITE usage: TRIGGER events for dialog state transition
    enum
    {
        TRIGGER_INIT = SipDState::TRIGGER_INIT,

        TRIGGER_1XX,
        TRIGGER_2XX,
        TRIGGER_NON_2XX,
        TRIGGER_2XX_BYE,
        TRIGGER_BYE,

        TRIGGER_MAX
    };

private:
    static const IMS_SINT32 STATE_TABLE[SipDState::STATE_MAX][TRIGGER_MAX];
};

#endif
