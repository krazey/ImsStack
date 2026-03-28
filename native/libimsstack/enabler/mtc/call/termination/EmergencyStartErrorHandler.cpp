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

#include "IMessage.h"
#include "IMtcService.h"
#include "ISipHeader.h"
#include "ImsAosParameter.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/termination/DefaultStatusCodeAndReasonCodeSets.h"
#include "call/termination/EmergencyStartErrorHandler.h"
#include "call/termination/StartErrorHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "helper/IMtcAosConnector.h"
#include "utility/IMessageUtils.h"
#include <optional>
#include <unordered_map>

__IMS_TRACE_TAG_COM_MTC__;

// clang-format off
const std::unordered_map<IMS_SINT32, EmergencyStartErrorHandler::ActionFunc>
        EmergencyStartErrorHandler::objActionFuncMap = {
    {ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_NEXT_PCSCF_IF_EPDN,
            &EmergencyStartErrorHandler::HandleSilentReinviteWithNextPcscfIfEpdn},
    {ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_BY_RETRY_AFTER,
            &EmergencyStartErrorHandler::HandleSilentReinviteByRetryAfter},
    {ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_VOIP_BY_RTT_REJECTION,
            &EmergencyStartErrorHandler::HandleSilentReinviteAsVoipByRttRejection},
    {ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_ANONYMOUS,
            &EmergencyStartErrorHandler::HandleSilentReinviteWithAnonymousByNetworkRejection},
    {ConfigEmergency::START_ERROR_ACTION_CROSS_SIM_TEMP_FAILURE,
            &EmergencyStartErrorHandler::HandleSilentReinviteCrossSimByTempFailure},
    {ConfigEmergency::START_ERROR_ACTION_CROSS_SIM_PERM_FAILURE,
            &EmergencyStartErrorHandler::HandleSilentReinviteCrossSimByPermFailure},
    {ConfigEmergency::START_ERROR_ACTION_TERMINATE,
            &EmergencyStartErrorHandler::HandleTerminate},
    {ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF_ONCE,
            &EmergencyStartErrorHandler::HandleSilentReinviteToAlternatePcscfOnce},
    {ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF,
            &EmergencyStartErrorHandler::HandleSilentReinviteToAlternatePcscf},
};
// clang-format on

PUBLIC
EmergencyStartErrorHandler::EmergencyStartErrorHandler(
        IN IMtcCallContext& objContext, IN ISession& objSession) :
        m_objContext(objContext),
        m_objSession(objSession)
{
}

PUBLIC
EmergencyStartErrorHandler::~EmergencyStartErrorHandler() {}

PUBLIC GLOBAL std::optional<CallReasonInfo>
EmergencyStartErrorHandler::MaybeGetOverriddenCallReasonInfo(
        IN const MtcConfigurationProxy& objProxy, IN const CallReasonInfo& objReason)
{
    if (objReason.nCode == CODE_TIMEOUT_1XX_WAITING || objReason.nCode == CODE_TIMEOUT_NO_ANSWER)
    {
        ImsVector<IMS_SINT32> objActions = MtcConfigurationResolver::LookupActionForStatusCode(
                objProxy, ConfigEmergency::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
                SipStatusCode::SC_INVALID);
        if (objActions.Contains(ConfigEmergency::START_ERROR_ACTION_TERMINATE))
        {
            return CallReasonInfo(GetDefaultReasonCode(objProxy, SipStatusCode::SC_INVALID));
        }
    }

    if (objReason.nCode == CODE_USER_TERMINATED || objReason.nCode == CODE_EMERGENCY_TEMP_FAILURE ||
            objReason.nCode == CODE_EMERGENCY_PERM_FAILURE)
    {
        return std::nullopt;
    }

    if (objReason.nCode == CODE_LOCAL_CALL_CS_RETRY_REQUIRED &&
            objReason.nExtraCode == EXTRA_CODE_CALL_RETRY_EMERGENCY)
    {
        return std::nullopt;
    }

    return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
}

PUBLIC
CallReasonInfo EmergencyStartErrorHandler::Handle(IN const IMessage* piMessage) const
{
    if (StartErrorHandler::ShouldTerminateWithoutActionConfig(m_objContext))
    {
        if (StartErrorHandler::IsTransactionTimeout(piMessage))
        {
            return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE);
        }
        else
        {
            return StartErrorHandler::GetDefaultCallReasonInfo(m_objContext, *piMessage);
        }
    }

    if (StartErrorHandler::IsTransactionTimeout(piMessage))
    {
        m_objContext.SetHadInviteTransactionTimeout(IMS_TRUE);
    }

    IMS_SINT32 nStatusCode = GetStatusCode(piMessage);
    ImsVector<IMS_SINT32> objActions = MtcConfigurationResolver::LookupActionForStatusCode(
            m_objContext.GetConfigurationProxy(),
            ConfigEmergency::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY, nStatusCode);
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

    IMS_TRACE_D("Default action : CSFB for [%d]", nStatusCode, 0, 0);
    return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
}

