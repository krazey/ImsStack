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

#include "options/UceOptionsManager.h"

#include "ICapabilities.h"
#include "ICoreService.h"
#include "IJniEnabler.h"
#include "IMessage.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IUce.h"
#include "ServiceMessage.h"
#include "Sip.h"
#include "SipAddress.h"
#include "options/UceOptions.h"
#include "IUceJniThread.h"
#include "JniEnablerConnector.h"

__IMS_TRACE_TAG_UCE__;

PUBLIC
UceOptionsManager::UceOptionsManager(
        IN const AString& strName, IN ICoreService* piCoreService, IN IMS_SINT32 simSlotId) :
        ImsActivityEx(strName),
        m_bAoSConnected(IMS_FALSE),
        m_nSimSlot(simSlotId),
        m_piCoreService(piCoreService),
        m_nReceivedOptionKey(0)
{
    m_nSimSlot = simSlotId;
    IMS_TRACE_D("UCE_M : UceOptionsManager = %" PFLS_u, sizeof(UceOptionsManager), 0, 0);
    IMS_TRACE_I("UceOptionsManager", 0, 0, 0);
    m_objSentUceOptionsMap = ImsMap<IMS_UINT32, UceOptions*>();
    m_objReceivedUceOptionsMap = ImsMap<IMS_UINT32, UceOptions*>();
}

PUBLIC VIRTUAL UceOptionsManager::~UceOptionsManager()
{
    IMS_TRACE_D("UCE_F : UceOptionsManager = %" PFLS_u, sizeof(UceOptionsManager), 0, 0);
    IMS_TRACE_I("~UceOptionsManager", 0, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objSentUceOptionsMap.GetSize(); i++)
    {
        UceOptions* pUceOptions = m_objSentUceOptionsMap.GetValueAt(i);
        if (pUceOptions != IMS_NULL)
        {
            delete pUceOptions;
        }
    }
    m_objSentUceOptionsMap.Clear();
    for (IMS_UINT32 i = 0; i < m_objReceivedUceOptionsMap.GetSize(); i++)
    {
        UceOptions* pUceOptions = m_objReceivedUceOptionsMap.GetValueAt(i);
        if (pUceOptions != IMS_NULL)
        {
            delete pUceOptions;
        }
    }
    m_objReceivedUceOptionsMap.Clear();
}

PUBLIC
IMS_BOOL UceOptionsManager::SendOptionsRequest(
        IN IMS_UINT32 nKey, IN const AString& strRemoteURI, IN IMS_UINT32 ownCapabilities)
{
    if (m_bAoSConnected == IMS_FALSE)
    {
        SendOptionsCommandError(nKey, IUUceService::COMMAND_CODE_GENERIC_FAILURE);
        return IMS_TRUE;
    }

    UceOptions* pOptions =
            new UceOptions(GetName(), m_piCoreService, IMS_NULL, nKey, IMS_TRUE, m_nSimSlot);
    if (pOptions->SendOptionsRequest(strRemoteURI, ownCapabilities))
    {
        m_objSentUceOptionsMap.Add(nKey, pOptions);
    }
    return IMS_TRUE;
}

IMS_BOOL UceOptionsManager::SendOptionsResponse(IN IMS_UINT32 nKey, IN IMS_UINT32 nResponse,
        IN const AString& reason, IN IMS_UINT32 ownCapabilities)
{
    UceOptions* pOptions = IMS_NULL;
    IMS_SLONG nIndex = m_objReceivedUceOptionsMap.GetIndexOfKey(nKey);
    if (nIndex < 0)
    {
        IMS_TRACE_I("SendOptionsResponse:Not handle the key[%d]", nKey, 0, 0);
        return IMS_FALSE;
    }
    pOptions = m_objReceivedUceOptionsMap.GetValueAt(nIndex);
    if (pOptions == IMS_NULL)
    {
        IMS_TRACE_I("SendOptionsResponse:Not handle the key[%d]", nKey, 0, 0);
        return IMS_FALSE;
    }
    pOptions->SendOptionsResponse(nResponse, reason, ownCapabilities);
    m_objReceivedUceOptionsMap.Remove(nKey);
    delete pOptions;
    return IMS_TRUE;
}

