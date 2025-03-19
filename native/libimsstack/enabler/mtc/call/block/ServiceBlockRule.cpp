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
#include "call/block/ServiceBlockRule.h"
#include "helper/IMtcAosConnector.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ServiceBlockRule::ServiceBlockRule(IN IMtcCallContext& objContext) :
        m_objService(objContext.GetService()),
        m_objContext(objContext)
{
}

PUBLIC VIRTUAL ServiceBlockRule::~ServiceBlockRule() {}

PUBLIC VIRTUAL ServiceBlockRule::Result ServiceBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objService.GetAosConnector()->IsFeatureConnected(ImsAosFeature::MMTEL))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : MMTel service is not available", 0, 0, 0);
    return Result(Result::Status::BLOCKED, GetBlockReason());
}

PRIVATE CallReasonInfo ServiceBlockRule::GetBlockReason() const
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
