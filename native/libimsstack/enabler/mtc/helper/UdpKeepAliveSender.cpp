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

#include "IImsAosInfo.h"
#include "IMtcService.h"
#include "ImsTypeDef.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosConnector.h"
#include "helper/UdpKeepAliveSender.h"
#include "sipcore/ISipKeepAliveHelper.h"
#include "sipcore/SipFactory.h"
#include "util/IpAddress.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UdpKeepAliveSender::UdpKeepAliveSender(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_piTimer(IMS_NULL),
        m_pKeepAliveHelper(SipFactory::CreateKeepAliveHelper(objContext.GetSlotId()))
{
    IMS_TRACE_D("+UdpKeepAliveSender[%d]", m_objContext.GetCallKey(), 0, 0);
    SetTransportInfo();
}

PUBLIC VIRTUAL UdpKeepAliveSender::~UdpKeepAliveSender()
{
    IMS_TRACE_D("~UdpKeepAliveSender[%d]", m_objContext.GetCallKey(), 0, 0);
    Stop();

    m_pKeepAliveHelper->Destroy();
}

PUBLIC GLOBAL IMS_BOOL UdpKeepAliveSender::IsRequired(IN MtcConfigurationProxy& objConfigProxy)
{
    return objConfigProxy.GetInt(Feature::SEND_UDP_KEEP_ALIVE_INTERVAL_TIME) > 0;
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

    m_piTimer->SetTimer(
            m_objContext.GetConfigurationProxy().GetInt(Feature::SEND_UDP_KEEP_ALIVE_INTERVAL_TIME),
            this);
}

PUBLIC
void UdpKeepAliveSender::Stop()
{
    IMS_TRACE_D("Stop", 0, 0, 0);
    if (m_piTimer == IMS_NULL)
    {
        return;
    }
    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;
}

PRIVATE
void UdpKeepAliveSender::SetTransportInfo()
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector == IMS_NULL)
    {
        // no exception handling.
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

    // no exception handling.
    m_pKeepAliveHelper->SendPacket(objPacket);
}
