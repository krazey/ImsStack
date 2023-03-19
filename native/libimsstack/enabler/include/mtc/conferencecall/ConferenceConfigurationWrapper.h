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

#ifndef CONFERENCE_CONFIGURATION_WRAPPER_H_
#define CONFERENCE_CONFIGURATION_WRAPPER_H_

#include "ImsTypeDef.h"

// TODO: this class will be deprecated and replaced by the MtcConfigurationProxy
class ConferenceConfigurationWrapper
{
public:
    static IMS_BOOL IsConferenceSubscriptionRequired();
    // LGU+ doesn't use Refer-sub.
    static IMS_BOOL IsReferSubscriptionRequired();
    static IMS_BOOL IsSubscriptionOutDialog();

    // SKT always set it 1.
    static IMS_BOOL IsPackageVersionCheckRequired();
    static IMS_BOOL IsSubscriptionFirst();
    static IMS_BOOL IsSubscriptionNotifyRequiredForRefer();
    static IMS_BOOL IsPaidPreferred();
    static IMS_BOOL IsReUseReferToUri();
    static IMS_BOOL IsReferUsed();
    // SKT always receive disconnecting status when participant leaves conference call.
    static IMS_BOOL IsDisconnectingStatusUsed();
    static IMS_BOOL IsReferToExHeaderUsed();

    static IMS_BOOL IsSubscriptionForParticipantRequired();

    // timer value. (-1) : permanent. (0) : not wait
    static IMS_SINT32 GetWaitTimeNotifyTerminated();  // 3s
    static IMS_SINT32 GetReferTypeForInvite();
};

#endif
