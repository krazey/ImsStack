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
#include "call/block/CallTrafficBlockRule.h"
#include "call/block/IMtcBlockRule.h"
#include "call/traffic/IMtcCallTrafficChecker.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallTrafficBlockRule::CallTrafficBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType) :
        m_piMtcBlockRuleCheckListener(IMS_NULL),
        m_objMtcCallTrafficChecker(objContext.GetCallTrafficChecker()),
        m_ePeerType(objContext.GetCallInfo().ePeerType),
        m_bEmergency(objContext.GetCallInfo().bEmergency),
        m_bWifi(objContext.GetService().IsWlanIpCanType()),
        m_eCallType(eCallType)
{
}

PUBLIC VIRTUAL CallTrafficBlockRule::~CallTrafficBlockRule()
{
    m_objMtcCallTrafficChecker.SetTrafficCheckerListener(IMS_NULL);
}

PUBLIC VIRTUAL CallTrafficBlockRule::Result CallTrafficBlockRule::Check(
        IN IMtcBlockRuleCheckListener& objListener)
{
    m_piMtcBlockRuleCheckListener = &objListener;
    m_objMtcCallTrafficChecker.SetTrafficCheckerListener(this);

    CheckResult eCheckResult =
            m_objMtcCallTrafficChecker.Check(m_eCallType, m_bEmergency, m_ePeerType, m_bWifi);

    switch (eCheckResult)
    {
        case CheckResult::UNBLOCKED:
            return Result(IMtcBlockRule::Result::Status::UNBLOCKED);
        case CheckResult::PENDING:
            return Result(IMtcBlockRule::Result::Status::PENDING);

        case CheckResult::BLOCKED:
            return Result(IMtcBlockRule::Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE));
    }
}

PUBLIC VIRTUAL void CallTrafficBlockRule::OnConnectionSetupPrepared()
{
    m_piMtcBlockRuleCheckListener->OnBlockRuleChecked(Result(Result::Status::UNBLOCKED));
}

PUBLIC VIRTUAL void CallTrafficBlockRule::OnConnectionFailed()
{
    m_piMtcBlockRuleCheckListener->OnBlockRuleChecked(
            Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE)));
}
