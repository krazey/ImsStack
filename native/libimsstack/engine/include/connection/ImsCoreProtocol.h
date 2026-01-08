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
#ifndef IMS_CORE_PROTOCOL_H_
#define IMS_CORE_PROTOCOL_H_

#include "ImsCore.h"
#include "ServiceProtocol.h"

class AppConfig;
class CoreServiceConfig;

class ImsCoreProtocol : public ServiceProtocol
{
public:
    inline ImsCoreProtocol() :
            ServiceProtocol()
    {
    }
    ~ImsCoreProtocol() override = default;

public:
    ImsCoreProtocol(IN const ImsCoreProtocol&) = delete;
    ImsCoreProtocol& operator=(IN const ImsCoreProtocol&) = delete;

private:
    // ServiceProtocol class
    IService* CreateService(IN const AString& strAppId, IN const AString& strServiceId,
            IN const AString& strUserId) override;
    inline const IMS_CHAR* GetConnectionScheme() const override
    {
        return ImsCore::CONNECTION_SCHEME;
    }

    static IMS_BOOL IsRegistryConsistent(
            IN const AppConfig* pAppConfig, IN const CoreServiceConfig* pServiceConfig);
};

#endif
