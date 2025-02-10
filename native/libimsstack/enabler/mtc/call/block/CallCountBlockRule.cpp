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

#include "CarrierConfig.h"
#include "ImsList.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/block/CallCountBlockRule.h"
#include "configuration/MtcConfigurationProxy.h"

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

    const IMS_SINT32 nMaxCallCount = m_objConfiguration.GetInt(ConfigVoice::KEY_CALL_MAX_COUNT_INT);
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
    const ImsList<IMtcCall*> lstCalls = m_objCallManager.GetCalls();
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
