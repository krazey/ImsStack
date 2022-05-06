/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090608  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"
#include "ConfigLoader.h"
#include "AsyncConfigHelper.h"
#include "private/AppConfig.h"
#include "private/SubscriberConfig.h"
#include "private/EngineConfig.h"
#include "private/SipConfig.h"
#include "private/MediaConfig.h"
#include "private/ConfigurationManager.h"

__IMS_TRACE_TAG_CONF__;

LOCAL
const IMS_CHAR SUBSCRIBER_CONFIG_NAMES[] = "subscriber,subscriber_fake";

template <class T>
LOCAL void configurationManager_DestroyConfig(IN T*& pBase)
{
    if (pBase != IMS_NULL)
    {
        delete pBase;
        pBase = IMS_NULL;
    }
}

class ConfigurationHolder
{
public:
    ConfigurationHolder();
    ~ConfigurationHolder();

private:
    ConfigurationHolder(IN const ConfigurationHolder& objRHS);
    ConfigurationHolder& operator=(IN const ConfigurationHolder& objRHS);

public:
    IMS_BOOL AddAppConfig(IN AppConfig* pAppConfig);
    AppConfig* RemoveAppConfig(IN const AString& strAppId);
    AppConfig* GetAppConfig(IN const AString& strAppId) const;
    inline const IMSList<AppConfig*>& GetAppConfigs() const { return objAppConfigs; }

    SubscriberConfig* GetSubscriberConfig(IN const AString& strId) const;
    inline const IMSList<SubscriberConfig*>& GetSubscriberConfigs() const
    {
        return objSubscriberConfigs;
    }
    inline EngineConfig* GetEngineConfig() const { return pEngineConfig; }
    inline SipConfig* GetSipConfig() const { return pSipConfig; }
    inline MediaConfig* GetMediaConfig() const { return pMediaConfig; }

    IMS_BOOL CreateConfigs(IN IMS_SINT32 nId);

    void InitConfigs(IN IMS_SINT32 nId);
    void RefreshConfigs(IN IMS_SINT32 nId);

private:
    void DestroyConfigs();

    IMS_BOOL CreateSubscriberConfig(IN IMS_SINT32 nId);
    IMS_BOOL CreateSubscriberConfigIfNotPresent(IN IMS_SINT32 nId);
    IMS_BOOL CreateEngineConfig(IN IMS_SINT32 nId);
    IMS_BOOL CreateSipConfig(IN IMS_SINT32 nId);
    IMS_BOOL CreateMediaConfig(IN IMS_SINT32 nId);

private:
    IMS_BOOL bCreated;
    AsyncConfigHelper* pConfigHelper;

    // List of IMS application configuration
    IMSList<AppConfig*> objAppConfigs;

    // Configuration for subscriber
    IMSList<SubscriberConfig*> objSubscriberConfigs;

    // Configuration for J281 engine classes
    EngineConfig* pEngineConfig;

    // Default configuration for SIP functionality
    SipConfig* pSipConfig;

    // Media profile (SDP, capabilities, codecs, ...)
    MediaConfig* pMediaConfig;
};

PUBLIC
ConfigurationHolder::ConfigurationHolder() :
        bCreated(IMS_FALSE),
        pConfigHelper(IMS_NULL),
        objAppConfigs(IMSList<AppConfig*>()),
        objSubscriberConfigs(IMSList<SubscriberConfig*>()),
        pEngineConfig(IMS_NULL),
        pSipConfig(IMS_NULL),
        pMediaConfig(IMS_NULL)
{
}

PUBLIC
ConfigurationHolder::~ConfigurationHolder()
{
    for (IMS_UINT32 i = 0; i < objAppConfigs.GetSize(); ++i)
    {
        AppConfig* pAppConfig = objAppConfigs.GetAt(i);

        if (pAppConfig != IMS_NULL)
        {
            delete pAppConfig;
        }
    }

    objAppConfigs.Clear();

    DestroyConfigs();

    if (pConfigHelper != IMS_NULL)
    {
        delete pConfigHelper;
    }
}

