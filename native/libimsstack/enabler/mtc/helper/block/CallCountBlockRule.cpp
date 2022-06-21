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

#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/block/CallCountBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallCountBlockRule::CallCountBlockRule(IN IMtcCallContext& objContext) :
        m_objConfiguration(objContext.GetConfigurationProxy()),
        m_objCallManager(objContext.GetCallManager()),
        m_objCallInfo(objContext.GetCallInfo())
{
}

PUBLIC VIRTUAL CallCountBlockRule::~CallCountBlockRule() {}

PUBLIC VIRTUAL CallCountBlockRule::Result CallCountBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objCallInfo.bConference && m_objCallInfo.ePeerType == PeerType::MO)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (!m_objConfiguration.Is(Feature::ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL))
    {
        if (IsOtherCallExists() && IsVideoCallExists())
        {
            IMS_TRACE_I("Check : Video call cannot be placed with another call", 0, 0, 0);
            if (m_objCallInfo.ePeerType == PeerType::MO)
            {
                return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED));
            }
            else
            {
                return Result(Result::Status::BLOCKED,
                        CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED));
            }
        }
    }

    const IMS_SINT32 nMaxCallCount = m_objConfiguration.GetInt(Feature::CALL_MAX_COUNT);
    if (GetActiveCallCount() > nMaxCallCount)
    {
        IMS_TRACE_I("Check : Max call count[%d] reached", nMaxCallCount, 0, 0);

        if (m_objCallInfo.ePeerType == PeerType::MO)
        {
            return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED));
        }
        else
        {
            return Result(
                    Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED));
        }
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE
IMS_UINT32 CallCountBlockRule::GetActiveCallCount()
{
    const IMSList<IMtcCall*> lstCalls = m_objCallManager.GetCalls();
    IMS_UINT32 nCount = 0;

    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState != IMtcCall::State::TERMINATING)
        {
            nCount += 1;
        }
    }

    return nCount;
}

PRIVATE IMS_BOOL CallCountBlockRule::IsOtherCallExists()
{
    return m_objCallManager.GetCalls().GetSize() > 1;
}

PRIVATE IMS_BOOL CallCountBlockRule::IsVideoCallExists()
{
    if (!m_objCallManager.GetCallsByType(CallType::VT).IsEmpty())
    {
        return IMS_TRUE;
    }
    if (!m_objCallManager.GetCallsByType(CallType::VIDEO_RTT).IsEmpty())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
