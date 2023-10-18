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
#ifndef CONFIG_ENABLER_H_
#define CONFIG_ENABLER_H_

#include "Enabler.h"

class ConfigApp;

class ConfigEnabler : public Enabler
{
public:
    explicit ConfigEnabler(IN IMS_SINT32 nSlotId);
    virtual ~ConfigEnabler();

    ConfigEnabler(IN const ConfigEnabler&) = delete;
    ConfigEnabler& operator=(IN const ConfigEnabler&) = delete;

private:
    // IEnabler class
    void Start() override;
    void Stop() override;

private:
    ConfigApp* m_pConfigApp;

    IMS_BOOL m_bUseResetWhenClosingSipTcpConnection;
};

#endif
