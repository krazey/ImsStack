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

#include "AString.h"
#include "CarrierConfig.h"
#include "IConfiguration.h"
#include "ICarrierConfig.h"
#include "IConfigurable.h"
#include "ISipConfig.h"
#include "ISipConfigV.h"
#include "ImsTypeDef.h"
#include "MtsDef.h"
#include "ServiceConfig.h"
#include "ServiceTrace.h"
#include "helper/MtsTransactionTimerUpdateHelper.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsTransactionTimerUpdateHelper::MtsTransactionTimerUpdateHelper(
        IN const ICarrierConfig* piCarrierConfig, IN const ISipConfig* piSipConfig) :
        m_piCarrierConfig(piCarrierConfig),
        m_piSipConfig(piSipConfig)
{
    IMS_TRACE_I("+MtsTransactionTimerUpdateHelper", 0, 0, 0);
}

PUBLIC
MtsTransactionTimerUpdateHelper::~MtsTransactionTimerUpdateHelper()
{
    IMS_TRACE_I("~MtsTransactionTimerUpdateHelper", 0, 0, 0);
}

PUBLIC
void MtsTransactionTimerUpdateHelper::SetMessageTransactionTimer(IN const IMS_SINT32 nMti) const
{
    if (IsNeedToUpdate(nMti) == IMS_FALSE)
    {
        return;
    }

    UpdateTimer(m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT));
}

PUBLIC
void MtsTransactionTimerUpdateHelper::ResetMessageTransactionTimer(IN const IMS_SINT32 nMti) const
{
    if (IsNeedToUpdate(nMti) == IMS_FALSE)
    {
        return;
    }

    UpdateTimer(m_piCarrierConfig->GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT));
}

PRIVATE
void MtsTransactionTimerUpdateHelper::UpdateTimer(IN const IMS_SINT32 nValue) const
{
    if (nValue <= 0)
    {
        return;
    }

    if (m_piSipConfig == IMS_NULL)
    {
        return;
    }

    const ISipConfigV* piSipConfigV = m_piSipConfig->GetSipConfigV();
    if (piSipConfigV == IMS_NULL)
    {
        return;
    }

    IConfigurable* piConfigurable = piSipConfigV->GetConfigurable();
    if (piConfigurable == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("UpdateTimer value[%d]", nValue, 0, 0);
    AString strValue;
    strValue.Sprintf("%d", nValue);
    if (!piConfigurable->Update(IConfigurable::CP_I_TIMER_F, strValue))
    {
        IMS_TRACE_E(0, "Update FAIL", 0, 0, 0);
    }
}

PRIVATE
IMS_BOOL MtsTransactionTimerUpdateHelper::IsNeedToUpdate(IN IMS_SINT32 nMti) const
{
    IMS_SINT32 nMilliSeconds = m_piCarrierConfig->GetInt(
            CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT);

    return (nMilliSeconds > 0) && (nMti == SMS_3GPP_MTI_RP_DATA_FROM_MS);
}
