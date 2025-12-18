/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "IMtcService.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/block/IncomingCallBarringBlockRule.h"
#include "call/ParticipantInfo.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
IncomingCallBarringBlockRule::IncomingCallBarringBlockRule(
        IN IMtcCallContext& objContext, IN CallType eCallType) :
        m_objContext(objContext),
        m_objService(objContext.GetService()),
        m_eCallType(eCallType)
{
}

PUBLIC VIRTUAL IncomingCallBarringBlockRule::~IncomingCallBarringBlockRule() {}

PUBLIC VIRTUAL IncomingCallBarringBlockRule::Result IncomingCallBarringBlockRule::Check(
        IN [[maybe_unused]] IMtcBlockRuleCheckListener& objListener)
{
    if (m_eCallType == CallType::VOIP || m_eCallType == CallType::RTT)
    {
        return IsBarringActivatedByCallType(PermanentSuppType::TB_CB_INCOMING_ALL_VOICE,
                PermanentSuppType::TB_CB_INCOMING_ROAMING_VOICE,
                PermanentSuppType::TB_CB_INCOMING_ANONYMOUS_VOICE);
    }
    else if (m_eCallType == CallType::VT || m_eCallType == CallType::VIDEO_RTT)
    {
        return IsBarringActivatedByCallType(PermanentSuppType::TB_CB_INCOMING_ALL_VIDEO,
                PermanentSuppType::TB_CB_INCOMING_ROAMING_VIDEO,
                PermanentSuppType::TB_CB_INCOMING_ANONYMOUS_VIDEO);
    }

    IMS_TRACE_D("Check : Unknown call", 0, 0, 0);
    return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
}

PRIVATE IncomingCallBarringBlockRule::Result
IncomingCallBarringBlockRule::IsBarringActivatedByCallType(IN PermanentSuppType eBarringTypeAll,
        IN PermanentSuppType eBarringTypeRoaming, IN PermanentSuppType eBarringTypeAnonymous) const
{
    if (m_objService.IsPermanentSuppServiceEnabled(eBarringTypeAll))
    {
        IMS_TRACE_I("IsBarringActivatedByCallType - All barred.", 0, 0, 0);
        return Result(IMtcBlockRule::Result::Status::BLOCKED, CallReasonInfo(CODE_CALL_BARRED));
    }

    if (m_objService.IsRoaming() && m_objService.IsPermanentSuppServiceEnabled(eBarringTypeRoaming))
    {
        IMS_TRACE_I("IsBarringActivatedByCallType - Roaming barred.", 0, 0, 0);
        return Result(IMtcBlockRule::Result::Status::BLOCKED, CallReasonInfo(CODE_CALL_BARRED));
    }

    if (m_objContext.GetParticipantInfo().GetOipType() == OipType::RESTRICTED &&
            m_objService.IsPermanentSuppServiceEnabled(eBarringTypeAnonymous))
    {
        IMS_TRACE_I("IsBarringActivatedByCallType - Anonymous barred.", 0, 0, 0);
        return Result(IMtcBlockRule::Result::Status::BLOCKED, CallReasonInfo(CODE_CALL_BARRED));
    }

    return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
}
