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
#include "ImsAosParameter.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/termination/EmergencyStartErrorHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "helper/IMtcAosConnector.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

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
        IMS_SINT32 nCallReasonInfoCode =
                MtcConfigurationResolver::LookupTerminateReasonCodeForEmergency(objProxy, 0);
        if (nCallReasonInfoCode != CODE_NONE)
        {
            return CallReasonInfo(nCallReasonInfoCode, -1);
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
    if (IsRedialWithVoipByRttEmergencyRejectionRequired())
    {
        return HandleRedialWithVoipByRttEmergencyRejection();
    }

    IMS_SINT32 nStatusCode =
            (piMessage != IMS_NULL) ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;

    IMS_SINT32 nCallReasonInfoCode = GetCrossSimRedialingReasonCode(nStatusCode);
    if (nCallReasonInfoCode != CODE_NONE)
    {
        return CallReasonInfo(nCallReasonInfoCode);
    }

    nCallReasonInfoCode = MtcConfigurationResolver::LookupTerminateReasonCodeForEmergency(
            m_objContext.GetConfigurationProxy(), nStatusCode);
    if (nCallReasonInfoCode != CODE_NONE)
    {
        return CallReasonInfo(nCallReasonInfoCode, GetExtraCode(nCallReasonInfoCode, piMessage));
    }

    if (IsRedialEmergencyWithNextPcscfRequired(piMessage))
    {
        return HandleRedialEmergencyWithNextPcscf();
    }

    return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
}

PRIVATE
IMS_SINT32 EmergencyStartErrorHandler::GetCrossSimRedialingReasonCode(
        IN IMS_SINT32 nStatusCode) const
{
    IMS_SINT32 nCallReasonInfoCode = CODE_NONE;
    // Since inclusion in both is not an expected behavior, priority is arbitrarily given to
    // CODE_EMERGENCY_TEMP_FAILURE.
    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigEmergency::KEY_REJECT_CODE_REQUIRE_TEMP_FAILURE_INT_ARRAY, nStatusCode))
    {
        nCallReasonInfoCode = CODE_EMERGENCY_TEMP_FAILURE;
    }
    else if (m_objContext.GetConfigurationProxy().Contains(
                     ConfigEmergency::KEY_REJECT_CODE_REQUIRE_PERM_FAILURE_INT_ARRAY, nStatusCode))
    {
        nCallReasonInfoCode = CODE_EMERGENCY_PERM_FAILURE;
    }

    if (nCallReasonInfoCode != CODE_NONE &&
            PhoneInfoService::GetPhoneInfoService()
                    ->GetCallInfo(m_objContext.GetSlotId())
                    ->IsCrossSimRedialingAvailable())
    {
        return nCallReasonInfoCode;
    }

    return CODE_NONE;
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
        nExtraCode = piMessage->GetStatusCode();
    }
    return nExtraCode;
}

PRIVATE
IMS_BOOL EmergencyStartErrorHandler::IsRedialEmergencyWithNextPcscfRequired(
        IN const IMessage* piMessage) const
{
    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigEmergency::KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL))
    {
        return IMS_FALSE;
    }

    if (!m_objContext.GetService().IsEmergency())
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nStatusCode =
            (piMessage == IMS_NULL) ? SipStatusCode::SC_INVALID : piMessage->GetStatusCode();
    if (nStatusCode >= SipStatusCode::SC_300 && nStatusCode < SipStatusCode::SC_400)
    {
        return IMS_FALSE;
    }

    if (m_objContext.GetMessageUtils().GetNumberOfPreviousResponses(
                &m_objSession, IMessage::SESSION_START) > 1)
    {
        return IMS_FALSE;
    }

    // support re-dial emergency with next P-CSCF,
    // an emergency call is over sos pdn,
    // and it receives error response except 3xx before receiving 100 Trying.
    return IMS_TRUE;
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleRedialEmergencyWithNextPcscf() const
{
    ControlAos(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
}

PRIVATE
void EmergencyStartErrorHandler::ControlAos(IN IMS_UINT32 nCommand) const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->Control(nCommand);
    }
}

PRIVATE
IMS_BOOL EmergencyStartErrorHandler::IsRedialWithVoipByRttEmergencyRejectionRequired() const
{
    if (m_objContext.GetSession(&m_objSession)->GetCallType() != CallType::RTT)
    {
        return IMS_FALSE;
    }

    return m_objContext.GetConfigurationProxy().GetBoolean(
            CarrierConfig::ImsEmergency::KEY_SILENT_REDIAL_WITH_VOIP_BY_RTT_REJECTION_BOOL);
}

PRIVATE
CallReasonInfo EmergencyStartErrorHandler::HandleRedialWithVoipByRttEmergencyRejection() const
{
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION);
}
