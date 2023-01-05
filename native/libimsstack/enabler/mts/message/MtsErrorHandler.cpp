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
#include "ImsAosParameter.h"
#include "IuMtsService.h"
#include "MtsDef.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "core/IMessage.h"
#include "message/IMtsErrorHandlerListener.h"
#include "message/MtsErrorHandler.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsErrorHandler::MtsErrorHandler(IN ICarrierConfig* piCarrierConfig) :
        m_piCarrierConfig(piCarrierConfig),
        m_piListener(IMS_NULL)
{
    IMS_TRACE_I("+MtsErrorHandler", 0, 0, 0);
}

PUBLIC
MtsErrorHandler::~MtsErrorHandler()
{
    IMS_TRACE_I("~MtsErrorHandler", 0, 0, 0);
}

PUBLIC
IMS_SINT32 MtsErrorHandler::Handle(IN const IMessage* piMessage)
{
    IMS_SINT32 nResult = MO_ERROR_RETRY;
    IMS_SINT32 nStatusCode =
            (piMessage != IMS_NULL) ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;
    ImsVector<IMS_SINT32> objGenericErrorCodes = m_piCarrierConfig->GetIntArray(
            CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY);

    for (IMS_UINT32 i = 0; i < objGenericErrorCodes.GetSize(); i++)
    {
        if (objGenericErrorCodes.GetAt(i) == nStatusCode)
        {
            nResult = MO_ERROR_GENERIC;
            break;
        }
    }

    IMS_SINT32 nPolicy = GetRegistrationRecoveryPolicy(piMessage);

    if (nPolicy != MTS_REG_RECOVERY_POLICY_NONE)
    {
        ControlAos(nPolicy);
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
void MtsErrorHandler::ControlAos(IN IMS_UINT32 nCommand) const
{
    m_piListener->NotifyControlAos(nCommand);
}

PRIVATE
IMS_SINT32 MtsErrorHandler::GetExpiryTimerFPolicy() const
{
    IMS_SINT32 nPolicy = m_piCarrierConfig->GetInt(
            CarrierConfig::Assets::KEY_SMS_POLICY_FOR_EXPIRY_TIMER_F_INT);

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
    IMS_SINT32 nPolicy =
            m_piCarrierConfig->GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_403_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get404ResponsePolicy() const
{
    IMS_SINT32 nPolicy =
            m_piCarrierConfig->GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_404_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get406ResponsePolicy() const
{
    IMS_SINT32 nPolicy =
            m_piCarrierConfig->GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_406_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get408ResponsePolicy() const
{
    IMS_SINT32 nPolicy =
            m_piCarrierConfig->GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_408_RESPONSE_INT);

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
    IMS_SINT32 nPolicy =
            m_piCarrierConfig->GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_500_RESPONSE_INT);

    return nPolicy;
}

PRIVATE
IMS_SINT32 MtsErrorHandler::Get503ResponsePolicy(IN const IMessage* piMessage) const
{
    IMS_SINT32 nPolicy =
            m_piCarrierConfig->GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_503_RESPONSE_INT);

    if (nPolicy == ImsAosControl::REGISTER_REINITIATE)
    {
        const AString& strPhrase = piMessage->GetReasonPhrase();
        if (strPhrase.GetLength() > 0)
        {
            IMS_TRACE_D("503 reason phrase (%s)", strPhrase.GetStr(), 0, 0);

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
    IMS_SINT32 nPolicy =
            m_piCarrierConfig->GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_504_RESPONSE_INT);

    return nPolicy;
}
