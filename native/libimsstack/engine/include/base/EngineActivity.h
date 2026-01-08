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
#ifndef ENGINE_ACTIVITY_H_
#define ENGINE_ACTIVITY_H_

#include "ImsActivity.h"
#include "ImsMessageDef.h"

class EngineActivity : public ImsActivity
{
public:
    inline explicit EngineActivity(IN const AString& strName = AString::ConstNull()) :
            ImsActivity(strName)
    {
    }
    ~EngineActivity() override = default;

    EngineActivity(IN const EngineActivity&) = delete;
    EngineActivity& operator=(IN const EngineActivity&) = delete;

protected:
    // ImsActivity class
    inline IImsActivityController* GetController() override { return IMS_NULL; }
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    virtual void OnDestroy();

protected:
    enum
    {
        /// Connection Message - System messages: resource release, ...
        /// IMS_MSG_USER ~ (IMS_MSG_USER + 100)
        AMSG_DESTROY = IMS_MSG_USER,

        /// Connection Message - User messages: start message of user messages for derived classes
        AMSG_USER = (IMS_MSG_USER + 101)
    };
};

#endif
