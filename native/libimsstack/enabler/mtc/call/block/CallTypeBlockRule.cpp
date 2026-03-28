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
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/block/CallTypeBlockRule.h"
#include "configuration/MtcConfigurationProxy.h"
#include "media/IMedia.h"
#include "utility/CallTypeUtil.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallTypeBlockRule::CallTypeBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType) :
        m_objContext(objContext),
        m_objConfiguration(objContext.GetConfigurationProxy()),
        m_eCallType(eCallType)
{
}

PUBLIC VIRTUAL CallTypeBlockRule::~CallTypeBlockRule() {}

PUBLIC VIRTUAL CallTypeBlockRule::Result CallTypeBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (IsBlockedByTextVideoCall())
    {
        return Result(Result::Status::BLOCKED,
                CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE));
    }

    if (IsBlockedByVideoMultipleCall())
    {
        CallReasonInfo objReason = m_objContext.GetCallInfo().ePeerType == PeerType::MO
                ? CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED)
                : CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED);
        return Result(Result::Status::BLOCKED, objReason);
    }

    if (m_objContext.GetCallInfo().bConference &&
            HasRttCall(m_objContext.GetCallManager().GetCalls()))
    {
        return Result(
                Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_RTT_ON));
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE IMS_BOOL CallTypeBlockRule::IsBlockedByTextVideoCall()
{
    IMS_SINT32 nPolicyForTextAndVideo =
            m_objConfiguration.GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT);
    if (nPolicyForTextAndVideo == ConfigVt::TEXT_VIDEO_ALLOWED)
    {
        return IMS_FALSE;
    }

    if ((nPolicyForTextAndVideo == ConfigVt::TEXT_VIDEO_NOT_ALLOWED ||
                nPolicyForTextAndVideo == ConfigVt::TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE) &&
            m_eCallType == CallType::VIDEO_RTT)
    {
        IMS_TRACE_I("IsBlockedByTextVideoCall : Video RTT is not supported", 0, 0, 0);
        return IMS_TRUE;
    }

    if (nPolicyForTextAndVideo == ConfigVt::TEXT_VIDEO_NOT_ALLOWED)
    {
        IMS_SINT32 nVideoPort = GetRemotePort(SdpMedia::TYPE_VIDEO);
        IMS_SINT32 nTextPort = GetRemotePort(SdpMedia::TYPE_TEXT);

        IMS_TRACE_I(
                "IsBlockedByTextVideoCall : video port=%d, text port=%d", nVideoPort, nTextPort, 0);

        if (HasVideoRttSdp(nVideoPort, nTextPort))
        {
            // Allow downgrade request
            if (nVideoPort == 0 && nTextPort == 0)
            {
                return IMS_FALSE;
            }
            // Allow hold/resume request of existing RTT/VT call
            IMtcSession* pMtcSession = m_objContext.GetSession();
            if (m_eCallType == pMtcSession->GetPreviousCallType())
            {
                return IMS_FALSE;
            }

            // Block simultaneous video and text descriptions
            IMS_TRACE_I("IsBlockedByTextVideoCall : SDP contains video and text", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE IMS_BOOL CallTypeBlockRule::IsBlockedByVideoMultipleCall()
{
    if (m_objConfiguration.GetBoolean(
                ConfigVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL))
    {
        return IMS_FALSE;
    }

    ImsList<IMtcCall*> lstOtherCalls = m_objContext.GetOtherCalls();
    if (lstOtherCalls.IsEmpty())
    {
        return IMS_FALSE;
    }

    if (CallTypeUtil::IsVideoCall(m_eCallType) || HasVideoCall(lstOtherCalls))
    {
        IMS_TRACE_I("IsBlockedByVideoMultipleCall : Video call cannot be placed with another call",
                0, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE IMS_SINT32 CallTypeBlockRule::GetRemotePort(IN IMS_SINT32 eMediaType) const
{
    ISession* piSession = &m_objContext.GetSession()->GetISession();
    return m_objContext.GetMessageUtils().GetRemotePortFromSdp(piSession, eMediaType);
}

PRIVATE IMS_BOOL CallTypeBlockRule::HasVideoCall(IN const ImsList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        if (CallTypeUtil::IsVideoCall(lstCalls.GetAt(nIndex)->GetCallType()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE IMS_BOOL CallTypeBlockRule::HasVideoRttSdp(IN IMS_SINT32 videoPort, IN IMS_SINT32 textPort)
{
    return videoPort != -1 && textPort != -1;
}

PRIVATE IMS_BOOL CallTypeBlockRule::HasRttCall(IN const ImsList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        if (CallTypeUtil::IsRttCall(lstCalls.GetAt(nIndex)->GetCallType()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
