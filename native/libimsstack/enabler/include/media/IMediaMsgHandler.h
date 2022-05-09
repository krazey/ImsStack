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

#ifndef _IMEDIA_MSG_HANDLER_H_
#define _IMEDIA_MSG_HANDLER_H_

#include "IUMedia.h"
#include "IMMedia.h"

class IMediaMsgHandler
{
public:
    virtual void SendMessageToMediaService(
            IN IMS_UINTP nCallKey, IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam) = 0;
    /*
        virtual void Destroy() = 0;
        virtual IMSMSG& ConvertMessage(IN IMSMSG &objMsg) = 0;
        virtual void SendMessageToUi(IN IMS_UINTP nCallKey, IMS_SINT32 eEvent,
                IN IMS_UINT32 eResult) = 0;
        virtual void OnRttReceivedInd(IN IMS_UINTP nCallKey, IN IMS_SINT32 eEvent,
                IN IUMediaRttDataParam* pParam) = 0;
        virtual void OnRttAudioIndicator(IN IMS_UINTP nCallKey, IN IMS_SINT32 eEvent,
                IN IMS_UINT32 eRttAudioInd) = 0;
        virtual void OnDataUsageChanged(IN IMS_UINTP nSession, IN IMS_SINT32 eEvent,
                IN IUMediaDataUsageInfoParam* pParam) = 0;
    */
};
#endif /* _IMEDIA_MSG_HANDLER_H_ */
