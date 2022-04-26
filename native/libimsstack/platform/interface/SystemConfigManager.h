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
#ifndef SYSTEM_CONFIG_MANAGER_H_
#define SYSTEM_CONFIG_MANAGER_H_

#include "IMSList.h"
#include "IMSMap.h"
#include "ImsMessage.h"
#include "ImsMessageDef.h"
#include "SystemConfig.h"

class IMutex;
class ISystemConfigListener;
class IThread;

class SystemConfigManager
    : public ImsMessage::IMessageCallback
{
private:
    SystemConfigManager();
    ~SystemConfigManager();

public:
    SystemConfigManager(IN const SystemConfigManager&) = delete;
    SystemConfigManager& operator=(IN const SystemConfigManager&) = delete;

public:
    IMS_SINT32 GetActiveSlotId() const;

    const SystemConfig* GetConfig(IN IMS_SINT32 nSlotId) const;
    const SystemConfig* GetOldConfig(IN IMS_SINT32 nSlotId) const;

    void AddListener(IN ISystemConfigListener* piListener);
    void RemoveListener(IN ISystemConfigListener* piListener);

    static SystemConfigManager* GetInstance();

    static void CacheSystemFeatures();

private:
    // ImsMessage::IMessageCallback class
    virtual void MessageCallback_OnMessage(IN ImsMessage& objMsg);

    void ClearAllConfigs();
    IMS_BOOL HasListener(IN ISystemConfigListener* piListener) const;
    void NotifyConfigChanged(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId);
    void PostConfigChanged(IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount,
            IN const __SystemConfig* pSysConfig);
    void StoreConfig(IN IMS_SINT32 nCount, IN const __SystemConfig* pSysConfig);

    // Invoked by ImsFramework
    // It's used to handle any events asynchronously.
    void SetProxyThread(IN IThread* piThread);

    // Invoked by __SystemConfigWrapper
    // It's used to update the system configuration after detecting operator.
    void UpdateSystemConfig(IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount,
            IN const __SystemConfig* pSysConfig);

private:
    // IMSCore.cpp
    friend class __SystemConfigWrapper;
    friend class IMSFramework;

    enum
    {
        OLD_CONFIG_INDEX_BASE = 10
    };

    enum
    {
        TMSG_CONFIG_CHANGED = IMS_MSG_THREAD_USER_BASE,
        TMSG_FEATURE_PERMISSIONS_CHANGED
    };

    IMutex* m_piLockForConfigs;
    IMutex* m_piLockForListeners;

    // Framework thread.
    // It's used to remove any blocking issues of main thread.
    IThread* m_piProxyThread;

    // < config-id(slot id), system-config >
    IMSMap<IMS_SINT32, SystemConfig*> m_objSystemConfigs;
    IMSList<ISystemConfigListener*> m_objListeners;
};

#endif
