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
#include "ServiceTrace.h"
#include "SystemConfig.h"

#include "AsyncConfigHelper.h"
#include "ConfigLoader.h"
#include "private/AppConfig.h"
#include "private/ConfigurationManager.h"
#include "private/EngineConfig.h"
#include "private/MediaConfig.h"
#include "private/SipConfig.h"
#include "private/SubscriberConfig.h"

__IMS_TRACE_TAG_CONF__;

static const IMS_CHAR SUBSCRIBER_CONFIG_NAMES[] = "subscriber,subscriber_fake";

template <class T>
static void configurationManager_DestroyConfig(IN T*& pBase)
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

    ConfigurationHolder(IN const ConfigurationHolder&) = delete;
    ConfigurationHolder& operator=(IN const ConfigurationHolder&) = delete;

public:
    IMS_BOOL AddAppConfig(IN AppConfig* pAppConfig);
    AppConfig* RemoveAppConfig(IN const AString& strAppId);
    AppConfig* GetAppConfig(IN const AString& strAppId) const;
    inline const ImsList<AppConfig*>& GetAppConfigs() const { return m_objAppConfigs; }

    SubscriberConfig* GetSubscriberConfig(IN const AString& strId) const;
    inline const ImsList<SubscriberConfig*>& GetSubscriberConfigs() const
    {
        return m_objSubscriberConfigs;
    }
    inline EngineConfig* GetEngineConfig() const { return m_pEngineConfig; }
    inline SipConfig* GetSipConfig() const { return m_pSipConfig; }
    inline MediaConfig* GetMediaConfig() const { return m_pMediaConfig; }

    IMS_BOOL CreateConfigs(IN IMS_SINT32 nId);
    void DestroyConfigs();

    void InitConfigs(IN IMS_SINT32 nId);
    void RefreshConfigs(IN IMS_SINT32 nId);

private:
    IMS_BOOL CreateSubscriberConfig(IN IMS_SINT32 nId);
    IMS_BOOL CreateSubscriberConfigIfNotPresent(IN IMS_SINT32 nId);
    IMS_BOOL CreateEngineConfig(IN IMS_SINT32 nId);
    IMS_BOOL CreateSipConfig(IN IMS_SINT32 nId);
    IMS_BOOL CreateMediaConfig(IN IMS_SINT32 nId);

private:
    IMS_BOOL m_bCreated;
    IMS_BOOL m_bInitialized;
    AsyncConfigHelper* m_pConfigHelper;

    // List of IMS application configuration
    ImsList<AppConfig*> m_objAppConfigs;

    // Configuration for subscriber
    ImsList<SubscriberConfig*> m_objSubscriberConfigs;

    // Configuration for J281 engine classes
    EngineConfig* m_pEngineConfig;

    // Default configuration for SIP functionality
    SipConfig* m_pSipConfig;

    // Media profile (SDP, capabilities, codecs, ...)
    MediaConfig* m_pMediaConfig;
};

PUBLIC
ConfigurationHolder::ConfigurationHolder() :
        m_bCreated(IMS_FALSE),
        m_bInitialized(IMS_FALSE),
        m_pConfigHelper(IMS_NULL),
        m_objAppConfigs(ImsList<AppConfig*>()),
        m_objSubscriberConfigs(ImsList<SubscriberConfig*>()),
        m_pEngineConfig(IMS_NULL),
        m_pSipConfig(IMS_NULL),
        m_pMediaConfig(IMS_NULL)
{
}

PUBLIC
ConfigurationHolder::~ConfigurationHolder()
{
    for (IMS_UINT32 i = 0; i < m_objAppConfigs.GetSize(); ++i)
    {
        AppConfig* pAppConfig = m_objAppConfigs.GetAt(i);

        if (pAppConfig != IMS_NULL)
        {
            delete pAppConfig;
        }
    }

    m_objAppConfigs.Clear();

    DestroyConfigs();

    if (m_pConfigHelper != IMS_NULL)
    {
        delete m_pConfigHelper;
    }
}

PUBLIC
IMS_BOOL ConfigurationHolder::AddAppConfig(IN AppConfig* pAppConfig)
{
    return m_objAppConfigs.Append(pAppConfig);
}

