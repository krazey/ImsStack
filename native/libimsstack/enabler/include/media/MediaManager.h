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

#ifndef MEDIA_MANAGER_H_
#define MEDIA_MANAGER_H_

#include "ServiceEvent.h"
#include "IEventListener.h"
#include "ImsActivityEx.h"
#include "ImsMap.h"
#include "IJniMediaManager.h"
#include "IMediaManager.h"
#include "IMMedia.h"
#include "MediaDef.h"

class IMediaSession;
class MediaMsgHandler;
class MediaSession;
class MediaResourceManager;

class MediaManager : public ImsActivityEx, public IJniMediaManager, public IMediaManager
{
public:
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
                IN IMS_SINTP callKey, IN MediaSession* pSession, IN MediaMsgHandler* pHandler) :
                nCallKey(callKey),
                pMediaSession(pSession),
                pMessageHandler(pHandler){};
    };

public:
    static MediaManager* GetInstance(IN IMS_SINT32 nSlotId = 0);
    static AString GetThreadName(IN IMS_SINT32 nSlotId);
    MediaMsgHandler* GetHandler(IN IMS_SINTP nCallKey);

    // TODO: temp inline. move to cpp
    inline void NotifyJniEnablerSet() override {}
    void SendMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam) override;

    /**
     * @brief Creates a MediaSession instacne with the service type and assign the jni thread
     * instance to communicate with jni thread to send and receive a message from java layer
     *
     * @param nService service type, normal or emergency
     * @param nCallKey The key to identify the call session, each MediaSession has a unique key to
     * match with the call session
     * @return IMediaSession* created IMediaSession instance
     */
    IMediaSession* CreateSession(IN MEDIA_SERVICE_TYPE nService, IN IMS_SINTP nCallKey);

    /**
     * @brief Destroys the MediaSession instance
     *
     * @param pSession The instance to destroy
     */
    void DestroySession(IN const IMediaSession* piSession);

    /**
     * @brief Gets MediaSession instance
     *
     * @param nCallKey The key to identify the session instance
     * @return MediaSession* The instance matched with the key, NULL if there is not matched session
     */
    MediaSession* GetSession(IN IMS_SINTP nCallKey);

    /**
     * @brief Gets the MediaResourceManager instance
     *
     * @return MediaResourceManager*
     */
    MediaResourceManager* GetResourceManager();

    /**
     * @brief Sends a request message from native to java layer
     *
     * @param eEvent enum of message event. It is define in IMMedia.h
     * @param nCallKey The key to identify the call session
     * @param param additional message parameters
     * @return IMS_BOOL
     */
    IMS_BOOL handleRequestMsg(
            IN IMS_SINT32 eEvent, IN IMS_SINTP nCallKey, IN ImsMediaMsgParamBase* param);

private:
    static const IMS_UINT32 TIME_WAIT_MEDIA_RESPONSE = 5000;

    MediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId);
    virtual ~MediaManager();
    MediaManager(IN const MediaManager& obj);
    MediaManager& operator=(IN const MediaManager& obj);
    void ClearMediaSessionNode();
    void DeleteMediaSessionNode(IN MediaSessionNode* pSessionNode, IMS_UINT32 nIndex);

    /**
     * @brief Finds MediaSessionNode with the parameter
     *
     * @param nCallKey session node identification
     * @return MediaSessionNode*
     */
    virtual MediaSessionNode* FindSessionNode(IN IMS_SINTP nCallKey);

    /**
     * @brief Sends message to repective session
     *
     * @param nMsg enum of message
     * @param nCallKey session identification
     * @param pParam message parameter
     * @return IMS_BOOL IMS_TRUE when the message is deliverd successfully, IMS_FALSE when it fails
     */
    virtual IMS_BOOL SendMessageToSessions(
            IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam);

protected:
    static ImsMap<IMS_SINT32, MediaManager*> m_objMapMediaManager;
    IMS_SINT32 m_nSlotId;
    ImsList<MediaSessionNode*> m_lstSessionNode;
    MediaResourceManager* m_pResourceMngr;
};

#endif
