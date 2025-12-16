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

#include "ImsList.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/MtcCallStringUtils.h"
#include "call/UpdatingInfo.h"
#include "call/block/ProcessingCallBlockRule.h"
#include "emergency/IMtcEmergencyServiceManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ProcessingCallBlockRule::ProcessingCallBlockRule(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC VIRTUAL ProcessingCallBlockRule::~ProcessingCallBlockRule() {}

PUBLIC VIRTUAL ProcessingCallBlockRule::Result ProcessingCallBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objContext.GetCallInfo().eEmergencyType != EmergencyType::NONE)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    ImsList<IMtcCall*> lstCalls = m_objContext.GetOtherCalls();

    PeerType ePeerType = m_objContext.GetCallInfo().ePeerType;
    if (m_objContext.GetEmergencyServiceManager().GetState() ==
                    IEmergencyServiceController::State::OPENING ||
            IsEmergencyCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED,
                ePeerType == PeerType::MO ? CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE)
                                          : CallReasonInfo(CODE_REJECT_ONGOING_E911_CALL));
    }

    if (IsCallSetupProcessing(lstCalls))
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP));
    }

    if (IsCallConverting(lstCalls) && ePeerType == PeerType::MT)
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_ONGOING_CALL_UPGRADE));
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsCallSetupProcessing(IN const ImsList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::IDLE || eState == IMtcCall::State::OUTGOING ||
                eState == IMtcCall::State::INCOMING || eState == IMtcCall::State::ALERTING)
        {
            IMS_TRACE_I("IsCallSetupProcessing : Call in [%s] state exists",
                    MtcCallStringUtils::ConvertCallState(eState), 0, 0);
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsCallConverting(IN const ImsList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::UPDATING)
        {
            const UpdateType eType =
                    lstCalls.GetAt(nIndex)->GetCallContext().GetUpdatingInfo().GetRequestingType();
            IMS_TRACE_I("IsCallConverting : Converting call exists eType[%s]",
                    MtcCallStringUtils::ConvertUpdateType(eType), 0, 0);
            if (eType == UpdateType::SESSION)
            {
                return IMS_TRUE;
            }
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsEmergencyCallExists(IN const ImsList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        if (lstCalls.GetAt(nIndex)->GetCallContext().GetCallInfo().IsEmergency())
        {
            IMS_TRACE_I("IsEmergencyCallExists : Emergency call exists", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}
