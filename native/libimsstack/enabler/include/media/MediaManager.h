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

#ifndef MEDIA_MANAGER_H_
#define MEDIA_MANAGER_H_

#include "IJniMedia.h"
#include "IJniMediaManager.h"
#include "ImsActivityEx.h"
#include "ImsMap.h"
#include "MediaDef.h"

class IMediaSession;
class MediaMsgHandler;
class MediaSession;
class IService;
class MediaResourceManager;

class MediaManager : public ImsActivityEx, public IJniMediaManager
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
                pMessageHandler(IMS_NULL) {};

        MediaSessionNode(
                IN IMS_SINTP callKey, IN MediaSession* pSession, IN MediaMsgHandler* pHandler) :
                nCallKey(callKey),
                pMediaSession(pSession),
                pMessageHandler(pHandler) {};
    };

public:
    /**
     * @brief Gets the singleton instance of the MediaManager for a specific slot.
     *
     * @param nSlotId The UICC slot ID.
     * @return MediaManager* A pointer to the MediaManager instance.
     */
    static MediaManager* GetInstance(IN IMS_SINT32 nSlotId = 0);

    /**
     * @brief Gets the thread name for the MediaManager of a specific slot.
     *
     * @param nSlotId The UICC slot ID.
     * @return AString The thread name.
     */
    static AString GetThreadName(IN IMS_SINT32 nSlotId);

    /**
     * @brief Gets the message handler for a specific call session.
     *
     * @param nCallKey The key to identify the call session.
     * @return MediaMsgHandler* A pointer to the message handler, or IMS_NULL if not found.
     */
    MediaMsgHandler* GetHandler(IN IMS_SINTP nCallKey);

    /**
     * @brief Notifies that the JNI enabler has been set.
     * @see IJniMediaManager::NotifyJniEnablerSet
     */
    inline void NotifyJniEnablerSet() override {}

    /**
     * @brief Sends a message to a specific call session.
     * @see IJniMediaManager::SendMessage
     * @param nMsg The message identifier.
     * @param nCallKey The key to identify the call session.
     * @param pParam Additional message parameters.
     * @return IMS_BOOL IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL SendMessage(
            IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam) override;

    virtual IMediaSession* CreateSession(IN MEDIA_NETWORK_TYPE eNetwork,
            IN MEDIA_SERVICE_TYPE eServiceType, IN IService* pService, IN IMS_SINTP nCallKey);

    /**
     * @brief Destroys the MediaSession instance
     *
     * @param pSession The instance to destroy
     */
    virtual void DestroySession(IN const IMediaSession* piSession);

    /**
     * @brief Gets MediaSession instance
     *
     * @param nCallKey The key to identify the session instance
     * @return MediaSession* The instance matched with the key, IMS_NULL if there is not matched
     * session
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
     * @param eEvent enum of message event. It is defined in IJniMedia.h
     * @param nCallKey The key to identify the call session
     * @param param additional message parameters
     * @return IMS_BOOL IMS_TRUE for success, IMS_FALSE for failure
     */
    virtual IMS_BOOL HandleRequestMsg(
            IN IMS_SINT32 eEvent, IN IMS_SINTP nCallKey, IN ImsMediaMsgParamBase* param);

    /**
     * @brief Handle the disaptch messages
     *
     * @param nMsg enum of message ID. It is defined in IJniMedia.h
     * @param wParam The key to identify the call session, nCallKey
     * @param param additional message parameters
     */
    virtual void HandleMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP wParam, IN IMS_SINTP lParam);

    // ImsActivity
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

protected:
    static const IMS_UINT32 TIME_WAIT_MEDIA_RESPONSE = 5000;

    MediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId);
    virtual ~MediaManager() override;
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
