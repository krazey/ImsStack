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
#include "IMtcService.h"
#include "ISipKeepAliveHelper.h"
#include "ImsTypeDef.h"
#include "IpAddress.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosConnector.h"
#include "helper/UdpKeepAliveSender.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UdpKeepAliveSender::UdpKeepAliveSender(
        IN ISipKeepAliveHelper* pKeepAliveHelper, IN IMtcCallContext& objContext) :
        m_pKeepAliveHelper(pKeepAliveHelper),
        m_nIntervalInMillis(objContext.GetConfigurationProxy().GetInt(
                ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT)),
        m_piTimer(IMS_NULL)
{
    SetTransportInfo(objContext.GetService().GetAosConnector());
}

PUBLIC VIRTUAL UdpKeepAliveSender::~UdpKeepAliveSender()
{
    Stop();

    m_pKeepAliveHelper->Destroy();
}

PUBLIC GLOBAL IMS_BOOL UdpKeepAliveSender::IsRequired(
        IN const MtcConfigurationProxy& objConfigProxy)
{
    return objConfigProxy.GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT) > 0;
}

PUBLIC VIRTUAL void UdpKeepAliveSender::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (m_piTimer != piTimer)
    {
        return;
    }
    IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);

    m_piTimer->KillTimer();
    Start();
}

PUBLIC
void UdpKeepAliveSender::Start()
{
    IMS_TRACE_D("Start", 0, 0, 0);
    SendDummyPacket();

    if (m_piTimer == IMS_NULL)
    {
        m_piTimer = TimerService::GetTimerService()->CreateTimer();
    }

    m_piTimer->SetTimer(m_nIntervalInMillis, this);
}

PUBLIC
void UdpKeepAliveSender::Stop()
{
    IMS_TRACE_D("Stop", 0, 0, 0);
    StopTimer();
}

PRIVATE
void UdpKeepAliveSender::SetTransportInfo(IN const IMtcAosConnector* pAosConnector)
{
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    m_pKeepAliveHelper->SetTransportTupleS(
            IpAddress(pAosConnector->GetLocalAddress()), pAosConnector->GetLocalPort());
    m_pKeepAliveHelper->SetTransportTupleD(
            IpAddress(pAosConnector->GetPcscfAddress()), pAosConnector->GetPcscfPort());
}

PRIVATE
void UdpKeepAliveSender::SendDummyPacket()
{
    const IMS_BYTE objDoubleCrlf[] = {0x0d, 0x0a, 0x0d, 0x0a};
    const ByteArray objPacket(objDoubleCrlf, 4);

    m_pKeepAliveHelper->SendPacket(objPacket);
}

PRIVATE
void UdpKeepAliveSender::StopTimer()
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }
    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;
}
