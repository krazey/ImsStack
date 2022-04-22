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
    INetWatcherInfo* GetNetWatcherInfo(IN IMS_SINT32 nSlotId);
    ISRVCC* GetSrvcc(IN IMS_SINT32 nSlotId);

    IPhoneInfoCall* GetPhoneInfoCall(IN IMS_SINT32 nSlotId);

    IISIM* GetIsim(IN IMS_SINT32 nSlotId);
    IUSIM* GetUsim(IN IMS_SINT32 nSlotId);

    ILocationInfo* GetLocationInfo(IN IMS_SINT32 nSlotId);

private:
    ISubscriberInfo *piSubscriberInfo;
    INetWatcherInfo *piNetWatcherInfo;
    ISRVCC *piSrvcc;

    IPhoneInfoCall *piPhoneInfoCall;

    IISIM *piIsim;
    IUSIM *piUsim;

    ILocationInfo *piLocationInfo;
};

PUBLIC
PhoneInfoHolder::PhoneInfoHolder()
    : piSubscriberInfo(IMS_NULL)
    , piNetWatcherInfo(IMS_NULL)
    , piSrvcc(IMS_NULL)
    , piPhoneInfoCall(IMS_NULL)
    , piIsim(IMS_NULL)
    , piUsim(IMS_NULL)
    , piLocationInfo(IMS_NULL)
{
}

PUBLIC
PhoneInfoHolder::~PhoneInfoHolder()
{
    PlatformFactory::DestroySubscriberInfo(piSubscriberInfo);
    PlatformFactory::DestroyNetworkWatcher(piNetWatcherInfo);
    PlatformFactory::DestroySrvcc(piSrvcc);
    PlatformFactory::DestroyPhoneInfoCall(piPhoneInfoCall);
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
INetWatcherInfo* PhoneInfoHolder::GetNetWatcherInfo(IN IMS_SINT32 nSlotId)
{
    if (piNetWatcherInfo == IMS_NULL)
    {
        piNetWatcherInfo = PlatformFactory::CreateNetworkWatcher(nSlotId);
    }

    return piNetWatcherInfo;
}

PUBLIC
ISRVCC* PhoneInfoHolder::GetSrvcc(IN IMS_SINT32 nSlotId)
{
    if (piSrvcc == IMS_NULL)
    {
        piSrvcc = PlatformFactory::CreateSrvcc(nSlotId);
    }

    return piSrvcc;
}

PUBLIC
IPhoneInfoCall* PhoneInfoHolder::GetPhoneInfoCall(IN IMS_SINT32 nSlotId)
{
    if (piPhoneInfoCall == IMS_NULL)
    {
        piPhoneInfoCall = PlatformFactory::CreatePhoneInfoCall(nSlotId);
    }

    return piPhoneInfoCall;
}

PUBLIC
IISIM* PhoneInfoHolder::GetIsim(IN IMS_SINT32 nSlotId)
{
    if (piIsim == IMS_NULL)
    {
        piIsim = PlatformFactory::CreateIsim(nSlotId);
    }

    return piIsim;
}

PUBLIC
IUSIM* PhoneInfoHolder::GetUsim(IN IMS_SINT32 nSlotId)
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
    ITRM* GetTRM();

private:
    IDeviceInfo *piDeviceInfo;
    IPowerInfo *piPowerInfo;
    IWifiWatcher *piWifiWatcher;
    ITRM *piTRM;

    PhoneInfoHolder **ppHolder;
};

PUBLIC
PhoneInfoServicePrivate::PhoneInfoServicePrivate()
    : piDeviceInfo(IMS_NULL)
    , piPowerInfo(IMS_NULL)
    , piWifiWatcher(IMS_NULL)
    , piTRM(IMS_NULL)
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
    PlatformFactory::DestroyTrm(piTRM);

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
ITRM* PhoneInfoServicePrivate::GetTRM()
{
    if (piTRM == IMS_NULL)
    {
        piTRM = PlatformFactory::CreateTrm();
    }

    return piTRM;
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
void PhoneInfoService::DispatchServiceMessage(IN IMSMSG &objMSG)
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
        INetWatcherInfo *piNWI = GetNetWatcherInfo(nSlotId);

        if (piNWI != IMS_NULL)
        {
            piNWI->ProcessNotify(objMSG);
        }
    }
    else if (nMessage == IMS_MSG_ISIM)
    {
        ImsIsim *pISIM = DYNAMIC_CAST(ImsIsim*, GetISIM(nSlotId));

        if (pISIM != IMS_NULL)
        {
            pISIM->DispatchServiceMessage(objMSG.nWparam, objMSG.nLparam);
        }
    }
    else if (nMessage == IMS_MSG_USIM)
    {
        ImsUsim *pUSIM = DYNAMIC_CAST(ImsUsim*, GetUSIM(nSlotId));

        if (pUSIM != IMS_NULL)
        {
            pUSIM->DispatchServiceMessage(objMSG.nWparam, objMSG.nLparam);
        }
    }
    else if (nMessage == IMS_MSG_TRM_PRIORITY_STATUS)
    {
        ITRM *piTRM = GetTRM();

        if (piTRM != IMS_NULL)
        {
            piTRM->ProcessNotify(objMSG);
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
IPhoneInfoCall* PhoneInfoService::GetPhoneInfoCall(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetPhoneInfoCall(nSlotId);
}

PUBLIC
IISIM* PhoneInfoService::GetISIM(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
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
INetWatcherInfo* PhoneInfoService::GetNetWatcherInfo(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetNetWatcherInfo(nSlotId);
}

PUBLIC
ILocationInfo* PhoneInfoService::GetLocationInfo(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetLocationInfo(nSlotId);
}

PUBLIC
ISRVCC* PhoneInfoService::GetSRVCC(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
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
IUSIM* PhoneInfoService::GetUSIM(IN IMS_SINT32 nSlotId /*= IMS_SLOT_0*/)
{
    PhoneInfoHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetUsim(nSlotId);
}

PUBLIC
ITRM* PhoneInfoService::GetTRM()
{
    return pPrivate->GetTRM();
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
