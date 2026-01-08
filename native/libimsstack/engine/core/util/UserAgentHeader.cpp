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
#include "AStringBuffer.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SipConfigProxy.h"
#include "SipMethod.h"
#include "util/UserAgentHeader.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL void UserAgentHeader::SetHeader(IN IMS_SINT32 nHeaderType,
        IN const SipProfile* pProfile, IN const AString& /*strServiceId*/,
        IN const IpAddress& /*objIpAddr*/, IN IMS_SINT32 nSlotId, IN_OUT ISipMessage*& piSipMsg)
{
    if (nHeaderType != ISipHeader::USER_AGENT && nHeaderType != ISipHeader::SERVER)
    {
        return;
    }

    if (!SipConfigProxy::IsUserAgentConfigured(nSlotId, pProfile))
    {
        return;
    }

    AString strUaString;

    if (piSipMsg->GetMethod().Equals(SipMethod::REGISTER))
    {
        strUaString = SipConfigProxy::GetRegUaString(nSlotId, pProfile);
    }
    else
    {
        strUaString = SipConfigProxy::GetUaString(nSlotId, pProfile);
    }

    if (strUaString.GetLength() == 0)
    {
        IMS_TRACE_D("UA version is empty", 0, 0, 0);
        return;
    }

    if (piSipMsg->SetHeader(nHeaderType, strUaString) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting %s header failed",
                (nHeaderType == ISipHeader::USER_AGENT ? "User-Agent" : "Server"), 0, 0);
        return;
    }
}
