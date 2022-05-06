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
#include "IThread.h"
#include "ISystemConfigListener.h"
#include "PlatformProperty.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SystemConfigManager.h"

__IMS_TRACE_TAG_BASE__;

PRIVATE
SystemConfigManager::SystemConfigManager() :
        m_piLockForConfigs(IMS_NULL),
        m_piLockForListeners(IMS_NULL),
        m_piProxyThread(IMS_NULL)
{
    m_piLockForConfigs = MutexService::GetMutexService()->CreateMutex();
    m_piLockForListeners = MutexService::GetMutexService()->CreateMutex();

    SystemConfig::CacheGlobalConfigs();
}

PRIVATE
SystemConfigManager::~SystemConfigManager()
{
    ClearAllConfigs();

    MutexService::GetMutexService()->DestroyMutex(m_piLockForConfigs);
    MutexService::GetMutexService()->DestroyMutex(m_piLockForListeners);
}

PUBLIC
IMS_SINT32 SystemConfigManager::GetActiveSlotId() const
{
    if (!SystemConfig::IsMultiSimEnabled())
    {
        return IMS_SLOT_0;
    }

    IMS_SINT32 nActiveSlotId = IMS_SLOT_0;

    LockGuard objLock(m_piLockForConfigs);

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        for (IMS_UINT32 i = 0; i < m_objSystemConfigs.GetSize(); i++)
        {
            const SystemConfig* pConfig = m_objSystemConfigs.GetValueAt(i);

            if (pConfig->GetOperator().GetLength() > 0)
            {
                nActiveSlotId = pConfig->GetSlotId();
                break;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objSystemConfigs.GetSize(); i++)
        {
            const SystemConfig* pConfig = m_objSystemConfigs.GetValueAt(i);

            if (pConfig->IsDds())
            {
                nActiveSlotId = pConfig->GetSlotId();
                break;
            }
        }
    }

    return nActiveSlotId;
}

PUBLIC
const SystemConfig* SystemConfigManager::GetConfig(IN IMS_SINT32 nSlotId) const
{
    if (nSlotId < IMS_SLOT_0)
    {
        nSlotId = IMS_SLOT_0;
    }

    LockGuard objLock(m_piLockForConfigs);

    IMS_SLONG nIndex = m_objSystemConfigs.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objSystemConfigs.GetValueAt(nIndex);
}

PUBLIC
const SystemConfig* SystemConfigManager::GetOldConfig(IN IMS_SINT32 nSlotId) const
{
    if (nSlotId < IMS_SLOT_0)
    {
        nSlotId = IMS_SLOT_0;
    }

    nSlotId += OLD_CONFIG_INDEX_BASE;

    LockGuard objLock(m_piLockForConfigs);

    IMS_SLONG nIndex = m_objSystemConfigs.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objSystemConfigs.GetValueAt(nIndex);
}

