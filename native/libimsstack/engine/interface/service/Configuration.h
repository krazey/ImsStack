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
#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include "IConfiguration.h"

/**
 * @brief This class implements the configuration interface for each IMS services.
 */
class Configuration : public IConfiguration
{
public:
    Configuration() = default;
    ~Configuration() override = default;

    Configuration(IN const Configuration&) = delete;
    Configuration& operator=(IN const Configuration&) = delete;

public:
    AStringArray GetLocalAppIds(IN IMS_SINT32 nSlotId) const override;
    const IAppConfig* GetAppConfig(
            IN const AString& strAppId, IN IMS_SINT32 nSlotId) const override;
    IMS_BOOL HasAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId) const override;
    void RemoveAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId) override;
    IMS_RESULT SetAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId) override;
    IMS_RESULT SetAppConfig(IN const AString& strAppId, IN const AString& strClassName,
            IN const ImsRegistry& objRegistry, IN IMS_SINT32 nSlotId) override;
    const IMediaConfig* GetMediaConfig(IN IMS_SINT32 nSlotId) const override;
    const ISipConfig* GetSipConfig(IN IMS_SINT32 nSlotId) const override;
    ISubscriberConfig* GetSubscriberConfig(
            IN IMS_SINT32 nSlotId, IN const AString& strId = AString::ConstNull()) const override;
    IMS_UINT32 GetTraceModule(IN IMS_SINT32 nSlotId) const override;
    IMS_UINT32 GetTraceOption(IN IMS_SINT32 nSlotId) const override;
    IMS_BOOL IsServerInfoHiddenInLog(IN IMS_SINT32 nSlotId) const override;
    void InitConfigs(IN IMS_SINT32 nSlotId) override;
    void RefreshConfigs(IN IMS_SINT32 nSlotId) override;
};

#endif
