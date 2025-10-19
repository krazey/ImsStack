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

#include "CallReasonInfo.h"
#include "IMtcService.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/MtcCall.h"
#include "call/MtcCallManager.h"
#include "call/NullCall.h"
#include "call/state/CallStateFactory.h"
#include <functional>
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

NullCall* const MtcCallManager::s_pNullCall = new NullCall();

PUBLIC
MtcCallManager::MtcCallManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_lstCalls(ImsList<MtcCall*>())
{
}

PUBLIC VIRTUAL MtcCallManager::~MtcCallManager() {}

PUBLIC
void MtcCallManager::Init() {}

PUBLIC
void MtcCallManager::DeInit()
{
    for (IMS_SINT32 nIndex = static_cast<IMS_SINT32>(m_lstCalls.GetSize()) - 1; nIndex >= 0;
            nIndex--)
    {
        MtcCall* pCall = m_lstCalls.GetAt(nIndex);
        pCall->Terminate(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE));
        delete pCall;
        m_lstCalls.RemoveAt(nIndex);
    }
}

PUBLIC VIRTUAL IMtcCall* MtcCallManager::CreateCall(
        IN ServiceType eServiceType, IN CallInfo& objCallInfo)
{
    IMtcService* pService = m_objContext.GetServiceByType(eServiceType);
    if (pService == IMS_NULL || pService->GetStatus() != ServiceStatus::SERVICE_ACTIVE)
    {
        IMS_TRACE_E(0, "CreateCall : Service not active - type[%d]",
                static_cast<IMS_SINT32>(eServiceType), 0, 0);
        return s_pNullCall;
    }

    MtcCall* pCall =
            new MtcCall(m_objContext, *pService, objCallInfo, std::make_unique<CallStateFactory>());
    m_lstCalls.Append(pCall);
    IMS_TRACE_D("CreateCall : call count[%d]", m_lstCalls.GetSize(), 0, 0);

    return pCall;
}

PUBLIC VIRTUAL void MtcCallManager::RemoveCall(IN CallKey nCallKey)
{
    IMS_SINT32 nIndex = GetFirstIndexByFilter(
            [nCallKey](const MtcCall* pCall)
            {
                return pCall->GetKey() == nCallKey;
            });

    if (nIndex >= 0)
    {
        MtcCall* pCall = m_lstCalls.GetAt(nIndex);
        delete pCall;
        m_lstCalls.RemoveAt(nIndex);
    }

    IMS_TRACE_D("RemoveCall : call count[%d]", m_lstCalls.GetSize(), 0, 0);
}

PUBLIC VIRTUAL IMtcCall* MtcCallManager::GetCallByCallKey(IN CallKey nCallKey)
{
    IMS_SINT32 nIndex = GetFirstIndexByFilter(
            [nCallKey](const MtcCall* pCall)
            {
                return pCall->GetKey() == nCallKey;
            });

    IMS_TRACE_D("GetCallByCallKey index[%d]", nIndex, 0, 0);
    if (nIndex >= 0)
    {
        return m_lstCalls.GetAt(nIndex);
    }
    else
    {
        return s_pNullCall;
    }
}

PUBLIC VIRTUAL ImsList<IMtcCall*> MtcCallManager::GetCalls()
{
    return GetCallsByFilter(
            [](MtcCall* /* pCall */)
            {
                return IMS_TRUE;
            });
}

PUBLIC VIRTUAL ImsList<IMtcCall*> MtcCallManager::GetCallsExcluding(IN CallKey nExcludingCallKey)
{
    return GetCallsByFilter(
            [nExcludingCallKey](const MtcCall* pCall)
            {
                return pCall->GetCallKey() != nExcludingCallKey;
            });
}

PUBLIC VIRTUAL ImsList<IMtcCall*> MtcCallManager::GetCallsByType(IN CallType eCallType)
{
    return GetCallsByFilter(
            [eCallType](const MtcCall* pCall)
            {
                return pCall->GetCallType() == eCallType;
            });
}

PUBLIC VIRTUAL ImsList<IMtcCall*> MtcCallManager::GetCallsByServiceType(IN ServiceType eServiceType)
{
    return GetCallsByFilter(
            [eServiceType](MtcCall* pCall)
            {
                return pCall->GetService().GetServiceType() == eServiceType;
            });
}

PUBLIC VIRTUAL ImsList<IMtcCall*> MtcCallManager::GetCallsInConference()
{
    return GetCallsByFilter(
            [](MtcCall* pCall)
            {
                return pCall->GetCallInfo().bConference;
            });
}

PUBLIC VIRTUAL ImsList<IMtcCall*> MtcCallManager::GetCallsByState(IN State eState)
{
    return GetCallsByFilter(
            [eState](const MtcCall* pCall)
            {
                return pCall->GetState() == eState;
            });
}

PRIVATE
IMS_SINT32 MtcCallManager::GetFirstIndexByFilter(
        IN const std::function<IMS_BOOL(MtcCall*)>& objFilter)
{
    // Call index mustn't be outside of IMS_SINT32 range.
    for (IMS_UINT32 nIndex = 0; nIndex < m_lstCalls.GetSize(); nIndex++)
    {
        MtcCall* pCall = m_lstCalls.GetAt(nIndex);

        if (pCall != IMS_NULL && objFilter(pCall))
        {
            return nIndex;
        }
    }

    return -1;
}

PRIVATE
ImsList<IMtcCall*> MtcCallManager::GetCallsByFilter(
        IN const std::function<IMS_BOOL(MtcCall*)>& objFilter)
{
    ImsList<IMtcCall*> lstResult;

    for (IMS_UINT32 nIndex = 0; nIndex < m_lstCalls.GetSize(); nIndex++)
    {
        MtcCall* pCall = m_lstCalls.GetAt(nIndex);

        if (pCall != IMS_NULL && objFilter(pCall))
        {
            lstResult.Append(pCall);
        }
    }

    return lstResult;
}
