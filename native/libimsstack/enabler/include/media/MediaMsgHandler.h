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

#ifndef _MEDIA_MSG_HANDLER_H_
#define _MEDIA_MSG_HANDLER_H_

#include "JniMediaSessionThread.h"
#include "IMMedia.h"

class MediaMsgHandler
{
public:
    MediaMsgHandler();
    virtual ~MediaMsgHandler();

    /**
     * @brief Set the JniMediaSesson thread name
     *
     * @param strName The thread name of JniMediaSession is using in this call
     */
    virtual void SetListener(IN const AString& strName);

    /**
     * @brief Set the JniMediaSessionThread instance to get the message from the jni layer
     *
     * @param pThread
     */
    virtual void SetJniMediaSessionThread(IN IJniMediaSessionThread* pThread);

    /**
     * @brief
     *
     * @return IMS_BOOL
     */
    virtual IMS_BOOL IsAvailableToSend();

    /**
     * @brief Send jni request message to media service
     *
     * @param eEvent The message event enum
     * @param pParam The event parameters to send
     * @return IMS_BOOL Returns IMS_TRUE when the message deliverd without error, IMS_FALSE when it
     * is failed to send
     */
    virtual IMS_BOOL SendMessageToMediaService(
            IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam);

private:
    AString m_strListenerThread;
    IJniMediaSessionThread* m_pThread;
};
#endif /* _MEDIA_MSG_HANDLER_H_ */
