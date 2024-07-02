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
#ifndef SIP_CORE_CONTEXT_H_
#define SIP_CORE_CONTEXT_H_

#include "ImsTypeDef.h"

#include "ISipCoreContext.h"

class SipProtocol;

/**
 * A context class for providing the singleton instances for sipcore layer.
 */
class SipCoreContext : public ISipCoreContext
{
private:
    SipCoreContext();
    virtual ~SipCoreContext();

public:
    SipCoreContext(IN const SipCoreContext&) = delete;
    SipCoreContext& operator=(IN const SipCoreContext&) = delete;

public:
    Protocol* GetSipProtocol() const override;

    /**
     * @brief Sets the specific SIP core context to return their own instances.
     */
    inline void SetSipCoreContext(IN ISipCoreContext* piSipCoreContext)
    {
        m_piSipCoreContext = piSipCoreContext;
    }

    /**
     * @brief Returns a singleton instance of this class.
     */
    static SipCoreContext* GetInstance();
    /**
     * @brief Destroys a singleton instance of this class.
     */
    static void DestroyInstance();

private:
    SipProtocol* m_pSipProtocol;
    ISipCoreContext* m_piSipCoreContext;

    static SipCoreContext* s_pContext;
};

#endif
