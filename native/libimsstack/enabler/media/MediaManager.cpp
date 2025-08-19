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

#include "EnablerUtils.h"
#include "ServiceTrace.h"

#include "IJniMedia.h"
#include "IMediaSession.h"
#include "MediaManager.h"
#include "MediaSession.h"
#include "MediaMsgHandler.h"
#include "MediaResourceManager.h"
#include "JniEnablerConnector.h"

__IMS_TRACE_TAG_MEDIA__;

#define CALL_KEY_BROADCAST 0

PROTECTED GLOBAL ImsMap<IMS_SINT32, MediaManager*> MediaManager::m_objMapMediaManager;

PROTECTED
MediaManager::MediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId) :
        ImsActivityEx(strName),
        m_nSlotId(nSlotId),
        m_lstSessionNode(ImsList<MediaSessionNode*>()),
        m_pResourceMngr(new MediaResourceManager(nSlotId))
{
    IMS_TRACE_D("+MediaManager() thread[%s], nSlotId[%d]", strName.GetStr(), nSlotId, 0);

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

    delete m_pResourceMngr;
    m_pResourceMngr = IMS_NULL;

    ClearMediaSessionNode();

    if (m_lstSessionNode.GetSize() == 0)
    {
        IMS_TRACE_D("~MediaManager() - MediaManager released.", 0, 0, 0);
    }
    else
    {
        IMS_TRACE_E(0, "~MediaManager() - MediaSession IS NOT released. Size[%d]",
                m_lstSessionNode.GetSize(), 0, 0);
    }

    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_nSlotId, EnablerType::MEDIA_SESSION, IMS_NULL);
}

PUBLIC
MediaManager* MediaManager::GetInstance(IN IMS_SINT32 nSlotId)
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

PUBLIC
AString MediaManager::GetThreadName(IN IMS_SINT32 nSlotId)
{
    AString strEnablerThread = EnablerUtils::GetEnablerThreadName(nSlotId);
    AString strMediaManagerThread;
    return strMediaManagerThread.Sprintf("%s.MediaManager", strEnablerThread.GetStr());
}

