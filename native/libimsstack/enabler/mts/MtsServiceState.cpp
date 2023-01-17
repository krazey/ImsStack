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
#include "ServiceConfig.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "ImsAos.h"
#include "MtsStringDef.h"
#include "MtsServiceState.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsServiceState::MtsServiceState(IN IMS_SINT32 nSlotId) :
        m_piImsAos(IMS_NULL),
        m_nState(STATE_INIT),
        m_bImsConnected(IMS_FALSE),
        m_bAosRegModAdmin(IMS_FALSE),
        m_bImsSuspend(IMS_FALSE),
        m_bSmsOverIpConf(IMS_FALSE),
        m_bAllowImsiBasedSipUri(IMS_FALSE),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_I("+MtsServiceState [slot_%d]", m_nSlotId, 0, 0);
}

PUBLIC
MtsServiceState::~MtsServiceState()
{
    IMS_TRACE_I("~MtsServiceState [slot_%d]", m_nSlotId, 0, 0);
}

PUBLIC
void MtsServiceState::Init(IN IImsAos* piImsAos)
{
    IMS_TRACE_I("Init", 0, 0, 0);

    m_piImsAos = piImsAos;
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    IMS_BOOL bSmsOverIpNetwork =
            piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL);
    m_bAllowImsiBasedSipUri =
            piCc->GetBoolean(CarrierConfig::Assets::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL);

    SetSmsOverIpState(bSmsOverIpNetwork);

    IMS_TRACE_I("GetSmOverIpConfigInfo : SmsOverIpNetwork[%s], AllowImsiBasedSipUri[%s]",
            _TRACE_B_(bSmsOverIpNetwork), _TRACE_B_(m_bAllowImsiBasedSipUri), 0);
}

PUBLIC
IMS_SINT32 MtsServiceState::GetState() const
{
    IMS_TRACE_I("GetState : State(%s)", PS_ServiceState(m_nState), 0, 0);

    return m_nState;
}

PUBLIC
void MtsServiceState::OnImsConnected()
{
    IMS_TRACE_I("OnImsConnected", 0, 0, 0);

    SetImsRegConnected(IMS_TRUE);
}

PUBLIC
void MtsServiceState::OnImsDisconnected(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnImsDisconnected : Reason is (%d)", nReason, 0, 0);

    SetImsSuspendState(IMS_FALSE);
    SetImsRegConnected(IMS_FALSE);
}

PUBLIC
void MtsServiceState::OnImsDisconnecting(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnImsDisconnecting : Reason is (%d)", nReason, 0, 0);
}

PUBLIC
void MtsServiceState::OnImsSuspended(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnImsSuspended : Reason is (%d)", nReason, 0, 0);
    SetImsSuspendState(IMS_TRUE);
}

PUBLIC
void MtsServiceState::OnImsResumed()
{
    IMS_TRACE_I("OnImsResumed", 0, 0, 0);

    SetImsSuspendState(IMS_FALSE);
}

PUBLIC
void MtsServiceState::SetImsRegConnected(IN IMS_BOOL bConnected)
{
    IMS_TRACE_I("SetImsRegConnected : m_bImsConnected[%s]/bConnected[%s]",
            _TRACE_B_(m_bImsConnected), _TRACE_B_(bConnected), 0);

    if (m_bImsConnected == bConnected)
    {
        return;
    }

    m_bImsConnected = bConnected;

    if (m_bImsConnected)
    {
        IMS_UINT32 nMode = m_piImsAos->GetAosInfo()->GetRegistrationMode();

        if (IImsAosInfo::REG_MODE_ADMIN == nMode)
        {
            m_bAosRegModAdmin = IMS_TRUE;
        }
        else
        {
            m_bAosRegModAdmin = IMS_FALSE;
        }
    }

    IMS_TRACE_I("SetImsRegConnected : IMS Reg State [%s], IMS Admin Reg [%s]",
            _TRACE_B_(m_bImsConnected), _TRACE_B_(m_bAosRegModAdmin), 0);

    Update();
}

PUBLIC
IMS_BOOL MtsServiceState::IsMoServiceBlocked() const
{
    if (m_bAllowImsiBasedSipUri)
    {
        return (m_bImsConnected == IMS_FALSE);
    }
    else
    {
        return (m_nState != STATE_READY);
    }
}

PUBLIC
IMS_BOOL MtsServiceState::IsMtServiceBlocked() const
{
    return (m_nState == STATE_NOTREADY);
}

PRIVATE
void MtsServiceState::SetImsSuspendState(IN IMS_BOOL bState)
{
    if (m_bImsSuspend == bState)
    {
        return;
    }

    m_bImsSuspend = bState;  // if IMSAoSApp_OnImsSuspended. block mo service

    IMS_TRACE_I("SetImsSuspendState : IMS Suspend State is [%s]", _TRACE_B_(m_bImsSuspend), 0, 0);

    Update();
}

PRIVATE
void MtsServiceState::SetSmsOverIpState(IN IMS_BOOL bState)
{
    if (m_bSmsOverIpConf == bState)
    {
        return;
    }

    m_bSmsOverIpConf = bState;

    IMS_TRACE_I("SetSmsOverIpState : Sms Over IP Network State is [%s]",
            _TRACE_B_(m_bSmsOverIpConf), 0, 0);

    Update();
}

PRIVATE
void MtsServiceState::Update()
{
    IMS_SINT32 nNewState = STATE_NOTREADY;

    if (m_bImsConnected)
    {
        if (m_bImsSuspend || (!m_bSmsOverIpConf) || m_bAosRegModAdmin)
        {
            nNewState = STATE_LIMITED;
        }
        else
        {
            nNewState = STATE_READY;
        }
    }

    IMS_TRACE_I("Update : OldState(%s), NewState(%s)", PS_ServiceState(m_nState),
            PS_ServiceState(nNewState), 0);

    if (m_nState != nNewState)
    {
        m_nState = nNewState;
    }
}
