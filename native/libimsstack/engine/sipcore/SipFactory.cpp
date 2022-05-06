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
#include "ServiceMemory.h"

#include "SipFactory.h"
#include "SIPFactoryProxy.h"
#include "SipFeatures.h"
#include "SIPIPSecState.h"
#include "SIPKeepAliveHelper.h"
#include "SIPMessageTracker.h"
#include "SIPPacketTracker.h"
#include "SipRoutingRejectNotifier.h"
#include "SIPRTConfigHelper.h"
#include "SIPTransportHelper.h"
#include "SIPUtil.h"

PUBLIC GLOBAL ISipKeepAliveHelper* SipFactory::CreateKeepAliveHelper(IN IMS_SINT32 nSlotId)
{
    return new SIPKeepAliveHelper(nSlotId);
}

PUBLIC GLOBAL void SipFactory::GenerateCallId(IN const AString& strHost, OUT AString& strCallId)
{
    strCallId = SIPUtil::GenerateCallId(strHost);
}

/**
 * HEADER_REQ_SESSION-ID
 */
PUBLIC GLOBAL void SipFactory::GenerateSessionId(
        IN IMS_SINT32 nSlotId, IN const AString& strCallId, OUT AString& strSessionId)
{
    if (SipFeatures::IsHeaderSessionIdRequired(nSlotId))
    {
        strSessionId = SIPUtil::GenerateSessionId(nSlotId, strCallId);
        return;
    }

    strSessionId = AString::ConstNull();
}

PUBLIC GLOBAL ISipIpSecState* SipFactory::GetIpSecState(IN IMS_SINT32 nSlotId)
{
    return SIPFactoryProxy::GetInstance()->GetIPSecState(nSlotId);
}

PUBLIC GLOBAL ISipMessageTracker* SipFactory::GetMessageTracker(IN IMS_SINT32 nSlotId)
{
    return SIPFactoryProxy::GetInstance()->GetMessageTracker(nSlotId);
}

PUBLIC GLOBAL ISipRoutingRejectNotifier* SipFactory::GetRoutingRejectNotifier(IN IMS_SINT32 nSlotId)
{
    return SIPFactoryProxy::GetInstance()->GetRoutingRejectNotifier(nSlotId);
}

PUBLIC GLOBAL ISipRtConfigHelper* SipFactory::GetRtConfigHelper(IN IMS_SINT32 nSlotId)
{
    return SIPFactoryProxy::GetInstance()->GetRtConfigHelper(nSlotId);
}

PUBLIC GLOBAL ISipTransportHelper* SipFactory::GetTransportHelper(IN IMS_SINT32 nSlotId)
{
    return SIPFactoryProxy::GetInstance()->GetTransportHelper(nSlotId);
}

PUBLIC GLOBAL ISipPacketTracker* SipFactory::GetPacketTracker(IN IMS_SINT32 nSlotId)
{
    return SIPFactoryProxy::GetInstance()->GetPacketTracker(nSlotId);
}

PUBLIC GLOBAL void SipFactory::SetTokenGenerator(
        IN IMS_SINT32 nSlotId, IN ISipTokenGenerator* piTokenGenerator)
{
    SIPFactoryProxy::GetInstance()->SetTokenGenerator(nSlotId, piTokenGenerator);
}
