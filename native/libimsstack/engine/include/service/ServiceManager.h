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
#ifndef SERVICE_MANAGER_H_
#define SERVICE_MANAGER_H_

#include "IServiceManager.h"

class IMutex;
class Service;

class ServiceManager : public IServiceManager
{
public:
    ServiceManager();
    ~ServiceManager() override;

    ServiceManager(IN const ServiceManager&) = delete;
    ServiceManager& operator=(IN const ServiceManager&) = delete;

public:
    IMS_BOOL AttachService(IN Service* pService) override;
    void DetachService(IN Service* pService) override;
    Service* GetService(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId) const override;
    inline const ImsList<Service*>& GetServices() const override { return m_objServices; }
    ImsList<Service*> GetServices(IN IMS_SINT32 nSlotId) const override;

private:
    // IServiceCloseListener interface
    inline void ServiceClosed(IN Service* pService) override { DetachService(pService); }

private:
    IMutex* m_piLock;
    ImsList<Service*> m_objServices;
};

#endif
