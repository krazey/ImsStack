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

#include "subscribe/UceSubscribeManager.h"

#include <stdlib.h>

#include "ICoreService.h"
#include "IMessage.h"
#include "IUce.h"
#include "ServiceTrace.h"
#include "subscribe/UceSubscribe.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");
/* -------------------------------------------------------------------------------------------------
    Constructor, Destructor, Operator Overloading
-------------------------------------------------------------------------------------------------
*/
PUBLIC
UceSubscribeManager::UceSubscribeManager(IN const AString& strName, ICoreService* _piCoreService,
        IN const AString& strAppName, IN IMS_SINT32 nSimSlot) :
        ImsActivityEx(strName),
        m_piCoreService(_piCoreService),
        m_strAppName(strAppName),
        m_nSimSlot(nSimSlot),
        m_nConnectedServices(0)
{
    IMS_TRACE_D("UCE_M : UceSubscribeManager = %" PFLS_u, sizeof(UceSubscribeManager), 0, 0);
    IMS_TRACE_I("UceSubscribeManager", 0, 0, 0);
}

PUBLIC VIRTUAL UceSubscribeManager::~UceSubscribeManager()
{
    IMS_TRACE_D("UCE_F : UceSubscribeManager = %" PFLS_u, sizeof(UceSubscribeManager), 0, 0);
    IMS_TRACE_I("~UceSubscribeManager", 0, 0, 0);

    ClearSubscribeList();
}
/* -------------------------------------------------------------------------------------------------
    Methods
-------------------------------------------------------------------------------------------------
*/
PUBLIC
IMS_BOOL UceSubscribeManager::QuerySingleCapability(IN const AString& strUser, IN IMS_UINT32 key)
{
    UceSubscribe* pUceSubscribe = new UceSubscribe(
            m_piCoreService, m_strAppName, GetName(), m_nConnectedServices, m_nSimSlot);
    if (pUceSubscribe == IMS_NULL)
    {
        IMS_TRACE_I("QuerySingleCapability:UceSubscribe create failed", 0, 0, 0);
        return IMS_FALSE;
    }
    m_objUceSubscribeList.Append(pUceSubscribe);
    pUceSubscribe->QuerySingleCapability(strUser, key);
    return IMS_TRUE;
}

IMS_BOOL UceSubscribeManager::QueryMultiCapability(
        IN const IMSList<AString>& objUsers, IN IMS_UINT32 key)
{
    UceSubscribe* pUceSubscribe = new UceSubscribe(
            m_piCoreService, m_strAppName, GetName(), m_nConnectedServices, m_nSimSlot);
    if (pUceSubscribe == IMS_NULL)
    {
        IMS_TRACE_I("QuerySingleCapability:UceSubscribe create failed", 0, 0, 0);
        return IMS_FALSE;
    }
    m_objUceSubscribeList.Append(pUceSubscribe);
    pUceSubscribe->QueryMultiCapability(objUsers, key);
    return IMS_TRUE;
}

IMS_BOOL UceSubscribeManager::AosConnected(IMS_UINT32 conectedService)
{
    IMS_TRACE_D("AosConnected", 0, 0, 0);
    m_nConnectedServices = conectedService;
    return IMS_TRUE;
}

IMS_BOOL UceSubscribeManager::AosDisConnected()
{
    IMS_TRACE_D("AosDisConnected", 0, 0, 0);
    if (m_objUceSubscribeList.IsEmpty())
    {
        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < m_objUceSubscribeList.GetSize(); i++)
    {
        UceSubscribe* pUceSubscribe = m_objUceSubscribeList.GetAt(i);
        if (pUceSubscribe != IMS_NULL)
        {
            pUceSubscribe->AosDisConnected();
        }
    }
    return IMS_TRUE;
}

void UceSubscribeManager::ClosedService()
{
    IMS_TRACE_D("ClosedService", 0, 0, 0);
    if (m_objUceSubscribeList.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objUceSubscribeList.GetSize(); i++)
    {
        UceSubscribe* pUceSubscribe = m_objUceSubscribeList.GetAt(i);
        if (pUceSubscribe != IMS_NULL)
        {
            pUceSubscribe->AosDisConnected();
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL UceSubscribeManager::OnMessage(IN IMSMSG& objMsg)
{
    if (objMsg.nMSG == IUUceService::UCE_SUBSCRIBE_DELETED_IND)
    {
        IMS_TRACE_D("UCE_SUBSCRIBE_DELETED_IND", 0, 0, 0);
        UceSubscribe* pUceSubscribe = reinterpret_cast<UceSubscribe*>(objMsg.nLparam);
        RemoveSubscribe(pUceSubscribe);
    }
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL UceSubscribeManager::RemoveSubscribe(IN UceSubscribe* subscribe)
{
    IMS_TRACE_D("RemoveSubscribe", 0, 0, 0);
    if (subscribe == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objUceSubscribeList.GetSize(); i++)
    {
        if (m_objUceSubscribeList.GetAt(i) == subscribe)
        {
            IMS_TRACE_D("RemoveSubscribe:Find[%d]", i, 0, 0);
            delete subscribe;
            m_objUceSubscribeList.RemoveAt(i);
            return IMS_TRUE;
        }
    }
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL UceSubscribeManager::ClearSubscribeList()
{
    IMS_TRACE_D("AosDisConnected", 0, 0, 0);
    if (m_objUceSubscribeList.IsEmpty())
    {
        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < m_objUceSubscribeList.GetSize(); i++)
    {
        UceSubscribe* pUceSubscribe = m_objUceSubscribeList.GetAt(i);
        if (pUceSubscribe != IMS_NULL)
        {
            delete pUceSubscribe;
        }
    }
    m_objUceSubscribeList.Clear();
    return IMS_TRUE;
}
