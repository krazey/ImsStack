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
#ifndef ENABLER_THREAD_H_
#define ENABLER_THREAD_H_

#include "ImsAppThread.h"
#include "ImsMessageDef.h"

class EnablerFactory;

class EnablerThread : public ImsAppThread
{
public:
    EnablerThread(IN EnablerFactory* pEnablerFactory, IN IMS_SINT32 nSlotId);
    inline virtual ~EnablerThread() {}

public:
    inline IMS_SINT32 GetSlotId() const { return m_nSlotId; }
    void ControlEnablers(IN IMS_SINT32 nCtrlFlags);

protected:
    // ImsAppThread class
    IMS_BOOL Initialize() override;

    IMS_BOOL OnStart(IN ImsMessage& objMsg) override;
    IMS_BOOL OnTerminate(IN ImsMessage& objMsg) override;
    IMS_BOOL OnMessage(IN ImsMessage& objMsg) override;

    inline IMS_BOOL IsControlSet(IN IMS_SINT32 nCtrlFlags, IN IMS_SINT32 nCtrlFlag) const
    {
        return (nCtrlFlags & nCtrlFlag) == nCtrlFlag;
    }
    inline IMS_SINT32 GetState() const { return m_nState; }

    void InitializeGlobals();
    void UninitializeGlobals();

    void ControlEnablersInternal(IN IMS_SINT32 nCtrlFlags);
    void NotifyEnablerStartCompleted();
    void SetState(IN IMS_SINT32 nState);
    IMS_BOOL StartEnablers();
    void StopEnablers();

public:
    /// Bitmask for operations
    enum
    {
        CONTROL_NONE = 0x0000,

        CONTROL_CREATE = 0x0001,
        CONTROL_DESTROY = 0x0002,

        CONTROL_START = 0x0010,
        CONTROL_STOP = 0x0020,

        CONTROL_ALL = (CONTROL_CREATE | CONTROL_DESTROY | CONTROL_START | CONTROL_STOP)
    };

private:
    /// Enabler's state
    enum
    {
        STATE_INACTIVE = 0,
        STATE_ACTIVE = 1
    };

    enum
    {
        TMSG_CONTROL_ENABLERS = IMS_MSG_THREAD_INTERNAL_BASE
    };

    EnablerFactory* m_pEnablerFactory;
    IMS_SINT32 m_nSlotId;
    IMS_SINT32 m_nState;
};

#endif
