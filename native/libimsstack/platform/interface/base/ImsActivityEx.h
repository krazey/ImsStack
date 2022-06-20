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
#ifndef IMS_ACTIVITY_EX_H_
#define IMS_ACTIVITY_EX_H_

#include "ImsActivity.h"

class ImsActivityEx : public ImsActivity
{
public:
    // When giving the activity name, the name MUST not contain the dot ('.').
    inline ImsActivityEx(IN const AString& strName = AString::ConstNull()) :
            ImsActivity(strName)
    {
    }
    inline virtual ~ImsActivityEx() {}

    ImsActivityEx(IN const ImsActivityEx&) = delete;
    ImsActivityEx& operator=(IN const ImsActivityEx&) = delete;

protected:
    inline IImsActivityController* GetController() override { return IMS_NULL; }
    inline virtual IMS_BOOL OnMessage(IN ImsMessage& /*objMsg*/) { return IMS_FALSE; }

private:
    // ImsActivity
    inline IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override { return OnMessage(objMsg); }
};

#endif
