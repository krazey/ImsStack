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
#ifndef IMS_SERVICE_H_
#define IMS_SERVICE_H_

#include "ImsActivity.h"
#include "ImsStateMap.h"
#include "ImsStateObject.h"

class ImsService : public ImsActivity, public ImsStateObject
{
    DECLARE_STATE_MAP_BASE()

public:
    explicit ImsService(IN const AString& strName);
    inline virtual ~ImsService() {}

protected:
    inline IImsActivityController* GetController() override { return IMS_NULL; }

    inline virtual IMS_BOOL OnPreprocess(IN ImsMessage& /*objMsg*/) { return IMS_FALSE; }
    inline virtual IMS_BOOL OnMessage(IN ImsMessage& /*objMsg*/) { return IMS_FALSE; }
    inline virtual IMS_BOOL OnPostprocess(IN ImsMessage& /*objMsg*/) { return IMS_FALSE; }

    IMS_BOOL SetState(IN IMS_UINT32 nState);
    inline IMS_UINT32 GetState() { return m_nState; }
    inline IMS_UINT32 GetOldState() { return m_nOldState; }

    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override final;

private:
    IMS_BOOL OnStateMsgProcess(IN ImsMessage& objMsg);

private:
    IMS_UINT32 m_nState;
    IMS_UINT32 m_nOldState;
};

#endif
