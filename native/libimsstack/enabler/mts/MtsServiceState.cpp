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

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    if (piCc != IMS_NULL)
    {
        piCc->RemoveListener(this);
    }
}

PUBLIC
void MtsServiceState::Init(IN IImsAos* piImsAos)
{
    IMS_TRACE_I("Init", 0, 0, 0);

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    if (piCc != IMS_NULL)
    {
        piCc->AddListener(this);
        if (LoadCarrierConfig(*piCc))
        {
            Update();
        }
    }

    m_piImsAos = piImsAos;
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

PROTECTED
void MtsServiceState::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("CarrierConfig_NotifyConfigChanged: (%d)", nSlotId, 0, 0);

    if (m_nSlotId != nSlotId)
    {
        return;
    }

    const ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(nSlotId);
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "CarrierConfig_NotifyConfigChanged : config failed", 0, 0, 0);
        return;
    }

    if (LoadCarrierConfig(*piCc))
    {
        Update();
    }
}

PRIVATE
IMS_BOOL MtsServiceState::LoadCarrierConfig(IN const ICarrierConfig& objCc)
{
    IMS_BOOL bResult = IMS_FALSE;

    IMS_BOOL bSmsOverIpConf =
            objCc.GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL);
    if (m_bSmsOverIpConf != bSmsOverIpConf)
    {
        IMS_TRACE_I("LoadCarrierConfig : SmsOverIp - Old(%s) -> New(%s)",
                _TRACE_B_(m_bSmsOverIpConf), _TRACE_B_(bSmsOverIpConf), 0);
        m_bSmsOverIpConf = bSmsOverIpConf;
        bResult = IMS_TRUE;
    }

    IMS_BOOL bAllowImsiBasedSipUri =
            objCc.GetBoolean(CarrierConfig::ImsSms::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL);
    if (m_bAllowImsiBasedSipUri != bAllowImsiBasedSipUri)
    {
        IMS_TRACE_I("LoadCarrierConfig : AllowImsiBasedSipUri - Old(%s) -> New(%s)",
                _TRACE_B_(m_bAllowImsiBasedSipUri), _TRACE_B_(bAllowImsiBasedSipUri), 0);
        m_bAllowImsiBasedSipUri = bAllowImsiBasedSipUri;
        bResult = IMS_TRUE;
    }

    return bResult;
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

    if (m_nState != nNewState)
    {
        IMS_TRACE_I("Update : OldState(%s) -> NewState(%s)", PS_ServiceState(m_nState),
                PS_ServiceState(nNewState), 0);
        m_nState = nNewState;
    }
}
