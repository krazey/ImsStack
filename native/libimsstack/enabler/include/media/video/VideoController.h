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

#ifndef _VIDEO_CONTROLLER_H_
#define _VIDEO_CONTROLLER_H_

#include "MediaDef.h"
#include "IMediaSessionListener.h"
#include "config/VideoConfiguration.h"
#include "video/VideoMediaSession.h"
#include "video/VideoNego.h"

class VideoController
{
public:
    VideoController();
    ~VideoController();
    IMS_BOOL SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);
    IMS_BOOL IsHoldSession();
    IMS_BOOL HoldSession();
    void CreateSession(IMediaSessionListener* pListener, VideoConfiguration* pConfig);
    IMS_BOOL OpenSession();
    IMS_BOOL UpdateSession();
    IMS_BOOL CloseSession();
    void UpdateLocalAddress(IN VideoNego* pNego);
    void UpdateRtpConfig(IN VideoNego* pNego);
    void UpdateQualityThreshold();

private:
    VideoMediaSession* m_pVideoSession;
    IPAddress m_objLocalAddr;
    IMS_UINT32 m_nPort;
};

#endif