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
#include "IImsRadio.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/EpsFallbackTrigger.h"
#include "call/block/IMtcBlockRule.h"
#include "call/block/RadioBlockRule.h"
#include "call/radio/IMtcRadioChecker.h"
#include "configuration/MtcConfigurationProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
RadioBlockRule::RadioBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType) :
        m_objContext(objContext),
        m_piMtcBlockRuleCheckListener(IMS_NULL),
        m_eCallType(eCallType)
{
}

PUBLIC VIRTUAL RadioBlockRule::~RadioBlockRule()
{
    m_objContext.GetRadioChecker().SetTrafficCheckerListener(IMS_NULL);
}

PUBLIC VIRTUAL RadioBlockRule::Result RadioBlockRule::Check(
        IN IMtcBlockRuleCheckListener& objListener)
{
    m_piMtcBlockRuleCheckListener = &objListener;
    m_objContext.GetRadioChecker().SetTrafficCheckerListener(this);

    CheckResult eCheckResult = m_objContext.GetRadioChecker().Check(m_eCallType,
            m_objContext.GetCallInfo().bEmergency, m_objContext.GetCallInfo().ePeerType,
            m_objContext.GetService().IsWlanIpCanType(), m_objContext.GetCallKey());

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

PUBLIC VIRTUAL void RadioBlockRule::OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis)
{
    CallReasonInfo objReason = IsEpsFallbackRequired(nFailureReason, nWaitTimeMillis)
            ? CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK)
            : CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE);

    m_piMtcBlockRuleCheckListener->OnBlockRuleChecked(Result(Result::Status::BLOCKED, objReason));
}

PRIVATE
IMS_BOOL RadioBlockRule::IsEpsFallbackRequired(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) const
{
    if (!m_objContext.GetEpsFallbackTrigger().IsVoNr())
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nTimerVzw =
            m_objContext.GetConfigurationProxy().GetInt(Feature::MO_CALL_REQUEST_TIMEOUT);
    return nFailureReason == IImsRadio::REASON_RRC_REJECT && nWaitTimeMillis >= nTimerVzw &&
            EpsFallbackTrigger::IsRequired(m_objContext.GetConfigurationProxy());
}
