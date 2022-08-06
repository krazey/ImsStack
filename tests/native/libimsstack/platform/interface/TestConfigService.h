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
#ifndef TEST_CONFIG_SERVICE_H_
#define TEST_CONFIG_SERVICE_H_

#include "MockICarrierConfig.h"
#include "ServiceConfig.h"

class TestConfigService : public ConfigService
{
public:
    inline TestConfigService() :
            ConfigService(),
            m_piCarrierConfig(&m_objCarrierConfig)
    {
    }

    ICarrierConfig* GetCarrierConfig(IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piCarrierConfig;
    }
    void LoadCarrierConfig(IN IMS_SINT32 /*nSlotId*/) override {}

    inline MockICarrierConfig& GetMockCarrierConfig() { return m_objCarrierConfig; }
    inline void SetCarrierConfig(IN ICarrierConfig* piCarrierConfig)
    {
        m_piCarrierConfig = piCarrierConfig;
    }

private:
    MockICarrierConfig m_objCarrierConfig;

    ICarrierConfig* m_piCarrierConfig;
};

#endif
