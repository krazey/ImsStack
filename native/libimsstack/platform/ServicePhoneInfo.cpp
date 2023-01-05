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
#include "ImsIsim.h"
#include "ImsUsim.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "SystemConfig.h"

class PhoneInfoHolder
{
public:
    PhoneInfoHolder();
    ~PhoneInfoHolder();

    PhoneInfoHolder(IN const PhoneInfoHolder&) = delete;
    PhoneInfoHolder& operator=(IN const PhoneInfoHolder&) = delete;

public:
    ISubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 nSlotId);
    INetworkWatcher* GetNetworkWatcher(IN IMS_SINT32 nSlotId);
    ICallInfo* GetCallInfo(IN IMS_SINT32 nSlotId);
    ImsIsim* GetIsim(IN IMS_SINT32 nSlotId);
    ImsUsim* GetUsim(IN IMS_SINT32 nSlotId);

    ILocationInfo* GetLocationInfo(IN IMS_SINT32 nSlotId);

private:
    ISubscriberInfo* m_piSubscriberInfo;
    INetworkWatcher* m_piNetworkWatcher;
    ICallInfo* m_piCallInfo;
    ImsIsim* m_pIsim;
    ImsUsim* m_pUsim;
    ILocationInfo* m_piLocationInfo;
};

PUBLIC
PhoneInfoHolder::PhoneInfoHolder() :
        m_piSubscriberInfo(IMS_NULL),
        m_piNetworkWatcher(IMS_NULL),
        m_piCallInfo(IMS_NULL),
        m_pIsim(IMS_NULL),
        m_pUsim(IMS_NULL),
        m_piLocationInfo(IMS_NULL)
{
}

PUBLIC
PhoneInfoHolder::~PhoneInfoHolder()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    piOsFactory->DestroySubscriberInfo(m_piSubscriberInfo);
    piOsFactory->DestroyNetworkWatcher(m_piNetworkWatcher);
    piOsFactory->DestroyCallInfo(m_piCallInfo);
    piOsFactory->DestroyLocationInfo(m_piLocationInfo);

    if (m_pIsim != IMS_NULL)
    {
        m_pIsim->Destroy();
        m_pIsim = IMS_NULL;
    }

    if (m_pUsim != IMS_NULL)
    {
        m_pUsim->Destroy();
        m_pUsim = IMS_NULL;
    }
}

PUBLIC
ISubscriberInfo* PhoneInfoHolder::GetSubscriberInfo(IN IMS_SINT32 nSlotId)
{
    if (m_piSubscriberInfo == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piSubscriberInfo = piOsFactory->CreateSubscriberInfo(nSlotId);
    }

    return m_piSubscriberInfo;
}

PUBLIC
INetworkWatcher* PhoneInfoHolder::GetNetworkWatcher(IN IMS_SINT32 nSlotId)
{
    if (m_piNetworkWatcher == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piNetworkWatcher = piOsFactory->CreateNetworkWatcher(nSlotId);
    }

    return m_piNetworkWatcher;
}

PUBLIC
ICallInfo* PhoneInfoHolder::GetCallInfo(IN IMS_SINT32 nSlotId)
{
    if (m_piCallInfo == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piCallInfo = piOsFactory->CreateCallInfo(nSlotId);
    }

    return m_piCallInfo;
}

PUBLIC
ImsIsim* PhoneInfoHolder::GetIsim(IN IMS_SINT32 nSlotId)
{
    if (m_pIsim == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_pIsim = piOsFactory->CreateIsim(nSlotId);
    }

    return m_pIsim;
}

PUBLIC
ImsUsim* PhoneInfoHolder::GetUsim(IN IMS_SINT32 nSlotId)
{
    if (m_pUsim == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_pUsim = piOsFactory->CreateUsim(nSlotId);
    }

    return m_pUsim;
}

PUBLIC
ILocationInfo* PhoneInfoHolder::GetLocationInfo(IN IMS_SINT32 nSlotId)
{
    if (m_piLocationInfo == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piLocationInfo = piOsFactory->CreateLocationInfo(nSlotId);
    }

    return m_piLocationInfo;
}

class PhoneInfoServicePrivate
{
public:
    PhoneInfoServicePrivate();
    ~PhoneInfoServicePrivate();

    PhoneInfoServicePrivate(IN const PhoneInfoServicePrivate&) = delete;
    PhoneInfoServicePrivate& operator=(IN const PhoneInfoServicePrivate&) = delete;

public:
    inline PhoneInfoHolder* GetHolder(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return m_ppHolder[nSlotId];
    }

    IDeviceInfo* GetDeviceInfo();
    IPowerInfo* GetPowerInfo();
    IWifiWatcher* GetWifiWatcher();

private:
    IDeviceInfo* m_piDeviceInfo;
    IPowerInfo* m_piPowerInfo;
    IWifiWatcher* m_piWifiWatcher;

