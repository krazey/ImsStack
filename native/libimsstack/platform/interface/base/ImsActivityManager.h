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
#ifndef IMS_ACTIVITY_MANAGER_H_
#define IMS_ACTIVITY_MANAGER_H_

#include "IImsActivityController.h"
#include "ImsActivity.h"

class ImsActivityManager
{
public:
    ImsActivityManager();
    inline ~ImsActivityManager() {}

    ImsActivityManager(IN const ImsActivityManager&) = delete;
    ImsActivityManager& operator=(IN const ImsActivityManager&) = delete;

public:
    IMS_BOOL Attach(IN ImsActivity* pActivity);
    void Detach(IN const ImsActivity* pActivity);
    ImsActivity* Get(IN const AString& strActivityName);
    AString GenerateName(IN const AString& strThreadName, IN const AString& strName);
    IMS_BOOL HandleMessage(IN ImsMessage& objMsg);
    IImsActivityController* GetController(IN const AString& strControllerName);

private:
    AString GetActivityNameFromMsg(IN const AString& strTargetName);

private:
    IMS_UINT32 m_nMajorId;
    IMS_UINT32 m_nMinorId;
    ImsList<ImsActivity*> m_objActivities;
};

#endif
