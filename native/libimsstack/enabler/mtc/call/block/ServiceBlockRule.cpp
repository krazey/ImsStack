/*
 * Copyright (C) 2024 The Android Open Source Project
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
#include "IMtcService.h"
#include "ImsAosParameter.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/MtcCallStringUtils.h"
#include "call/block/ServiceBlockRule.h"
#include "helper/IMtcAosConnector.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ServiceBlockRule::ServiceBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType) :
        m_objService(objContext.GetService()),
        m_objContext(objContext),
        m_eCallType(eCallType)
{
}

PUBLIC VIRTUAL ServiceBlockRule::~ServiceBlockRule() {}

PUBLIC VIRTUAL ServiceBlockRule::Result ServiceBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    return m_objContext.IsEstablished() ? CheckForEstablishedCall() : CheckForInitiatingCall();
}

PRIVATE ServiceBlockRule::Result ServiceBlockRule::CheckForInitiatingCall() const
{
    if (m_objService.GetAosConnector()->IsFeatureConnected(ImsAosFeature::MMTEL))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (m_eCallType == CallType::VT &&
            m_objService.GetAosConnector()->IsFeatureConnected(ImsAosFeature::VIDEO))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("CheckForInitiatingCall : MMTel service is not available", 0, 0, 0);
    return Result(Result::Status::BLOCKED, GetBlockReasonForInitiatingCall());
}

PRIVATE ServiceBlockRule::Result ServiceBlockRule::CheckForEstablishedCall() const
{
    if (HasCapabilitiesForCallType(m_eCallType))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("CheckForEstablishedCall : Capability for [%s] is not available",
            MtcCallStringUtils::ConvertCallType(m_eCallType), 0, 0);
    return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION));
}

PRIVATE CallReasonInfo ServiceBlockRule::GetBlockReasonForInitiatingCall() const
{
    if (m_objContext.GetCallInfo().ePeerType == PeerType::MO)
    {
        if (m_objContext.IsCsfbAvailable())
        {
            return CallReasonInfo(
                    CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
        }
        else
        {
            return CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE);
        }
    }
    else
    {
        return CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_SIP_488);
    }
}

PRIVATE IMS_BOOL ServiceBlockRule::HasCapabilitiesForCallType(IN const CallType eCallType) const
{
    switch (eCallType)
    {
        case CallType::VT:
            return m_objService.GetAosConnector()->IsFeatureConnected(ImsAosFeature::VIDEO);
        case CallType::RTT:
            return m_objService.GetAosConnector()->IsFeatureConnected(ImsAosFeature::TEXT);
        case CallType::VIDEO_RTT:
            return m_objService.GetAosConnector()->IsFeatureConnected(ImsAosFeature::VIDEO) &&
                    m_objService.GetAosConnector()->IsFeatureConnected(ImsAosFeature::TEXT);
        default:  // VOIP is for usually downgrade cases after established, so just don't check
            return IMS_TRUE;
    }
}
