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
#ifndef CONFIGURATION_MANAGER_H_
#define CONFIGURATION_MANAGER_H_

#include "AStringArray.h"

class AppConfig;
class ConfigurationManagerPrivate;
class EngineConfig;
class MediaConfig;
class SipConfig;
class SubscriberConfig;

class ConfigurationManager
{
private:
    ConfigurationManager();

public:
    ~ConfigurationManager();

    ConfigurationManager(IN const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(IN const ConfigurationManager&) = delete;

public:
    // IMS registry: application configuration
    const AppConfig* GetAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId) const;
    AStringArray GetAppIds(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsAppConfigured(IN const AString& strAppId, IN IMS_SINT32 nSlotId) const;
    void RemoveAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId);
    IMS_RESULT StoreAppConfig(
            IN AppConfig* pAppConfig, IN const AString& strAppId, IN IMS_SINT32 nSlotId);

    // Returns the configuration mode (file / xml / db)
    IMS_SINT32 GetConfigMode() const;
    // Returns the locator of the configuration
    const AString& GetConfigLocator() const;

    // Subscriber configuration - impl. defined
    //    This config. includes IMS-related information in the ISIM.
    const SubscriberConfig* GetSubscriberConfig(
            IN const AString& strId, IN IMS_SINT32 nSlotId) const;
    const IMSList<SubscriberConfig*>& GetSubscriberConfigs(IN IMS_SINT32 nSlotId) const;
    // Engine configuration - impl. defined
    //    This config. includes the information for an optional/additional operation
    //    in J281 engine implementation.
    const EngineConfig* GetEngineConfig(IN IMS_SINT32 nSlotId) const;
    // SIP configuration - impl. defined
    //    This config. includes the SIP-specific information for a default UA behavior.
    const SipConfig* GetSipConfig(IN IMS_SINT32 nSlotId) const;
    // Media configuration - impl. defined
    //    This config. includes the media-specific information (SDP for session & capabilities).
    const MediaConfig* GetMediaConfig(IN IMS_SINT32 nSlotId) const;

    static ConfigurationManager* GetInstance();

    //// Load a default basic configuration for IMS client platform
    IMS_BOOL Initialize(IN const AString& strLocator, IN IMS_SINT32 nMode);

    // Invoked by enabler threads
    void InitConfigs(IN IMS_SINT32 nSlotId);
    void RefreshConfigs(IN IMS_SINT32 nSlotId);

public:
    enum
    {
        /// .conf file
        MODE_FILE = 0,
        /// .xml file
        MODE_XML,
        /// database
        MODE_DB,
        /// Code-base
        MODE_CODE
    };

private:
    ConfigurationManagerPrivate* m_pConfigMngrPrivate;
};

#endif
