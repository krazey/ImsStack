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

#ifndef JNI_MEDIA_SESSION_THREAD_H_
#define JNI_MEDIA_SESSION_THREAD_H_

#include <MediaQualityThreshold.h>
#include "BaseServiceThread.h"
#include "IJniMediaSessionThread.h"

class JniMediaSessionThread final : public BaseServiceThread, public IJniMediaSessionThread
{
public:
    JniMediaSessionThread();
    virtual ~JniMediaSessionThread();
    virtual IMS_BOOL OnOpenSession(IN ImsMediaMsgOpenConfigParam* pParam);
    virtual IMS_BOOL OnModifySession(IN ImsMediaMsgConfigParam* pParam);
    virtual IMS_BOOL OnCloseSession(IN ImsMediaMsgParamBase* pParam);
    virtual IMS_BOOL OnAddConfig(IN ImsMediaMsgConfigParam* pParam);
    virtual IMS_BOOL OnDeleteConfig(IN ImsMediaMsgConfigParam* pParam);
    virtual IMS_BOOL OnConfirmConfig(IN ImsMediaMsgConfigParam* pParam);
    virtual IMS_BOOL OnSendDtmf(IN ImsMediaMsgDtmfParam* pParam);
    virtual IMS_BOOL OnSetMediaQualityThreshold(IN ImsMediaMsgSetMediaQualityParam* pParam);
    virtual IMS_BOOL OnRequestQos(IN ImsMediaMsgQosParam* pParam);
    virtual void OnSetPreviewSurface();
    virtual void OnSetDisplaySurface();

protected:
    virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const;

private:
    SessionType ConvertToSessionType(IN MEDIA_CONTENT_TYPE eMediaType);
};

#endif