PUBLIC
void SystemConfigManager::AddListener(IN ISystemConfigListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(m_piLockForListeners);

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISystemConfigListener* piTmpListener = m_objListeners.GetAt(i);

        if (piListener == piTmpListener)
        {
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC
void SystemConfigManager::RemoveListener(IN ISystemConfigListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(m_piLockForListeners);

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISystemConfigListener* piTmpListener = m_objListeners.GetAt(i);

        if (piListener == piTmpListener)
        {
            m_objListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC GLOBAL SystemConfigManager* SystemConfigManager::GetInstance()
{
    static SystemConfigManager* s_pSystemConfigManager = IMS_NULL;

    if (s_pSystemConfigManager == IMS_NULL)
    {
        s_pSystemConfigManager = new SystemConfigManager();
    }

    return s_pSystemConfigManager;
}

PUBLIC GLOBAL void SystemConfigManager::CacheSystemFeatures()
{
    PlatformProperty::InitializeOnImsThread();
    SystemConfig::UpdateGlobalConfigsOnFeatureChanged();
}

PRIVATE VIRTUAL void SystemConfigManager::MessageCallback_OnMessage(IN ImsMessage& objMsg)
{
    IMS_TRACE_D("MessageCallback_OnMessage :: msg=%d", objMsg.GetName(), 0, 0);

    switch (objMsg.GetName())
    {
        case TMSG_CONFIG_CHANGED:
        {
            IMS_SINT32 nEvent = LONG_TO_INT(objMsg.nWparam);
            IMSList<IMS_SINT32>* pSlots = reinterpret_cast<IMSList<IMS_SINT32>*>(objMsg.nLparam);

            if (pSlots != IMS_NULL)
            {
                for (IMS_UINT32 i = 0; i < pSlots->GetSize(); ++i)
                {
                    NotifyConfigChanged(nEvent, pSlots->GetAt(i));
                }

                delete pSlots;
            }
            break;
        }
        case TMSG_FEATURE_PERMISSIONS_CHANGED:
        {
            IMS_TRACE_I("Cache system features", 0, 0, 0);
            CacheSystemFeatures();
            break;
        }
        default:
            break;
    }
}

PRIVATE
void SystemConfigManager::ClearAllConfigs()
{
    LockGuard objLock(m_piLockForConfigs);

    for (IMS_UINT32 i = 0; i < m_objSystemConfigs.GetSize(); ++i)
    {
        SystemConfig* pConfig = m_objSystemConfigs.GetValueAt(i);

        if (pConfig != IMS_NULL)
        {
            delete pConfig;
        }
    }

    m_objSystemConfigs.Clear();
}

PRIVATE
IMS_BOOL SystemConfigManager::HasListener(IN ISystemConfigListener* piListener) const
{
    LockGuard objLock(m_piLockForListeners);

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ISystemConfigListener* piTmpListener = m_objListeners.GetAt(i);

        if (piListener == piTmpListener)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
void SystemConfigManager::NotifyConfigChanged(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId)
{
    IMSList<ISystemConfigListener*> objTmpListeners;

    {
        LockGuard objLock(m_piLockForListeners);

        objTmpListeners = m_objListeners;
    }

    for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
    {
        ISystemConfigListener* piListener = objTmpListeners.GetAt(i);

        if ((piListener != IMS_NULL) && HasListener(piListener))
        {
            piListener->SystemConfig_ConfigurationChanged(nEvent, nSlotId);
        }
    }
}

PRIVATE
void SystemConfigManager::PostConfigChanged(
        IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount, IN const __SystemConfig* pSysConfig)
{
    IMS_BOOL bNotificationSuccess = IMS_FALSE;

    if ((nCount > 0) && (m_piProxyThread != IMS_NULL))
    {
        IMSList<IMS_SINT32>* pSlots = new IMSList<IMS_SINT32>();

        for (IMS_SINT32 i = 0; i < nCount; ++i)
        {
            const __SystemConfig* pConfig = &pSysConfig[i];

            pSlots->Append(pConfig->nSlotId);
        }

        ImsMessage objMsg(TMSG_CONFIG_CHANGED, nEvent, reinterpret_cast<IMS_UINTP>(pSlots), this);

        bNotificationSuccess = m_piProxyThread->PostMessageI(objMsg);

        if (!bNotificationSuccess)
        {
            delete pSlots;
        }
    }

    if (!bNotificationSuccess)
    {
        for (IMS_SINT32 i = 0; i < nCount; ++i)
        {
            const __SystemConfig* pConfig = &pSysConfig[i];

            NotifyConfigChanged(nEvent, pConfig->nSlotId);
        }
    }
}

PRIVATE
void SystemConfigManager::StoreConfig(IN IMS_SINT32 nCount, IN const __SystemConfig* pSysConfig)
{
    LockGuard objLock(m_piLockForConfigs);

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        SystemConfig* pNewConfig = new SystemConfig(&pSysConfig[i]);

        IMS_TRACE_I("(%d)=%s", i, pNewConfig->ToString().GetStr(), 0);

        // Remove old config. if present
        IMS_SINT32 nOldConfigId = pNewConfig->GetSlotId() + OLD_CONFIG_INDEX_BASE;
        IMS_SLONG nOldConfigIndex = m_objSystemConfigs.GetIndexOfKey(nOldConfigId);

        if (nOldConfigIndex >= 0)
        {
            SystemConfig* pOldConfig = m_objSystemConfigs.GetValueAt(nOldConfigIndex);
            m_objSystemConfigs.RemoveAt(nOldConfigIndex);

            if (pOldConfig != IMS_NULL)
            {
                delete pOldConfig;
            }
        }

        // Move current config. to old config.
        IMS_SINT32 nConfigId = pNewConfig->GetSlotId();
        IMS_SLONG nConfigIndex = m_objSystemConfigs.GetIndexOfKey(nConfigId);

        if (nConfigIndex >= 0)
        {
            SystemConfig* pConfig = m_objSystemConfigs.GetValueAt(nConfigIndex);
            m_objSystemConfigs.RemoveAt(nConfigIndex);

            if (pConfig != IMS_NULL)
            {
                m_objSystemConfigs.SetValue(nConfigId + OLD_CONFIG_INDEX_BASE, pConfig);
            }
        }

        // Set new config. as a current config.
        m_objSystemConfigs.SetValue(nConfigId, pNewConfig);
    }
}

PRIVATE
void SystemConfigManager::SetProxyThread(IN IThread* piThread)
{
    m_piProxyThread = piThread;
}

PRIVATE
void SystemConfigManager::UpdateSystemConfig(
        IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount, IN const __SystemConfig* pSysConfig)
{
    if (nEvent == SystemConfig::EVENT_FEATURE_PERMISSIONS_CHANGED)
    {
        if (m_piProxyThread != IMS_NULL)
        {
            ImsMessage objMsg(TMSG_FEATURE_PERMISSIONS_CHANGED, 0, 0, this);
            m_piProxyThread->PostMessageI(objMsg);
        }
        return;
    }

    if (pSysConfig == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("UpdateSystemConfig :: event=%d, count=%d", nEvent, nCount, 0);

    if (nEvent == SystemConfig::EVENT_ON_BOOT)
    {
        ClearAllConfigs();
    }

    StoreConfig(nCount, pSysConfig);

    PostConfigChanged(nEvent, nCount, pSysConfig);
}