IMS_BOOL UceOptionsManager::ReceivedOptions(
        IN const ICoreService* piCoreService, IN ICapabilities* piCapabilities)
{
    if (piCoreService == IMS_NULL || piCapabilities == IMS_NULL || m_piCoreService != piCoreService)
    {
        IMS_TRACE_I("ReceivedOptions:piCoreService or piCapabilities is null", 0, 0, 0);
        return IMS_FALSE;
    }

    const IMessage* piMessage = piCapabilities->GetPreviousRequest(IMessage::CAPABILITIES_QUERY);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("ReceivedOptions:IMessage is null", 0, 0, 0);
        return IMS_FALSE;
    }

    const ISipMessage* piSIPMessage = piMessage->GetMessage();
    if (piSIPMessage == IMS_NULL)
    {
        IMS_TRACE_I("ReceivedOptions:ISipMessage is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 key = getReceivedKey();

    UceOptions* pOptions =
            new UceOptions(GetName(), m_piCoreService, piCapabilities, key, IMS_FALSE, m_nSimSlot);
    m_objReceivedUceOptionsMap.Add(key, pOptions);

    SipAddress objSIPAddress(piSIPMessage->GetHeader(ISipHeader::FROM));
    AString from = objSIPAddress.ToString();
    IMS_TRACE_D("ReceivedOptions:From [%s]", from.GetStr(), 0, 0);

    ImsList<AString> objContactList = piSIPMessage->GetHeaders(ISipHeader::CONTACT_NORMAL);
    IMS_UINT32 capabilities = pOptions->GetCapability(objContactList);
    SendOptionsReceivedInd(key, from, capabilities);
    return IMS_TRUE;
}

void UceOptionsManager::AoSConnected()
{
    m_bAoSConnected = IMS_TRUE;
}

void UceOptionsManager::AoSDisconnected()
{
    m_bAoSConnected = IMS_FALSE;
}

IMS_BOOL UceOptionsManager::ClosedService()
{
    IMS_TRACE_D("ClosedService", 0, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objSentUceOptionsMap.GetSize(); i++)
    {
        UceOptions* pUceOptions = m_objSentUceOptionsMap.GetValueAt(i);
        if (pUceOptions != IMS_NULL)
        {
            pUceOptions->AoSDisconnected();
        }
    }
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL UceOptionsManager::OnMessage(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("OnMessage: msg [%d]", objMsg.nMSG, 0, 0);
    if (objMsg.nMSG != IUUceService::UCE_OPTIONS_DELETED_IND)
    {
        IMS_TRACE_I("OnMessage:Not Support MSG", 0, 0, 0);
        return IMS_FALSE;
    }
    IMS_UINT32 nKey = objMsg.nLparam;
    UceOptions* pOptions = IMS_NULL;
    IMS_SLONG nIndex = m_objSentUceOptionsMap.GetIndexOfKey(nKey);
    if (nIndex < 0)
    {
        IMS_TRACE_I("OnMessage:Not handle the key[%d]", nKey, 0, 0);
        return IMS_FALSE;
    }
    pOptions = m_objSentUceOptionsMap.GetValueAt(nIndex);
    if (pOptions == IMS_NULL)
    {
        IMS_TRACE_I("OnMessage:Not handle the key[%d]", nKey, 0, 0);
        return IMS_FALSE;
    }
    m_objSentUceOptionsMap.Remove(nKey);
    delete pOptions;
    return IMS_TRUE;
}

PRIVATE
IMS_UINT32 UceOptionsManager::getReceivedKey()
{
    m_nReceivedOptionKey++;
    return m_nReceivedOptionKey;
}

void UceOptionsManager::SendOptionsReceivedInd(
        IN IMS_UINT32 nKey, IN AString from, IN IMS_UINT32 capabilities)
{
    IMS_TRACE_D("SendOptionsReceivedInd:key[%d], From[%s]", nKey, from.GetStr(), 0);
    IUceJniThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendOptionsReceivedInd:piJniThread is null", 0, 0, 0);
        return;
    }
    piJniThread->OptionsReceivedInd(nKey, from, capabilities);
}

void UceOptionsManager::SendOptionsCommandError(IN IMS_UINT32 nKey, IN IMS_UINT32 code)
{
    IMS_TRACE_D("SendOptionsCommandError:key[%d], error[%d]", nKey, code, 0);
    IUceJniThread* piJniThread = GetJniThread();
    if (piJniThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendOptionsCommandError:piJniThread is null", 0, 0, 0);
        return;
    }
    piJniThread->OptionsErrorInd(nKey, code);
}

IUceJniThread* UceOptionsManager::GetJniThread()
{
    const IJniEnabler* piJniEnabler =
            JniEnablerConnector::GetInstance().GetJniEnabler(m_nSimSlot, EnablerType::UCE);
    if (piJniEnabler == IMS_NULL)
    {
        return IMS_NULL;
    }

    return reinterpret_cast<IUceJniThread*>(piJniEnabler->GetJniThread());
}