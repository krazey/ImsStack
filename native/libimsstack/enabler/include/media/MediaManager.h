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

#include "IJniMediaManager.h"
#include "ImsActivityEx.h"
#include "ImsMap.h"
#include "MediaDef.h"

class IMediaSession;
class IService;
class ImsMediaMsgParamBase;
class MediaMsgHandler;
class MediaResourceManager;
class MediaSession;

class MediaManager : public ImsActivityEx, public IJniMediaManager
{
public:
    /**
     * @brief A node to hold and manage the lifecycle of a media session and its handler.
     */
    class MediaSessionNode
    {
    public:
        IMS_SINTP nCallKey = 0;
        std::shared_ptr<MediaSession> pMediaSession;
        std::shared_ptr<MediaMsgHandler> pMessageHandler;

    public:
        MediaSessionNode() = default;

        /**
         * @brief Constructs a MediaSessionNode, taking ownership of the session and handler.
         *
         * @param callKey The unique identifier for the call session.
         * @param pSession A shared_ptr to the MediaSession object.
         * @param pHandler A shared_ptr to the MediaMsgHandler object.
         */
        MediaSessionNode(IN IMS_SINTP callKey, IN std::shared_ptr<MediaSession> pSession,
                IN std::shared_ptr<MediaMsgHandler> pHandler) :
                nCallKey(callKey),
                pMediaSession(std::move(pSession)),
                pMessageHandler(std::move(pHandler)) {};
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
     * @return std::shared_ptr<MediaMsgHandler> A pointer to the message handler, or IMS_NULL if not
     * found.
     */
    virtual std::shared_ptr<MediaMsgHandler> GetHandler(IN IMS_SINTP nCallKey);

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

    /**
     * @brief Creates a new media session for a call.
     *
     * This method instantiates a new MediaSession, a corresponding MediaMsgHandler,
     * and encapsulates them within a MediaSessionNode, which is then stored in the
     * MediaManager's list of active sessions.
     *
     * @param eNetwork The network type for the session.
     * @param eServiceType The service type (e.g., default, emergency).
     * @param pService A pointer to the service interface for accessing network information.
     * @param nCallKey A unique key to identify the call session.
     * @return IMediaSession* A pointer to the newly created IMediaSession, or IMS_NULL on failure.
     */
    virtual IMediaSession* CreateSession(IN MEDIA_NETWORK_TYPE eNetwork,
            IN MEDIA_SERVICE_TYPE eServiceType, IN IService* pService, IN IMS_SINTP nCallKey);

    /**
     * @brief Destroys a media session identified by its interface pointer.
     *
     * @param piSession A pointer to the IMediaSession interface of the session to destroy.
     */
    virtual void DestroySession(IN const IMediaSession* piSession);

    /**
     * @brief Gets MediaSession instance
     *
     * @param nCallKey The key to identify the session instance
     * @return MediaSession* The instance matched with the key, IMS_NULL if there is not matched
     * session
     */
    virtual MediaSession* GetSession(IN IMS_SINTP nCallKey);

    /**
     * @brief Gets the MediaResourceManager instance
     *
     * @return MediaResourceManager* A pointer to the resource manager.
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
     * @brief Handles internal messages posted to the MediaManager's activity thread.
     *
     * @param nMsg The message identifier.
     * @param wParam The first message parameter (typically the call key).
     * @param lParam The second message parameter (typically a pointer to message data).
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
    void DeleteMediaSessionNode(
            IN const std::shared_ptr<MediaSessionNode> pSessionNode, IMS_UINT32 nIndex);

    virtual std::shared_ptr<MediaMsgHandler> CreateMessageHandler(IN IMS_SINTP nCallKey);

    /**
     * @brief Finds a MediaSessionNode by its call key.
     *
     * @param nCallKey The key identifying the call session.
     * @return std::shared_ptr<MediaSessionNode> A pointer to the found node, or IMS_NULL if not
     * found.
     */
    virtual std::shared_ptr<MediaSessionNode> FindSessionNode(IN IMS_SINTP nCallKey);

    /**
     * @brief Sends a message to a specific session identified by its call key.
     *
     * @param nMsg The message identifier.
     * @param nCallKey The key identifying the target session.
     * @param pParam Additional message parameters.
     * @return IMS_BOOL IMS_TRUE when the message is deliverd successfully, IMS_FALSE when it fails
     */
    virtual IMS_BOOL SendMessageToSessions(
            IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam);

protected:
    static ImsMap<IMS_SINT32, MediaManager*> m_objMapMediaManager;
    IMS_SINT32 m_nSlotId;
    ImsList<std::shared_ptr<MediaSessionNode>> m_lstSessionNode;
    std::shared_ptr<MediaResourceManager> m_pResourceMngr;
};

#endif
