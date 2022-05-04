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
#ifndef INTERFACE_AOS_LOCATION_STARTER_H_
#define INTERFACE_AOS_LOCATION_STARTER_H_

#include "interface/IAosBlock.h"

class IAosAppContext;

class IAosLocationStarter
{
public:
    virtual IMS_SINT32 GetSlotId() const = 0;
    virtual void SetSlotId(IN IMS_SINT32 nSlotId) = 0;

    virtual void Init(IN IAosAppContext* piContext,
            IN IMS_UINT32 nPolicy = POLICY_START_ON_WFC_AVAILABILITY) = 0;
    virtual void SetPolicy(
            IN IMS_UINT32 nPolicy, IN IMS_SINT32 nOperation = 0 /* (0: add, 1: remove) */) = 0;
    virtual IMS_BOOL IsPolicyEnabled(IN IMS_UINT32 nPolicy) = 0;

    virtual void AddBlockReason(
            IN BLOCK_REASON nReason, IN IMS_SINT32 nType = TYPE_VOLTE /* (0: VoLTE, 1: WFC) */) = 0;

    virtual void SetUpdateInterval(IN IMS_UINT32 nInterval) = 0;
    virtual void StartLocationInfoUpdate() = 0;
    virtual void StopLocationInfoUpdate() = 0;

    enum
    {
        POLICY_NONE = (0x00000000),

        /*
         * Starts location search when IMS over WiFi is available.
         */
        POLICY_START_ON_WFC_AVAILABILITY = (0x00000001),

        /*
         * Starts location search when the user setting of VoWiFi is on.
         */
        POLICY_START_ON_WFC_SETTING = (0x00000002),

        /*
         * Starts location search when IMS over cellular is available.
         */
        POLICY_START_ON_VOLTE_AVAILABLE = (0x00000004),

        /*
         * Starts location search when AosServiceAvailable don't have specific block reason.
         */
        POLICY_START_AFTER_CHECKING_VOLTE_BLOCK_REASON = (0x00000008),
        POLICY_START_AFTER_CHECKING_WFC_BLOCK_REASON = (0x00000010)
    };

    enum
    {
        TYPE_VOLTE = 0,
        TYPE_WFC = 1
    };

    enum
    {
        DEFAULT_UPDATE_INTERVAL = 600  // 10min
    };
};

#endif  // INTERFACE_AOS_LOCATION_STARTER_H_
