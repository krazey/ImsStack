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
#include "IUMedia.h"
#include "IMMedia.h"

class MediaMsgHandler
{
public:
    MediaMsgHandler(IMS_SINT32 _nAppId);
    virtual ~MediaMsgHandler();

    // == PUBLIC METHOD ==============================================================
public:
    void SetListener(IN const AString& strName);
    void SetJniMediaSessionThread(IN JniMediaSessionThread* pThread);
    IMS_BOOL IsAvailableToSend();
    IMS_BOOL SendMessageToMediaService(IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam);
    /*
        void SendMessageToUi(IN IMS_UINTP nCallKey, IMS_SINT32 eEvent,
            IN IMS_UINT32 eResult);
        void OnRttReceivedInd(IN IMS_UINTP nCallKey, IMS_SINT32 eEvent,
            IN IUMediaRttDataParam* pParam);
        void OnRttAudioIndicator(IN IMS_UINTP nCallKey, IN IMS_SINT32 eEvent,
            IN IMS_UINT32 eRttAudioInd);
        void OnDataUsageChanged(IN IMS_UINTP nCallKey, IMS_SINT32 eEvent,
            IN IUMediaDataUsageInfoParam* pParam);
    */
    // == PRIVATE VARIABLE ============================================================
private:
    IMS_SINT32 nAppId;
    AString strListenerThread;
    JniMediaSessionThread* m_pThread;
};
#endif /* _MEDIA_MSG_HANDLER_H_ */
