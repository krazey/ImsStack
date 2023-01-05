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

#ifndef _IMS_UCE_SERVICE_THREAD_H_
#define _IMS_UCE_SERVICE_THREAD_H_

#include "BaseServiceThread.h"
#include "IUceJniThread.h"

class JniUceServiceThread : public BaseServiceThread, public IUceJniThread
{
public:
    JniUceServiceThread();
    virtual ~JniUceServiceThread();

    virtual IMS_BOOL NotifyImsDeregistered() override;
    virtual IMS_BOOL NotifyImsRegistered(
            IN IMS_UINT32 registeredService, IN IMS_SINT32 registeredNetwork) override;
    virtual IMS_BOOL PublishResponseInd(IMS_UINT32 key, IMS_UINT32 responseCode,
            IMS_UINT32 capability, AString reason, IMS_UINT32 reasonHeaderCause,
            AString reasonHeaderText, AString etag, IMS_UINT32 needToRetry) override;
    virtual IMS_BOOL PublishUpdatedInd(IMS_UINT32 capability, IMS_SINT32 responseCode,
            AString reason, IMS_SINT32 reasonHeaderCause, AString reasonHeaderText, AString eTag,
            IMS_UINT32 needToRetry) override;
    virtual IMS_BOOL PublishErrorInd(IMS_UINT32 key, IMS_UINT32 commandError) override;
    virtual IMS_BOOL UnPublishedInd() override;
    virtual IMS_BOOL SubscribeResponseInd(IMS_UINT32 key, IMS_SINT32 responseCode, AString reason,
            IMS_SINT32 reasonHeaderCause, AString reasonHeaderText) override;
    virtual IMS_BOOL NotifyInd(
            IMS_UINT32 key, IMS_UINT32 count, IMSList<AString> pidfXmls) override;
    virtual IMS_BOOL SubscribeErrorInd(IMS_UINT32 key, IMS_UINT32 commandError) override;
    virtual IMS_BOOL SubscribeResourceTerminatedInd(IMS_UINT32 key, IMS_UINT32 count,
            IMSList<IUceTerminatedReason*> terminateContacts) override;
    virtual IMS_BOOL SubscribeTerminatedInd(
            IMS_UINT32 key, AString reason, IMS_UINT32 retryAfterMillsecond) override;
    virtual IMS_BOOL OptionsResponseInd(
            IMS_UINT32 key, IMS_UINT32 responseCode, AString reason, IMS_UINT32 theirCaps) override;
    virtual IMS_BOOL OptionsErrorInd(IMS_UINT32 key, IMS_UINT32 commandError) override;
    virtual IMS_BOOL OptionsReceivedInd(
            IMS_UINT32 key, AString remote, IMS_UINT32 remoteCaps) override;
    virtual IMS_BOOL NotifyImsRegiRefreshed(IN IMS_SINT32 registeredNetwork) override;
    virtual IMS_BOOL NotifyNetworkChanged(IN IMS_SINT32 changedNetwork) override;
};

#endif
