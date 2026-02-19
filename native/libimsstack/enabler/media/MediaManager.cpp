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

#include "MediaManager.h"

#include "EnablerUtils.h"
#include "IJniMedia.h"
#include "IMediaSession.h"
#include "IService.h"
#include "JniEnablerConnector.h"
#include "MediaMsgHandler.h"
#include "MediaResourceManager.h"
#include "MediaSession.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_MEDIA__;

#define CALL_KEY_BROADCAST 0

PROTECTED GLOBAL ImsMap<IMS_SINT32, MediaManager*> MediaManager::m_objMapMediaManager;

PROTECTED MediaManager::MediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId) :
        ImsActivityEx(strName),
        m_nSlotId(nSlotId),
        m_lstSessionNode(ImsList<std::shared_ptr<MediaSessionNode>>()),
        m_pResourceMngr(std::make_shared<MediaResourceManager>())
{
    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_nSlotId, EnablerType::MEDIA_SESSION, DYNAMIC_CAST(INativeEnabler*, this));
}

PROTECTED VIRTUAL MediaManager::~MediaManager()
{
    IMS_SLONG nIndex = m_objMapMediaManager.GetIndexOfKey(m_nSlotId);

    if (nIndex >= 0)
    {
        m_objMapMediaManager.RemoveAt(nIndex);
    }

    ClearMediaSessionNode();

    if (m_lstSessionNode.GetSize() == 0)
    {
        IMS_TRACE_D("~MediaManager(): MediaManager released.", 0, 0, 0);
    }

    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_nSlotId, EnablerType::MEDIA_SESSION, IMS_NULL);
}

PUBLIC MediaManager* MediaManager::GetInstance(IN IMS_SINT32 nSlotId)
{
    IMS_SLONG nIndex = m_objMapMediaManager.GetIndexOfKey(nSlotId);
    MediaManager* pMediaManager = IMS_NULL;

    if (nIndex < 0)
    {
        pMediaManager = new MediaManager("MediaManager", nSlotId);
        m_objMapMediaManager.Add(nSlotId, pMediaManager);
    }
    else
    {
        pMediaManager = m_objMapMediaManager.GetValueAt(nIndex);
    }

    return pMediaManager;
}

PUBLIC AString MediaManager::GetThreadName(IN IMS_SINT32 nSlotId)
{
    AString strEnablerThread = EnablerUtils::GetEnablerThreadName(nSlotId);
    AString strMediaManagerThread;
    return strMediaManagerThread.Sprintf("%s.MediaManager", strEnablerThread.GetStr());
}

