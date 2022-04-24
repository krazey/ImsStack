/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170501  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "PlatformProperty.h"
#include "IThread.h"
#include "ISystemConfigListener.h"
#include "SystemConfigManager.h"

__IMS_TRACE_TAG_BASE__;

PRIVATE
SystemConfigManager::SystemConfigManager()
    : piLockForConfigs(IMS_NULL)
    , piLockForListeners(IMS_NULL)
    , piProxyThread(IMS_NULL)
{
    piLockForConfigs = MutexService::GetMutexService()->CreateMutex();
    piLockForListeners = MutexService::GetMutexService()->CreateMutex();

    SystemConfig::CacheGlobalConfigs();
}

PRIVATE
SystemConfigManager::~SystemConfigManager()
{
    ClearAllConfigs();

    MutexService::GetMutexService()->DestroyMutex(piLockForConfigs);
    MutexService::GetMutexService()->DestroyMutex(piLockForListeners);
}

PUBLIC
IMS_SINT32 SystemConfigManager::GetActiveSlotId() const
{
    if (!SystemConfig::IsMultiSimEnabled())
    {
        return IMS_SLOT_0;
    }

    IMS_SINT32 nActiveSlotId = IMS_SLOT_0;

    LockGuard objLock(piLockForConfigs);

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        for (IMS_UINT32 i = 0; i < objSystemConfigs.GetSize(); i++)
        {
            const SystemConfig* pSC = objSystemConfigs.GetValueAt(i);

            if (pSC->GetOperator().GetLength() > 0)
            {
                nActiveSlotId = pSC->GetSlotId();
                break;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objSystemConfigs.GetSize(); i++)
        {
            const SystemConfig* pSC = objSystemConfigs.GetValueAt(i);

            if (pSC->IsDds())
            {
                nActiveSlotId = pSC->GetSlotId();
                break;
            }
        }
    }

    return nActiveSlotId;
}

PUBLIC
const SystemConfig* SystemConfigManager::GetConfig(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/) const
{
    if (nSlotId < IMS_SLOT_0)
    {
        nSlotId = IMS_SLOT_0;
    }

    LockGuard objLock(piLockForConfigs);

    IMS_SLONG nIndex = objSystemConfigs.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return objSystemConfigs.GetValueAt(nIndex);
}

PUBLIC
const SystemConfig* SystemConfigManager::GetOldConfig(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/) const
{
    if (nSlotId < IMS_SLOT_0)
    {
        nSlotId = IMS_SLOT_0;
    }

    nSlotId += OLD_CONFIG_INDEX_BASE;

    LockGuard objLock(piLockForConfigs);

    IMS_SLONG nIndex = objSystemConfigs.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return objSystemConfigs.GetValueAt(nIndex);
}

PUBLIC
void SystemConfigManager::AddListener(IN ISystemConfigListener *piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(piLockForListeners);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISystemConfigListener *piTmpListener = objListeners.GetAt(i);

        if (piListener == piTmpListener)
        {
            return;
        }
    }

    objListeners.Append(piListener);
}

