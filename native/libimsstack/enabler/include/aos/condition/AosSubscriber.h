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
#ifndef AOS_SUBSCRIBER_H_
#define AOS_SUBSCRIBER_H_

#include "interface/IAosSubscriberManagerListener.h"
#include "interface/IAosSubscriber.h"

class IAosAppContext;
class IAosSubscriberManager;
class AosStaticProfile;
class AosUtil;
class AosServicePhoneListener;

class AosSubscriber : public IAosSubscriber, public IAosSubscriberManagerListener
{
public:
    AosSubscriber(IN IAosAppContext* piAppContext);
    virtual ~AosSubscriber();

    // IAosSubscriber
    IMS_BOOL IsReady() const override;
    void SetListener(IN IAosSubscriberListener* piListener) override;
    const AStringArray& GetConfiguredImpus() const override;
    const AStringArray& GetFakeImpus() const override;
    const ISubscriberConfig* GetSubscriberConfig(IN IMS_SINT32 nType = NORMAL) const override;

private:
    // IAosSubscriber
    void Init() override;
    void CleanUp() override;
    void Notify(IN IMS_UINT32 nState);

    // IAosSubscriberManagerListener
    void AosSubscriberManager_NotifyState(IN IMS_UINT32 nState) override;

private:
    IAosAppContext* m_piAppContext;
    IAosSubscriberManager* m_piSubscriberManager;
    IMS_SINT32 m_nSlotId;
    IAosSubscriberListener* m_piListener;

    AosRegistrationType m_eRegType;

    AString m_strTag;

private:
    friend class AosSubscriberTest;
};

#endif  // AOS_SUBSCRIBER_H_