PUBLIC
AppConfig* ConfigurationHolder::RemoveAppConfig(IN const AString& strAppId)
{
    for (IMS_UINT32 i = 0; i < m_objAppConfigs.GetSize(); ++i)
    {
        AppConfig* pAppConfig = m_objAppConfigs.GetAt(i);

        if (pAppConfig != IMS_NULL)
        {
            if (strAppId.Equals(pAppConfig->GetAppId()))
            {
                m_objAppConfigs.RemoveAt(i);
                return pAppConfig;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
AppConfig* ConfigurationHolder::GetAppConfig(IN const AString& strAppId) const
{
    for (IMS_UINT32 i = 0; i < m_objAppConfigs.GetSize(); ++i)
    {
        AppConfig* pAppConfig = m_objAppConfigs.GetAt(i);

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
    if (m_objSubscriberConfigs.IsEmpty())
    {
        return IMS_NULL;
    }

    // In case of only one subscriber configuration, it just returns the first one.
    if (m_objSubscriberConfigs.GetSize() == 1)
    {
        return m_objSubscriberConfigs.GetAt(0);
    }

    for (IMS_UINT32 i = 0; i < m_objSubscriberConfigs.GetSize(); ++i)
    {
        SubscriberConfig* pSubsConfig = m_objSubscriberConfigs.GetAt(i);

        if (pSubsConfig != IMS_NULL)
        {
            if (strId.Equals(pSubsConfig->GetId()))
            {
                return pSubsConfig;
            }
        }
    }

    // No matched subscriber configuration, just uses the default subscriber configuration
    return m_objSubscriberConfigs.GetAt(0);
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

    m_bCreated = IMS_TRUE;

    return IMS_TRUE;
}

PUBLIC
void ConfigurationHolder::InitConfigs(IN IMS_SINT32 nId)
{
    if (m_pEngineConfig != IMS_NULL)
    {
        m_pEngineConfig->Init();

        if (nId == IMS_SLOT_0)
        {
            // Updates the trace option & module mask
            TraceService::GetTraceService()->SetOption(
                    m_pEngineConfig->GetTraceOption(), m_pEngineConfig->GetTraceModule());
        }
    }

    for (IMS_UINT32 i = 0; i < m_objSubscriberConfigs.GetSize(); ++i)
    {
        SubscriberConfig* pSubsConfig = m_objSubscriberConfigs.GetAt(i);

        if (pSubsConfig != IMS_NULL)
        {
            if (!pSubsConfig->Init())
            {
                IMS_TRACE_E(0, "Creating a subscriber config. failed", 0, 0, 0);
            }
        }
    }

    if (m_pSipConfig != IMS_NULL)
    {
        if (!m_pSipConfig->Init())
        {
            IMS_TRACE_E(0, "Creating a sip config. failed", 0, 0, 0);
        }
    }

    //
    // In this moment, the subscriber configuration only supports the async. configuration.
    // 120510, EngineConfig added
    //
    if (m_pConfigHelper == IMS_NULL)
    {
        m_pConfigHelper = new AsyncConfigHelper();
    }

    for (IMS_UINT32 i = 0; i < m_objSubscriberConfigs.GetSize(); ++i)
    {
        IAsyncConfig* piAsyncConfig = m_objSubscriberConfigs.GetAt(i);

        if (piAsyncConfig != IMS_NULL)
        {
            piAsyncConfig->HandleMessage(
                    IAsyncConfig::ACMSG_START, reinterpret_cast<IMS_SINTP>(m_pConfigHelper), 0);
        }
    }

    m_bInitialized = IMS_TRUE;
}

PUBLIC
void ConfigurationHolder::RefreshConfigs(IN IMS_SINT32 nId)
{
    if (!m_bCreated)
    {
        if (CreateConfigs(nId))
        {
            InitConfigs(nId);
        }
        return;
    }

    if (!m_bInitialized)
    {
        InitConfigs(nId);
        return;
    }

    if (m_pEngineConfig != IMS_NULL)
    {
        m_pEngineConfig->Refresh();
    }

    if (!m_objSubscriberConfigs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objSubscriberConfigs.GetSize(); ++i)
        {
            SubscriberConfig* pSubsConfig = m_objSubscriberConfigs.GetAt(i);
            pSubsConfig->Refresh();
        }

        CreateSubscriberConfigIfNotPresent(nId);
    }

    if (m_pSipConfig != IMS_NULL)
    {
        m_pSipConfig->Refresh();
    }

    if (m_pMediaConfig != IMS_NULL)
    {
        m_pMediaConfig->Refresh();
    }
}

PUBLIC
void ConfigurationHolder::DestroyConfigs()
{
    if (!m_objSubscriberConfigs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objSubscriberConfigs.GetSize(); ++i)
        {
            SubscriberConfig* pSubsConfig = m_objSubscriberConfigs.GetAt(i);
            configurationManager_DestroyConfig(pSubsConfig);
        }

        m_objSubscriberConfigs.Clear();
    }

    configurationManager_DestroyConfig(m_pEngineConfig);
    configurationManager_DestroyConfig(m_pSipConfig);
    configurationManager_DestroyConfig(m_pMediaConfig);

    m_bCreated = IMS_FALSE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateSubscriberConfig(IN IMS_SINT32 nId)
{
    const AString strConfName = SUBSCRIBER_CONFIG_NAMES;
    ImsList<AString> objConfNames = strConfName.Split(',');

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

        m_objSubscriberConfigs.Append(pSubsConfig);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateSubscriberConfigIfNotPresent(IN IMS_SINT32 nId)
{
    const AString strConfNames = SUBSCRIBER_CONFIG_NAMES;
    ImsList<AString> objConfNames = strConfNames.Split(',');

    if (objConfNames.IsEmpty())
    {
        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < m_objSubscriberConfigs.GetSize(); ++i)
    {
        const SubscriberConfig* pSubsConfig = m_objSubscriberConfigs.GetAt(i);

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

        m_objSubscriberConfigs.Append(pSubsConfig);

        IAsyncConfig* piAsyncConfig = pSubsConfig;
        piAsyncConfig->HandleMessage(
                IAsyncConfig::ACMSG_START, reinterpret_cast<IMS_SINTP>(m_pConfigHelper), 0);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateEngineConfig(IN IMS_SINT32 nId)
{
    if (m_pEngineConfig != IMS_NULL)
    {
        return IMS_TRUE;
    }

    m_pEngineConfig = new EngineConfig(nId);

    if (m_pEngineConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (nId == IMS_SLOT_0)
    {
        // Updates the trace option & module mask
        TraceService::GetTraceService()->SetOption(
                m_pEngineConfig->GetTraceOption(), m_pEngineConfig->GetTraceModule());
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateSipConfig(IN IMS_SINT32 nId)
{
    if (m_pSipConfig != IMS_NULL)
    {
        return IMS_TRUE;
    }

    m_pSipConfig = new SipConfig(nId);

    if (m_pSipConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ConfigurationHolder::CreateMediaConfig(IN IMS_SINT32 nId)
{
    if (m_pMediaConfig != IMS_NULL)
    {
        return IMS_TRUE;
    }

    m_pMediaConfig = new MediaConfig(nId);

    if (m_pMediaConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_pMediaConfig->Init())
    {
        delete m_pMediaConfig;
        m_pMediaConfig = IMS_NULL;

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

    ConfigurationManagerPrivate(IN const ConfigurationManagerPrivate&) = delete;
    ConfigurationManagerPrivate& operator=(IN const ConfigurationManagerPrivate&) = delete;

public:
    inline ConfigurationHolder* GetHolder(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return m_ppHolder[nSlotId];
    }

    IMS_BOOL Initialize();
    void DestroyConfigs();

private:
    friend class ConfigurationManager;

    ConfigurationHolder** m_ppHolder;
};

PUBLIC
ConfigurationManagerPrivate::ConfigurationManagerPrivate() :
        m_ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

    m_ppHolder = new ConfigurationHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppHolder[i] = new ConfigurationHolder();
    }
}

PUBLIC
ConfigurationManagerPrivate::~ConfigurationManagerPrivate()
{
    if (m_ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppHolder[i] != IMS_NULL)
            {
                delete m_ppHolder[i];
            }
        }

        delete[] m_ppHolder;
    }
}

PUBLIC
IMS_BOOL ConfigurationManagerPrivate::Initialize()
{
    IMS_SINT32 nSimCount = SystemConfig::GetActiveSimCount();

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

PUBLIC
void ConfigurationManagerPrivate::DestroyConfigs()
{
    IMS_SINT32 nSimCount = SystemConfig::GetActiveSimCount();

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        ConfigurationHolder* pHolder = GetHolder(i);

        if (pHolder == IMS_NULL)
        {
            continue;
        }

        pHolder->DestroyConfigs();
    }
}

PRIVATE
ConfigurationManager::ConfigurationManager() :
        m_pConfigMngrPrivate(new ConfigurationManagerPrivate())
{
}

PUBLIC
ConfigurationManager::~ConfigurationManager()
{
    delete m_pConfigMngrPrivate;
}

PUBLIC
const AppConfig* ConfigurationManager::GetAppConfig(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId) const
{
    const ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    return pHolder->GetAppConfig(strAppId);
}

PUBLIC
AStringArray ConfigurationManager::GetAppIds(IN IMS_SINT32 nSlotId) const
{
    const ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    const ImsList<AppConfig*>& objAppConfigs = pHolder->GetAppConfigs();
    AStringArray objAppIds;

    for (IMS_UINT32 i = 0; i < objAppConfigs.GetSize(); ++i)
    {
        const AppConfig* pAppConfig = objAppConfigs.GetAt(i);

        if (pAppConfig != IMS_NULL)
        {
            objAppIds.AddElement(pAppConfig->GetAppId());
        }
    }

    return objAppIds;
}

PUBLIC
IMS_BOOL ConfigurationManager::IsAppConfigured(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId) const
{
    if (GetAppConfig(strAppId, nSlotId) != IMS_NULL)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
void ConfigurationManager::RemoveAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId)
{
    ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    AppConfig* pAppConfig = pHolder->RemoveAppConfig(strAppId);

    if (pAppConfig != IMS_NULL)
    {
        IMS_TRACE_I("App (%d: %s) configuration is uninstalled", nSlotId, strAppId.GetStr(), 0);

        delete pAppConfig;
    }
}

PUBLIC
IMS_RESULT ConfigurationManager::StoreAppConfig(
        IN AppConfig* pAppConfig, IN const AString& strAppId, IN IMS_SINT32 nSlotId)
{
    ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);

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

/**
 * @brief Subscriber configuration - impl. defined.
 *        This config. includes IMS-related information in the ISIM.
 */
PUBLIC
const SubscriberConfig* ConfigurationManager::GetSubscriberConfig(
        IN const AString& strId, IN IMS_SINT32 nSlotId) const
{
    const ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    return pHolder->GetSubscriberConfig(strId);
}

PUBLIC
const ImsList<SubscriberConfig*>& ConfigurationManager::GetSubscriberConfigs(
        IN IMS_SINT32 nSlotId) const
{
    const ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    return pHolder->GetSubscriberConfigs();
}

/**
 * @brief Engine configuration - impl. defined.
 *        This config. includes the information for an optional/additional operation
 *        in core engine implementation.
 */
PUBLIC
const EngineConfig* ConfigurationManager::GetEngineConfig(IN IMS_SINT32 nSlotId) const
{
    const ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    return pHolder->GetEngineConfig();
}

/**
 * @brief SIP configuration - impl. defined.
 *        This config. includes the SIP-specific information for a default UA behavior.
 */
PUBLIC
const SipConfig* ConfigurationManager::GetSipConfig(IN IMS_SINT32 nSlotId) const
{
    const ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    return pHolder->GetSipConfig();
}

/**
 * @brief Media configuration - impl. defined.
 *        This config. includes the media-specific information (SDP for session & capabilities).
 */
PUBLIC
const MediaConfig* ConfigurationManager::GetMediaConfig(IN IMS_SINT32 nSlotId) const
{
    const ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    return pHolder->GetMediaConfig();
}

PUBLIC GLOBAL ConfigurationManager* ConfigurationManager::GetInstance()
{
    static ConfigurationManager* s_pConfigMngr = IMS_NULL;

    if (s_pConfigMngr == IMS_NULL)
    {
        s_pConfigMngr = new ConfigurationManager();
    }

    return s_pConfigMngr;
}

PUBLIC
IMS_BOOL ConfigurationManager::Initialize()
{
    return m_pConfigMngrPrivate->Initialize();
}

PUBLIC
void ConfigurationManager::DestroyConfigs()
{
    m_pConfigMngrPrivate->DestroyConfigs();
}

PUBLIC
void ConfigurationManager::InitConfigs(IN IMS_SINT32 nSlotId)
{
    ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    pHolder->InitConfigs(nSlotId);
}

PUBLIC
void ConfigurationManager::RefreshConfigs(IN IMS_SINT32 nSlotId)
{
    ConfigurationHolder* pHolder = m_pConfigMngrPrivate->GetHolder(nSlotId);
    pHolder->RefreshConfigs(nSlotId);
}