PUBLIC
void SystemConfigManager::RemoveListener(IN ISystemConfigListener *piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(piLockForListeners);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISystemConfigListener *piTmpListener = objListeners.GetAt(i);

        if (piListener == piTmpListener)
        {
            objListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC GLOBAL
SystemConfigManager* SystemConfigManager::GetInstance()
{
    static SystemConfigManager *pSystemConfigManager = IMS_NULL;

    if (pSystemConfigManager == IMS_NULL)
    {
        pSystemConfigManager = new SystemConfigManager();
    }

    return pSystemConfigManager;
}

PUBLIC GLOBAL
void SystemConfigManager::CacheSystemFeatures()
{
    PlatformProperty::InitializeOnImsThread();
    SystemConfig::UpdateGlobalConfigsOnFeatureChanged();
}

PRIVATE VIRTUAL
void SystemConfigManager::MessageCallback_OnMessage(IN ImsMessage& objMsg)
{
    IMS_TRACE_D("MessageCallback_OnMessage :: msg=%d", objMsg.GetName(), 0, 0);

    switch (objMsg.GetName())
    {
    case TMSG_CONFIG_CHANGED: {
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
    case TMSG_FEATURE_PERMISSIONS_CHANGED: {
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
    LockGuard objLock(piLockForConfigs);

    for (IMS_UINT32 i = 0; i < objSystemConfigs.GetSize(); ++i)
    {
        SystemConfig *pConfig = objSystemConfigs.GetValueAt(i);

        if (pConfig != IMS_NULL)
        {
            delete pConfig;
        }
    }

    objSystemConfigs.Clear();
}

PRIVATE
IMS_BOOL SystemConfigManager::HasListener(IN ISystemConfigListener *piListener) const
{
    LockGuard objLock(piLockForListeners);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISystemConfigListener *piTmpListener = objListeners.GetAt(i);

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
        LockGuard objLock(piLockForListeners);

        objTmpListeners = objListeners;
    }

    for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
    {
        ISystemConfigListener *piListener = objTmpListeners.GetAt(i);

        if ((piListener != IMS_NULL) && HasListener(piListener))
        {
            piListener->SystemConfig_ConfigurationChanged(nEvent, nSlotId);
        }
    }
}

PRIVATE
void SystemConfigManager::PostConfigChanged(IN IMS_SINT32 nEvent,
        IN IMS_SINT32 nCount, IN const __SystemConfig* pstSysConfig)
{
    IMS_BOOL bNotificationSuccess = IMS_FALSE;

    if ((nCount > 0) && (piProxyThread != IMS_NULL))
    {
        IMSList<IMS_SINT32>* pSlots = new IMSList<IMS_SINT32>();

        for (IMS_SINT32 i = 0; i < nCount; ++i)
        {
            const __SystemConfig *pSC = &pstSysConfig[i];

            pSlots->Append(pSC->nSlotId);
        }

        ImsMessage objMsg(TMSG_CONFIG_CHANGED,
                nEvent, reinterpret_cast<IMS_UINTP>(pSlots), this);

        bNotificationSuccess = piProxyThread->PostMessageI(objMsg);

        if (!bNotificationSuccess)
        {
            delete pSlots;
        }
    }

    if (!bNotificationSuccess)
    {
        for (IMS_SINT32 i = 0; i < nCount; ++i)
        {
            const __SystemConfig *pSC = &pstSysConfig[i];

            NotifyConfigChanged(nEvent, pSC->nSlotId);
        }
    }
}

PRIVATE
void SystemConfigManager::StoreConfig(IN IMS_SINT32 nCount,
        IN const __SystemConfig* pstSysConfig)
{
    LockGuard objLock(piLockForConfigs);

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        SystemConfig *pNewConfig = new SystemConfig(&pstSysConfig[i]);

        IMS_TRACE_I("(%d)=%s", i, pNewConfig->ToString().GetStr(), 0);

        // Remove old config. if present
        IMS_SINT32 nOldConfigId = pNewConfig->GetSlotId() + OLD_CONFIG_INDEX_BASE;
        IMS_SLONG nOldConfigIndex = objSystemConfigs.GetIndexOfKey(nOldConfigId);

        if (nOldConfigIndex >= 0)
        {
            SystemConfig *pOldConfig = objSystemConfigs.GetValueAt(nOldConfigIndex);
            objSystemConfigs.RemoveAt(nOldConfigIndex);

            if (pOldConfig != IMS_NULL)
            {
                delete pOldConfig;
            }
        }

        // Move current config. to old config.
        IMS_SINT32 nConfigId = pNewConfig->GetSlotId();
        IMS_SLONG nConfigIndex = objSystemConfigs.GetIndexOfKey(nConfigId);

        if (nConfigIndex >= 0)
        {
            SystemConfig *pConfig = objSystemConfigs.GetValueAt(nConfigIndex);
            objSystemConfigs.RemoveAt(nConfigIndex);

            if (pConfig != IMS_NULL)
            {
                objSystemConfigs.SetValue(nConfigId + OLD_CONFIG_INDEX_BASE, pConfig);
            }
        }

        // Set new config. as a current config.
        objSystemConfigs.SetValue(nConfigId, pNewConfig);
    }
}

PRIVATE
void SystemConfigManager::SetProxyThread(IN IThread* piThread)
{
    piProxyThread = piThread;
}

PRIVATE
void SystemConfigManager::UpdateSystemConfig(IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount,
        IN const __SystemConfig* pstSysConfig)
{
    if (nEvent == SystemConfig::EVENT_FEATURE_PERMISSIONS_CHANGED)
    {
        if (piProxyThread != IMS_NULL)
        {
            ImsMessage objMsg(TMSG_FEATURE_PERMISSIONS_CHANGED, 0, 0, this);
            piProxyThread->PostMessageI(objMsg);
        }
        return;
    }

    if (pstSysConfig == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("UpdateSystemConfig :: event=%d, count=%d", nEvent, nCount, 0);

    if (nEvent == SystemConfig::EVENT_ON_BOOT)
    {
        ClearAllConfigs();
    }

    StoreConfig(nCount, pstSysConfig);

    PostConfigChanged(nEvent, nCount, pstSysConfig);
}
