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
#ifndef OS_PTHREAD_H_
#define OS_PTHREAD_H_

#include "ImsThread.h"
#include "IThreadImpListener.h"

class OsPthreadPrivate;

class OsPthread : public ImsThread
{
public:
    OsPthread();
    virtual ~OsPthread();

    OsPthread(IN const OsPthread&) = delete;
    OsPthread& operator=(IN const OsPthread&) = delete;

public:
    // IThread class
    IMS_BOOL Activate() override;
    void Deactivate() override;
    IMS_BOOL Equals(IN const IThread* piThread) const override;
    const AString& GetName() const override;
    IMS_BOOL IsRunning() const override;
    // Only for IThreadListener
    void SetRunnable(IN IRunnable* piListener) override;
    IMS_BOOL PostMessageI(IN ImsMessage& objMsg) override;
    IMS_BOOL PostMessageI(IN IMS_UINT32 nMsg, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;

    // ImsThread class
    IMS_BOOL Create(IN const AString& strName) override;
    IMS_ULONG GetThreadId() const override;

    static IMS_ULONG GetCurrentThreadId();

    virtual IMS_ULONG Run();
    virtual void SetImpListener(IN IThreadImpListener* piListener);

protected:
    void CleanUp();

private:
    OsPthreadPrivate* m_pThreadP;
    IThreadImpListener* m_piImpListener;
};

#endif
