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
#ifndef IMS_FRAMEWORK_H_
#define IMS_FRAMEWORK_H_

#include "ImsAppThread.h"

class IFrameworkThreadListener;
class IMutex;

class ImsFramework : public ImsAppThread
{
public:
    ImsFramework();
    ~ImsFramework() override;

    ImsFramework(IN const ImsFramework&) = delete;
    ImsFramework& operator=(IN const ImsFramework&) = delete;

public:
    void AddListener(IN IFrameworkThreadListener* piListener);
    void RemoveListener(IN const IFrameworkThreadListener* piListener);

protected:
    IMS_BOOL OnStart(IN ImsMessage& objMsg) override;
    IMS_BOOL OnTerminate(IN ImsMessage& objMsg) override;
    IMS_BOOL OnMessage(IN ImsMessage& objMsg) override;

private:
    void NotifyThreadStarted();
    void NotifyThreadTerminated();

private:
    IMutex* m_piLock;
    ImsList<IFrameworkThreadListener*> m_objListeners;
};

#endif
