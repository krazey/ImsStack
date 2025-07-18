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
#ifndef IMS_APP_THREAD_H_
#define IMS_APP_THREAD_H_

#include "BaseThread.h"
#include "ImsActivityManager.h"
#include "ImsApp.h"

class ImsAppThread : public BaseThread
{
private:
    class AppInfo
    {
    public:
        inline AppInfo(IN const AString& strName, IN ImsApp_Creator pfnCreator) :
                m_strName(strName),
                m_pfnCreator(pfnCreator)
        {
        }
        inline ~AppInfo() {}

        AppInfo(IN const AppInfo&) = delete;
        AppInfo& operator=(IN const AppInfo&) = delete;

    public:
        AString m_strName;
        ImsApp_Creator m_pfnCreator;
    };

public:
    ImsAppThread();
    ~ImsAppThread() override = default;

    ImsAppThread(IN const ImsAppThread&) = delete;
    ImsAppThread& operator=(IN const ImsAppThread&) = delete;

public:
    inline ImsActivityManager* GetActivityManager() { return &m_objActivityManager; }

    void AddApp(IN ImsApp_Creator pfnCreator, IN const AString& strName);
    void RemoveApp(IN const AString& strName);
    void RemoveAndDestroyApp(IN const AString& strName);

protected:
    virtual void OnAppControl(IN IMS_SINT32 nParam, IN const AppInfo* pAppInfo);

    IMS_BOOL AttachApp(IN ImsApp* pApp);
    void DetachApp(IN const AString& strName, IN IMS_BOOL bDestroy = IMS_FALSE);
    void ControlAppAsync(IN IMS_SINT32 nParam, IN const AString& strName,
            IN ImsApp_Creator pfnCreator = IMS_NULL) const;

private:
    void UnloadAllApp();

    // IRunnable class
    IMS_BOOL Runnable_Run(IN ImsMessage& objMsg) override;

protected:
    /// WParam for application control
    enum
    {
        PARAM_APP_CONTROL_ADD = 1,
        PARAM_APP_CONTROL_REMOVE = 2,
        PARAM_APP_CONTROL_REMOVE_N_DESTROY = 3,

        PARAM_APP_CONTROL_BASE_MAX
    };

private:
    ImsList<ImsApp*> m_objApps;
    ImsActivityManager m_objActivityManager;
};

#endif
