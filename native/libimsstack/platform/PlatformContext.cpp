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
#include "OsFactory.h"
#include "PlatformContext.h"
#include "ServiceConfig.h"
#include "ServiceEvent.h"
#include "ServiceFile.h"
#include "ServiceImsRadio.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceThread.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "system-intf/System.h"

PUBLIC GLOBAL PlatformContext* PlatformContext::s_pContext = IMS_NULL;

PRIVATE
PlatformContext::PlatformContext() :
        m_piDefaultOsFactory(IMS_NULL),
        m_piOsFactory(IMS_NULL),
        m_piDefaultSystem(IMS_NULL),
        m_piSystem(IMS_NULL)
{
    m_piDefaultOsFactory = new OsFactory();
    m_objDefaultServices.Add(SERVICE_MUTEX, new MutexService());
    m_objDefaultServices.Add(SERVICE_TRACE, new TraceService());
    m_objDefaultServices.Add(SERVICE_FILE, new FileService());
    m_objDefaultServices.Add(SERVICE_SYSTEM_TIME, new SystemTimeService());
    m_objDefaultServices.Add(SERVICE_UTIL, new UtilService());

    // All other default services: lazy initialization.
}

PRIVATE
PlatformContext::~PlatformContext()
{
    IMS_SINT32 nServiceCount = static_cast<IMS_SINT32>(m_objServices.GetSize());

    for (IMS_SINT32 i = nServiceCount - 1; i >= 0; --i)
    {
        PlatformService* pService = m_objServices.GetValueAt(i);

        if (pService != IMS_NULL)
        {
            pService->Destroy();
        }
    }

    m_objServices.Clear();

    nServiceCount = static_cast<IMS_SINT32>(m_objDefaultServices.GetSize());

    for (IMS_SINT32 i = nServiceCount - 1; i >= 0; --i)
    {
        PlatformService* pService = m_objDefaultServices.GetValueAt(i);

        if (pService != IMS_NULL)
        {
            pService->Destroy();
        }
    }

    m_objDefaultServices.Clear();

    if (m_piOsFactory != IMS_NULL)
    {
        m_piOsFactory->Destroy();
    }

    if (m_piDefaultOsFactory != IMS_NULL)
    {
        m_piDefaultOsFactory->Destroy();
    }

    if (m_piSystem != IMS_NULL)
    {
        m_piSystem->Destroy();
    }

    if (m_piDefaultSystem != IMS_NULL)
    {
        m_piDefaultSystem->Destroy();
    }
}

PUBLIC
PlatformService* PlatformContext::GetService(IN PlatformServiceType eType) const
{
    if (eType <= SERVICE_NONE || eType >= SERVICE_MAX)
    {
        return IMS_NULL;
    }

    PlatformService* pService = IMS_NULL;
    IMS_SLONG nIndex = m_objServices.GetIndexOfKey(eType);

    if (nIndex >= 0)
    {
        pService = m_objServices.GetValueAt(nIndex);
    }

    if (pService == IMS_NULL)
    {
        pService = GetDefaultService(eType);
    }

    return pService;
}

PUBLIC
PlatformService* PlatformContext::SetService(
        IN PlatformServiceType eType, IN PlatformService* pService)
{
    if (eType <= SERVICE_NONE || eType >= SERVICE_MAX)
    {
        return IMS_NULL;
    }

    PlatformService* pOldService = IMS_NULL;
    IMS_SLONG nIndex = m_objServices.GetIndexOfKey(eType);

    if (nIndex >= 0)
    {
        pOldService = m_objServices.GetValueAt(nIndex);
    }

    m_objServices.SetValue(eType, pService);

    return pOldService;
}

PUBLIC
IOsFactory* PlatformContext::GetOsFactory() const
{
    if (m_piOsFactory != IMS_NULL)
    {
        return m_piOsFactory;
    }

    return m_piDefaultOsFactory;
}

PUBLIC
IOsFactory* PlatformContext::SetOsFactory(IN IOsFactory* piOsFactory)
{
    IOsFactory* piOldOsFactory = m_piOsFactory;
    m_piOsFactory = piOsFactory;
    return piOldOsFactory;
}

PUBLIC
ISystem* PlatformContext::GetSystem() const
{
    if (m_piSystem != IMS_NULL)
    {
        return m_piSystem;
    }

    if (m_piDefaultSystem == IMS_NULL)
    {
        // Lazy binding.
        m_piDefaultSystem = System::GetInstance();
    }

    return m_piDefaultSystem;
}

PUBLIC
ISystem* PlatformContext::SetSystem(IN ISystem* piSystem)
{
    ISystem* piOldSystem = m_piSystem;
    m_piSystem = piSystem;
    return piOldSystem;
}

PUBLIC GLOBAL PlatformContext* PlatformContext::GetInstance()
{
    if (s_pContext == IMS_NULL)
    {
        s_pContext = new PlatformContext();
    }

    return s_pContext;
}

PUBLIC GLOBAL void PlatformContext::DestroyInstance()
{
    if (s_pContext != IMS_NULL)
    {
        delete s_pContext;
        s_pContext = IMS_NULL;
    }
}

PRIVATE
PlatformService* PlatformContext::GetDefaultService(IN PlatformServiceType eType) const
{
    PlatformService* pService = IMS_NULL;
    IMS_SLONG nIndex = m_objDefaultServices.GetIndexOfKey(eType);

    if (nIndex >= 0)
    {
        pService = m_objDefaultServices.GetValueAt(nIndex);
    }

    if (pService != IMS_NULL)
    {
        return pService;
    }

    switch (eType)
    {
        case SERVICE_MUTEX:
            pService = new MutexService();
            break;
        case SERVICE_FILE:
            pService = new FileService();
            break;
        case SERVICE_SYSTEM_TIME:
            pService = new SystemTimeService();
            break;
        case SERVICE_TRACE:
            pService = new TraceService();
            break;
        case SERVICE_PHONE_INFO:
            pService = new PhoneInfoService();
            break;
        case SERVICE_UTIL:
            pService = new UtilService();
            break;
        case SERVICE_CONFIG:
            pService = new ConfigService();
            break;
        case SERVICE_TIMER:
            pService = new TimerService();
            break;
        case SERVICE_THREAD:
            pService = new ThreadService();
            break;
        case SERVICE_EVENT:
            pService = new EventService();
            break;
        case SERVICE_NETWORK:
            pService = new NetworkService();
            break;
        case SERVICE_NETWORK_POLICY:
            pService = new NetworkServicePolicy();
            break;
        case SERVICE_RADIO:
            pService = new ImsRadioService();
            break;
        default:
            // no-op
            break;
    }

    if (pService != IMS_NULL)
    {
        m_objDefaultServices.Add(eType, pService);
    }

    return pService;
}
