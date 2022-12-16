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

#include "ImsTypeDef.h"
#include "base/IMessageMediator.h"

class IMessage;
class ICoreService;
class ICapabilities;
class IMtcContext;
class AString;
class AStringArray;

class MtcCapabilityQueryHandler : public IMessageMediator
{
public:
    explicit MtcCapabilityQueryHandler(IN IMtcContext& objContext);
    virtual ~MtcCapabilityQueryHandler();
    MtcCapabilityQueryHandler(IN const MtcCapabilityQueryHandler&) = delete;
    MtcCapabilityQueryHandler& operator=(IN const MtcCapabilityQueryHandler&) = delete;

    IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 nMessage) override;

    virtual IMS_RESULT HandleIncomingCapabilityQuery(IN ICoreService* piService,
            IN ICapabilities* piCapabilities, IN const AString& strAppId,
            IN const AString& strServiceId, IN IMS_UINT32 nFeatures);

private:
    void SetHeaderForCapabilityQuery(IN IMessage* piMessage);
    IMS_RESULT SetBodyForCapabilityQuery(IN ICoreService* piService, IN IMessage* piMessage,
            IN const AString& strAppId, IN const AString& strServiceId, IN IMS_UINT32 nFeatures);
    static IMS_RESULT SetSessionLevelDescription(IN ICoreService* piService, OUT AString& strDesc);

    // if EVS is supported by default, this should be removed.
    static AString GetAdjustedCodecList(IN const AStringArray& objAudioCaps);

    IMtcContext& m_objContext;
};

#endif
