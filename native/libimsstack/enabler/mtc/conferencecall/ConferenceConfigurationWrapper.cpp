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
#include "IMtcContext.h"
#include "MtcContextRepository.h"
#include "MtcDef.h"
#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsConferenceSubscriptionRequired()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().GetInt(
                   Feature::CONFERENCE_SUBSCRIBE_TYPE) !=
            CarrierConfig::ImsVoice::CONFERENCE_SUBSCRIBE_NOT_SUPPORT;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsReferSubscriptionRequired()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().Is(
            Feature::SUPPORT_CONFERENCE_REFER_SUBSCRIBE);
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionOutDialog()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().GetInt(
                   Feature::CONFERENCE_SUBSCRIBE_TYPE) ==
            CarrierConfig::ImsVoice::CONFERENCE_SUBSCRIBE_TYPE_OUT_OF_DIALOG;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsPackageVersionCheckRequired()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().Is(
            Feature::CHECK_CONFERENCE_EVENT_PACKAGE_VERSION);
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionFirst()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().GetInt(
                   Feature::CONFERENCE_SIP_FLOW_ORDER) ==
            CarrierConfig::ImsVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_REFER;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionNotifyRequiredForRefer()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().GetInt(
                   Feature::CONFERENCE_SIP_FLOW_ORDER) ==
            CarrierConfig::ImsVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsPaidPreferred()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().Is(
            Feature::CONFERENCE_REFER_TO_URI_SOURCE_PAID);
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsReUseReferToUri()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().GetInt(
                   Feature::CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE) ==
            CarrierConfig::ImsVoice::CONFERENCE_DROP_REFER_TO_URI_SOURCE_REFER_TO_URI_FOR_INVITE;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsReferUsed()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().GetInt(
                   Feature::CONFERENCE_INVITING_REFER_TYPE) == 1;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsDisconnectingStatusUsed()
{
    // TODO: it's really not necessary?
    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsReferToExHeaderUsed()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().Is(
            Feature::ADD_REPLACE_HEADER_FOR_CONFERENCE);
}

PUBLIC GLOBAL IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionForParticipantRequired()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().Is(
            Feature::ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT);
}

PUBLIC GLOBAL IMS_SINT32 ConferenceConfigurationWrapper::GetWaitTimeNotifyTerminated()
{
    // if this value is less than 0, no Un-Subscription
    return 3000;
}

PUBLIC GLOBAL IMS_SINT32 ConferenceConfigurationWrapper::GetReferTypeForInvite()
{
    return MtcContextRepository::GetContext()->GetConfigurationProxy().GetInt(
            Feature::CONFERENCE_INVITING_REFER_TYPE);
}
