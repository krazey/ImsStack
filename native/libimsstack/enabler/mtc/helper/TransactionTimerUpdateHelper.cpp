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

#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "AString.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/TransactionTimerUpdateHelper.h"
#include "common/ISipConfigV.h"
#include "IConfigurable.h"
#include "Configuration.h"
#include "ServiceConfig.h"
#include "ICarrierConfig.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
TransactionTimerUpdateHelper::TransactionTimerUpdateHelper(
        IN IMS_SINT32 nSlotId, IN MtcConfigurationProxy& objConfiguration) :
        m_nSlotId(nSlotId),
        m_objConfiguration(objConfiguration)
{
    IMS_TRACE_I("+TransactionTimerUpdateHelper", 0, 0, 0);
}

PUBLIC
TransactionTimerUpdateHelper::~TransactionTimerUpdateHelper()
{
    IMS_TRACE_I("~TransactionTimerUpdateHelper", 0, 0, 0);
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::SetInviteTransactionTimer()
{
    IMS_SINT32 nValue = m_objConfiguration.GetInt(Feature::MO_CALL_REQUEST_TIMEOUT);
    UpdateTimer(IMS_TRUE, nValue);
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::ResetInviteTransactionTimer()
{
    // TODO: read from mtc config...
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    IMS_SINT32 nValue = piCc->GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT);
    UpdateTimer(IMS_TRUE, nValue);
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::SetNonInviteTransactionTimer()
{
    IMS_SINT32 nValue = m_objConfiguration.GetInt(Feature::PRACK_UPDATE_RESPONSE_WAIT_TIMER);
    UpdateTimer(IMS_FALSE, nValue);
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::ResetNonInviteTransactionTimer()
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    IMS_SINT32 nValue = piCc->GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT);
    UpdateTimer(IMS_FALSE, nValue);
}

PRIVATE
void TransactionTimerUpdateHelper::UpdateTimer(IN IMS_BOOL bInviteTransaction, IN IMS_SINT32 nValue)
{
    if (nValue <= 0)
    {
        return;
    }

    const ISipConfigV* piSipConfigV =
            Configuration::GetInstance()->GetSipConfig(m_nSlotId)->GetSipConfigV();

    if (piSipConfigV == IMS_NULL)
    {
        return;
    }

    IConfigurable* piConfigurable = piSipConfigV->GetConfigurable();
    if (piConfigurable == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("UpdateTimer INVITE[%s] value[%d]", _TRACE_B_(bInviteTransaction), nValue, 0);
    IMS_SINT32 nTimer = bInviteTransaction ? IConfigurable::CP_I_TV_TB : IConfigurable::CP_I_TV_TF;
    AString strValue;
    strValue.Sprintf("%d", nValue);
    if (!piConfigurable->Update(nTimer, strValue))
    {
        IMS_TRACE_E(0, "Update FAIL", 0, 0, 0);
    }
}
