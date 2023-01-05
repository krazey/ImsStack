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
#include "ImsAosParameter.h"
#include "MtsStringDef.h"
#include "MtsServiceState.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsServiceState::MtsServiceState(IN IMS_SINT32 nSlotId) :
        m_nMtsServiceState(STATE_INIT),
        m_bImsConnected(IMS_FALSE),
        m_bAosRegModAdmin(IMS_FALSE),
        m_bImsSuspend(IMS_FALSE),
        m_bSmsOverIpConf(IMS_FALSE),
        m_nConnectedServices(ImsAosService::NONE),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_I("+MtsServiceState [slot_%d]", m_nSlotId, 0, 0);

    Init();
}

PUBLIC
MtsServiceState::~MtsServiceState()
{
    IMS_TRACE_I("~MtsServiceState [slot_%d]", m_nSlotId, 0, 0);

    DeInit();
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
        IImsAos* piImsAos =
                ImsAos::GetImsAos(AString("ims.app.mts"), AString("ims.service.mts"), m_nSlotId);

        if (piImsAos == IMS_NULL)
        {
            IMS_TRACE_E(0, "Fail to get AoSApp", 0, 0, 0);
            return;
        }

        IMS_UINT32 nType = piImsAos->GetAosInfo()->GetRegistrationMode();

        if (IImsAosInfo::REG_MODE_ADMIN == nType)
        {
            m_bAosRegModAdmin = IMS_TRUE;
        }
        else
        {
            m_bAosRegModAdmin = IMS_FALSE;

            if (IImsAosInfo::REG_MODE_UNKNOWN == nType)
            {
                IMS_TRACE_I("IMS Reg Mod is UNKNOWN!!", 0, 0, 0);
            }
        }
    }

    IMS_TRACE_I("SetImsRegConnected : IMS Reg State [%s], IMS Admin Reg [%s]",
            _TRACE_B_(m_bImsConnected), _TRACE_B_(m_bAosRegModAdmin), 0);

    UpdateServiceState();
}

PUBLIC
void MtsServiceState::SetSmsOverIpState(IN IMS_BOOL bState)
{
    if (m_bSmsOverIpConf == bState)
    {
        return;
    }

    m_bSmsOverIpConf = bState;

    IMS_TRACE_I("SetSmsOverIpState : Sms Over IP Network State is [%s]",
            _TRACE_B_(m_bSmsOverIpConf), 0, 0);

    UpdateServiceState();
}

PUBLIC
IMS_BOOL MtsServiceState::IsServiceConnected(IN IMS_UINT32 nService)
{
    return (m_nConnectedServices & nService) != 0;
}

PUBLIC
void MtsServiceState::SetConnectedServices(IN IMS_UINT32 nServices)
{
    m_nConnectedServices = nServices;
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
IMS_SINT32 MtsServiceState::GetServiceState()
{
    IMS_SINT32 nState = STATE_NOTREADY;

    if (m_bImsConnected)
    {
        if (m_bImsSuspend || (!m_bSmsOverIpConf) || m_bAosRegModAdmin)
        {
            nState = STATE_LIMITED;
        }
        else
        {
            nState = STATE_READY;
        }
    }

    IMS_TRACE_I("GetServiceState : nState(%d)", nState, 0, 0);

    return nState;
}

PUBLIC
void MtsServiceState::UpdateServiceState()
{
    IMS_SINT32 nTempState = GetServiceState();

    IMS_TRACE_I("UpdateServiceState : nTempState(%d) m_nMtsServiceState(%d)", nTempState,
            m_nMtsServiceState, 0);

    if (nTempState != m_nMtsServiceState)
    {
        m_nMtsServiceState = nTempState;
    }
}

PUBLIC
IMS_BOOL MtsServiceState::IsMoServiceBlocked() const
{
    return (m_nMtsServiceState != STATE_READY);
}

PUBLIC
IMS_BOOL MtsServiceState::IsMtServiceBlocked() const
{
    return (m_nMtsServiceState == STATE_NOTREADY);
}

PRIVATE
void MtsServiceState::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    IMS_BOOL bSmsOverIpNetwork =
            piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL);

    // TODO(Mts): Check whether Mts should consider KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY
    IMSVector<IMS_SINT32> objSupportedRats =
            piCc->GetIntArray(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY);

    IMS_TRACE_I("GetSmOverIpConfigInfo : bSmsOverIpNetwork[%d]", bSmsOverIpNetwork, 0, 0);

    for (IMS_UINT32 i = 0; i < objSupportedRats.GetSize(); ++i)
    {
        IMS_SINT32 nValue = objSupportedRats.GetAt(i);
        IMS_TRACE_I("GetSmOverIpConfigInfo : objSupportedRats[%d][%d]", i, nValue, 0);
    }

    if (bSmsOverIpNetwork)
    {
        SetSmsOverIpState(IMS_TRUE);
    }
    else
    {
        SetSmsOverIpState(IMS_FALSE);
    }
}

PRIVATE
void MtsServiceState::DeInit()
{
    IMS_TRACE_I("DeInit", 0, 0, 0);
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

    UpdateServiceState();
}
