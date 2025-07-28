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
#include "Engine.h"
#include "ICarrierConfig.h"
#include "IConfiguration.h"
#include "IMessage.h"
#include "IMtsContext.h"
#include "IMtsNetworkTracker.h"
#include "IMtsService.h"
#include "INetworkWatcher.h"
#include "ISipConfigV.h"
#include "IPageMessage.h"
#include "ImsEventDef.h"
#include "IuMtsApp.h"
#include "MtsDef.h"
#include "MtsStringDef.h"
#include "ServiceConfig.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "message/IMtsMessage.h"
#include "message/MtsErrorHandler.h"
#include "utility/IMtsDynamicLoader.h"
#include "utility/MtsSipFormUtils.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsErrorHandler::MtsErrorHandler(IN IMtsContext& objContext) :
        m_objContext(objContext),
        m_nCumulativeDuration(0),
        m_nCurrentRetryCount(0),
        m_nRetryAfterValue(0),
        m_nSlotId(m_objContext.GetSlotId()),
        m_piCarrierConfig(ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId))
{
    IMS_TRACE_I("+MtsErrorHandler", 0, 0, 0);
}

PUBLIC
MtsErrorHandler::~MtsErrorHandler()
{
    IMS_TRACE_I("~MtsErrorHandler", 0, 0, 0);
}

