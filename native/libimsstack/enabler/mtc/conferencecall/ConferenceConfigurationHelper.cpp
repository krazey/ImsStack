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
#include "MtcDef.h"
#include "conferencecall/ConferenceConfigurationHelper.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsConferenceSubscriptionRequired(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.GetInt(Feature::CONFERENCE_SUBSCRIBE_TYPE) !=
            CarrierConfig::ImsVoice::CONFERENCE_SUBSCRIBE_NOT_SUPPORT;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsReferSubscriptionRequired(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.Is(Feature::SUPPORT_CONFERENCE_REFER_SUBSCRIBE);
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsSubscriptionOutDialog(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.GetInt(Feature::CONFERENCE_SUBSCRIBE_TYPE) ==
            CarrierConfig::ImsVoice::CONFERENCE_SUBSCRIBE_TYPE_OUT_OF_DIALOG;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsPackageVersionCheckRequired(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.Is(Feature::CHECK_CONFERENCE_EVENT_PACKAGE_VERSION);
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsSubscriptionFirst(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.GetInt(Feature::CONFERENCE_SIP_FLOW_ORDER) ==
            CarrierConfig::ImsVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_REFER;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsSubscriptionNotifyRequiredForRefer(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.GetInt(Feature::CONFERENCE_SIP_FLOW_ORDER) ==
            CarrierConfig::ImsVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsPaidPreferred(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.Is(Feature::CONFERENCE_REFER_TO_URI_SOURCE_PAID);
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsReUseReferToUri(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.GetInt(Feature::CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE) ==
            CarrierConfig::ImsVoice::CONFERENCE_DROP_REFER_TO_URI_SOURCE_REFER_TO_URI_FOR_INVITE;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsReferToExHeaderUsed(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.Is(Feature::ADD_REPLACE_HEADER_FOR_CONFERENCE);
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationHelper::IsSubscriptionForParticipantRequired(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.Is(Feature::ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT);
}

PUBLIC GLOBAL IMS_SINT32 ConferenceConfigurationHelper::GetWaitTimeNotifyTerminated(
        IN MtcConfigurationProxy& /*objProxy*/)
{
    // TODO: Add configuration for VZW.
    // if this value is less than 0, no Un-Subscription
    return 3000;
}

PUBLIC GLOBAL IMS_SINT32 ConferenceConfigurationHelper::GetReferTypeForInvite(
        IN MtcConfigurationProxy& objProxy)
{
    return objProxy.GetInt(Feature::CONFERENCE_INVITING_REFER_TYPE);
}
