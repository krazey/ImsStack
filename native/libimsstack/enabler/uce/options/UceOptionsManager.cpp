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
#include "IMessage.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IUUceService.h"
#include "IUce.h"
#include "ServiceMessage.h"
#include "Sip.h"
#include "options/UceOptions.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

PUBLIC
UceOptionsManager::UceOptionsManager(
        IN const AString& _strName, IN ICoreService* piCoreService, IN IMS_SINT32 simSlotId) :
        ImsActivityEx(_strName),
        m_nSimSlot(simSlotId),
        m_piCoreService(piCoreService),
        m_bAoSConnected(IMS_FALSE),
        m_nReceivedOptionKey(0)
{
    m_nSimSlot = simSlotId;
    IMS_TRACE_D("UCE_M : UceOptionsManager = %" PFLS_u, sizeof(UceOptionsManager), 0, 0);
    IMS_TRACE_I("UceOptionsManager", 0, 0, 0);
    m_objSentUceOptionsMap = IMSMap<IMS_UINT32, UceOptions*>();
    m_objReceivedUceOptionsMap = IMSMap<IMS_UINT32, UceOptions*>();
}

PUBLIC VIRTUAL UceOptionsManager::~UceOptionsManager()
{
    IMS_TRACE_D("UCE_F : UceOptionsManager = %" PFLS_u, sizeof(UceOptionsManager), 0, 0);
    IMS_TRACE_I("~UceOptionsManager", 0, 0, 0);
    UceOptions* pUceOptions = IMS_NULL;
    for (IMS_UINT32 i = 0; i < m_objSentUceOptionsMap.GetSize(); i++)
    {
        pUceOptions = m_objSentUceOptionsMap.GetValueAt(i);
        if (pUceOptions != IMS_NULL)
        {
            delete pUceOptions;
            pUceOptions = IMS_NULL;
        }
    }
    m_objSentUceOptionsMap.Clear();
    for (IMS_UINT32 i = 0; i < m_objReceivedUceOptionsMap.GetSize(); i++)
    {
        pUceOptions = m_objReceivedUceOptionsMap.GetValueAt(i);
        if (pUceOptions != IMS_NULL)
        {
            delete pUceOptions;
            pUceOptions = IMS_NULL;
        }
    }
    m_objReceivedUceOptionsMap.Clear();
}

PUBLIC
IMS_BOOL UceOptionsManager::SendOptionsRequest(
        IN IMS_UINT32 nKey, IN AString strRemoteURI, IN IMS_UINT32 ownCapabilities)
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
        IN AString reason, IN IMS_UINT32 ownCapabilities)
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
    pOptions = IMS_NULL;
    return IMS_TRUE;
}

IMS_BOOL UceOptionsManager::ReceivedOptions(
        IN ICoreService* piCoreService, IN ICapabilities* piCapabilities)
{
    if (piCoreService == IMS_NULL || piCapabilities || m_piCoreService != piCoreService)
    {
        IMS_TRACE_I("ReceivedOptions:piCoreService or piCapabilities is null", 0, 0, 0);
        return IMS_FALSE;
    }
    IMS_UINT32 key = getReceivedKey();

    UceOptions* pOptions =
            new UceOptions(GetName(), m_piCoreService, piCapabilities, key, IMS_FALSE, m_nSimSlot);
    m_objReceivedUceOptionsMap.Add(key, pOptions);

    ISipMessage* piSIPMessage =
            piCapabilities->GetPreviousRequest(IMessage::CAPABILITIES_QUERY)->GetMessage();
    SipAddress objSIPAddress(piSIPMessage->GetHeader(ISipHeader::FROM));
    AString from = objSIPAddress.ToString();
    IMS_TRACE_D("ReceivedOptions:From [%s]", from.GetStr(), 0, 0);

    IMSList<AString> objContactList = piSIPMessage->GetHeaders(ISipHeader::CONTACT_NORMAL);
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

void UceOptionsManager::ClosedService()
{
    IMS_TRACE_D("ClosedService", 0, 0, 0);
    UceOptions* pUceOptions = IMS_NULL;
    for (IMS_UINT32 i = 0; i < m_objSentUceOptionsMap.GetSize(); i++)
    {
        pUceOptions = m_objSentUceOptionsMap.GetValueAt(i);
        if (pUceOptions != IMS_NULL)
        {
            pUceOptions->AoSDisconnected();
        }
    }
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
    pOptions = IMS_NULL;
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
    IMS_TRACE_I("SendOptionsReceivedInd:key[%d], From[%s]", nKey, from.GetStr(), 0);
    IUceOptionsReceivedIndPrm* pParam = new IUceOptionsReceivedIndPrm();

    pParam->m_nKey = nKey;
    pParam->m_strRemote = from;
    pParam->m_nRemoteCaps = capabilities;

    IMSMSG objUIMsg(IUUceService::UCE_OPTIONS_RECEIVED_IND, 0, reinterpret_cast<IMS_UINTP>(pParam));
    MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
}

void UceOptionsManager::SendOptionsCommandError(IN IMS_UINT32 nKey, IN IMS_UINT32 code)
{
    IMS_TRACE_I("SendOptionsCommandError:key[%d], error[%d]", nKey, code, 0);
    IUceOptionsCmdErrorIndPrm* pParam = new IUceOptionsCmdErrorIndPrm();
    pParam->m_nKey = nKey;
    pParam->m_nCommandError = code;

    IMSMSG objUIMsg(
            IUUceService::UCE_OPTIONS_CMD_ERROR_IND, 0, reinterpret_cast<IMS_UINTP>(pParam));
    MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
}