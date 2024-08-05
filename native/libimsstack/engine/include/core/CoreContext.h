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
#ifndef CORE_CONTEXT_H_
#define CORE_CONTEXT_H_

#include "ImsTypeDef.h"

#include "ICoreContext.h"

class ImsCoreProtocol;

/**
 * A context class for providing the singleton instances for core (imscore) layer.
 */
class CoreContext : public ICoreContext
{
private:
    CoreContext();
    virtual ~CoreContext();

public:
    CoreContext(IN const CoreContext&) = delete;
    CoreContext& operator=(IN const CoreContext&) = delete;

public:
    ServiceProtocol* GetImsCoreProtocol() const override;
    CallControlHelper* GetCallControlHelper() override;
    CallerPreferenceManager* GetCallerPreferenceManager() override;

    /**
     * @brief Sets the specific core context to return their own instances.
     */
    inline void SetCoreContext(IN ICoreContext* piCoreContext) { m_piCoreContext = piCoreContext; }

    /**
     * @brief Returns a singleton instance of this class.
     */
    static CoreContext* GetInstance();
    /**
     * @brief Destroys a singleton instance of this class.
     */
    static void DestroyInstance();

private:
    ImsCoreProtocol* m_pImsCoreProtocol;
    CallControlHelper* m_pCallControlHelper;
    CallerPreferenceManager* m_pCallerPreferenceManager;
    ICoreContext* m_piCoreContext;

    static CoreContext* s_pContext;
};

#endif
