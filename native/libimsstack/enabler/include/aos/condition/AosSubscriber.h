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

class AosSubscriber : public IAosSubscriber, public IAosSubscriberManagerListener
{
public:
    explicit AosSubscriber(IN IAosAppContext* piAppContext);
    ~AosSubscriber() override;

    // IAosSubscriber
    IMS_BOOL IsReady() const override;
    IMS_BOOL IsIsim() const override;
    IMS_BOOL IsUsim() const override;
    void SetListener(IN IAosSubscriberListener* piListener) override;
    const AStringArray& GetConfiguredImpus() const override;
    const AStringArray& GetFakeImpus() const override;
    const ISubscriberConfig* GetSubscriberConfig(IN IMS_SINT32 nType = NORMAL) const override;
    SimState GetSimState() const override;

    void CreateTemporaryPublicUserIdForGiba() override;
    void ClearTemporaryPublicUserIdForGiba() override;
    IMS_BOOL HasValidTemporaryPublicUserIdForGiba() const override;
    const AString& GetTemporaryPublicUserIdForGiba() const override;

protected:
    // IAosSubscriber
    IMS_BOOL Init() override;
    IMS_BOOL CleanUp() override;
    void Notify(IN IMS_UINT32 nState);

    // IAosSubscriberManagerListener
    void AosSubscriberManager_NotifyState(IN IMS_UINT32 nState) override;

protected:
    IAosAppContext* m_piAppContext;
    IAosSubscriberManager* m_piSubscriberManager;
    IMS_SINT32 m_nSlotId;
    IAosSubscriberListener* m_piListener;
    AosRegistrationType m_eRegType;
    AString m_strTempPuidForGiba;

    AString m_strTag;
};

#endif  // AOS_SUBSCRIBER_H_
