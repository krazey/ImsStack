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
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "SystemConfig.h"
#include "IAosService.h"

#include "interface/IAosCallTracker.h"
#include "interface/IAosLocationStarter.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegStateManager.h"
#include "interface/IAosRetryRepository.h"
#include "interface/IAosSubscriberManager.h"
#include "interface/IAosTransaction.h"

#include "provider/AosDnsQuery.h"
#include "provider/AosKeepAlive.h"
#include "provider/AosLog.h"

#include "provider/AosProvider.h"

__IMS_TRACE_TAG_AOS__;

PUBLIC
AosProvider::AosProvider() :
        m_piLock(IMS_NULL),
        m_objParam(ImsMap<IMS_SINT32, ProviderParam*>())

{
    IMS_TRACE_D("AosProvider()", 0, 0, 0);

    m_piLock = MutexService::GetMutexService()->CreateMutex();

    for (int i = 0; i < SystemConfig::GetSupportedSimCount(); i++)
    {
        m_objParam.Add(i, new ProviderParam());
    }
}

PUBLIC VIRTUAL AosProvider::~AosProvider()
{
    IMS_TRACE_D("~AosProvider()", 0, 0, 0);

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC GLOBAL AosProvider* AosProvider::GetInstance()
{
    static AosProvider* s_pProvider = IMS_NULL;

    if (s_pProvider == IMS_NULL)
    {
        s_pProvider = new AosProvider();
    }

    return s_pProvider;
}

PUBLIC GLOBAL AosLog* AosProvider::GetLog()
{
    static AosLog* s_pLog = IMS_NULL;

    if (s_pLog == IMS_NULL)
    {
        s_pLog = new AosLog();
    }

    return s_pLog;
}

PUBLIC GLOBAL AosDnsQuery* AosProvider::CreateDnsQuery()
{
    return new AosDnsQuery();
}

PUBLIC GLOBAL AosKeepAlive* AosProvider::CreateKeepAlive(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return new AosKeepAlive(nSlotId);
}

PUBLIC
IAosCallTracker* AosProvider::GetCallTracker(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return m_objParam.GetValue(nSlotId)->m_piCallTracker;
}

PUBLIC
IAosLocationStarter* AosProvider::GetLocationStarter(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return m_objParam.GetValue(nSlotId)->m_piLocationStarter;
}

PUBLIC
IAosNConfiguration* AosProvider::GetNConfiguration(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return m_objParam.GetValue(nSlotId)->m_piNConfiguration;
}

PUBLIC
IAosRegStateManager* AosProvider::GetRegStateManager(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return m_objParam.GetValue(nSlotId)->m_piRegStateManager;
}

PUBLIC
IAosRetryRepository* AosProvider::GetRetryRepository(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return m_objParam.GetValue(nSlotId)->m_piRetryRepository;
}

PUBLIC
IAosService* AosProvider::GetService(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return m_objParam.GetValue(nSlotId)->m_piService;
}

PUBLIC
IAosSubscriberManager* AosProvider::GetSubscriberManager(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return m_objParam.GetValue(nSlotId)->m_piSubscriberManager;
}

PUBLIC
IAosTransaction* AosProvider::GetTransaction(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    return m_objParam.GetValue(nSlotId)->m_piTransaction;
}

PUBLIC
void AosProvider::SetCallTracker(IN IAosCallTracker* piCt, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_piLock);

    ProviderParam* pParam = m_objParam.GetValue(nSlotId);
    if (pParam != IMS_NULL)
    {
        pParam->m_piCallTracker = piCt;
    }
}

PUBLIC
void AosProvider::SetLocationStarter(
        IN IAosLocationStarter* piLs, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_piLock);

    ProviderParam* pParam = m_objParam.GetValue(nSlotId);
    if (pParam != IMS_NULL)
    {
        pParam->m_piLocationStarter = piLs;
        if (piLs != IMS_NULL)
        {
            piLs->SetSlotId(nSlotId);
        }
    }
}

PUBLIC
void AosProvider::SetNConfiguration(
        IN IAosNConfiguration* piNc, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_piLock);

    ProviderParam* pParam = m_objParam.GetValue(nSlotId);
    if (pParam != IMS_NULL)
    {
        pParam->m_piNConfiguration = piNc;
    }
}

PUBLIC
void AosProvider::SetRegStateManager(
        IN IAosRegStateManager* piRsm, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_piLock);

    ProviderParam* pParam = m_objParam.GetValue(nSlotId);
    if (pParam != IMS_NULL)
    {
        pParam->m_piRegStateManager = piRsm;
        if (piRsm != IMS_NULL)
        {
            piRsm->SetSlotId(nSlotId);
        }
    }
}

PUBLIC
void AosProvider::SetRetryRepository(
        IN IAosRetryRepository* piRetryRepository, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_piLock);

    ProviderParam* pParam = m_objParam.GetValue(nSlotId);
    if (pParam != IMS_NULL)
    {
        pParam->m_piRetryRepository = piRetryRepository;
    }
}

PUBLIC
void AosProvider::SetService(IN IAosService* piService, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_piLock);

    ProviderParam* pParam = m_objParam.GetValue(nSlotId);
    if (pParam != IMS_NULL)
    {
        pParam->m_piService = piService;
    }
}

PUBLIC
void AosProvider::SetSubscriberManager(
        IN IAosSubscriberManager* piSubscriberManager, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_piLock);

    ProviderParam* pParam = m_objParam.GetValue(nSlotId);
    if (pParam != IMS_NULL)
    {
        pParam->m_piSubscriberManager = piSubscriberManager;
    }
}

PUBLIC
void AosProvider::SetTransaction(
        IN IAosTransaction* piTransaction, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_piLock);

    ProviderParam* pParam = m_objParam.GetValue(nSlotId);
    if (pParam != IMS_NULL)
    {
        pParam->m_piTransaction = piTransaction;
    }
}
