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

#include "CallReasonInfo.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/block/IMtcBlockRule.h"
#include "call/block/RadioBlockRule.h"
#include "call/radio/IMtcRadioChecker.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
RadioBlockRule::RadioBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType) :
        m_piMtcBlockRuleCheckListener(IMS_NULL),
        m_objMtcRadioChecker(objContext.GetRadioChecker()),
        m_ePeerType(objContext.GetCallInfo().ePeerType),
        m_bEmergency(objContext.GetCallInfo().bEmergency),
        m_bWifi(objContext.GetService().IsWlanIpCanType()),
        m_eCallType(eCallType),
        m_nCallKey(objContext.GetCallKey())
{
}

PUBLIC VIRTUAL RadioBlockRule::~RadioBlockRule()
{
    m_objMtcRadioChecker.SetTrafficCheckerListener(IMS_NULL);
}

PUBLIC VIRTUAL RadioBlockRule::Result RadioBlockRule::Check(
        IN IMtcBlockRuleCheckListener& objListener)
{
    m_piMtcBlockRuleCheckListener = &objListener;
    m_objMtcRadioChecker.SetTrafficCheckerListener(this);

    CheckResult eCheckResult =
            m_objMtcRadioChecker.Check(m_eCallType, m_bEmergency, m_ePeerType, m_bWifi, m_nCallKey);

    switch (eCheckResult)
    {
        case CheckResult::UNBLOCKED:
            return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
        case CheckResult::PENDING:
            return Result(IMtcBlockRule::Result::Status::PENDING);
        default:  //  CheckResult::BLOCKED:
            return Result(IMtcBlockRule::Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE));
    }
}

PUBLIC VIRTUAL void RadioBlockRule::OnConnectionSetupPrepared()
{
    m_piMtcBlockRuleCheckListener->OnBlockRuleChecked(Result(Result::Status::UNBLOCKED));
}

PUBLIC VIRTUAL void RadioBlockRule::OnConnectionFailed()
{
    m_piMtcBlockRuleCheckListener->OnBlockRuleChecked(
            Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE)));
}