PUBLIC
IMS_SINT32 MtsErrorHandler::Handle(
        IN const IMtsService& objMtsService, IN IMtsMessage* piMtsMessage)
{
    IMS_SINT32 nResult = MO_INVALID;
    ImsVector<IMS_SINT32> objGenericErrorCodes = m_piCarrierConfig->GetIntArray(
            CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY);
    ImsVector<IMS_SINT32> objFallbackErrorCodes = m_piCarrierConfig->GetIntArray(
            CarrierConfig::ImsSms::KEY_SMS_FALLBACK_ERROR_CODES_INT_ARRAY);
    ImsVector<IMS_SINT32> objSmmaGenericErrorCodes = m_piCarrierConfig->GetIntArray(
            CarrierConfig::ImsSms::KEY_SMS_SMMA_GENERIC_ERROR_CODES_INT_ARRAY);

    // VZW Requirements - VZ_REQ_5GNRSA SMS_4105999311952943 and VZ_REQ_LTESMS_29553
    if (NeedToCheckRadioStatusForRetry(piMtsMessage->GetRetryCount()))
    {
        IMS_SINT32 nNetworkType = m_objContext.GetNetworkTracker().GetNetworkType();
        IMS_TRACE_I(
                "Handle : Check current network status[%s]", PS_RadioTechType(nNetworkType), 0, 0);

        if (nNetworkType == INetworkWatcher::RADIOTECH_TYPE_NR)
        {
            IMS_BOOL bDataRoaming = m_objContext.GetNetworkTracker().IsInRoamingState();

            return bDataRoaming ? MO_ERROR_FALLBACK : MO_ERROR_GENERIC;
        }
        else if (nNetworkType == INetworkWatcher::RADIOTECH_TYPE_LTE ||
                nNetworkType == INetworkWatcher::RADIOTECH_TYPE_LTE_CA)
        {
            IMS_UINT32 nLteAttachState = m_objContext.GetNetworkTracker().GetLteAttachState();

            return (nLteAttachState == IMS_LTE_INFO_COMBINED_ATTACHED) ? MO_ERROR_FALLBACK
                                                                       : MO_ERROR_GENERIC;
        }
    }

    IMessage* piMessage =
            piMtsMessage->GetPageMessage()->GetPreviousResponse(IMessage::PAGEMESSAGE_SEND);
    // ORG Requirements - IMS_SMSIP_40, IMS_SMSIP_43, IMS_SMSIP_540, IMS_SMSIP_543
    nResult = EvaluateNetworkStatusForErrorCode(objMtsService, piMessage);
    if (nResult != MO_INVALID)
    {
        return nResult;
    }

    if (piMessage != IMS_NULL)
    {
        // A status code should only be present in either 'imssms.sms_generic_error_codes_int_array'
        // or 'imssms.sms_fallback_error_codes_int_array', but not both.
        IMS_SINT32 nMti = piMtsMessage->GetMti();
        if (nMti == SMS_3GPP_MTI_RP_SMMA)
        {
            if (objSmmaGenericErrorCodes.Contains(piMessage->GetStatusCode()))
            {
                nResult = MO_ERROR_GENERIC;
            }
            else
            {
                nResult = MO_ERROR_RETRY;
            }
        }
        else if (objGenericErrorCodes.Contains(piMessage->GetStatusCode()))
        {
            nResult = MO_ERROR_GENERIC;
        }
        else if (objFallbackErrorCodes.Contains(piMessage->GetStatusCode()))
        {
            nResult = MO_ERROR_FALLBACK;
        }
        else
        {
            nResult = MO_ERROR_RETRY;
        }
    }
    else if (objGenericErrorCodes.Contains(SipStatusCode::SC_INVALID))
    {
        nResult = MO_ERROR_GENERIC;
    }
    else
    {
        nResult = m_piCarrierConfig->GetInt(
                CarrierConfig::ImsSms::KEY_SMS_RETRY_POLICY_FOR_EXPIRY_TIMER_F_INT, MO_ERROR_RETRY);
    }

    IMS_SINT32 nPolicy = GetRegistrationRecoveryPolicy(piMessage);
    if (nPolicy != MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE)
    {
        objMtsService.RequestRegistrationRecovery(nPolicy);
    }

    if (nResult != MO_ERROR_GENERIC && nResult != MO_ERROR_FALLBACK)
    {
        SetRetryAfterStatus(
                m_objContext.GetDynamicLoader().GetMtsSipFormUtils()->GetRetryAfterValue(
                        piMessage));
        if (IsRegisterWithNextPcscfRequired(piMessage) &&
                nPolicy == MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE)
        {
            objMtsService.RequestRegisterWithNextPcscf(m_nRetryAfterValue);
            nResult = MO_ERROR_GENERIC;
        }
        else if (IsRetryPossible())
        {
            nResult = MO_ERROR_BY_RETRY_AFTER;
        }
        else
        {
            if (m_piCarrierConfig->GetBoolean(CarrierConfig::ImsSms::
                                KEY_SMS_REPORT_GENERIC_ERROR_WHEN_RETRY_AFTER_NOT_POSSIBLE_BOOL))
            {
                nResult = MO_ERROR_GENERIC;
            }
        }
    }

    IMS_TRACE_I("Handle : nResult[%d], nPolicy[%d]", nResult, nPolicy, 0);

    return nResult;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::GetRegistrationRecoveryPolicy(IN const IMessage* piMessage) const
{
    IMS_SINT32 nStatusCode =
            (piMessage != IMS_NULL) ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;

    if (nStatusCode == SipStatusCode::SC_INVALID)
    {
        return GetExpiryTimerFPolicy();
    }
    else if (SipStatusCode::SC_400 <= nStatusCode && nStatusCode < SipStatusCode::SC_500)
    {
        return Get4xxResponsePolicy(piMessage);
    }
    else if (SipStatusCode::SC_500 <= nStatusCode && nStatusCode < SipStatusCode::SC_600)
    {
        return Get5xxResponsePolicy(piMessage);
    }

    return MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::GetExpiryTimerFPolicy() const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_EXPIRY_TIMER_F_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get4xxResponsePolicy(IN const IMessage* piMessage) const
{
    IMS_SINT32 nPolicy = MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE;
    IMS_SINT32 nStatusCode = piMessage->GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_403:
            nPolicy = Get403ResponsePolicy();
            break;
        case SipStatusCode::SC_404:
            nPolicy = Get404ResponsePolicy();
            break;
        case SipStatusCode::SC_406:
            nPolicy = Get406ResponsePolicy();
            break;
        case SipStatusCode::SC_408:
            nPolicy = Get408ResponsePolicy();
            break;
    }

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get403ResponsePolicy() const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_403_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get404ResponsePolicy() const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_404_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get406ResponsePolicy() const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_406_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get408ResponsePolicy() const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_408_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get5xxResponsePolicy(IN const IMessage* piMessage) const
{
    IMS_SINT32 nPolicy = MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE;
    IMS_SINT32 nStatusCode = piMessage->GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_500:
            nPolicy = Get500ResponsePolicy();
            break;
        case SipStatusCode::SC_503:
            nPolicy = Get503ResponsePolicy(piMessage);
            break;
        case SipStatusCode::SC_504:
            nPolicy = Get504ResponsePolicy();
            break;
    }

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get500ResponsePolicy() const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_500_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get503ResponsePolicy(IN const IMessage* piMessage) const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_503_RESPONSE_INT);

    if (nPolicy == MtsRegRecoveryPolicy::REGISTER_REINITIATE)
    {
        const AString& strPhrase = piMessage->GetReasonPhrase();
        if (strPhrase.GetLength() > 0)
        {
            IMS_TRACE_D("Get503ResponsePolicy : reason phrase [%s]", strPhrase.GetStr(), 0, 0);

            if (strPhrase.MakeUpper().Contains("OUTAGE"))
            {
                // TODO(Mts): Check IMS is connected or not, then report MO_ERROR_GENERIC
                return nPolicy;
            }
        }
        // In case of no reason header and no "OUTAGE"
        nPolicy = MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE;
    }

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get504ResponsePolicy() const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_504_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
void MtsErrorHandler::SetRetryAfterStatus(IN const IMS_SINT32 nRetryAfterValue)
{
    if (nRetryAfterValue <= 0)
    {
        // The error response does not have Retry-After header
        ResetRetryAfterStatus();
        return;
    }

    m_nRetryAfterValue = nRetryAfterValue;
    m_nCurrentRetryCount++;
    m_nCumulativeDuration += m_nRetryAfterValue;

    IMS_TRACE_I("SetRetryAfterStatus : RetryAfterValue[%d], CurrentRetryCount[%d], "
                "CumulativeDuration[%d]",
            m_nRetryAfterValue, m_nCurrentRetryCount, m_nCumulativeDuration);
}

PRIVATE
IMS_BOOL MtsErrorHandler::IsRetryPossible() const
{
    if (m_nRetryAfterValue <= 0)
    {
        // The error response does not have Retry-After header
        return IMS_FALSE;
    }

    IMS_SINT32 nMaxRetryCount =
            m_piCarrierConfig->GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT);
    if ((nMaxRetryCount > 0) && (m_nCurrentRetryCount >= nMaxRetryCount))
    {
        // imssms.sms_retry_after_max_count_int has reached
        IMS_TRACE_I("IsRetryPossible : Reached the max retry counter", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nMaxRetryDuration =
            m_piCarrierConfig->GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT);
    if (m_nCumulativeDuration >= nMaxRetryDuration)
    {
        // imssms.sms_retry_after_max_time_sec_int has reached
        IMS_TRACE_I("IsRetryPossible : Reached the max retry duration", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL MtsErrorHandler::IsRegisterWithNextPcscfRequired(IN const IMessage* piMessage) const
{
    IMS_SINT32 nStatusCode =
            (piMessage != IMS_NULL) ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;

    if (nStatusCode != SipStatusCode::SC_503 || m_nRetryAfterValue <= 0)
    {
        return IMS_FALSE;
    }

    return m_nRetryAfterValue * 1000 >
            Engine::GetConfiguration()->GetSipConfig(m_nSlotId)->GetSipConfigV()->GetTimerValue(
                    ISipConfigV::TIMER_F);
}

PRIVATE
IMS_BOOL MtsErrorHandler::NeedToCheckRadioStatusForRetry(IN IMS_UINT32 nRetryCount) const
{
    // TODO: To be more generic, introduce an int config for customizable retry attempts.
    return m_piCarrierConfig->GetBoolean(
                   CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_3RD_ATTEMPT_BOOL,
                   IMS_FALSE) &&
            (nRetryCount >= 1);
}

PRIVATE
IMS_SINT32 MtsErrorHandler::EvaluateNetworkStatusForErrorCode(
        IN const IMtsService& objMtsService, IN const IMessage* piMessage) const
{
    ImsVector<IMS_SINT32> objEvaluateNetworkErrorCodes = m_piCarrierConfig->GetIntArray(
            CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_ERROR_CODES_INT_ARRAY);

    if (piMessage == IMS_NULL || !objEvaluateNetworkErrorCodes.Contains(piMessage->GetStatusCode()))
    {
        return MO_INVALID;
    }

    if (!objMtsService.IsWlan())
    {
        return MO_ERROR_FALLBACK;
    }

    IMS_BOOL bDataRoaming = m_objContext.GetNetworkTracker().IsInRoamingState();
    IMS_SINT32 nCellularState = m_objContext.GetNetworkTracker().GetCellularServiceState();
    if (bDataRoaming || (nCellularState != INetworkWatcher::STATE_IN_SERVICE))
    {
        return MO_ERROR_GENERIC;
    }

    return MO_ERROR_FALLBACK;
}
