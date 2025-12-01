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

#ifndef MEDIA_MSG_HANDLER_H_
#define MEDIA_MSG_HANDLER_H_

#include "AString.h"
#include "ImsTypeDef.h"

class ImsMediaMsgParamBase;

/**
 * @brief Handles sending messages from the native media session to the Java layer.
 *
 * This class acts as a bridge to communicate events and requests from the native C++
 * side to the corresponding Java `ImsMediaSession` instance via JNI. Each
 * `MediaMsgHandler` is associated with a specific call session.
 */
class MediaMsgHandler
{
public:
    /**
     * @param nSlotId The UICC slot ID.
     * @param nCallKey A unique key to identify the call session.
     */
    MediaMsgHandler(IN IMS_SINT32 nSlotId, IN IMS_SINTP nCallKey);
    virtual ~MediaMsgHandler();

    /**
     * @brief Set the JniMediaSesson thread name
     *
     * @param strName The thread name of JniMediaSession is using in this call
     */
    virtual void SetListener(IN const AString& strName);

    /**
     * @brief Sends a JNI request message to the Java media service.
     *
     * @param eEvent The message event enum, as defined in `IJniMedia.h`.
     * @param pParam The parameters for the event.
     * @return IMS_BOOL Returns `IMS_TRUE` if the message was delivered successfully,
     * `IMS_FALSE` otherwise.
     */
    virtual IMS_BOOL SendMessageToJava(IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam);

private:
    IMS_SINT32 m_nSlotId;
    AString m_strListenerThread;
    IMS_SINTP m_nCallKey;  // TODO: IMS_ULONG
};

#endif
