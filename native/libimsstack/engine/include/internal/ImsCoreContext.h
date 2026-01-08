/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef IMS_CORE_CONTEXT_H_
#define IMS_CORE_CONTEXT_H_

#include "ImsTypeDef.h"

#include "IImsCoreContext.h"

class Configuration;
class ImsCoreProtocol;
class RegInfoManager;
class RegistrationManager;
class ServiceManager;
class SipConnectionNotifierManager;

/**
 * A context class for providing the singleton instances for imscore layer.
 */
class ImsCoreContext : public IImsCoreContext
{
private:
    ImsCoreContext();
    ~ImsCoreContext() override;

public:
    ImsCoreContext(IN const ImsCoreContext&) = delete;
    ImsCoreContext& operator=(IN const ImsCoreContext&) = delete;

public:
    // service
    IConfiguration* GetConfiguration() const override;
    IServiceManager* GetServiceManager() override;

    // registration
    IRegistrationManager* GetRegistrationManager() override;
    IRegInfoManager* GetRegInfoManager() override;

    // core
    ServiceProtocol* GetImsCoreProtocol() const override;
    CallControlHelper* GetCallControlHelper() override;
    CallerPreferenceManager* GetCallerPreferenceManager() override;
    ISipConnectionNotifierManager* GetSipConnectionNotifierManager() override;

    /**
     * @brief Sets the specific ims core context to return their own instances.
     */
    inline void SetImsCoreContext(IN IImsCoreContext* piImsCoreContext)
    {
        m_piImsCoreContext = piImsCoreContext;
    }

    /**
     * @brief Returns a singleton instance of this class.
     */
    static ImsCoreContext* GetInstance();
    /**
     * @brief Destroys a singleton instance of this class.
     */
    static void DestroyInstance();

private:
    // service
    Configuration* m_pConfiguration;
    ServiceManager* m_pServiceManager;

    // registration
    RegistrationManager* m_pRegistrationManager;
    RegInfoManager* m_pRegInfoManager;

    // core
    ImsCoreProtocol* m_pImsCoreProtocol;
    CallControlHelper* m_pCallControlHelper;
    CallerPreferenceManager* m_pCallerPreferenceManager;
    SipConnectionNotifierManager* m_pScnManager;

    IImsCoreContext* m_piImsCoreContext;

    static ImsCoreContext* s_pContext;
};

#endif
