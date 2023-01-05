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
#ifndef SERVICE_IMS_CONFIG_H_
#define SERVICE_IMS_CONFIG_H_

#include "ICarrierConfig.h"
#include "ImsMessage.h"
#include "PlatformService.h"

class ConfigServicePrivate;

class ConfigService : public PlatformService
{
public:
    ConfigService();
    ConfigService(IN const ConfigService&) = delete;
    ConfigService& operator=(IN const ConfigService&) = delete;

protected:
    virtual ~ConfigService();

public:
    virtual ICarrierConfig* GetCarrierConfig(IN IMS_SINT32 nSlotId);
    virtual void LoadCarrierConfig(IN IMS_SINT32 nSlotId);

    void DispatchServiceMessage(IN ImsMessage& objMsg);
    static ConfigService* GetConfigService();

private:
    ConfigServicePrivate* m_pPrivate;
};

#endif
