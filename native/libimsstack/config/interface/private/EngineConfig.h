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
#ifndef ENGINE_CONFIG_H_
#define ENGINE_CONFIG_H_

#include "private/ConfigBase.h"

class EngineConfig : public ConfigBase
{
public:
    explicit EngineConfig(IN IMS_SINT32 nSlotId);
    virtual ~EngineConfig();

    EngineConfig(IN const EngineConfig&) = delete;
    EngineConfig& operator=(IN const EngineConfig&) = delete;

public:
    // ConfigBase class
    void Refresh() override;

    inline IMS_UINT32 GetTraceOption() const { return m_nTraceOption; }
    inline IMS_UINT32 GetTraceModule() const { return m_nTraceModule; }

protected:
    IMS_BOOL ReadFrom() override;

private:
    IMS_UINT32 m_nTraceOption;
    IMS_UINT32 m_nTraceModule;
};

#endif
