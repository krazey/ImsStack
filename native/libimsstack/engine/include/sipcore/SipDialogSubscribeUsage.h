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
#ifndef SIP_DIALOG_SUBSCRIBE_USAGE_H_
#define SIP_DIALOG_SUBSCRIBE_USAGE_H_

#include "SipDialogUsage.h"

/**
 * @brief This class defines a SUBSCRIBE dialog usage.
 *
 * Created by:
 *  1) 200 class responses to SUBSCRIBE
 *  2) 200 class responses to REFER
 *  3) NOTIFY requests
 * Destroyed by:
 *  1) 200 class responses to NOTIFY-terminated
 *  2) NOTIFY or refresh-SUBSCRIBE request timeout
 *  3) Certain failure responses to NOTIFY or SUBSCRIBE
 *  4) Expiration without refresh if network issues prevent the terminal NOTIFY from arriving
 *  5) Anything that destroys a dialog and all its usages.
 */
class SipDialogSubscribeUsage : public SipDialogUsage
{
public:
    SipDialogSubscribeUsage(IN SipDialogBase* pDialog);
    SipDialogSubscribeUsage(IN const SipDialogSubscribeUsage& other);
    inline virtual ~SipDialogSubscribeUsage() {}

    SipDialogSubscribeUsage() = delete;
    SipDialogSubscribeUsage& operator=(IN const SipDialogSubscribeUsage&) = delete;

public:
    // SipDialogUsage class
    IMS_BOOL InitDialogUsage(IN const SipMessageInfo& objMsgInfo) override;
    inline SipDialogUsage* Clone() const override { return new SipDialogSubscribeUsage(*this); }
    IMS_BOOL CompareTo(IN const SipMessageInfo& objMsgInfo) const override;
    IMS_BOOL Equals(IN SipDialogUsage* pDUsage) const override;
    AString ToString() const override;
    IMS_SINT32 UpdateUsageDetails(IN const SipMessageInfo& objMsgInfo) override;

    IMS_BOOL InitDialogUsage(IN const SipMethod& objMethod);

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

public:
    /// State of Subscription-State
    enum
    {
        SUB_STATE_INIT = 0,

        SUB_STATE_PENDING,
        SUB_STATE_ACTIVE,
        SUB_STATE_TERMINATED
    };

    /// Method of initial subscription (explicit / implicit <-> SUBSCRIBE / REFER)
    enum
    {
        METHOD_SUBSCRIBE,
        METHOD_REFER
    };

private:
    /// SUBSCRIBE usage: TRIGGER events for dialog state transition
    enum
    {
        TRIGGER_INIT = SipDState::TRIGGER_INIT,

        TRIGGER_1XX,
        TRIGGER_2XX,
        TRIGGER_NON_2XX,
        TRIGGER_XXX_NOTIFY_TERMINATED,
        TRIGGER_NOTIFY,

        TRIGGER_MAX
    };

    static const IMS_SINT32 STATE_TABLE[SipDState::STATE_MAX][TRIGGER_MAX];

    // 4 Shall we need to check Expires header in SUBSCRIBE request or expires parameter
    // 4 in NOTIFY request
    //  For Subscription-State header info. of NOTIFY request
    IMS_SINT32 m_nSubState;
    // Method type: SUBSCRIBE or REFER
    IMS_SINT32 m_nMethod;
    // Event package name & id which is a parameter of Event header
    AString m_strEvent;
    AString m_strEventId;
    // For CSeq number of the NOTIFY request with "terminated"
    // when the subsequent NOTIFY request is received
    // before sending the response of the previous NOTIFY request...
    IMS_UINT32 m_nCSeqForNotifyWithTerminated;
};

#endif
