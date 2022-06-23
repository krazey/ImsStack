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
#ifndef AOS_CARRIER_CONFIG_BUNDLE_H_
#define AOS_CARRIER_CONFIG_BUNDLE_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "ImsVector.h"

/// ims.
struct AosMmtelRequiresProvisioningBundle
{
public:
    AosMmtelRequiresProvisioningBundle() :
            objCapabilityTypeVoice(IMSVector<IMS_SINT32>()),
            objCapabilityTypeVideo(IMSVector<IMS_SINT32>()),
            objCapabilityTypeSms(IMSVector<IMS_SINT32>())
    {
    }

    AosMmtelRequiresProvisioningBundle(IN const AosMmtelRequiresProvisioningBundle&) = delete;
    AosMmtelRequiresProvisioningBundle& operator=(
            IN const AosMmtelRequiresProvisioningBundle&) = delete;

public:
    IMSVector<IMS_SINT32> objCapabilityTypeVoice;
    IMSVector<IMS_SINT32> objCapabilityTypeVideo;
    IMSVector<IMS_SINT32> objCapabilityTypeSms;
};

struct AosNotifyTerminatedForRegEventWithInitialRegistrationBundle
{
public:
    AosNotifyTerminatedForRegEventWithInitialRegistrationBundle() :
            nWaitTimeForInitRegOnTerminatedState(0),
            objEventForInitRegOnTerminatedState(IMSVector<IMS_SINT32>()),
            objEventToFollowWtForInitRegOnTerminatedState(IMSVector<IMS_SINT32>())
    {
    }

    AosNotifyTerminatedForRegEventWithInitialRegistrationBundle(
            IN const AosNotifyTerminatedForRegEventWithInitialRegistrationBundle&) = delete;
    AosNotifyTerminatedForRegEventWithInitialRegistrationBundle& operator=(
            IN const AosNotifyTerminatedForRegEventWithInitialRegistrationBundle&) = delete;

public:
    IMS_SINT32 nWaitTimeForInitRegOnTerminatedState;
    IMSVector<IMS_SINT32> objEventForInitRegOnTerminatedState;
    IMSVector<IMS_SINT32> objEventToFollowWtForInitRegOnTerminatedState;
};

struct AosRegistrationRetryIntervalBundle
{
public:
    AosRegistrationRetryIntervalBundle() :
            objRegistrationRetryRandomUpperValueSec(IMSVector<IMS_SINT32>()),
            objRegistrationRetryIntervalSec(IMSVector<IMS_SINT32>()),
            bUseRegistrationRetryIntervalForSubscriptionRetry(IMS_TRUE)
    {
    }

    AosRegistrationRetryIntervalBundle(IN const AosRegistrationRetryIntervalBundle&) = delete;
    AosRegistrationRetryIntervalBundle& operator=(
            IN const AosRegistrationRetryIntervalBundle&) = delete;

public:
    IMSVector<IMS_SINT32> objRegistrationRetryRandomUpperValueSec;
    IMSVector<IMS_SINT32> objRegistrationRetryIntervalSec;
    IMS_BOOL bUseRegistrationRetryIntervalForSubscriptionRetry;
};

#endif  // AOS_CARRIER_CONFIG_BUNDLE_H_
