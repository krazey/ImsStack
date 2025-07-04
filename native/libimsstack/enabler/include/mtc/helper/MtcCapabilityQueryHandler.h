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

#include "AString.h"
#include "ImsTypeDef.h"
#include "base/IMessageMediator.h"

class AStringArray;
class ICapabilities;
class ICoreService;
class ICoreServiceConfig;
class IMediaConfig;
class IMessage;
class IMtcContext;

class MtcCapabilityQueryHandler : public IMessageMediator
{
public:
    explicit MtcCapabilityQueryHandler(IN IMtcContext& objContext,
            IN const ICoreServiceConfig* piCoreServiceConfig, IN const IMediaConfig* piMediaConfig);
    virtual ~MtcCapabilityQueryHandler() override;
    MtcCapabilityQueryHandler(IN const MtcCapabilityQueryHandler&) = delete;
    MtcCapabilityQueryHandler& operator=(IN const MtcCapabilityQueryHandler&) = delete;

    IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 nMessage) override;

    virtual IMS_RESULT HandleIncomingCapabilityQuery(
            IN ICoreService* piService, IN ICapabilities* piCapabilities, IN IMS_UINT32 nFeatures);

private:
    void SetHeaderForCapabilityQuery(IN IMessage* piMessage);
    IMS_RESULT SetBodyForCapabilityQuery(
            IN const ICoreService* piService, IN IMessage* piMessage, IN IMS_UINT32 nFeatures);
    virtual const AStringArray& GetMediaCapability(IN IMS_SINT32 nMediaType) const;
    static IMS_RESULT SetSessionLevelDescription(
            IN const ICoreService* piService, OUT AString& strDesc);

    // if EVS is supported by default, this should be removed.
    static AString GetAdjustedCodecList(IN const AStringArray& objAudioCaps);

    IMtcContext& m_objContext;
    const ICoreServiceConfig* m_piCoreServiceConfig;
    const IMediaConfig* m_piMediaConfig;
};

#endif
