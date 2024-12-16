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

#include "Engine.h"
#include "IConfiguration.h"
#include "IMessage.h"
#include "ISipConfig.h"
#include "ISipConfigV.h"
#include "ISipHeader.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/termination/DefaultStatusCodeAndReasonCodeSets.h"
#include "call/termination/UpdateErrorHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IPassiveTimerHolder.h"
#include "utility/IMessageUtils.h"
#include <unordered_map>

__IMS_TRACE_TAG_COM_MTC__;

// clang-format off
const std::unordered_map<IMS_SINT32, UpdateErrorHandler::ActionFunc>
        UpdateErrorHandler::objActionFuncMap = {
    {ConfigVoice::UPDATE_ERROR_ACTION_TERMINATE,
            &UpdateErrorHandler::HandleTerminate},
    {ConfigVoice::UPDATE_ERROR_ACTION_RETRY,
            &UpdateErrorHandler::HandleRetry},
    {ConfigVoice::UPDATE_ERROR_ACTION_GLARE_CONDITION,
            &UpdateErrorHandler::HandleGlareCondition},
    {ConfigVoice::UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER,
            &UpdateErrorHandler::HandleBlockCallByTimer}
};
// clang-format on

PUBLIC
UpdateErrorHandler::UpdateErrorHandler(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
UpdateErrorHandler::~UpdateErrorHandler() {}

PUBLIC
CallReasonInfo UpdateErrorHandler::Handle(IN const IMessage* piMessage) const
{
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("Handle : piMessage is null", 0, 0, 0);
        return CallReasonInfo(CODE_SIP_SERVER_ERROR);
    }

    ImsVector<IMS_SINT32> objActions = MtcConfigurationResolver::LookupActionForStatusCode(
            m_objContext.GetConfigurationProxy(),
            ConfigVoice::KEY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            piMessage->GetStatusCode());
    for (IMS_UINT32 i = 0; i < objActions.GetSize(); ++i)
    {
        auto it = objActionFuncMap.find(objActions.GetAt(i));
        if (it != objActionFuncMap.end())
        {
            CallReasonInfo objResult = (this->*(it->second))(*piMessage);
            if (objResult.nCode != CODE_NONE)
            {
                return objResult;
            }
        }
    }

    return CallReasonInfo(CODE_UNSPECIFIED);
}

PUBLIC GLOBAL IMS_UINT32 UpdateErrorHandler::GetGlareTimeMillisecond(IN PeerType ePeerType)
{
    IMS_UINT32 nUpperT = 0;
    IMS_UINT32 nBaseT = 0;

    // RFC 3261 14.1: Glare condition for 491 response
    if (ePeerType == PeerType::MO)
    {
        nUpperT = 4000;
        nBaseT = 2100;
    }
    else
    {
        nUpperT = 2000;
        nBaseT = 0;
    }

    return nBaseT + IMS_SYS_GetRandom(nUpperT - nBaseT);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::HandleTerminate(IN const IMessage& objMessage) const
{
    return GetDefaultCallReasonInfo(objMessage);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::HandleRetry(IN const IMessage& objMessage) const
{
    IMS_SINT32 nRetryAfterInSeconds = m_objContext.GetMessageUtils().GetHeaderValueInt(
            &objMessage, ISipHeader::RETRY_AFTER_ANY);
    if (nRetryAfterInSeconds > 0)
    {
        return CallReasonInfo(CODE_INTERNAL_RETRY_UPDATE, nRetryAfterInSeconds);
    }
    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::HandleGlareCondition(IN const IMessage& /* objMessage */) const
{
    return CallReasonInfo(CODE_INTERNAL_RETRY_UPDATE,
            GetGlareTimeMillisecond(m_objContext.GetCallInfo().ePeerType));
}

PRIVATE
CallReasonInfo UpdateErrorHandler::HandleBlockCallByTimer(IN const IMessage& objMessage) const
{
    IMS_SINT32 nRetryAfter = m_objContext.GetMessageUtils().GetHeaderValueInt(
            &objMessage, ISipHeader::RETRY_AFTER_ANY);
    if (IsRegisterWithNextPcscfRequiredFor503(nRetryAfter, objMessage.GetMethod()))
    {
        RegisterFor503(nRetryAfter);
        return CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, objMessage.GetStatusCode());
    }

    IMS_SINT32 nRetryAfterInMillis = nRetryAfter * 1000;
    m_objContext.GetPassiveTimerHolder().AddTimer(
            IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nRetryAfterInMillis);

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, objMessage.GetStatusCode());
}

PRIVATE
void UpdateErrorHandler::RegisterFor503(IN IMS_SINT32 nRetryAfter) const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->RegisterWithNextPcscf(nRetryAfter > 0 ? nRetryAfter : 0);
    }
}

PRIVATE
IMS_BOOL UpdateErrorHandler::IsRegisterWithNextPcscfRequiredFor503(
        IN IMS_SINT32 nRetryAfter, IN const SipMethod& objMethod) const
{
    return nRetryAfter <= 0 ||
            nRetryAfter * 1000 > Engine::GetConfiguration()
                                         ->GetSipConfig(m_objContext.GetSlotId())
                                         ->GetSipConfigV()
                                         ->GetTimerValue(objMethod.Equals(SipMethod::INVITE)
                                                         ? ISipConfigV::TIMER_B
                                                         : ISipConfigV::TIMER_F);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::GetDefaultCallReasonInfo(IN const IMessage& objMessage) const
{
    const IMS_SINT32 nStatusCode = objMessage.GetStatusCode();
    IMS_SINT32 nReasonCode = MtcConfigurationResolver::LookupReasonCodeByStatusCodeForNormal(
            m_objContext.GetConfigurationProxy(), nStatusCode);
    if (nReasonCode == CODE_NONE)
    {
        auto it = s_defaultStatusCodeAndReasonCodeMap.find(nStatusCode);
        if (it != s_defaultStatusCodeAndReasonCodeMap.end())
        {
            nReasonCode = it->second;
        }
        else
        {
            nReasonCode = CODE_SIP_SERVER_ERROR;
        }
    }
    IMS_TRACE_I("GetDefaultCallReasonInfo [%d]", nReasonCode, 0, 0);
    return CallReasonInfo(nReasonCode, GetDefaultExtraCode(objMessage));
}

PRIVATE
IMS_SINT32 UpdateErrorHandler::GetDefaultExtraCode(IN const IMessage& objMessage) const
{
    IMS_SINT32 nExtraCode = m_objContext.GetMessageUtils().GetCauseFromReasonHeader(&objMessage);
    if (nExtraCode == -1)
    {
        nExtraCode = objMessage.GetStatusCode();
    }
    return nExtraCode;
}
