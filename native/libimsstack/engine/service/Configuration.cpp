/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceUtil.h"
#include "private/ConfigurationManager.h"
#include "private/AppConfig.h"
#include "private/SubscriberConfig.h"
#include "private/EngineConfig.h"
#include "private/MediaConfig.h"
#include "private/SipConfig.h"
#include "private/ImsRegistryLoader.h"
#include "ConfigLoader.h"
#include "Configuration.h"

PRIVATE
Configuration::Configuration() {}

PUBLIC
Configuration::~Configuration() {}

PUBLIC
AStringArray Configuration::GetLocalAppIds(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    return ConfigurationManager::GetInstance()->GetAppIds(nSlotId);
}

PUBLIC
const IAppConfig* Configuration::GetAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    return ConfigurationManager::GetInstance()->GetAppConfig(strAppId, nSlotId);
}

PUBLIC
IMS_BOOL Configuration::HasAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    return ConfigurationManager::GetInstance()->IsAppConfigured(strAppId, nSlotId);
}

PUBLIC
void Configuration::RemoveAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    ConfigurationManager::GetInstance()->RemoveAppConfig(strAppId, nSlotId);
}

PUBLIC
IMS_RESULT Configuration::SetAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
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

PUBLIC
IMS_RESULT Configuration::SetAppConfig(IN const AString& strAppId, IN const AString& strClassName,
        IN const ImsRegistry& objRegistry, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    if (strAppId.GetLength() == 0)
    {
        return IMS_FAILURE;  // Throw exception
    }

    if (strClassName.GetLength() == 0)
    {
        return IMS_FAILURE;  // Throw exception
    }

    if (objRegistry.GetCount() == 0)
    {
        return IMS_FAILURE;  // Throw exception
    }

    AppConfig* pAppConfig = new AppConfig(strAppId);

    if (pAppConfig == IMS_NULL)
    {
        return IMS_FAILURE;  // throw exception
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

PUBLIC GLOBAL Configuration* Configuration::GetInstance()
{
    static Configuration* pConfiguration = IMS_NULL;

    if (pConfiguration == IMS_NULL)
    {
        pConfiguration = new Configuration();
    }

    return pConfiguration;
}

//// IMS extensions

PUBLIC
const IMediaConfig* Configuration::GetMediaConfig(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    return ConfigurationManager::GetInstance()->GetMediaConfig(nSlotId);
}

PUBLIC
const ISipConfig* Configuration::GetSipConfig(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    return ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
}

PUBLIC
const ISubscriberConfig* Configuration::GetSubscriberConfig(
        IN IMS_SINT32 nSlotId, IN const AString& strId /* = AString::ConstNull()*/) const
{
    if (strId.IsNULL())
    {
        return ConfigurationManager::GetInstance()->GetSubscriberConfig(
                SubscriberConfig::GetDefaultId(), nSlotId);
    }

    return ConfigurationManager::GetInstance()->GetSubscriberConfig(strId, nSlotId);
}

PUBLIC
const ISubscriberConfig* Configuration::GetSubscriberConfig(
        IN const AString& strId /* = AString::ConstNull()*/,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    return GetSubscriberConfig(nSlotId, strId);
}

PUBLIC
IMS_UINT32 Configuration::GetTraceModule(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    const EngineConfig* pEngineConfig =
            ConfigurationManager::GetInstance()->GetEngineConfig(nSlotId);

    if (pEngineConfig == IMS_NULL)
    {
        return 0;
    }

    return pEngineConfig->GetTraceModule();
}

PUBLIC
IMS_UINT32 Configuration::GetTraceOption(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    const EngineConfig* pEngineConfig =
            ConfigurationManager::GetInstance()->GetEngineConfig(nSlotId);

    if (pEngineConfig == IMS_NULL)
    {
        return 0;
    }

    return pEngineConfig->GetTraceOption();
}

PUBLIC
IMS_BOOL Configuration::IsServerInfoHiddenInLog(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    const SubscriberConfig* pSubsConfig = ConfigurationManager::GetInstance()->GetSubscriberConfig(
            SubscriberConfig::GetDefaultId(), nSlotId);

    if ((pSubsConfig != IMS_NULL) && pSubsConfig->IsDebugOn())
    {
        return IMS_FALSE;
    }

    return IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG();
}

PUBLIC
void Configuration::InitConfigs(IN IMS_SINT32 nSlotId)
{
    ConfigurationManager::GetInstance()->InitConfigs(nSlotId);
}

PUBLIC
void Configuration::RefreshConfigs(IN IMS_SINT32 nSlotId)
{
    ConfigurationManager::GetInstance()->RefreshConfigs(nSlotId);
}
