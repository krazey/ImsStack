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
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/UpdatingInfo.h"
#include "call/block/CallTypeBlockRule.h"
#include "configuration/MtcConfigurationProxy.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallTypeBlockRule::CallTypeBlockRule(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_objConfiguration(objContext.GetConfigurationProxy())
{
}

PUBLIC VIRTUAL CallTypeBlockRule::~CallTypeBlockRule() {}

PUBLIC VIRTUAL CallTypeBlockRule::Result CallTypeBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    Result objResult = CheckSupportTextVideo();
    if (objResult.eStatus != Result::Status::UNBLOCKED)
    {
        return objResult;
    }

    return CheckSupportVideoMultipleCall();
}

PRIVATE CallTypeBlockRule::Result CallTypeBlockRule::CheckSupportTextVideo()
{
    IMS_SINT32 nPolicyForTextAndVideo =
            m_objConfiguration.GetInt(Feature::POLICY_FOR_TEXT_WITH_VIDEO);
    if (nPolicyForTextAndVideo == CarrierConfig::ImsVt::TEXT_VIDEO_ALLOWED)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    CallType eTargetCallToCheck;
    if (m_objContext.GetCall().GetState() == IMtcCall::State::IDLE)
    {
        eTargetCallToCheck = m_objContext.GetSession()->GetCallType();
    }
    else if (nPolicyForTextAndVideo == CarrierConfig::ImsVt::TEXT_VIDEO_NOT_ALLOWED)
    {
        // not supporting text and video media description simultaneously irrespective of port
        // eg. VZW
        eTargetCallToCheck = m_objContext.GetMessageUtils().GetCallTypeFromSdp(
                &m_objContext.GetSession()->GetISession(), IMS_FALSE, IMS_TRUE, IMS_FALSE);
    }
    else  // TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE
    {
        // not supporting text and video media description simultaneously.
        // port 0 media is not included.
        // eg. ATT
        eTargetCallToCheck = m_objContext.GetUpdatingInfo().GetTargetCallType();
    }

    if (eTargetCallToCheck == CallType::VIDEO_RTT)
    {
        IMS_TRACE_I("CheckSupportTextVideo : Video RTT is not supported", 0, 0, 0);
        return Result(Result::Status::BLOCKED,
                CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE));
    }
    return Result(Result::Status::UNBLOCKED);
}

PRIVATE CallTypeBlockRule::Result CallTypeBlockRule::CheckSupportVideoMultipleCall()
{
    if (m_objConfiguration.Is(Feature::ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    ImsList<IMtcCall*> lstOtherCalls = m_objContext.GetOtherCalls();
    if (lstOtherCalls.IsEmpty())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (!HasVideoCall(lstOtherCalls))
    {
        CallType eTargetCallToCheck;
        if (m_objContext.GetCall().GetState() == IMtcCall::State::ESTABLISHED)
        {
            eTargetCallToCheck = m_objContext.GetUpdatingInfo().GetTargetCallType();
        }
        else
        {
            eTargetCallToCheck = m_objContext.GetSession()->GetCallType();
        }

        if (!IsVideoCall(eTargetCallToCheck))
        {
            return Result(Result::Status::UNBLOCKED);
        }
    }

    IMS_TRACE_I("CheckSupportVideoMultipleCall : Video call cannot be placed with another call", 0,
            0, 0);

    if (m_objContext.GetCallInfo().ePeerType == PeerType::MO)
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED));
    }
    else
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED));
    }
}

PRIVATE IMS_BOOL CallTypeBlockRule::HasVideoCall(IN const ImsList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        if (IsVideoCall(lstCalls.GetAt(nIndex)->GetCallType()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE IMS_BOOL CallTypeBlockRule::IsVideoCall(IN CallType eCallType)
{
    return eCallType == CallType::VT || eCallType == CallType::VIDEO_RTT;
}
