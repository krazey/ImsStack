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
#ifndef IMS_ACTIVITY_H_
#define IMS_ACTIVITY_H_

#include "IThread.h"

#include "IImsActivityController.h"

class ImsActivity
{
public:
    // When giving the activity name, the name MUST not contain the dot ('.').
    ImsActivity(IN const AString& strName = AString::ConstNull());
    virtual ~ImsActivity();

    ImsActivity(IN const ImsActivity&) = delete;
    ImsActivity& operator=(IN const ImsActivity&) = delete;

public:
    inline const AString& GetName() const { return m_strName; }
    inline IMS_SINT32 GetSlotId() const
    {
        return (m_piThread == IMS_NULL) ? IMS_SLOT_ANY : m_piThread->GetSlotId();
    }
    virtual IImsActivityController* GetController() = 0;
    IMS_BOOL PostMessage(IN ImsMessage& objMsg);
    IMS_BOOL PostMessage(IN IMS_UINT32 nMsg, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam);

protected:
    virtual IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) = 0;

private:
    static AString GetOwnerThreadName(IN const AString& strTargetName);

private:
    friend class ImsActivityManager;

    AString m_strName;
    IThread* m_piThread;
};

#endif