PRIVATE GLOBAL IMS_SINT32 EmergencyStartErrorHandler::GetDefaultReasonCode(
        IN const MtcConfigurationProxy& objProxy, IN IMS_SINT32 nStatusCode)
{
    IMS_SINT32 nReasonCode = MtcConfigurationResolver::LookupReasonCodeByStatusCodeForEmergency(
            objProxy, nStatusCode);

    if (nReasonCode != CODE_NONE)
    {
        return nReasonCode;
    }

    auto it = s_defaultStatusCodeAndReasonCodeMap.find(nStatusCode);
    if (it != s_defaultStatusCodeAndReasonCodeMap.end())
    {
        nReasonCode = it->second;
    }

    return nReasonCode == CODE_NONE ? CODE_SIP_SERVER_ERROR : nReasonCode;
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteWithNextPcscfIfEpdn(
        IN [[maybe_unused]] const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleSilentReinviteWithNextPcscfIfEpdn", 0, 0, 0);
    if (!m_objContext.GetService().IsEmergency())
    {
        return CallReasonInfo(CODE_NONE);
    }

    if (m_objContext.GetMessageUtils().GetNumberOfPreviousResponses(
                &m_objSession, IMessage::SESSION_START) > 1)
    {
        return CallReasonInfo(CODE_NONE);
    }

    // support re-dial emergency with next P-CSCF,
    // an emergency call is over sos pdn,
    // and it receives an error response without receiving 100 Trying.
    ControlAos(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteByRetryAfter(
        IN const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleSilentReinviteByRetryAfter", 0, 0, 0);
    IMS_SINT32 nRetryAfterInSeconds = m_objContext.GetMessageUtils().GetHeaderValueInt(
            piMessage, ISipHeader::RETRY_AFTER_ANY);
    if (nRetryAfterInSeconds > 0)
    {
        AString strRetryAfterInMillis;
        strRetryAfterInMillis.SetNumber(nRetryAfterInSeconds * 1000);
        return CallReasonInfo(
                    CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfterInMillis);
    }
    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteAsVoipByRttRejection(
        IN [[maybe_unused]] const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleSilentReinviteAsVoipByRttRejection", 0, 0, 0);
    if (m_objContext.GetSession(&m_objSession)->GetCallType() != CallType::RTT)
    {
        return CallReasonInfo(CODE_NONE);
    }

    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteWithAnonymousByNetworkRejection(
        IN [[maybe_unused]] const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleSilentReinviteWithAnonymousByNetworkRejection", 0, 0, 0);
    const IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    const IMS_UINT32 nAosRegistrationMode =
            pAosConnector ? pAosConnector->GetRegistrationMode() : IImsAosInfo::REG_MODE_INTERNAL;
    if (nAosRegistrationMode == IImsAosInfo::REG_MODE_INTERNAL ||
            nAosRegistrationMode == IImsAosInfo::REG_MODE_NOUICC)
    {
        return CallReasonInfo(CODE_NONE);
    }

    ControlAos(ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_ANONYMOUS);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteCrossSimByTempFailure(
        IN [[maybe_unused]] const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleSilentReinviteCrossSimByTempFailure", 0, 0, 0);
    return IsCrossSimRedialAvailable() ? CallReasonInfo(CODE_EMERGENCY_TEMP_FAILURE)
                                       : CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteCrossSimByPermFailure(
        IN [[maybe_unused]] const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleSilentReinviteCrossSimByPermFailure", 0, 0, 0);
    return IsCrossSimRedialAvailable() ? CallReasonInfo(CODE_EMERGENCY_PERM_FAILURE)
                                       : CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleTerminate(IN const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleTerminate", 0, 0, 0);
    const IMS_SINT32 nStatusCode = GetStatusCode(piMessage);

    AString strRequiredReasonText =
            MtcConfigurationResolver::GetRequiredReasonTextForEmergencyTermination(
                    m_objContext.GetConfigurationProxy(), nStatusCode);
    if (strRequiredReasonText.GetLength() > 0)
    {
        const ReasonHeaderValue objValue =
                m_objContext.GetMessageUtils().GetCauseAndTextFromReasonHeader(piMessage);
        if (!objValue.strText.MakeLower().Contains(strRequiredReasonText.MakeLower()))
        {
            // text parameter in Reason header not matched.
            return CallReasonInfo(CODE_NONE);
        }
    }
    const IMS_SINT32 nReasonCode =
            GetDefaultReasonCode(m_objContext.GetConfigurationProxy(), nStatusCode);
    return CallReasonInfo(nReasonCode, GetExtraCode(nReasonCode, piMessage));
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteToAlternatePcscfOnce(
        IN [[maybe_unused]] const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleSilentReinviteToAlternatePcscfOnce", 0, 0, 0);
    return HandleSilentReinviteToAlternatePcscfInternal(EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF_ONCE);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteToAlternatePcscf(
        IN [[maybe_unused]] const IMessage* piMessage) const
{
    IMS_TRACE_D("HandleSilentReinviteToAlternatePcscf", 0, 0, 0);
    return HandleSilentReinviteToAlternatePcscfInternal(EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleSilentReinviteToAlternatePcscfInternal(
        IN IMS_SINT32 nExtraCode) const
{
    const IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (!pAosConnector)
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
    }

    pAosConnector->RegisterWithNextPcscf(0);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, nExtraCode);
}

PRIVATE
IMS_SINT32 EmergencyStartErrorHandler::GetExtraCode(
        IN IMS_SINT32 nCode, IN const IMessage* piMessage) const
{
    if (nCode == CODE_NETWORK_RESP_TIMEOUT)
    {
        return EXTRA_CODE_METHOD_INVITE;
    }

    IMS_SINT32 nExtraCode = m_objContext.GetMessageUtils().GetCauseFromReasonHeader(piMessage);
    if (nExtraCode == -1 && piMessage)
    {
        nExtraCode = GetStatusCode(piMessage);
    }
    return nExtraCode;
}

PRIVATE
void EmergencyStartErrorHandler::ControlAos(IN IMS_UINT32 nCommand) const
{
    const IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->Control(nCommand);
    }
}
