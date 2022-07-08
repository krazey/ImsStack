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

#include "INetworkWatcher.h"
#include "call/IMtcCallContext.h"
#include "call/block/NetworkBlockRule.h"
#include "helper/IMtcAosConnector.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
NetworkBlockRule::NetworkBlockRule(
        IN IMtcCallContext& objContext, IN INetworkWatcher& objNetworkWatcher) :
        m_objService(objContext.GetService()),
        m_objNetworkWatcher(objNetworkWatcher)
{
}

PUBLIC VIRTUAL NetworkBlockRule::~NetworkBlockRule() {}

PUBLIC VIRTUAL NetworkBlockRule::Result NetworkBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (IsInEpdg(m_objService) || IsWifiRegistered(m_objService.GetAosConnector()))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_UINT32 nNetworkType = m_objNetworkWatcher.GetNetRadioTechType();
    if (nNetworkType == NW_REPORT_RADIO_LTE || nNetworkType == NW_REPORT_RADIO_NR)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : Network type[%d] is not applicable", nNetworkType, 0, 0);

    return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE));
}

PRIVATE
IMS_BOOL NetworkBlockRule::IsInEpdg(IN const IMtcService& objService)
{
    return objService.IsWlanIpCanType();
}

PRIVATE
IMS_BOOL NetworkBlockRule::IsWifiRegistered(IN IMtcAosConnector* pAosConnector)
{
    IMS_UINT32 nAosRegisteredNetworkType =
            pAosConnector ? pAosConnector->GetRegisteredNetworkType() : NW_REPORT_RADIO_INVALID;

    return nAosRegisteredNetworkType == NW_REPORT_RADIO_WLAN;
}
