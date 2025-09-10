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

#ifndef SDP_PRECONDITION_HELPER_H_
#define SDP_PRECONDITION_HELPER_H_

#include "ISession.h"
#include "ISipMessage.h"
#include "ImsTypeDef.h"
#include "media/IMediaDescriptor.h"
#include "offeranswer/SdpSegmentedPrecondition.h"
#include "precondition/SdpPreconditionHelper.h"
#include "precondition/QosStatusTable.h"

class SdpPreconditionHelper
{
public:
    SdpPreconditionHelper();
    virtual ~SdpPreconditionHelper();
    SdpPreconditionHelper(IN const SdpPreconditionHelper&) = delete;
    SdpPreconditionHelper& operator=(IN const SdpPreconditionHelper) = delete;

    virtual void FormPreconditionSdp(
            IN ISession* piSession, IN QosStatusTable* pStatusTable, IN IMS_BOOL bUseConf);
    virtual void RemovePreconditionSdp(IN ISession* piSession);
    virtual void FormFailurePreconditionSdp(IN ISession* piSession);
    virtual IMS_UINT32 GetMediaType(IN const SdpMedia* pSdpMedia, IN IMS_SINT32 nMediaState);
    virtual IMS_BOOL IsPreconditionIncludedInSdp(IN ISession* piSession) const;
    virtual IMS_BOOL IsLocalResourceReservedInSdp(
            IN ISession* piSession, IN IMS_SINT32 nServiceMethod) const;

private:
    static void FormCurrentAttribute(
            IN IMediaDescriptor* piMediaDescriptor, IN QosStatusTable* pStatusTable);
    static void FormDesiredAttribute(
            IN IMediaDescriptor* piMediaDescriptor, IN QosStatusTable* pStatusTable);
    static void AddDesiredStatus(OUT SdpSegmentedPrecondition* objDesired,
            IN QosStatusTable* pStatusTable, IN IMS_SINT32 eSdpMediaType,
            IN IMS_SINT32 eStatusType);
    static void FormConfirmAttribute(IN IMediaDescriptor* piMediaDescriptor,
            IN QosStatusTable* pStatusTable, IN IMS_BOOL bUseConf);
    static IMediaDescriptor* GetMediaDescriptor(IN const IMedia* piMedia);
    static IMS_BOOL HasReservedResourceInSdp(
            IN const ISipMessage* piSipMessage, IN IMS_SINT32 eSdpMediaType);
};
#endif
