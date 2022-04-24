/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  yhrhee@                   Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SystemConfig.h"
#include "PlatformFactory.h"
#include "ImsIsim.h"
#include "ImsUsim.h"
#include "ServicePhoneInfo.h"

class PhoneInfoHolder
{
public:
    PhoneInfoHolder();
    ~PhoneInfoHolder();

private:
    PhoneInfoHolder(IN const PhoneInfoHolder& objRHS);
    PhoneInfoHolder& operator=(IN const PhoneInfoHolder& objRHS);

public:
    ISubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 nSlotId);
    INetworkWatcher* GetNetworkWatcher(IN IMS_SINT32 nSlotId);
    ISrvcc* GetSrvcc(IN IMS_SINT32 nSlotId);
    ICallInfo* GetCallInfo(IN IMS_SINT32 nSlotId);
    IIsim* GetIsim(IN IMS_SINT32 nSlotId);
    IUsim* GetUsim(IN IMS_SINT32 nSlotId);

    ILocationInfo* GetLocationInfo(IN IMS_SINT32 nSlotId);

private:
    ISubscriberInfo *piSubscriberInfo;
    INetworkWatcher *piNetworkWatcher;
    ISrvcc *piSrvcc;
    ICallInfo *piCallInfo;
    IIsim *piIsim;
    IUsim *piUsim;
    ILocationInfo *piLocationInfo;
};

PUBLIC
PhoneInfoHolder::PhoneInfoHolder()
    : piSubscriberInfo(IMS_NULL)
    , piNetworkWatcher(IMS_NULL)
    , piSrvcc(IMS_NULL)
    , piCallInfo(IMS_NULL)
    , piIsim(IMS_NULL)
    , piUsim(IMS_NULL)
    , piLocationInfo(IMS_NULL)
{
}

PUBLIC
PhoneInfoHolder::~PhoneInfoHolder()
{
    PlatformFactory::DestroySubscriberInfo(piSubscriberInfo);
    PlatformFactory::DestroyNetworkWatcher(piNetworkWatcher);
    PlatformFactory::DestroySrvcc(piSrvcc);
    PlatformFactory::DestroyCallInfo(piCallInfo);
    PlatformFactory::DestroyIsim(piIsim);
    PlatformFactory::DestroyUsim(piUsim);
    PlatformFactory::DestroyLocationInfo(piLocationInfo);
}

PUBLIC
ISubscriberInfo* PhoneInfoHolder::GetSubscriberInfo(IN IMS_SINT32 nSlotId)
{
    if (piSubscriberInfo == IMS_NULL)
    {
        piSubscriberInfo = PlatformFactory::CreateSubscriberInfo(nSlotId);
    }

    return piSubscriberInfo;
}

PUBLIC
INetworkWatcher* PhoneInfoHolder::GetNetworkWatcher(IN IMS_SINT32 nSlotId)
{
    if (piNetworkWatcher == IMS_NULL)
    {
        piNetworkWatcher = PlatformFactory::CreateNetworkWatcher(nSlotId);
    }

    return piNetworkWatcher;
}

PUBLIC
ISrvcc* PhoneInfoHolder::GetSrvcc(IN IMS_SINT32 nSlotId)
{
    if (piSrvcc == IMS_NULL)
    {
        piSrvcc = PlatformFactory::CreateSrvcc(nSlotId);
    }

    return piSrvcc;
}

PUBLIC
ICallInfo* PhoneInfoHolder::GetCallInfo(IN IMS_SINT32 nSlotId)
{
    if (piCallInfo == IMS_NULL)
    {
        piCallInfo = PlatformFactory::CreateCallInfo(nSlotId);
    }

    return piCallInfo;
}

PUBLIC
IIsim* PhoneInfoHolder::GetIsim(IN IMS_SINT32 nSlotId)
{
    if (piIsim == IMS_NULL)
    {
        piIsim = PlatformFactory::CreateIsim(nSlotId);
    }

    return piIsim;
}

PUBLIC
IUsim* PhoneInfoHolder::GetUsim(IN IMS_SINT32 nSlotId)
{
    if (piUsim == IMS_NULL)
    {
        piUsim = PlatformFactory::CreateUsim(nSlotId);
    }

    return piUsim;
}

PUBLIC
ILocationInfo* PhoneInfoHolder::GetLocationInfo(IN IMS_SINT32 nSlotId)
{
    if (piLocationInfo == IMS_NULL)
    {
        piLocationInfo = PlatformFactory::CreateLocationInfo(nSlotId);
    }

    return piLocationInfo;
}



class PhoneInfoServicePrivate
{
public:
    PhoneInfoServicePrivate();
    ~PhoneInfoServicePrivate();

private:
    PhoneInfoServicePrivate(IN const PhoneInfoServicePrivate& objRHS);
    PhoneInfoServicePrivate& operator=(IN const PhoneInfoServicePrivate& objRHS);

public:
    inline PhoneInfoHolder* GetHolder(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return ppHolder[nSlotId];
    }

    IDeviceInfo* GetDeviceInfo();
    IPowerInfo* GetPowerInfo();
    IWifiWatcher* GetWifiWatcher();
    ITrm* GetTrm();

private:
    IDeviceInfo *piDeviceInfo;
    IPowerInfo *piPowerInfo;
    IWifiWatcher *piWifiWatcher;
    ITrm *piTrm;

    PhoneInfoHolder **ppHolder;
};

