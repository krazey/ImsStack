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
#include "ImsAosParameter.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/MtcCallManager.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "call/termination/StartErrorHandler.h"
#include "call/termination/UpdateErrorHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IPassiveTimerHolder.h"
#include "utility/IMessageUtils.h"
#include <unordered_map>

__IMS_TRACE_TAG_COM_MTC__;

// clang-format off
const std::unordered_map<IMS_SINT32, EarlyUpdateErrorHandler::ActionFunc>
        EarlyUpdateErrorHandler::objActionFuncMap = {
    {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_TERMINATE_DIALOG,
            &EarlyUpdateErrorHandler::HandleTerminateDialog},
    {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_TERMINATE_CALL,
            &EarlyUpdateErrorHandler::HandleTerminateCall},
    {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_GLARE_CONDITION,
            &EarlyUpdateErrorHandler::HandleGlareCondition},
    {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER,
            &EarlyUpdateErrorHandler::HandleBlockCallByTimer},
    {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_TIMEOUT,
            &EarlyUpdateErrorHandler::HandleTimeout},
    {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_REGISTRATION_RESTORATION,
            &EarlyUpdateErrorHandler::HandleRegistrationRestoration},
};
// clang-format on

PUBLIC
EarlyUpdateErrorHandler::EarlyUpdateErrorHandler(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_eStatusCode(SipStatusCode::SC_INVALID)
{
}

PUBLIC
EarlyUpdateErrorHandler::~EarlyUpdateErrorHandler() {}

PUBLIC
CallReasonInfo EarlyUpdateErrorHandler::Handle(IN const IMessage* piMessage)
{
    m_eStatusCode = piMessage ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;

    IMS_TRACE_I("Handle : StatusCode[%d]", m_eStatusCode, 0, 0);
    ImsVector<IMS_SINT32> objActions = MtcConfigurationResolver::LookupActionForStatusCode(
            m_objContext.GetConfigurationProxy(),
            ConfigVoice::KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY, m_eStatusCode);
    for (IMS_UINT32 i = 0; i < objActions.GetSize(); ++i)
    {
        auto it = objActionFuncMap.find(objActions.GetAt(i));
        if (it != objActionFuncMap.end())
        {
            CallReasonInfo objResult = (this->*(it->second))(piMessage);
            if (objResult.nCode != CODE_NONE)
            {
                return objResult;
            }
        }
    }

    return HandleTerminateCall(piMessage);
}

PRIVATE
CallReasonInfo EarlyUpdateErrorHandler::HandleTerminateDialog(
        [[maybe_unused]] IN const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleTerminateDialog", 0, 0, 0);
    return CallReasonInfo(CODE_INTERNAL_TERMINATE_EARLYDIALOG, m_eStatusCode);
}

PRIVATE
CallReasonInfo EarlyUpdateErrorHandler::HandleTerminateCall(
        [[maybe_unused]] IN const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleTerminateCall", 0, 0, 0);
    return CallReasonInfo(CODE_REJECT_INTERNAL_ERROR, m_eStatusCode);
}

PRIVATE
CallReasonInfo EarlyUpdateErrorHandler::HandleGlareCondition(
        [[maybe_unused]] IN const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleGlareCondition", 0, 0, 0);
    return CallReasonInfo(CODE_SIP_REQUEST_PENDING,
            UpdateErrorHandler::GetGlareTimeMillisecond(m_objContext.GetCallInfo().ePeerType));
}

PRIVATE
CallReasonInfo EarlyUpdateErrorHandler::HandleBlockCallByTimer(IN const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleBlockCallByTimer", 0, 0, 0);
    if (!m_objContext.GetCallManager().GetCallsByState(IMtcCall::State::ESTABLISHED).IsEmpty())
    {
        return CallReasonInfo(CODE_REJECT_INTERNAL_ERROR, m_eStatusCode);
    }

    IMS_SINT32 nRetryAfter = m_objContext.GetMessageUtils().GetHeaderValueInt(
            piMessage, ISipHeader::RETRY_AFTER_ANY);
    IMS_SINT32 nRetryAfterInMillis = nRetryAfter * 1000;
    if (IsRegisterWithNextPcscfAndRedialRequiredFor503(nRetryAfter))
    {
        if (RegisterFor503(nRetryAfter))
        {
            return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF);
        }

        return CallReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    }

    if (m_objContext.IsCsfbAvailable())
    {
        SetTimerForImsCallBlocking(nRetryAfterInMillis);
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    AString strRetryAfter;
    strRetryAfter.SetNumber(nRetryAfterInMillis);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfter);
}

PRIVATE
CallReasonInfo EarlyUpdateErrorHandler::HandleTimeout(
        [[maybe_unused]] IN const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleTimeout", 0, 0, 0);
    return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE);
}

PRIVATE
CallReasonInfo EarlyUpdateErrorHandler::HandleRegistrationRestoration(
        [[maybe_unused]] IN const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleRegistrationRestoration", 0, 0, 0);

    ControlAos(ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY);

    return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE);
}

PRIVATE
void EarlyUpdateErrorHandler::ControlAos(IN IMS_UINT32 nCommand) const
{
    const IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->Control(nCommand);
    }
}

PRIVATE
IMS_BOOL EarlyUpdateErrorHandler::RegisterFor503(IN IMS_SINT32 nRetryAfter) const
{
    const IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->RegisterWithNextPcscf(nRetryAfter > 0 ? nRetryAfter : 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL EarlyUpdateErrorHandler::IsRegisterWithNextPcscfAndRedialRequiredFor503(
        IN IMS_SINT32 nRetryAfter) const
{
    return nRetryAfter <= 0 ||
            nRetryAfter * 1000 > Engine::GetConfiguration()
                                         ->GetSipConfig(m_objContext.GetSlotId())
                                         ->GetSipConfigV()
                                         ->GetTimerValue(ISipConfigV::TIMER_F);
}

PRIVATE
void EarlyUpdateErrorHandler::SetTimerForImsCallBlocking(IN IMS_SINT32 nRetryAfterInMillis) const
{
    if (nRetryAfterInMillis > 0)
    {
        m_objContext.GetPassiveTimerHolder().AddTimer(
                IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nRetryAfterInMillis);
    }
}
