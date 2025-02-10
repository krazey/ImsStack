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
        m_objContext(objContext),
        m_pSipConfig(pSipConfig),
        m_objConfiguration(objContext.GetConfigurationProxy())
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
    m_bUpdated = MayUpdateForEpsFallbackTrigger() || MayUpdateForTcallTimerExpiry();
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::ResetInviteTransactionTimer()
{
    if (m_bUpdated)
    {
        UpdateTimer(IMS_TRUE, m_objConfiguration.GetInt(ConfigIms::KEY_SIP_TIMER_B_MILLIS_INT));
        m_bUpdated = IMS_FALSE;
    }
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::SetNonInviteTransactionTimer()
{
    UpdateTimer(IMS_FALSE,
            m_objConfiguration.GetInt(
                    ConfigVoice::KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT));
    m_bUpdated = IMS_TRUE;
}

PUBLIC VIRTUAL void TransactionTimerUpdateHelper::ResetNonInviteTransactionTimer()
{
    UpdateTimer(IMS_FALSE, m_objConfiguration.GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT));
    m_bUpdated = IMS_FALSE;
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
IMS_BOOL TransactionTimerUpdateHelper::MayUpdateForEpsFallbackTrigger()
{
    IMS_SINT32 nTimeMillis = m_objConfiguration.GetInt(
            ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT);

    if (nTimeMillis >= 0 && m_objContext.GetService().IsNr())
    {
        UpdateTimer(IMS_TRUE, nTimeMillis);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL TransactionTimerUpdateHelper::MayUpdateForTcallTimerExpiry()
{
    if (GetPolicyForTcallTimerExpiry(m_objContext.GetCallInfo().IsEmergency(),
                m_objContext.GetService().IsWlanIpCanType()) ==
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE)
    {
        return IMS_FALSE;
    }

    const IMS_CHAR* pszKey = m_objContext.GetCallInfo().IsEmergency()
            ? ConfigEmergency::KEY_EMERGENCY_TCALL_TIMER_MILLIS_INT
            : ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT;
    UpdateTimer(IMS_TRUE, m_objConfiguration.GetInt(pszKey));
    return IMS_TRUE;
}

PRIVATE
IMS_SINT32 TransactionTimerUpdateHelper::GetPolicyForTcallTimerExpiry(
        IN const IMS_BOOL bEmergency, IN const IMS_BOOL bWifi) const
{
    const IMS_CHAR* pszKey = bEmergency
            ? ConfigEmergency::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT
            : (bWifi ? ConfigWfc::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT
                     : ConfigVoice::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT);
    return m_objConfiguration.GetInt(pszKey);
}
