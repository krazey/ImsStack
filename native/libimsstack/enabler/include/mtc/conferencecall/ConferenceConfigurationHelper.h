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

#ifndef CONFERENCE_CONFIGURATION_HELPER_H_
#define CONFERENCE_CONFIGURATION_HELPER_H_

#include "ImsTypeDef.h"

class MtcConfigurationProxy;

class ConferenceConfigurationHelper
{
public:
    static IMS_BOOL IsConferenceSubscriptionRequired(IN MtcConfigurationProxy& objProxy);
    // LGU+ doesn't use Refer-sub.
    static IMS_BOOL IsReferSubscriptionRequired(IN MtcConfigurationProxy& objProxy);
    static IMS_BOOL IsSubscriptionOutDialog(IN MtcConfigurationProxy& objProxy);

    // SKT always set it 1.
    static IMS_BOOL IsPackageVersionCheckRequired(IN MtcConfigurationProxy& objProxy);
    static IMS_BOOL IsSubscriptionFirst(IN MtcConfigurationProxy& objProxy);
    static IMS_BOOL IsSubscriptionNotifyRequiredForRefer(IN MtcConfigurationProxy& objProxy);
    static IMS_BOOL IsPaidPreferred(IN MtcConfigurationProxy& objProxy);
    static IMS_BOOL IsReUseReferToUri(IN MtcConfigurationProxy& objProxy);
    static IMS_BOOL IsReferToExHeaderUsed(IN MtcConfigurationProxy& objProxy);

    static IMS_BOOL IsSubscriptionForParticipantRequired(IN MtcConfigurationProxy& objProxy);

    // timer value. (-1) : permanent. (0) : not wait
    static IMS_SINT32 GetWaitTimeNotifyTerminated(IN MtcConfigurationProxy& objProxy);  // 3s
    static IMS_SINT32 GetReferTypeForInvite(IN MtcConfigurationProxy& objProxy);
};

#endif
