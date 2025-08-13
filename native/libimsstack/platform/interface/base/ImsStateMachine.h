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
#ifndef IMS_STATE_MACHINE_H_
#define IMS_STATE_MACHINE_H_

#include "ImsStateMap.h"
#include "ImsStateObject.h"

class ImsStateMachine : public ImsStateObject
{
    DECLARE_STATE_MAP_BASE()

public:
    explicit ImsStateMachine(IN IMS_UINT32 nState = IMS_INVALID_STATE);
    ~ImsStateMachine() override = default;

    ImsStateMachine(IN const ImsStateMachine&) = delete;
    ImsStateMachine& operator=(IN const ImsStateMachine&) = delete;

    IMS_BOOL OnStateMessage(IN ImsMessage& objMsg);

protected:
    IMS_BOOL SetState(IN IMS_UINT32 nState);
    inline IMS_UINT32 GetState() const { return m_nState; }
    inline IMS_UINT32 GetOldState() const { return m_nOldState; }

private:
    IMS_UINT32 m_nState;
    IMS_UINT32 m_nOldState;
};

#endif