PUBLIC
PhoneInfoServicePrivate::PhoneInfoServicePrivate()
    : piDeviceInfo(IMS_NULL)
    , piPowerInfo(IMS_NULL)
    , piWifiWatcher(IMS_NULL)
    , piTrm(IMS_NULL)
    , ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ppHolder = new PhoneInfoHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        ppHolder[i] = new PhoneInfoHolder();
    }
}

PUBLIC
PhoneInfoServicePrivate::~PhoneInfoServicePrivate()
{
    PlatformFactory::DestroyDeviceInfo(piDeviceInfo);
    PlatformFactory::DestroyPowerInfo(piPowerInfo);
    PlatformFactory::DestroyWifiWatcher(piWifiWatcher);
    PlatformFactory::DestroyTrm(piTrm);

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
IDeviceInfo* PhoneInfoServicePrivate::GetDeviceInfo()
{
    if (piDeviceInfo == IMS_NULL)
    {
        piDeviceInfo = PlatformFactory::CreateDeviceInfo();
    }

    return piDeviceInfo;
}

PUBLIC
IPowerInfo* PhoneInfoServicePrivate::GetPowerInfo()
{
    if (piPowerInfo == IMS_NULL)
    {
        piPowerInfo = PlatformFactory::CreatePowerInfo();
    }

    return piPowerInfo;
}

PUBLIC
IWifiWatcher* PhoneInfoServicePrivate::GetWifiWatcher()
{
    if (piWifiWatcher == IMS_NULL)
    {
        piWifiWatcher = PlatformFactory::CreateWifiWatcher();
    }

    return piWifiWatcher;
}

PUBLIC
ITrm* PhoneInfoServicePrivate::GetTrm()
{
    if (piTrm == IMS_NULL)
    {
        piTrm = PlatformFactory::CreateTrm();
    }

    return piTrm;
}

PRIVATE
PhoneInfoService::PhoneInfoService()
    : pPrivate(new PhoneInfoServicePrivate())
{
}

PRIVATE
PhoneInfoService::~PhoneInfoService()
{
    if (pPrivate != IMS_NULL)
    {
        delete pPrivate;
    }
}

PUBLIC
void PhoneInfoService::DispatchServiceMessage(IN ImsMessage &objMSG)
{
    IMS_SINT32 nMessage = objMSG.GetName();

    if (nMessage == IMS_MSG_BATTERY)
    {
        GetPowerInfo()->ProcessNotify(objMSG);
        return;
    }
    else if (nMessage == IMS_MSG_WIFI_STATUS)
    {
        GetWifiWatcher()->ProcessNotify(objMSG);
        return;
    }

    IMS_SINT32 nSlotId = LONG_TO_INT(objMSG.nWparam);

    if (nMessage == IMS_MSG_NETWORK_STATUS)
    {
        INetworkWatcher* piWatcher = GetNetworkWatcher(nSlotId);

        if (piWatcher != IMS_NULL)
        {
            piWatcher->ProcessNotify(objMSG);
        }
    }
    else if (nMessage == IMS_MSG_ISIM)
    {
        ImsIsim *pISIM = DYNAMIC_CAST(ImsIsim*, GetIsim(nSlotId));

        if (pISIM != IMS_NULL)
        {
            pISIM->DispatchServiceMessage(objMSG.nWparam, objMSG.nLparam);
        }
    }
    else if (nMessage == IMS_MSG_USIM)
    {
        ImsUsim *pUSIM = DYNAMIC_CAST(ImsUsim*, GetUsim(nSlotId));

        if (pUSIM != IMS_NULL)
        {
            pUSIM->DispatchServiceMessage(objMSG.nWparam, objMSG.nLparam);
        }
    }
    else if (nMessage == IMS_MSG_TRM_PRIORITY_STATUS)
    {
        ITrm *piTrm = GetTrm();

        if (piTrm != IMS_NULL)
        {
            piTrm->ProcessNotify(objMSG);
        }
    }
}

PUBLIC
IDeviceInfo* PhoneInfoService::GetDeviceInfo()
{
    return pPrivate->GetDeviceInfo();
}

PUBLIC
ISubscriberInfo* PhoneInfoService::GetSubscriberInfo(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetSubscriberInfo(nSlotId);
}

PUBLIC
ICallInfo* PhoneInfoService::GetCallInfo(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetCallInfo(nSlotId);
}

PUBLIC
IIsim* PhoneInfoService::GetIsim(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetIsim(nSlotId);
}

PUBLIC
IPowerInfo* PhoneInfoService::GetPowerInfo()
{
    return pPrivate->GetPowerInfo();
}

PUBLIC
INetworkWatcher* PhoneInfoService::GetNetworkWatcher(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetNetworkWatcher(nSlotId);
}

PUBLIC
ILocationInfo* PhoneInfoService::GetLocationInfo(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetLocationInfo(nSlotId);
}

PUBLIC
ISrvcc* PhoneInfoService::GetSrvcc(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetSrvcc(nSlotId);
}

PUBLIC
IWifiWatcher* PhoneInfoService::GetWifiWatcher()
{
    return pPrivate->GetWifiWatcher();
}

PUBLIC
IUsim* PhoneInfoService::GetUsim(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetUsim(nSlotId);
}

PUBLIC
ITrm* PhoneInfoService::GetTrm()
{
    return pPrivate->GetTrm();
}

PUBLIC GLOBAL
PhoneInfoService* PhoneInfoService::GetPhoneInfoService()
{
    static PhoneInfoService *pPhoneInfoService = IMS_NULL;

    if (pPhoneInfoService == IMS_NULL)
    {
        pPhoneInfoService = new PhoneInfoService();
    }

    return pPhoneInfoService;
}
