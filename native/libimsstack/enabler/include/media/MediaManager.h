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

#ifndef _MEDIA_MANAGER_H_
#define _MEDIA_MANAGER_H_

// == INCLUDES =========================================================
#include "IUMedia.h"
#include "ServiceEvent.h"
#include "IEventListener.h"
#include "IMSActivityEx.h"
#include "MediaMsgHandler.h"
#include "MediaSession.h"
#include "MediaResourceMngr.h"
#include "IMediaManager.h"

class JniMediaSession;

class MediaManager : public IMSActivityEx, public IMediaManager
{
public:
    enum MessageType
    {
        MSG_NONE = 0,
        MSG_REQUEST,
        MSG_REQUEST_SET_WAIT,
        MSG_RESPONSE,
        MSG_RESPONSE_RELEASE_WAIT,
        MSG_NOTIFICATION,
        MSG_OPERATION,
    };

    class MediaSessionNode
    {
    public:
        IMS_SINTP nCallKey;
        MediaSession* pMediaSession;
        MediaMsgHandler* pMessageHandler;

    public:
        MediaSessionNode() :
                nCallKey(0),
                pMediaSession(IMS_NULL),
                pMessageHandler(IMS_NULL){};

        MediaSessionNode(
                IN IMS_SINTP callKey, IN MediaSession* pSession, MediaMsgHandler* pHandler) :
                nCallKey(callKey),
                pMediaSession(pSession),
                pMessageHandler(pHandler){};
    };

public:
    static MediaManager* GetInstance(IN IMS_SINT32 nSlotId = 0);
    static AString GetThreadName(IN IMS_SINT32 nSlotId);
    MediaMsgHandler* GetHandler(IN IMS_SINTP nCallKey);
    virtual void SetJniMediaSessionThread(IN IMS_SINTP nCallKey, IN JniMediaSessionThread* pThread);

    enum
    {
        INTERNAL_SESSION_TERMINATED_IND = IMS_MSG_USER
    };

    /**
     *
     */
    MediaSession* CreateSession(IN MEDIA_SERVICE_TYPE nService, IN IMS_SINTP callKey,
            IN JniMediaSessionThread* pThread);

    /**
     *
     */
    void DestroySession(IN MediaSession* pSession);

    /**
     *
     */
    MediaSession* GetSession(IN IMS_SINTP nCallKey);

    /**
     *
     */
    MediaResourceMngr* GetResourceManager();

    void OnResponse(IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam);

private:
    MediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId);
    virtual ~MediaManager();
    MediaManager(IN const MediaManager& obj);
    MediaManager& operator=(IN const MediaManager& obj);

    // == PROTECTED METHOD ==========================================================
protected:
    void ClearInternalMsgBuffer();
    void ClearMediaSessionNode();
    void DeleteMediaSessionNode(IN MediaSessionNode* pSessionNode, IMS_UINT32 nIndex);
    MessageType parseMessageType(IN IMS_SINT32 eMsg);
    IMS_BOOL handleRequestMsg(IN IMSMSG& objMsg);
    IMS_BOOL handleResponseMsg(IN IMSMSG& objMsg);
    IMS_BOOL handleNotificationMsg(IN IMSMSG& objMsg);
    IMS_BOOL handleOperationMsg(IN IMSMSG& objMsg);
    /**
     *
     */
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMsg);
    /**
     *
     */
    virtual MediaSessionNode* FindSessionNode(IN IMS_SINTP nCallKey);

    /**
     *
     */
    virtual IMS_BOOL SendMessageToSessions(IN IMS_SINTP nCallKey, IMSMSG& objMsg);

    // == PRIVATE VARIABLE ============================================================
protected:
    static IMSMap<IMS_SINT32, MediaManager*> m_objMapMediaManager;
    IMS_SINT32 m_nSlotId;
    IMSList<MediaSessionNode*> m_lstSessionNode;
    MediaResourceMngr* m_pResourceMngr;
    IMSList<IMSMSG*> m_lstInternalMsgBuffer;
    IMS_BOOL m_bWaitResponse;
};
#endif /* _MEDIA_MANAGER_H_ */
