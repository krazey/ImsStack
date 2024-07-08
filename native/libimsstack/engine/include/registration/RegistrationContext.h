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
#ifndef REGISTRATION_CONTEXT_H_
#define REGISTRATION_CONTEXT_H_

#include "IRegistrationContext.h"

class RegInfoManager;
class RegistrationManager;
class SipConnectionNotifierManager;

/**
 * A context class for providing the singleton instances for registration layer.
 */
class RegistrationContext : public IRegistrationContext
{
private:
    RegistrationContext();
    virtual ~RegistrationContext();

public:
    RegistrationContext(IN const RegistrationContext&) = delete;
    RegistrationContext& operator=(IN const RegistrationContext&) = delete;

public:
    IRegistrationManager* GetRegistrationManager() override;
    IRegInfoManager* GetRegInfoManager() override;
    ISipConnectionNotifierManager* GetSipConnectionNotifierManager() override;

    /**
     * @brief Sets the specific registration context to return its own instances.
     */
    inline void SetRegistrationContext(IN IRegistrationContext* piRegistrationContext)
    {
        m_piRegistrationContext = piRegistrationContext;
    }

    /**
     * @brief Returns a singleton instance of this class.
     */
    static RegistrationContext* GetInstance();
    /**
     * @brief Destroys a singleton instance of this class.
     */
    static void DestroyInstance();

private:
    RegistrationManager* m_pRegistrationManager;
    RegInfoManager* m_pRegInfoManager;
    SipConnectionNotifierManager* m_pScnManager;
    IRegistrationContext* m_piRegistrationContext;

    static RegistrationContext* s_pContext;
};

#endif
