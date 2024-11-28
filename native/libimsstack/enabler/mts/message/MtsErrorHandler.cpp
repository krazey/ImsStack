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
#include "ICarrierConfig.h"
#include "IMessage.h"
#include "IMtsService.h"
#include "ImsAosParameter.h"
#include "IuMtsService.h"
#include "MtsDef.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "message/MtsErrorHandler.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsErrorHandler::MtsErrorHandler(IN ICarrierConfig* piCarrierConfig) :
        m_piCarrierConfig(piCarrierConfig),
        m_nCumulativeDuration(0),
        m_nCurrentRetryCount(0),
        m_nRetryAfterValue(0)
{
    IMS_TRACE_I("+MtsErrorHandler", 0, 0, 0);
}

PUBLIC
MtsErrorHandler::~MtsErrorHandler()
{
    IMS_TRACE_I("~MtsErrorHandler", 0, 0, 0);
}

PUBLIC
IMS_SINT32 MtsErrorHandler::Handle(IN IMtsService* piMtsService,
        IN MtsDynamicLoader* pMtsDynamicLoader, IN const IMessage* piMessage)
{
    IMS_SINT32 nResult;
    ImsVector<IMS_SINT32> objGenericErrorCodes = m_piCarrierConfig->GetIntArray(
            CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY);
    if (piMessage != IMS_NULL)
    {
        if (objGenericErrorCodes.Contains(piMessage->GetStatusCode()))
        {
            nResult = MO_ERROR_GENERIC;
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
                CarrierConfig::ImsSms::KEY_SMS_RETRY_POLICY_FOR_EXPIRY_TIMER_F_INT);
    }

    IMS_SINT32 nPolicy = GetRegistrationRecoveryPolicy(piMessage);
    if (nPolicy != MTS_REG_RECOVERY_POLICY_NONE)
    {
        piMtsService->RequestRegistrationRecovery(nPolicy);
    }

    if (nResult != MO_ERROR_GENERIC)
    {
        CalculateRetryAfterCondition(
                pMtsDynamicLoader->GetMtsSipFormUtils()->GetRetryAfterValue(piMessage));
        if (IsRetryPossible())
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

    return MTS_REG_RECOVERY_POLICY_NONE;
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
    IMS_SINT32 nPolicy = MTS_REG_RECOVERY_POLICY_NONE;
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
    IMS_SINT32 nPolicy = MTS_REG_RECOVERY_POLICY_NONE;
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

    if (nPolicy == ImsAosControl::REGISTER_REINITIATE)
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
        nPolicy = MTS_REG_RECOVERY_POLICY_NONE;
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
void MtsErrorHandler::CalculateRetryAfterCondition(IN const IMS_SINT32 nRetryAfterValue)
{
    m_nRetryAfterValue = nRetryAfterValue;
    m_nCurrentRetryCount++;
    m_nCumulativeDuration += m_nRetryAfterValue;

    IMS_TRACE_I("CalculateRetryAfterCondition : RetryAfterValue[%d], CurrentRetryCount[%d], "
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
        return IMS_FALSE;
    }

    IMS_SINT32 nMaxRetryDuration =
            m_piCarrierConfig->GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT);
    if (m_nCumulativeDuration >= nMaxRetryDuration)
    {
        // imssms.sms_retry_after_max_time_sec_int has reached
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
