/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170501  hwangoo.park@             Created
    </table>

    Description

*/
#ifndef _SYSTEM_CONFIG_MANAGER_H_
#define _SYSTEM_CONFIG_MANAGER_H_

#include "ImsMessageDef.h"
#include "ImsMessage.h"
#include "IMSList.h"
#include "IMSMap.h"
#include "SystemConfig.h"

class IMutex;
class IThread;
class ISystemConfigListener;

class SystemConfigManager
    : public ImsMessage::IMessageCallback
{
private:
    SystemConfigManager();
    SystemConfigManager(IN const SystemConfigManager &objRHS);
    ~SystemConfigManager();

private:
    SystemConfigManager& operator=(IN const SystemConfigManager &objRHS);

public:
    IMS_SINT32 GetActiveSlotId() const;

    const SystemConfig* GetConfig(IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;
    const SystemConfig* GetOldConfig(IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;

    void AddListener(IN ISystemConfigListener *piListener);
    void RemoveListener(IN ISystemConfigListener *piListener);

    static SystemConfigManager* GetInstance();

    static void CacheSystemFeatures();

private:
    // ImsMessage::IMessageCallback class
    virtual void MessageCallback_OnMessage(IN ImsMessage& objMsg);

    void ClearAllConfigs();
    IMS_BOOL HasListener(IN ISystemConfigListener *piListener) const;
    void NotifyConfigChanged(IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId);
    void PostConfigChanged(IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount,
            IN const __SystemConfig* pstSysConfig);
    void StoreConfig(IN IMS_SINT32 nCount, IN const __SystemConfig* pstSysConfig);

    // Invoked by ImsFramework
    // It's used to handle any events asynchronously.
    void SetProxyThread(IN IThread* piThread);

    // Invoked by __SystemConfigWrapper
    // It's used to update the system configuration after detecting operator.
    void UpdateSystemConfig(IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount,
            IN const __SystemConfig* pstSysConfig);

private:
    // IMSCore.cpp
    friend class __SystemConfigWrapper;
    friend class IMSFramework;

    enum
    {
        OLD_CONFIG_INDEX_BASE = 10
    };

    enum
    {
        TMSG_CONFIG_CHANGED = IMS_MSG_THREAD_USER_BASE,
        TMSG_FEATURE_PERMISSIONS_CHANGED
    };

    IMutex* piLockForConfigs;
    IMutex* piLockForListeners;

    // Framework thread.
    // It's used to remove any blocking issues of main thread.
    IThread* piProxyThread;

    // < config-id(slot id), system-config >
    IMSMap<IMS_SINT32, SystemConfig*> objSystemConfigs;
    IMSList<ISystemConfigListener*> objListeners;
};

#endif // _SYSTEM_CONFIG_MANAGER_H_
