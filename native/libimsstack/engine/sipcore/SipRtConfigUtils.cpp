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
#include "SipFactoryProxy.h"
#include "SipRtConfigUtils.h"

PUBLIC GLOBAL SipRtConfigHelper* SipRtConfigUtils::GetConfigHelper(IN IMS_SINT32 nSlotId)
{
    return SipFactoryProxy::GetInstance()->GetRtConfigHelper(nSlotId);
}

PUBLIC GLOBAL IMS_BOOL SipRtConfigUtils::IsMessageHiddenInLog(IN IMS_SINT32 nSlotId)
{
    SipRtConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) && pConfigHelper->IsMessageHiddenInLog();
}

PUBLIC GLOBAL IMS_BOOL SipRtConfigUtils::IsRoutingInfoHiddenInLog(IN IMS_SINT32 nSlotId)
{
    SipRtConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) && pConfigHelper->IsRoutingInfoHiddenInLog();
}

PUBLIC GLOBAL IMS_BOOL SipRtConfigUtils::IsFeatureSipTxPacketBlockedEnabled(IN IMS_SINT32 nSlotId)
{
    SipRtConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) &&
            pConfigHelper->IsFeatureEnabled(SipRtConfig::FEATURE_SIP_TX_PACKET_BLOCKED);
}

PUBLIC GLOBAL IMS_BOOL SipRtConfigUtils::IsIpSecSaConfigured(IN IMS_SINT32 nSlotId)
{
    SipRtConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) &&
            pConfigHelper->IsItemConfigured(SipRtConfig::CONFIG_I_IPSEC_SA);
}

PUBLIC GLOBAL IMS_BOOL SipRtConfigUtils::IsRegContactAddressConfigured(IN IMS_SINT32 nSlotId)
{
    SipRtConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) &&
            pConfigHelper->IsItemConfigured(SipRtConfig::CONFIG_I_REG_CONTACT_ADDRESS);
}

PUBLIC GLOBAL IMS_BOOL SipRtConfigUtils::IsTcpPortRangeConfigured(IN IMS_SINT32 nSlotId)
{
    SipRtConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) &&
            pConfigHelper->IsItemConfigured(SipRtConfig::CONFIG_I_TCP_PORT_RANGE);
}
