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

#ifndef MEDIA_MSG_HANDLER_H_
#define MEDIA_MSG_HANDLER_H_

#include "IJniMedia.h"

class MediaMsgHandler
{
public:
    MediaMsgHandler(IN IMS_SINT32 nSlotId, IN IMS_SINTP nCallKey);
    virtual ~MediaMsgHandler();

    /**
     * @brief Set the JniMediaSesson thread name
     *
     * @param strName The thread name of JniMediaSession is using in this call
     */
    virtual void SetListener(IN const AString& strName);

    /**
     * @brief Send jni request message to media service
     *
     * @param eEvent The message event enum
     * @param pParam The event parameters to send
     * @return IMS_BOOL Returns IMS_TRUE when the message deliverd without error, IMS_FALSE when it
     * is failed to send
     */
    virtual IMS_BOOL SendMessageToJava(IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam);

private:
    IMS_SINT32 m_nSlotId;
    AString m_strListenerThread;
    IMS_SINTP m_nCallKey;  // TODO: IMS_ULONG
};

#endif