PUBLIC VIRTUAL std::shared_ptr<MediaMsgHandler> MediaManager::GetHandler(IN IMS_SINTP nCallKey)
{
    const std::shared_ptr<MediaSessionNode> pSessionNode = FindSessionNode(nCallKey);

    if (pSessionNode == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pSessionNode->pMessageHandler;
}

PUBLIC VIRTUAL IMediaSession* MediaManager::CreateSession(IN MEDIA_NETWORK_TYPE eNetwork,
        IN MEDIA_SERVICE_TYPE eServiceType, IN IService* pIService, IN IMS_SINTP nCallKey)
{
    IMS_TRACE_D("CreateSession(): NetworkType[%d], ServiceType[%d], CallKey[%d]", eNetwork,
            eServiceType, nCallKey);

    if (pIService == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSession(): invalid service interface", 0, 0, 0);
        return IMS_NULL;
    }

    auto pSession =
            std::make_shared<MediaSession>(eNetwork, eServiceType, pIService, nCallKey, m_nSlotId);
    auto pHandler = CreateMessageHandler(nCallKey);
    auto pSessionNode =
            std::make_shared<MediaSessionNode>(nCallKey, std::move(pSession), std::move(pHandler));
    IMediaSession* result = pSessionNode->pMediaSession.get();
    m_lstSessionNode.Append(std::move(pSessionNode));
    IMS_TRACE_D("CreateSession(): ListSize[%d]", m_lstSessionNode.GetSize(), 0, 0);
    return result;
}

PUBLIC VIRTUAL void MediaManager::DestroySession(IN const IMediaSession* piSession)
{
    if (piSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "DestroySession(): invalid session", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
    {
        auto pSessionNode = m_lstSessionNode.GetAt(i);

        if (pSessionNode->pMediaSession.get() == piSession)
        {
            pSessionNode->pMediaSession->Terminate();
            DeleteMediaSessionNode(pSessionNode, i);
            return;
        }
    }
}

PUBLIC VIRTUAL MediaSession* MediaManager::GetSession(IN IMS_SINTP nCallKey)
{
    const std::shared_ptr<MediaSessionNode> pSessionNode = FindSessionNode(nCallKey);

    if (pSessionNode == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pSessionNode->pMediaSession.get();
}
PUBLIC MediaResourceManager* MediaManager::GetResourceManager()
{
    return m_pResourceMngr.get();
}

PUBLIC VIRTUAL IMS_BOOL MediaManager::SendMessage(
        IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam)
{
    IMS_TRACE_I(
            "SendMessage(): MSG[%d, %s], CallKey[%d]", nMsg, IJniMedia::PrintMsg(nMsg), nCallKey);

    IMS_BOOL bResult = IMS_TRUE;

    if (nCallKey == CALL_KEY_BROADCAST)  // broadcast message to the all sessions
    {
        for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
        {
            std::shared_ptr<MediaSessionNode> pSessionNode = m_lstSessionNode.GetAt(i);

            if (pSessionNode != IMS_NULL && pSessionNode->pMediaSession)
            {
                if (!pSessionNode->pMediaSession->SendMessage(nMsg, pParam))
                {
                    IMS_TRACE_I("SendMessage() failed MSG[%d, %s], CallKey[%d]", nMsg,
                            IJniMedia::PrintMsg(nMsg), pSessionNode->nCallKey);
                    bResult = IMS_FALSE;
                }
            }
            else
            {
                bResult = IMS_FALSE;
            }
        }
    }
    else
    {
        if (!SendMessageToSessions(nMsg, nCallKey, pParam))
        {
            IMS_TRACE_I("SendMessage(): Fail to process nMsg", 0, 0, 0);
            bResult = IMS_FALSE;
        }
    }

    delete reinterpret_cast<ImsMediaMsgParamBase*>(pParam);
    return bResult;
}

PUBLIC VIRTUAL IMS_BOOL MediaManager::HandleRequestMsg(
        IN IMS_SINT32 eEvent, IN IMS_SINTP nCallKey, IN ImsMediaMsgParamBase* param)
{
    IMS_TRACE_I("HandleRequestMsg(): MediaType[%s], MSG[%s], CallKey[%d]",
            IJniMedia::PrintMediaType(param->m_eMediaType), IJniMedia::PrintMsg(eEvent), nCallKey);

    auto pNode = FindSessionNode(nCallKey);

    if (pNode == IMS_NULL || pNode->pMessageHandler == IMS_NULL)
    {
        IMS_TRACE_I(
                "HandleRequestMsg(): Can't find session node or message handler for CallKey[%d]",
                nCallKey, 0, 0);
        return IMS_FALSE;
    }

    if (pNode->pMessageHandler->SendMessageToJava(eEvent, param))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
VIRTUAL void MediaManager::HandleMessage(
        IN IMS_SINT32 nMsg, IN IMS_UINTP wParam, IN IMS_SINTP lParam)
{
    SendMessage(nMsg, wParam, lParam);
}

PROTECTED
IMS_BOOL MediaManager::DispatchMessage(IN ImsMessage& objMsg)
{
    IMS_TRACE_I("DispatchMessage(): nMsg[%d]", objMsg.GetName(), 0, 0);
    HandleMessage(objMsg.GetName(), objMsg.nWparam, objMsg.nLparam);
    return IMS_TRUE;
}

PROTECTED
void MediaManager::ClearMediaSessionNode()
{
    IMS_TRACE_D("ClearMediaSessionNode(): list size[%d]", m_lstSessionNode.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
    {
        auto pSessionNode = m_lstSessionNode.GetAt(i);
        if (pSessionNode != IMS_NULL && pSessionNode->pMediaSession != IMS_NULL)
        {
            pSessionNode->pMediaSession->Terminate();
        }
    }

    m_lstSessionNode.Clear();
}

PROTECTED
void MediaManager::DeleteMediaSessionNode(
        IN std::shared_ptr<MediaSessionNode> pSessionNode, IMS_UINT32 nIndex)
{
    if (pSessionNode != IMS_NULL)
    {
        IMS_TRACE_D("DeleteMediaSessionNode(): Index[%d]", nIndex, 0, 0);
    }

    m_lstSessionNode.RemoveAt(nIndex);
}

PROTECTED VIRTUAL std::shared_ptr<MediaMsgHandler> MediaManager::CreateMessageHandler(
        IN IMS_SINTP nCallKey)
{
    return std::make_shared<MediaMsgHandler>(m_nSlotId, nCallKey);
}

PROTECTED VIRTUAL std::shared_ptr<MediaManager::MediaSessionNode> MediaManager::FindSessionNode(
        IN IMS_SINTP nCallKey)
{
    for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
    {
        auto pSessionNode = m_lstSessionNode.GetAt(i);

        if (pSessionNode != IMS_NULL)
        {
            if (pSessionNode->nCallKey == nCallKey)
            {
                return pSessionNode;
            }
        }
    }

    return IMS_NULL;
}

PROTECTED VIRTUAL IMS_BOOL MediaManager::SendMessageToSessions(
        IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam)
{
    MediaSession* pMediaSession = GetSession(nCallKey);

    if (pMediaSession != IMS_NULL)
    {
        return pMediaSession->SendMessage(nMsg, pParam);
    }

    return IMS_FALSE;
}
