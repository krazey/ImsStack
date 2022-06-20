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
#ifndef BASE_THREAD_H_
#define BASE_THREAD_H_

#include "IThread.h"

class BaseThread : public IRunnable
{
public:
    BaseThread();
    virtual ~BaseThread();

    BaseThread(IN const BaseThread&) = delete;
    BaseThread& operator=(IN const BaseThread&) = delete;

public:
    inline const AString& GetName() const { return m_strName; }
    inline IThread* GetThread() const { return m_piThread; }

    IMS_BOOL Start(IN const AString& strName, IN IMS_SINT32 nSlotId);
    void Terminate();

protected:
    // IRunnable class
    IMS_BOOL Runnable_Run(IN ImsMessage& objMsg) override;

    inline virtual IMS_BOOL Initialize() { return IMS_TRUE; }
    inline virtual void Uninitialize() {}

    inline virtual IMS_BOOL OnStart(IN ImsMessage& /*objMsg*/) { return IMS_TRUE; }
    inline virtual IMS_BOOL OnTerminate(IN ImsMessage& /*objMsg*/) { return IMS_TRUE; }
    inline virtual IMS_BOOL OnMessage(IN ImsMessage& /*objMsg*/) { return IMS_FALSE; }

    IMS_BOOL IsThreadMessage(IN ImsMessage& objMsg) const;

private:
    AString m_strName;
    IThread* m_piThread;
};

#endif
