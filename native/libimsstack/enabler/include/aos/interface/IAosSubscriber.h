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
#ifndef INTERFACE_AOS_SUBSCRIBER_H_
#define INTERFACE_AOS_SUBSCRIBER_H_

#include "ImsTypeDef.h"
#include "AStringArray.h"

class IAosSubscriberListener;
class ISubscriberConfig;

class IAosSubscriber
{
public:
    virtual ~IAosSubscriber(){};

    virtual IMS_BOOL IsReady() const = 0;
    virtual IMS_BOOL IsIsim() const = 0;
    virtual IMS_BOOL IsUsim() const = 0;
    virtual void SetListener(IN IAosSubscriberListener* piListener) = 0;

    virtual const AStringArray& GetConfiguredImpus() const = 0;
    virtual const AStringArray& GetFakeImpus() const = 0;

    virtual const ISubscriberConfig* GetSubscriberConfig(IMS_SINT32 nType = NORMAL) const = 0;

    virtual void CreateTemporaryPublicUserIdForGiba() = 0;
    virtual void ClearTemporaryPublicUserIdForGiba() = 0;
    virtual IMS_BOOL HasValidTemporaryPublicUserIdForGiba() const = 0;
    virtual const AString& GetTemporaryPublicUserIdForGiba() const = 0;

    // Subscriber_StateChanged(nState)
    enum
    {
        NOT_READY = 0,
        READY = 1,

        REFRESH_STARTED = 2,
        REFRESH_COMPLETED = 3,
        REFRESH_FAILED = 4  // refresh is done but credentials are missing.
    };

    // GetSubscriberConfig(nType)
    enum
    {
        NORMAL = 0,
        FAKE = 1
    };

protected:
    friend class AosBuildDirector;
    friend class AosAppContext;
    virtual IMS_BOOL Init() = 0;
    virtual IMS_BOOL CleanUp() = 0;
};

#endif  // INTERFACE_AOS_SUBSCRIBER_H_