PUBLIC
MediaMsgHandler* MediaManager::GetHandler(IN IMS_SINTP nCallKey)
{
    const MediaSessionNode* pSessionNode = FindSessionNode(nCallKey);

    if (pSessionNode == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pSessionNode->pMessageHandler;
}

PUBLIC VIRTUAL IMediaSession* MediaManager::CreateSession(IN MEDIA_NETWORK_TYPE eNetwork,
        IN MEDIA_SERVICE_TYPE eServiceType, IN IService* pIService, IN IMS_SINTP nCallKey)
{
    IMS_TRACE_D("CreateSession() - NetworkType[%d], ServiceType[%d], CallKey[%d]", eNetwork,
            eServiceType, nCallKey);

    if (pIService == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSession() - invalid service interface", 0, 0, 0);
        return IMS_NULL;
    }

    MediaSession* pSession =
            new MediaSession(eNetwork, eServiceType, pIService, nCallKey, m_nSlotId);

    MediaMsgHandler* pHandler = new MediaMsgHandler(m_nSlotId, nCallKey);
    MediaSessionNode* pSessionNode = new MediaSessionNode(nCallKey, pSession, pHandler);
    m_lstSessionNode.Append(pSessionNode);

    // update pdn
    if (m_pResourceMngr == IMS_NULL ||
            !m_pResourceMngr->UpdatePdn(eServiceType == MEDIA_SERVICE_EMERGENCY
                            ? MediaResourceManager::PDN_EMERGENCY
                            : MediaResourceManager::PDN_IMS,
                    pIService->GetIpAddress()))
    {
        IMS_TRACE_E(0, "CreateSession() - fail to update pdn", 0, 0, 0);
    }

    IMS_TRACE_D("CreateSession() - ListSize[%d]", m_lstSessionNode.GetSize(), 0, 0);

    return pSession;
}

PUBLIC VIRTUAL void MediaManager::DestroySession(IN const IMediaSession* piSession)
{
    if (piSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "DestroySession() - invalid session", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("DestroySession", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
    {
        MediaSessionNode* pSessionNode = m_lstSessionNode.GetAt(i);

        if (pSessionNode->pMediaSession == piSession)
        {
            DeleteMediaSessionNode(pSessionNode, i);
            return;
        }
    }
}

PUBLIC
MediaSession* MediaManager::GetSession(IN IMS_SINTP nCallKey)
{
    const MediaSessionNode* pSessionNode = FindSessionNode(nCallKey);

    if (pSessionNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetSession() - cannot find matched session node", 0, 0, 0);
        return IMS_NULL;
    }

    return pSessionNode->pMediaSession;
}

PUBLIC
MediaResourceManager* MediaManager::GetResourceManager()
{
    return m_pResourceMngr;
}

PUBLIC VIRTUAL IMS_BOOL MediaManager::SendMessage(
        IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam)
{
    IMS_TRACE_I(
            "SendMessage() - MSG[%d, %s], CallKey[%d]", nMsg, IJniMedia::PrintMsg(nMsg), nCallKey);

    IMS_BOOL bResult = IMS_TRUE;

    if (nCallKey == CALL_KEY_BROADCAST)  // broadcast message to the all sessions
    {
        for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
        {
            MediaSessionNode* pSessionNode = m_lstSessionNode.GetAt(i);

            if (pSessionNode != IMS_NULL && pSessionNode->pMediaSession != IMS_NULL)
            {
                if (pSessionNode->pMediaSession->SendMessage(nMsg, pParam) != IMS_TRUE)
                {
                    IMS_TRACE_E(0, "SendMessage() failed MSG[%d, %s], CallKey[%d]", nMsg,
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
        if (SendMessageToSessions(nMsg, nCallKey, pParam) != IMS_TRUE)
        {
            IMS_TRACE_E(0, "SendMessage() - Fail to process nMsg", 0, 0, 0);
            bResult = IMS_FALSE;
        }
    }

    ImsMediaMsgParamBase* pTempParam = reinterpret_cast<ImsMediaMsgParamBase*>(pParam);
    delete pTempParam;

    return bResult;
}

PUBLIC VIRTUAL IMS_BOOL MediaManager::HandleRequestMsg(
        IN IMS_SINT32 eEvent, IN IMS_SINTP nCallKey, IN ImsMediaMsgParamBase* param)
{
    IMS_TRACE_I("HandleRequestMsg() - MediaType[%s], MSG[%s], CallKey[%d]",
            IJniMedia::PrintMediaType(param->m_eMediaType), IJniMedia::PrintMsg(eEvent), nCallKey);

    MediaSessionNode* pNode = FindSessionNode(nCallKey);

    if (pNode == IMS_NULL || pNode->pMessageHandler == IMS_NULL)
    {
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
    IMS_TRACE_I("HandleMessage() - nMsg[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IJniMedia::NOTIFY_QOS_INFO:
        {
            ImsMediaMsgQosParam* pQosNotifyParam = reinterpret_cast<ImsMediaMsgQosParam*>(lParam);

            if (pQosNotifyParam != IMS_NULL)
            {
                IMS_SINTP nCallKey = wParam;
                MediaSession* pSession = GetSession(nCallKey);
                if (pSession != IMS_NULL)
                {
                    IMS_TRACE_I("HandleMessage() - QOS - CallKey[%d], nPort[%d], bResult[%d]",
                            nCallKey, pQosNotifyParam->m_nPort, pQosNotifyParam->m_bResult);
                    pSession->SendMessage(nMsg, lParam);
                }
            }
        }
        break;
    }
}

PROTECTED
IMS_BOOL MediaManager::DispatchMessage(IN ImsMessage& objMsg)
{
    IMS_TRACE_I("DispatchMessage() - nMsg[%d]", objMsg.GetName(), 0, 0);
    HandleMessage(objMsg.GetName(), objMsg.nWparam, objMsg.nLparam);
    return IMS_TRUE;
}

PROTECTED
void MediaManager::ClearMediaSessionNode()
{
    IMS_TRACE_D("ClearMediaSessionNode() - list size[%d]", m_lstSessionNode.GetSize(), 0, 0);

    while (m_lstSessionNode.GetSize() > 0)
    {
        MediaSessionNode* pSessionNode = m_lstSessionNode.GetValueAt(0);
        DeleteMediaSessionNode(pSessionNode, 0);
    }

    m_lstSessionNode.Clear();
}

PROTECTED
void MediaManager::DeleteMediaSessionNode(IN MediaSessionNode* pSessionNode, IMS_UINT32 nIndex)
{
    if (pSessionNode != IMS_NULL)
    {
        IMS_TRACE_D("DeleteMediaSessionNode() - Index[%d]", nIndex, 0, 0);

        if (pSessionNode->pMediaSession != IMS_NULL)
        {
            delete pSessionNode->pMediaSession;
        }

        if (pSessionNode->pMessageHandler != IMS_NULL)
        {
            delete pSessionNode->pMessageHandler;
        }

        delete pSessionNode;
    }

    m_lstSessionNode.RemoveAt(nIndex);
}

PROTECTED VIRTUAL MediaManager::MediaSessionNode* MediaManager::FindSessionNode(
        IN IMS_SINTP nCallKey)
{
    for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
    {
        MediaSessionNode* pSessionNode = m_lstSessionNode.GetAt(i);

        if (pSessionNode != IMS_NULL)
        {
            if (pSessionNode->nCallKey == nCallKey)
            {
                return pSessionNode;
            }
        }
    }

    IMS_TRACE_E(0, "FindSessionNode() - nCallKey[%d], nListSize[%d]", nCallKey,
            m_lstSessionNode.GetSize(), 0);

    return IMS_NULL;
}

PROTECTED VIRTUAL IMS_BOOL MediaManager::SendMessageToSessions(
        IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam)
{
    MediaSession* pMediaSession = GetSession(nCallKey);

    if (pMediaSession != IMS_NULL)
    {
        IMS_TRACE_I("SendMessageToSessions() - nMsg[%d], CallKey[%d]", nMsg, nCallKey, 0);

        if (pMediaSession->SendMessage(nMsg, pParam) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "SendMessageToSessions() - failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}
