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

#include "AString.h"
#include "CarrierConfig.h"
#include "ICarrierConfig.h"
#include "IConfigurable.h"
#include "ISipConfig.h"
#include "ISipConfigV.h"
#include "ImsTypeDef.h"
#include "ServiceConfig.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/TransactionTimerUpdateHelper.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
TransactionTimerUpdateHelper::TransactionTimerUpdateHelper(
        IN IMtcCallContext& objContext, IN const ISipConfig* pSipConfig) :
        m_pSipConfig(pSipConfig),
        m_nSlotId(objContext.GetSlotId()),
        m_objConfiguration(objContext.GetConfigurationProxy()),
        m_bEmergency(objContext.GetCallInfo().IsEmergency()),
        m_bWifi(objContext.GetService().IsWlanIpCanType())
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
    if (IsNeedToUpdate() == IMS_FALSE)
    {
        return;
    }

    Feature eFeature =
            m_bEmergency ? Feature::EMERGENCY_T_CALL_TIMER : Feature::MO_CALL_REQUEST_TIMEOUT;
    UpdateTimer(IMS_TRUE, m_objConfiguration.GetInt(eFeature));
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::ResetInviteTransactionTimer()
{
    if (IsNeedToUpdate() == IMS_FALSE)
    {
        return;
    }

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    UpdateTimer(IMS_TRUE, piCc->GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT));
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::SetNonInviteTransactionTimer()
{
    UpdateTimer(IMS_FALSE, m_objConfiguration.GetInt(Feature::PRACK_UPDATE_RESPONSE_WAIT_TIMER));
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::ResetNonInviteTransactionTimer()
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    UpdateTimer(IMS_FALSE, piCc->GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT));
}

PRIVATE
void TransactionTimerUpdateHelper::UpdateTimer(IN IMS_BOOL bInviteTransaction, IN IMS_SINT32 nValue)
{
    if (nValue <= 0)
    {
        return;
    }

    if (m_pSipConfig == IMS_NULL)
    {
        return;
    }

    const ISipConfigV* piSipConfigV = m_pSipConfig->GetSipConfigV();
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
    IMS_SINT32 nTimer =
            bInviteTransaction ? IConfigurable::CP_I_TIMER_B : IConfigurable::CP_I_TIMER_F;
    AString strValue;
    strValue.Sprintf("%d", nValue);
    if (!piConfigurable->Update(nTimer, strValue))
    {
        IMS_TRACE_E(0, "Update FAIL", 0, 0, 0);
    }
}

PRIVATE
IMS_BOOL TransactionTimerUpdateHelper::IsNeedToUpdate() const
{
    Feature eFeature = m_bEmergency
            ? Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL
            : (m_bWifi ? Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL
                       : Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL);
    return m_objConfiguration.GetInt(eFeature) !=
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE;
}
