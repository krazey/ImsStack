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
#include "SipConfigProxy.h"
#include "SipFeatures.h"

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsEventHeaderApplicableForRefer(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsHeaderSessionIdRequired(IN IMS_SINT32 nSlotId)
{
    return SipConfigProxy::IsSessionIdHeaderSupported(nSlotId);
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsHostPartValidationRequiredForIncomingRequestRouting(
        IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsMultipleDialogUsagesRequiredForNonSharedDialog(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsPaniHeaderForAckRequired(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsSocketOptionRequiredForTcpMaxSeg(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsReferSubHeaderSupported(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsStandard2XXRetransmissionIntervalRequired(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsTransportParameterIgnoredForIncomingRequestRouting(
        IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsTransportParameterUdpIgnoredForOutgoingRequest(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SipFeatures::IsTransportParameterIgnoredForRegBinding(IN IMS_SINT32/* nSlotId*/)
{
    return IMS_FALSE;
}
