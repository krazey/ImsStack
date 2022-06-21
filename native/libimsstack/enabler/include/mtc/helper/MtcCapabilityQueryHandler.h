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

#ifndef MTC_CAPABILITY_QUERY_HANDLER_H_
#define MTC_CAPABILITY_QUERY_HANDLER_H_

#include "IMSTypeDef.h"

class IMessage;
class ICoreService;
class ICapabilities;
class AString;
class AStringArray;

class MtcCapabilityQueryHandler
{
public:
    static void HandleIncomingCapabilityQuery(IN ICoreService* piService,
            IN ICapabilities* piCapabilities, IN const AString& strAppId,
            IN const AString& strServiceId, IN IMS_SINT32 nSlotId);

private:
    static void SetHeaderForCapabilityQuery(IN ICoreService* piService, IN IMessage* piMessage);
    static void SetBodyForCapabilityQuery(IN ICoreService* piService, IN IMessage* piMessage,
            IN const AString& strAppId, IN const AString& strServiceId, IN IMS_SINT32 nSlotId);
    static void SetSessionLevelDescription(IN ICoreService* piService, OUT AString& strDesc);

    // if EVS is supported by default, this should be removed.
    static AString GetAdjustedCodecList(IN const AStringArray& objAudioCaps);
};

#endif
