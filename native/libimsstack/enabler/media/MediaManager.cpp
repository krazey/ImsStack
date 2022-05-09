/**
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
#include "ServiceMessage.h"
#include "ServiceTrace.h"
#include "IMMedia.h"
#include "JniConnectorFactory.h"

__IMS_TRACE_TAG_USER_DECL__("MED.MM");

PRIVATE GLOBAL IMSMap<IMS_SINT32, MediaManager*> MediaManager::m_objMapMediaManager;

PRIVATE
MediaManager::MediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId) :
        IMSActivityEx(strName),
        m_nSlotId(nSlotId),
        m_lstSessionNode(IMSList<MediaSessionNode*>()),
        m_pResourceMngr(IMS_NULL),
        m_lstInternalMsgBuffer(IMSList<IMSMSG*>()),
        m_bWaitResponse(IMS_FALSE)
{
    IMS_TRACE_D("+MediaManager() thread[%s], nSlotId[%d]", strName.GetStr(), nSlotId, 0);

    if (m_pResourceMngr == IMS_NULL)
    {
        m_pResourceMngr = new MediaResourceMngr(m_nSlotId);
    }

    JniConnectorFactory::GetInstance()->GetMediaSessionConnector(m_nSlotId)->SetEnablerService(
            static_cast<IMediaManager*>(this));
}

PRIVATE VIRTUAL MediaManager::~MediaManager()
{
    IMS_SLONG nIndex = m_objMapMediaManager.GetIndexOfKey(m_nSlotId);
    if (nIndex >= 0)
    {
        m_objMapMediaManager.RemoveAt(nIndex);
    }

    if (m_pResourceMngr)
    {
        delete m_pResourceMngr;
        m_pResourceMngr = IMS_NULL;
    }

    ClearInternalMsgBuffer();
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
    MediaSessionNode* pSessionNode = FindSessionNode(nCallKey);
    if (pSessionNode == IMS_NULL)
    {
        return IMS_NULL;
    }
    return pSessionNode->pMessageHandler;
}

PUBLIC VIRTUAL void MediaManager::SetJniMediaSessionThread(
        IN IMS_SINTP nCallKey, IN JniMediaSessionThread* pThread)
{
    MediaMsgHandler* pHandler = GetHandler(nCallKey);
    if (pHandler != IMS_NULL)
    {
        pHandler->SetJniMediaSessionThread(pThread);
    }
}

PUBLIC
MediaSession* MediaManager::CreateSession(
        IN MEDIA_SERVICE_TYPE nService, IN IMS_SINTP nCallKey, IN JniMediaSessionThread* pThread)
{
    IMS_TRACE_D("CreateSession() - CallKey[%" PFLS_u "], nService[%d]", nCallKey, nService, 0);

    MediaSession* pSession = new MediaSession(nService, nCallKey, m_nSlotId);
    if (pSession == IMS_NULL)
    {
        return IMS_NULL;
    }

    MediaMsgHandler* pHandler = new MediaMsgHandler(IUIMS::APP_MTC);

    if (pHandler != IMS_NULL && pThread != IMS_NULL)
    {
        pHandler->SetJniMediaSessionThread(pThread);
    }

    MediaSessionNode* pSessionNode = new MediaSessionNode(nCallKey, pSession, pHandler);
    m_lstSessionNode.Append(pSessionNode);

    IMS_TRACE_D("CreateSession() - ListSize[%d]", m_lstSessionNode.GetSize(), 0, 0);

    return pSession;
}

PUBLIC
void MediaManager::DestroySession(IN MediaSession* pSession)
{
    IMS_TRACE_I("DestroySession", 0, 0, 0);
    for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
    {
        MediaSessionNode* pSessionNode = m_lstSessionNode.GetAt(i);
        if (pSessionNode->pMediaSession == pSession)
        {
            DeleteMediaSessionNode(pSessionNode, i);
            return;
        }
    }
}

PUBLIC
MediaSession* MediaManager::GetSession(IN IMS_SINTP nCallKey)
{
    MediaSessionNode* pSessionNode = FindSessionNode(nCallKey);
    if (pSessionNode == IMS_NULL)
        return IMS_NULL;
    return pSessionNode->pMediaSession;
}

PUBLIC
MediaResourceMngr* MediaManager::GetResourceManager()
{
    return m_pResourceMngr;
}

PUBLIC VIRTUAL void MediaManager::OnResponse(
        IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam)
{
    IMSMSG objMsg(nMsg, nCallKey, pParam, IMS_NULL);
    MessageService::PostMessage(GetThreadName(m_nSlotId), objMsg);
}

PROTECTED
void MediaManager::ClearInternalMsgBuffer()
{
    IMS_TRACE_D("ClearInternalMsgBuffer() m_lstInternalMsgBuffer size=%d",
            m_lstInternalMsgBuffer.GetSize(), 0, 0);

    IMSMSG* pImsMsg = IMS_NULL;

    while (m_lstInternalMsgBuffer.GetSize() > 0)
    {
        pImsMsg = m_lstInternalMsgBuffer.GetValueAt(0);
        if (pImsMsg)
        {
            delete pImsMsg;
            pImsMsg = IMS_NULL;
        }
        m_lstInternalMsgBuffer.RemoveAt(0);
    }
    m_lstInternalMsgBuffer.Clear();
}

PROTECTED
void MediaManager::ClearMediaSessionNode()
{
    IMS_TRACE_D(
            "ClearMediaSessionNode() m_lstSessionNode size=%d", m_lstSessionNode.GetSize(), 0, 0);

    MediaSessionNode* pSessionNode = IMS_NULL;

    while (m_lstSessionNode.GetSize() > 0)
    {
        pSessionNode = m_lstSessionNode.GetValueAt(0);
        DeleteMediaSessionNode(pSessionNode, 0);
    }
    m_lstSessionNode.Clear();
}

PROTECTED
void MediaManager::DeleteMediaSessionNode(IN MediaSessionNode* pSessionNode, IMS_UINT32 nIndex)
{
    if (pSessionNode)
    {
        IMS_TRACE_D("DeleteMediaSessionNode() - Index[%d]", nIndex, 0, 0);
        if (pSessionNode->pMediaSession)
        {
            delete pSessionNode->pMediaSession;
            pSessionNode->pMediaSession = IMS_NULL;
        }
        delete pSessionNode;
        pSessionNode = IMS_NULL;
    }
    m_lstSessionNode.RemoveAt(nIndex);
}

PROTECTED
MediaManager::MessageType MediaManager::parseMessageType(IN IMS_SINT32 nMsg)
{
    MessageType nMsgType = MSG_NONE;

    if (nMsg >= IMMedia::REQUEST_OPEN_SESSION && nMsg <= IMMedia::REQUEST_HEADER_EXTENSION)
    {
        nMsgType = MSG_REQUEST;

        if (nMsg == IMMedia::REQUEST_OPEN_SESSION || nMsg == IMMedia::REQUEST_MODIFY_SESSION ||
                nMsg == IMMedia::REQUEST_ADD_CONFIG || nMsg == IMMedia::REQUEST_CONFIRM_CONFIG)
        {
            nMsgType = MSG_REQUEST_SET_WAIT;
        }
    }
    else if (nMsg >= IMMedia::RESPONSE_OPEN_SESSION && nMsg <= IMMedia::NOTIFY_FIRST_PACKET)
    {
        nMsgType = MSG_RESPONSE;

        if (nMsg == IMMedia::RESPONSE_OPEN_SESSION || nMsg == IMMedia::RESPONSE_MODIFY_SESSION ||
                nMsg == IMMedia::RESPONSE_ADD_CONFIG || nMsg == IMMedia::RESPONSE_CONFIRM_CONFIG)
        {
            nMsgType = MSG_RESPONSE_RELEASE_WAIT;
        }
    }
    else if (nMsg >= IMMedia::NOTIFY_HEADER_EXTENSION && nMsg <= IMMedia::NOTIFY_QOS_INFO)
    {
        nMsgType = MSG_NOTIFICATION;
    }
    else if (nMsg == MediaSession::RUN)
    {
        nMsgType = MSG_OPERATION;
    }

    IMS_TRACE_I("parseMessageType() - nMsgType[%d]", nMsgType, 0, 0);
    return nMsgType;
}

PROTECTED
IMS_BOOL MediaManager::handleRequestMsg(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("handleRequestMsg() - MSG[%d]", objMsg.nMSG, 0, 0);

    MediaSessionNode* pNode = FindSessionNode(objMsg.nWparam);
    if (pNode == IMS_NULL || pNode->pMessageHandler == IMS_NULL)
    {
        return IMS_FALSE;
    }
    ImsMediaMsgParamBase* param = reinterpret_cast<ImsMediaMsgParamBase*>(objMsg.nLparam);
    return pNode->pMessageHandler->SendMessageToMediaService(objMsg.nMSG, param);
}

PROTECTED
IMS_BOOL MediaManager::handleResponseMsg(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("handleResponseMsg() - MSG[%d]", objMsg.nMSG, 0, 0);
    if (SendMessageToSessions(objMsg.nWparam, objMsg) != IMS_TRUE)
    {
        IMS_TRACE_E(0, "handleResponseMsg() - Fail to process nMsg", 0, 0, 0);
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaManager::handleNotificationMsg(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("handleNotificationMsg() - MSG[%d]", objMsg.nMSG, 0, 0);
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaManager::handleOperationMsg(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("handleOperationMsg() - MSG[%d]", objMsg.nMSG, 0, 0);
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL MediaManager::OnMessage(IN IMSMSG& objMsg)
{
    IMS_TRACE_I("OnMessage() - MSG[%d]", objMsg.nMSG, 0, 0);
    MessageType nMsgType = parseMessageType(objMsg.nMSG);

    if (nMsgType == MSG_RESPONSE_RELEASE_WAIT)
    {
        m_bWaitResponse = IMS_FALSE;
    }

    IMSMSG* pendingMsg = new IMSMSG(objMsg.nMSG, objMsg.nWparam, objMsg.nLparam, IMS_NULL);
    m_lstInternalMsgBuffer.Append(pendingMsg);

    if (m_bWaitResponse == IMS_TRUE)
    {
        IMS_TRACE_D("OnMessage() still waiting response : size of pending MSG[%d]",
                m_lstInternalMsgBuffer.GetSize(), 0, 0);
        return IMS_TRUE;
    }

    IMS_BOOL bRet = IMS_TRUE;
    while (m_lstInternalMsgBuffer.GetSize() > 0 && m_bWaitResponse == IMS_FALSE)
    {
        IMSMSG* pMsg = m_lstInternalMsgBuffer.GetAt(0);

        switch (nMsgType)
        {
            case MSG_REQUEST:
            case MSG_REQUEST_SET_WAIT:
                bRet = handleRequestMsg(*pMsg);
                if (nMsgType == MSG_REQUEST_SET_WAIT)
                {
                    m_bWaitResponse = IMS_TRUE;
                }
                break;
            case MSG_RESPONSE:
            case MSG_RESPONSE_RELEASE_WAIT:
                bRet = handleResponseMsg(*pMsg);
                break;
            case MSG_NOTIFICATION:
                bRet = handleResponseMsg(*pMsg);
                break;
            case MSG_OPERATION:
                bRet = handleResponseMsg(*pMsg);
                break;
            default:
                break;
        }

        if (bRet == IMS_TRUE)
        {
            if (pMsg->nMSG == IMMedia::REQUEST_CLOSE_SESSION)
            {
                IMS_TRACE_I("OnMessage() CloseSession is successfully sent to JNI", 0, 0, 0);
                DestroySession(GetSession(pMsg->nWparam));
            }
        }
        delete pMsg;
        m_lstInternalMsgBuffer.RemoveAt(0);

        if (bRet == IMS_FALSE)
        {
            return bRet;
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL MediaManager::MediaSessionNode* MediaManager::FindSessionNode(
        IN IMS_SINTP nCallKey)
{
    for (IMS_UINT32 i = 0; i < m_lstSessionNode.GetSize(); i++)
    {
        MediaSessionNode* pSessionNode = (MediaSessionNode*)m_lstSessionNode.GetAt(i);
        if (pSessionNode != IMS_NULL)
        {
            if (pSessionNode->nCallKey == nCallKey)
            {
                return pSessionNode;
            }
        }
    }

    IMS_TRACE_E(0, "FindSessionNode() - nCallKey[%" PFLS_u "], nListSize[%d]", nCallKey,
            m_lstSessionNode.GetSize(), 0);

    return IMS_NULL;
}

PROTECTED VIRTUAL IMS_BOOL MediaManager::SendMessageToSessions(
        IN IMS_SINTP nCallKey, IMSMSG& objMsg)
{
    MediaSession* pMediaSession = GetSession(nCallKey);  // Find next session
    if (pMediaSession != IMS_NULL)
    {
        IMS_TRACE_I("SendMessageToSessions() - CallKey[%" PFLS_u "]", nCallKey, 0, 0);
        if (pMediaSession->SendMessage(objMsg) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "SendMessageToSessions() - failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}
