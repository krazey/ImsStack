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
#ifndef SUBSCRIBER_TRACKER_H_
#define SUBSCRIBER_TRACKER_H_

#include "ImsList.h"
#include "ImsMap.h"

#include "ISubscriberInfoListener.h"
#include "SipAddress.h"

class IMutex;

class SubscriberTracker : public ISubscriberInfoListener
{
private:
    SubscriberTracker();

public:
    ~SubscriberTracker() override;

    SubscriberTracker(IN const SubscriberTracker&) = delete;
    SubscriberTracker& operator=(IN const SubscriberTracker&) = delete;

public:
    const AString& GetSubscriberId(IN IMS_SINT32 nSlotId, IN const AString& strAor) const;
    const AString& GetSubscriberId(IN IMS_SINT32 nSlotId, IN const SipAddress* pAor) const;

    static SubscriberTracker* GetInstance();

protected:
    // ISubscriberInfoListener class
    void SubscriberInfo_UpdateImpu(IN IMS_SINT32 nSlotId, IN const AString& strId,
            IN const AString& strOld, IN const AString& strNew) override;

private:
    ImsMap<AString, ImsList<SipAddress*>>* GetSubscribers(IN IMS_SINT32 nSlotId) const;
    void Initialize();
    void InitForSlot(IN IMS_SINT32 nSlotId);

private:
    friend class EngineLoader;

    IMutex* m_piLock;
    // < SubscriberConfig id, IMPUs >
    // DEFAULT_ID : "default"
    ImsMap<AString, ImsList<SipAddress*>>* m_pSubscriberMaps;
};

#endif