PUBLIC
IMS_BOOL ConfigurationHolder::AddAppConfig(IN AppConfig* pAppConfig)
{
    return objAppConfigs.Append(pAppConfig);
}

PUBLIC
AppConfig* ConfigurationHolder::RemoveAppConfig(IN const AString& strAppId)
{
    for (IMS_UINT32 i = 0; i < objAppConfigs.GetSize(); ++i)
    {
        AppConfig* pAppConfig = objAppConfigs.GetAt(i);

        if (pAppConfig != IMS_NULL)
        {
            if (strAppId.Equals(pAppConfig->GetAppId()))
            {
                objAppConfigs.RemoveAt(i);
                return pAppConfig;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
AppConfig* ConfigurationHolder::GetAppConfig(IN const AString& strAppId) const
{
    for (IMS_UINT32 i = 0; i < objAppConfigs.GetSize(); ++i)
    {
        AppConfig* pAppConfig = objAppConfigs.GetAt(i);

        if (pAppConfig != IMS_NULL)
        {
            if (strAppId.Equals(pAppConfig->GetAppId()))
            {
                return pAppConfig;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
SubscriberConfig* ConfigurationHolder::GetSubscriberConfig(IN const AString& strId) const
{
    if (objSubscriberConfigs.IsEmpty())
    {
        return IMS_NULL;
    }

    // In case of only one subscriber configuration, it just returns the first one.
    if (objSubscriberConfigs.GetSize() == 1)
    {
        return objSubscriberConfigs.GetAt(0);
    }

    for (IMS_UINT32 i = 0; i < objSubscriberConfigs.GetSize(); ++i)
    {
        SubscriberConfig* pSubsConfig = objSubscriberConfigs.GetAt(i);

        if (pSubsConfig != IMS_NULL)
        {
            if (strId.Equals(pSubsConfig->GetId()))
            {
                return pSubsConfig;
            }
        }
    }

    // No matched subscriber configuration, just uses the default subscriber configuration
    return objSubscriberConfigs.GetAt(0);
}

PUBLIC
IMS_BOOL ConfigurationHolder::CreateConfigs(IN IMS_SINT32 nId)
{
    // Destroy all the configs
    DestroyConfigs();

    // Create an engine configuration
    if (!CreateEngineConfig(nId))
    {
        DestroyConfigs();
        return IMS_FALSE;
    }

    // Create a subscriber configuration
    if (!CreateSubscriberConfig(nId))
    {
        DestroyConfigs();
        return IMS_FALSE;
    }

    // Create a sip configuration
    if (!CreateSipConfig(nId))
    {
        DestroyConfigs();
        return IMS_FALSE;
    }

    // Create a media configuration
    if (!CreateMediaConfig(nId))
    {
        DestroyConfigs();
        return IMS_FALSE;
    }

    bCreated = IMS_TRUE;

    return IMS_TRUE;
}

PUBLIC
void ConfigurationHolder::InitConfigs(IN IMS_SINT32 nId)
{
    if (pEngineConfig != IMS_NULL)
    {
        pEngineConfig->Init();

        if (nId == IMS_SLOT_0)
        {
            // Updates the trace option & module mask
            TraceService::GetTraceService()->SetOption(
                    pEngineConfig->GetTraceOption(), pEngineConfig->GetTraceModule());
        }
    }

    for (IMS_UINT32 i = 0; i < objSubscriberConfigs.GetSize(); ++i)
    {
        SubscriberConfig* pSubsConfig = objSubscriberConfigs.GetAt(i);

        if (pSubsConfig != IMS_NULL)
        {
            if (!pSubsConfig->Init())
            {
                IMS_TRACE_E(0, "Creating a subscriber config. failed", 0, 0, 0);
            }
        }
    }

    if (pSipConfig != IMS_NULL)
    {
        if (!pSipConfig->Init())
        {
            IMS_TRACE_E(0, "Creating a sip config. failed", 0, 0, 0);
        }

        if (!pSipConfig->CreateDefaultServiceConfig())
        {
            IMS_TRACE_E(0, "Creating a default service-specific sip config. failed", 0, 0, 0);
        }
    }

    //
    // In this moment, the subscriber configuration only supports the async. configuration.
    // 120510, EngineConfig added
    //
    if (pConfigHelper == IMS_NULL)
    {
        pConfigHelper = new AsyncConfigHelper();
    }

    for (IMS_UINT32 i = 0; i < objSubscriberConfigs.GetSize(); ++i)
    {
        IAsyncConfig* piAsyncConfig = objSubscriberConfigs.GetAt(i);

        if (piAsyncConfig != IMS_NULL)
        {
            piAsyncConfig->HandleMessage(
                    IAsyncConfig::ACMSG_START, reinterpret_cast<IMS_SINTP>(pConfigHelper), 0);
        }
    }
}

PUBLIC
void ConfigurationHolder::RefreshConfigs(IN IMS_SINT32 nId)
{
    if (!bCreated)
    {
        if (CreateConfigs(nId))
        {
            InitConfigs(nId);
        }
        return;
    }

    if (pEngineConfig != IMS_NULL)
    {
        pEngineConfig->Refresh();
    }

    if (!objSubscriberConfigs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSubscriberConfigs.GetSize(); ++i)
        {
            SubscriberConfig* pSubsConfig = objSubscriberConfigs.GetAt(i);
            pSubsConfig->Refresh();
        }

        CreateSubscriberConfigIfNotPresent(nId);
    }

    if (pSipConfig != IMS_NULL)
    {
        pSipConfig->Refresh();
        pSipConfig->CreateDefaultServiceConfig();
    }

    if (pMediaConfig != IMS_NULL)
    {
        pMediaConfig->Refresh();
    }
}

PRIVATE
void ConfigurationHolder::DestroyConfigs()
{
    if (!objSubscriberConfigs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSubscriberConfigs.GetSize(); ++i)
        {
            SubscriberConfig* pSubsConfig = objSubscriberConfigs.GetAt(i);
            configurationManager_DestroyConfig(pSubsConfig);
        }

        objSubscriberConfigs.Clear();
    }

    configurationManager_DestroyConfig(pEngineConfig);
    configurationManager_DestroyConfig(pSipConfig);
    configurationManager_DestroyConfig(pMediaConfig);

    bCreated = IMS_FALSE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateSubscriberConfig(IN IMS_SINT32 nId)
{
    const AString strConfName = SUBSCRIBER_CONFIG_NAMES;
    IMSList<AString> objConfNames = strConfName.Split(',');

    if (objConfNames.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objConfNames.GetSize(); ++i)
    {
        SubscriberConfig* pSubsConfig = new SubscriberConfig(nId, objConfNames.GetAt(i));

        if (pSubsConfig == IMS_NULL)
        {
            return IMS_FALSE;
        }

        objSubscriberConfigs.Append(pSubsConfig);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateSubscriberConfigIfNotPresent(IN IMS_SINT32 nId)
{
    const AString strConfName = SUBSCRIBER_CONFIG_NAMES;
    IMSList<AString> objConfNames = strConfName.Split(',');

    if (objConfNames.IsEmpty())
    {
        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < objSubscriberConfigs.GetSize(); ++i)
    {
        SubscriberConfig* pSubsConfig = objSubscriberConfigs.GetAt(i);

        if (pSubsConfig != IMS_NULL)
        {
            for (IMS_UINT32 j = 0; j < objConfNames.GetSize(); ++j)
            {
                if (pSubsConfig->GetConfName().Equals(objConfNames.GetAt(j)))
                {
                    objConfNames.RemoveAt(j);
                    break;
                }
            }
        }
    }

    for (IMS_UINT32 i = 0; i < objConfNames.GetSize(); ++i)
    {
        const AString& strConfName = objConfNames.GetAt(i);
        SubscriberConfig* pSubsConfig = new SubscriberConfig(nId, strConfName);

        if (pSubsConfig == IMS_NULL)
        {
            return IMS_FALSE;
        }

        IMS_TRACE_I("SubscriberConfig(%s) is newly added", strConfName.GetStr(), 0, 0);

        if (!pSubsConfig->Init())
        {
            delete pSubsConfig;

            IMS_TRACE_E(0, "Creating a subscriber config. failed", 0, 0, 0);
            return IMS_FALSE;
        }

        objSubscriberConfigs.Append(pSubsConfig);

        IAsyncConfig* piAsyncConfig = pSubsConfig;
        piAsyncConfig->HandleMessage(
                IAsyncConfig::ACMSG_START, reinterpret_cast<IMS_SINTP>(pConfigHelper), 0);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateEngineConfig(IN IMS_SINT32 nId)
{
    if (pEngineConfig != IMS_NULL)
    {
        return IMS_TRUE;
    }

    pEngineConfig = new EngineConfig(nId);

    if (pEngineConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (nId == IMS_SLOT_0)
    {
        // Updates the trace option & module mask
        TraceService::GetTraceService()->SetOption(
                pEngineConfig->GetTraceOption(), pEngineConfig->GetTraceModule());
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateSipConfig(IN IMS_SINT32 nId)
{
    if (pSipConfig != IMS_NULL)
    {
        return IMS_TRUE;
    }

    pSipConfig = new SipConfig(nId);

    if (pSipConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateMediaConfig(IN IMS_SINT32 nId)
{
    if (pMediaConfig != IMS_NULL)
    {
        return IMS_TRUE;
    }

    pMediaConfig = new MediaConfig(nId);

    if (pMediaConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pMediaConfig->Init())
    {
        delete pMediaConfig;
        pMediaConfig = IMS_NULL;

        IMS_TRACE_E(0, "Creating a media config. failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

class ConfigurationManagerPrivate
{
public:
    ConfigurationManagerPrivate();
    ~ConfigurationManagerPrivate();

private:
    ConfigurationManagerPrivate(IN const ConfigurationManagerPrivate& objRHS);
    ConfigurationManagerPrivate& operator=(IN const ConfigurationManagerPrivate& objRHS);

public:
    inline ConfigurationHolder* GetHolder(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return ppHolder[nSlotId];
    }

    inline const AString& GetLocator() const { return strConfigLocator; }
    inline IMS_SINT32 GetMode() const { return nConfigMode; }

    IMS_BOOL Initialize(IN const AString& strConfigLocator, IN IMS_SINT32 nConfigMode);

private:
    friend class ConfigurationManager;

    // Mode of configuration (file / xml / db)
    IMS_SINT32 nConfigMode;
    // Locator of configuration
    AString strConfigLocator;

    ConfigurationHolder** ppHolder;
};

PUBLIC
ConfigurationManagerPrivate::ConfigurationManagerPrivate() :
        nConfigMode(ConfigurationManager::MODE_DB),
        strConfigLocator(AString::ConstNull()),
        ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ppHolder = new ConfigurationHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        ppHolder[i] = new ConfigurationHolder();
    }
}

PUBLIC
ConfigurationManagerPrivate::~ConfigurationManagerPrivate()
{
    if (ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (ppHolder[i] != IMS_NULL)
            {
                delete ppHolder[i];
            }
        }

        delete[] ppHolder;
    }
}

PUBLIC
IMS_BOOL ConfigurationManagerPrivate::Initialize(
        IN const AString& strConfigLocator, IN IMS_SINT32 nConfigMode)
{
    this->strConfigLocator = strConfigLocator;
    this->nConfigMode = nConfigMode;

    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        ConfigurationHolder* pHolder = GetHolder(i);

        if (pHolder == IMS_NULL)
        {
            continue;
        }

        if (!pHolder->CreateConfigs(i))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
ConfigurationManager::ConfigurationManager() :
        pConfigMngrP(new ConfigurationManagerPrivate())
{
}

PUBLIC
ConfigurationManager::~ConfigurationManager()
{
    delete pConfigMngrP;
}

/*

Remarks

*/
PUBLIC
const AppConfig* ConfigurationManager::GetAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    return pHolder->GetAppConfig(strAppId);
}

/*

Remarks

*/
PUBLIC
AStringArray ConfigurationManager::GetAppIds(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    const IMSList<AppConfig*>& objAppConfigs = pHolder->GetAppConfigs();
    AStringArray objAppIds;

    for (IMS_UINT32 i = 0; i < objAppConfigs.GetSize(); ++i)
    {
        AppConfig* pAppConfig = objAppConfigs.GetAt(i);

        if (pAppConfig != IMS_NULL)
        {
            objAppIds.AddElement(pAppConfig->GetAppId());
        }
    }

    return objAppIds;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL ConfigurationManager::IsAppConfigured(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    if (GetAppConfig(strAppId, nSlotId) != IMS_NULL)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
void ConfigurationManager::RemoveAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    AppConfig* pAppConfig = pHolder->RemoveAppConfig(strAppId);

    if (pAppConfig != IMS_NULL)
    {
        IMS_TRACE_I("App (%d: %s) configuration is uninstalled", nSlotId, strAppId.GetStr(), 0);

        delete pAppConfig;
    }
}

/*

Remarks

*/
PUBLIC
IMS_RESULT ConfigurationManager::StoreAppConfig(IN AppConfig* pAppConfig,
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);

    // If the app. config is already present, remove and add it.
    RemoveAppConfig(strAppId, nSlotId);

    if (!pHolder->AddAppConfig(pAppConfig))
    {
        IMS_TRACE_E(0, "Setting app (%d: %s) configuration failed", nSlotId, strAppId.GetStr(), 0);
        return IMS_FAILURE;
    }

    IMS_TRACE_D("App (%d: %s) configuration is installed", nSlotId, strAppId.GetStr(), 0);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 ConfigurationManager::GetConfigMode() const
{
    return pConfigMngrP->GetMode();
}

/*

Remarks

*/
PUBLIC
const AString& ConfigurationManager::GetConfigLocator() const
{
    return pConfigMngrP->GetLocator();
}

/*
 Subscriber configuration - impl. defined
 This config. includes IMS-related information in the ISIM.

Remarks

*/
PUBLIC
const SubscriberConfig* ConfigurationManager::GetSubscriberConfig(
        IN const AString& strId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    return pHolder->GetSubscriberConfig(strId);
}

PUBLIC
const IMSList<SubscriberConfig*>& ConfigurationManager::GetSubscriberConfigs(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    return pHolder->GetSubscriberConfigs();
}

/*
 Engine configuration - impl. defined
 This config. includes the information for an optional/additional operation
 in J281 engine implementation.

Remarks

*/
PUBLIC
const EngineConfig* ConfigurationManager::GetEngineConfig(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    return pHolder->GetEngineConfig();
}

/*
 SIP configuration - impl. defined
 This config. includes the SIP-specific information for a default UA behavior.

Remarks

*/
PUBLIC
const SipConfig* ConfigurationManager::GetSipConfig(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    return pHolder->GetSipConfig();
}

/*
 Media configuration - impl. defined
 This config. includes the media-specific information (SDP for session & capabilities).

Remarks

*/
PUBLIC
const MediaConfig* ConfigurationManager::GetMediaConfig(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/) const
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    return pHolder->GetMediaConfig();
}

/*

Remarks

*/
PUBLIC GLOBAL ConfigurationManager* ConfigurationManager::GetInstance()
{
    static ConfigurationManager* pConfigMngr = IMS_NULL;

    if (pConfigMngr == IMS_NULL)
    {
        pConfigMngr = new ConfigurationManager();
    }

    return pConfigMngr;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL ConfigurationManager::Initialize(IN const AString& strLocator, IN IMS_SINT32 nMode)
{
    return pConfigMngrP->Initialize(strLocator, nMode);
}

/*

Remarks

*/
PUBLIC
void ConfigurationManager::InitConfigs(IN IMS_SINT32 nSlotId)
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    pHolder->InitConfigs(nSlotId);
}

/*

Remarks

*/
PUBLIC
void ConfigurationManager::RefreshConfigs(IN IMS_SINT32 nSlotId)
{
    ConfigurationHolder* pHolder = pConfigMngrP->GetHolder(nSlotId);
    pHolder->RefreshConfigs(nSlotId);
}
