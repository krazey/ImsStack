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
#ifndef INTERFACE_AOS_SUBSCRIBER_MANAGERH_
#define INTERFACE_AOS_SUBSCRIBER_MANAGERH_

#include "ImsTypeDef.h"
#include "interface/IAosSubscriber.h"
#include "interface/IAosSubscriberManagerListener.h"

class IAosSubscriberManager
{
public:
    virtual ~IAosSubscriberManager(){};

    virtual IMS_BOOL IsReady(IN IMS_BOOL bIsFake = IMS_FALSE) const = 0;
    virtual IMS_BOOL IsIsim() const = 0;
    virtual IMS_BOOL IsUsim() const = 0;

    virtual void AddListener(IN IAosSubscriberManagerListener* piListener) = 0;
    virtual void RemoveListener(IN IAosSubscriberManagerListener* piListener) = 0;
    virtual void AddListenerForMonitor(IN IAosSubscriberManagerListener* piListener) = 0;
    virtual void RemoveListenerForMonitor(IN IAosSubscriberManagerListener* piListener) = 0;

    virtual const AStringArray& GetConfiguredImpus() const = 0;
    virtual const AStringArray& GetOrderedImpus() const = 0;
    virtual const AStringArray& GetConfiguredImpusForFake() const = 0;
    virtual const AStringArray& GetFakeImpus() const = 0;

    virtual const ISubscriberConfig* GetSubscriberConfig(
            IN IMS_SINT32 nType = IAosSubscriber::NORMAL) const = 0;

protected:
    friend class AosBuildDirector;
    friend class AosAppContext;
};

#endif  // INTERFACE_AOS_SUBSCRIBER_MANAGERH_