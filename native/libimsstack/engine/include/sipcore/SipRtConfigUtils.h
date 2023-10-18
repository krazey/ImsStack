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
#ifndef SIP_RT_CONFIG_UTILS_H_
#define SIP_RT_CONFIG_UTILS_H_

#include "SipRtConfigHelper.h"

class SipRtConfigUtils
{
public:
    SipRtConfigUtils() = delete;

public:
    static SipRtConfigHelper* GetConfigHelper(IN IMS_SINT32 nSlotId);

    static IMS_BOOL IsMessageHiddenInLog(IN IMS_SINT32 nSlotId);
    static IMS_BOOL IsRoutingInfoHiddenInLog(IN IMS_SINT32 nSlotId);

    /** Features */
    static IMS_BOOL IsFeatureSipTxPacketBlockedEnabled(IN IMS_SINT32 nSlotId);

    /** Configurations */
    static IMS_BOOL IsIpSecSaConfigured(IN IMS_SINT32 nSlotId);
    static IMS_BOOL IsRegContactAddressConfigured(IN IMS_SINT32 nSlotId);
    static IMS_BOOL IsTcpPortRangeConfigured(IN IMS_SINT32 nSlotId);
};

#endif
