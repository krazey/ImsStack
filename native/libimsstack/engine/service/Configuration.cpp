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
#include "ServiceMemory.h"
#include "ServiceUtil.h"

#include "private/AppConfig.h"
#include "private/ConfigurationManager.h"
#include "private/EngineConfig.h"
#include "private/ImsRegistryLoader.h"
#include "private/MediaConfig.h"
#include "private/SipConfig.h"
#include "private/SubscriberConfig.h"

#include "ConfigLoader.h"
#include "Configuration.h"

PUBLIC VIRTUAL AStringArray Configuration::GetLocalAppIds(IN IMS_SINT32 nSlotId) const
{
    return ConfigurationManager::GetInstance()->GetAppIds(nSlotId);
}

PUBLIC VIRTUAL const IAppConfig* Configuration::GetAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId) const
{
    return ConfigurationManager::GetInstance()->GetAppConfig(strAppId, nSlotId);
}

PUBLIC VIRTUAL IMS_BOOL Configuration::HasAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId) const
{
    return ConfigurationManager::GetInstance()->IsAppConfigured(strAppId, nSlotId);
}

PUBLIC VIRTUAL void Configuration::RemoveAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId)
{
    ConfigurationManager::GetInstance()->RemoveAppConfig(strAppId, nSlotId);
}

PUBLIC VIRTUAL IMS_RESULT Configuration::SetAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId)
{
    if (strAppId.GetLength() == 0)
    {
        return IMS_FAILURE;
    }

    ImsRegistry objRegistry;

    if (!ImsRegistryLoader::GetRegistry(strAppId, objRegistry))
    {
        return IMS_FAILURE;
    }

    AppConfig* pAppConfig = new AppConfig(strAppId);

    if (pAppConfig == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!pAppConfig->Create(objRegistry, nSlotId))
    {
        delete pAppConfig;
        return IMS_FAILURE;
    }

    if (ConfigurationManager::GetInstance()->StoreAppConfig(pAppConfig, strAppId, nSlotId) !=
            IMS_SUCCESS)
    {
        delete pAppConfig;
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT Configuration::SetAppConfig(IN const AString& strAppId,
        IN const AString& strClassName, IN const ImsRegistry& objRegistry, IN IMS_SINT32 nSlotId)
{
    if (strAppId.GetLength() == 0)
    {
        return IMS_FAILURE;
    }

    if (strClassName.GetLength() == 0)
    {
        return IMS_FAILURE;
    }

    if (objRegistry.GetCount() == 0)
    {
        return IMS_FAILURE;
    }

    AppConfig* pAppConfig = new AppConfig(strAppId);

    if (pAppConfig == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!pAppConfig->Create(objRegistry, nSlotId))
    {
        delete pAppConfig;
        return IMS_FAILURE;
    }

    if (ConfigurationManager::GetInstance()->StoreAppConfig(pAppConfig, strAppId, nSlotId) !=
            IMS_SUCCESS)
    {
        delete pAppConfig;
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL const IMediaConfig* Configuration::GetMediaConfig(IN IMS_SINT32 nSlotId) const
{
    return ConfigurationManager::GetInstance()->GetMediaConfig(nSlotId);
}

PUBLIC VIRTUAL const ISipConfig* Configuration::GetSipConfig(IN IMS_SINT32 nSlotId) const
{
    return ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
}

PUBLIC VIRTUAL ISubscriberConfig* Configuration::GetSubscriberConfig(
        IN IMS_SINT32 nSlotId, IN const AString& strId /*= AString::ConstNull()*/) const
{
    const ISubscriberConfig* piSubsConfig;

    if (strId.IsNULL())
    {
        piSubsConfig = ConfigurationManager::GetInstance()->GetSubscriberConfig(
                SubscriberConfig::GetDefaultId(), nSlotId);
    }

    piSubsConfig = ConfigurationManager::GetInstance()->GetSubscriberConfig(strId, nSlotId);

    return const_cast<ISubscriberConfig*>(piSubsConfig);
}

PUBLIC VIRTUAL IMS_UINT32 Configuration::GetTraceModule(IN IMS_SINT32 nSlotId) const
{
    const EngineConfig* pEngineConfig =
            ConfigurationManager::GetInstance()->GetEngineConfig(nSlotId);

    if (pEngineConfig == IMS_NULL)
    {
        return 0;
    }

    return pEngineConfig->GetTraceModule();
}

PUBLIC VIRTUAL IMS_UINT32 Configuration::GetTraceOption(IN IMS_SINT32 nSlotId) const
{
    const EngineConfig* pEngineConfig =
            ConfigurationManager::GetInstance()->GetEngineConfig(nSlotId);

    if (pEngineConfig == IMS_NULL)
    {
        return 0;
    }

    return pEngineConfig->GetTraceOption();
}

PUBLIC VIRTUAL IMS_BOOL Configuration::IsServerInfoHiddenInLog(IN IMS_SINT32 nSlotId) const
{
    const SubscriberConfig* pSubsConfig = ConfigurationManager::GetInstance()->GetSubscriberConfig(
            SubscriberConfig::GetDefaultId(), nSlotId);

    if ((pSubsConfig != IMS_NULL) && pSubsConfig->IsDebugOn())
    {
        return IMS_FALSE;
    }

    return IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG();
}

PUBLIC VIRTUAL void Configuration::InitConfigs(IN IMS_SINT32 nSlotId)
{
    ConfigurationManager::GetInstance()->InitConfigs(nSlotId);
}

PUBLIC VIRTUAL void Configuration::RefreshConfigs(IN IMS_SINT32 nSlotId)
{
    ConfigurationManager::GetInstance()->RefreshConfigs(nSlotId);
}
