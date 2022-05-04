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

#include <AudioConfig.h>
#include <MediaQualityThreshold.h>
#include "BaseServiceThread.h"
#include "IMMedia.h"

using namespace android::telephony::imsmedia;

class JniMediaSessionThread final : public BaseServiceThread
{
public:
    JniMediaSessionThread();
    virtual ~JniMediaSessionThread();

    void SetSlotId(IN IMS_SINT32 nSlotId);

    IMS_BOOL OnOpenSession(IN ImsMediaMsgOpenConfigParam* pParam);
    IMS_BOOL OnModifySession(IN ImsMediaMsgConfigParam* pParam);
    IMS_BOOL OnCloseSession(IN ImsMediaMsgParamBase* pParam);
    IMS_BOOL OnAddConfig(IN ImsMediaMsgConfigParam* pParam);
    IMS_BOOL OnDeleteConfig(IN ImsMediaMsgConfigParam* pParam);
    IMS_BOOL OnConfirmConfig(IN ImsMediaMsgConfigParam* pParam);
    IMS_BOOL OnSendDtmf(IN ImsMediaMsgDtmfParam* pParam);
    IMS_BOOL OnSetMediaQualityThreshold(IN ImsMediaMsgSetMediaQualityParam* pParam);

protected:
    virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const;

private:
    SessionType ConvertToSessionType(IN MEDIA_CONTENT_TYPE eMediaType);

private:
    IMS_SINT32 m_nSlotId;
};

#endif