    PhoneInfoHolder** m_ppHolder;
};

PUBLIC
PhoneInfoServicePrivate::PhoneInfoServicePrivate() :
        m_piDeviceInfo(IMS_NULL),
        m_piPowerInfo(IMS_NULL),
        m_piWifiWatcher(IMS_NULL),
        m_ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

    m_ppHolder = new PhoneInfoHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppHolder[i] = new PhoneInfoHolder();
    }
}

PUBLIC
PhoneInfoServicePrivate::~PhoneInfoServicePrivate()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    piOsFactory->DestroyDeviceInfo(m_piDeviceInfo);
    piOsFactory->DestroyPowerInfo(m_piPowerInfo);
    piOsFactory->DestroyWifiWatcher(m_piWifiWatcher);

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
IDeviceInfo* PhoneInfoServicePrivate::GetDeviceInfo()
{
    if (m_piDeviceInfo == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piDeviceInfo = piOsFactory->CreateDeviceInfo();
    }

    return m_piDeviceInfo;
}

PUBLIC
IPowerInfo* PhoneInfoServicePrivate::GetPowerInfo()
{
    if (m_piPowerInfo == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piPowerInfo = piOsFactory->CreatePowerInfo();
    }

    return m_piPowerInfo;
}

PUBLIC
IWifiWatcher* PhoneInfoServicePrivate::GetWifiWatcher()
{
    if (m_piWifiWatcher == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piWifiWatcher = piOsFactory->CreateWifiWatcher();
    }

    return m_piWifiWatcher;
}

PRIVATE
PhoneInfoService::PhoneInfoService() :
        m_pPrivate(new PhoneInfoServicePrivate())
{
}

PRIVATE
PhoneInfoService::~PhoneInfoService()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
IDeviceInfo* PhoneInfoService::GetDeviceInfo()
{
    return m_pPrivate->GetDeviceInfo();
}

PUBLIC
IPowerInfo* PhoneInfoService::GetPowerInfo()
{
    return m_pPrivate->GetPowerInfo();
}

PUBLIC
ICallInfo* PhoneInfoService::GetCallInfo(IN IMS_SINT32 nSlotId)
{
    PhoneInfoHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetCallInfo(nSlotId);
}

PUBLIC
ILocationInfo* PhoneInfoService::GetLocationInfo(IN IMS_SINT32 nSlotId)
{
    PhoneInfoHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetLocationInfo(nSlotId);
}

PUBLIC
ISubscriberInfo* PhoneInfoService::GetSubscriberInfo(IN IMS_SINT32 nSlotId)
{
    PhoneInfoHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetSubscriberInfo(nSlotId);
}

PUBLIC
IIsim* PhoneInfoService::GetIsim(IN IMS_SINT32 nSlotId)
{
    PhoneInfoHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetIsim(nSlotId);
}

PUBLIC
IUsim* PhoneInfoService::GetUsim(IN IMS_SINT32 nSlotId)
{
    PhoneInfoHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetUsim(nSlotId);
}

PUBLIC
INetworkWatcher* PhoneInfoService::GetNetworkWatcher(IN IMS_SINT32 nSlotId)
{
    PhoneInfoHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetNetworkWatcher(nSlotId);
}

PUBLIC
IWifiWatcher* PhoneInfoService::GetWifiWatcher()
{
    return m_pPrivate->GetWifiWatcher();
}

PUBLIC
void PhoneInfoService::DispatchServiceMessage(IN ImsMessage& objMsg)
{
    IMS_SINT32 nMessage = objMsg.GetName();

    if (nMessage == IMS_MSG_BATTERY)
    {
        GetPowerInfo()->ProcessNotify(objMsg);
        return;
    }
    else if (nMessage == IMS_MSG_WIFI_STATUS)
    {
        GetWifiWatcher()->ProcessNotify(objMsg);
        return;
    }

    IMS_SINT32 nSlotId = LONG_TO_INT(objMsg.nWparam);

    if (nMessage == IMS_MSG_NETWORK_STATUS)
    {
        INetworkWatcher* piWatcher = GetNetworkWatcher(nSlotId);

        if (piWatcher != IMS_NULL)
        {
            piWatcher->ProcessNotify(objMsg);
        }
    }
    else if (nMessage == IMS_MSG_ISIM)
    {
        PhoneInfoHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
        ImsIsim* pIsim = pHolder->GetIsim(nSlotId);

        if (pIsim != IMS_NULL)
        {
            pIsim->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
        }
    }
    else if (nMessage == IMS_MSG_USIM)
    {
        PhoneInfoHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
        ImsUsim* pUsim = pHolder->GetUsim(nSlotId);

        if (pUsim != IMS_NULL)
        {
            pUsim->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
        }
    }
}

PUBLIC GLOBAL PhoneInfoService* PhoneInfoService::GetPhoneInfoService()
{
    return DYNAMIC_CAST(PhoneInfoService*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_PHONE_INFO));
}
