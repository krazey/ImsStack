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
#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include "IAppConfig.h"
#include "ImsRegistry.h"

class AppConfigPrivate;
class CoreServiceConfig;

class AppConfig : public IAppConfig
{
public:
    explicit AppConfig(IN const AString& strAppId);
    AppConfig(IN const AppConfig& other);
    virtual ~AppConfig();

public:
    AppConfig& operator=(IN const AppConfig& other);

public:
    // IAppConfig interface
    const AString& GetAppId() const override;
    const ICoreServiceConfig* GetCoreServiceConfig(IN const AString& strServiceId) const override;
    ImsRegistry* ToRegistry() const override;

    IMS_BOOL Create(IN const ImsRegistry& objRegistry, IN IMS_SINT32 nSlotId);
    IMS_BOOL Equals(IN const AString& strAppId) const;
    IMS_BOOL IsStreamMediaSupported() const;
    IMS_BOOL IsStreamMediaAudioSupported() const;
    IMS_BOOL IsStreamMediaVideoSupported() const;
    IMS_BOOL IsStreamMediaTextSupported() const;
    IMS_BOOL IsFramedMediaSupported() const;
    const AStringArray& GetFramedMediaMimeTypes() const;
    IMS_BOOL IsFramedMediaMaxSizePresent() const;
    IMS_UINT32 GetFramedMediaMaxSize() const;
    IMS_BOOL IsBasicMediaSupported() const;
    const AStringArray& GetBasicMediaMimeTypes() const;
    IMS_BOOL IsEventPackageSupported(IN const AString& strEvent) const;
    const AStringArray& GetSupportedEventPackages() const;
    IMS_BOOL IsHeaderReadable(IN const AString& strHeader) const;
    IMS_BOOL IsHeaderWritable(IN const AString& strHeader) const;
    AStringArray GetCapabilitySdps(IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType) const;
    const CoreServiceConfig* GetCoreServiceConfigEx(IN const AString& strServiceId) const;
    const ImsList<CoreServiceConfig*>& GetCoreServiceConfigs() const;

private:
    AppConfigPrivate* m_pConfigPrivate;
};

#endif
