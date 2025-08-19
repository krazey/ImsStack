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
    ~JniMediaSessionThread() override;
    IMS_BOOL OnOpenSession(IN ImsMediaMsgOpenConfigParam* pParam) override;
    IMS_BOOL OnModifySession(IN ImsMediaMsgConfigParam* pParam) override;
    IMS_BOOL OnCloseSession(IN ImsMediaMsgParamBase* pParam) override;
    IMS_BOOL OnAddConfig(IN ImsMediaMsgConfigParam* pParam) override;
    IMS_BOOL OnDeleteConfig(IN ImsMediaMsgConfigParam* pParam) override;
    IMS_BOOL OnConfirmConfig(IN ImsMediaMsgConfigParam* pParam) override;
    IMS_BOOL OnSendDtmf(IN ImsMediaMsgDtmfParam* pParam) override;
    IMS_BOOL OnSetMediaQualityThreshold(IN ImsMediaMsgSetMediaQualityParam* pParam) override;
    IMS_BOOL OnRequestQos(IN ImsMediaMsgQosParam* pParam) override;
    IMS_BOOL OnRequestUpdateAnbrEnabledConfig(IN ImsMediaMsgAnbrNegotiationParam* pParam) override;
    void OnSetPreviewSurface() override;
    void OnSetDisplaySurface() override;

protected:
    IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const override;

private:
    SessionType ConvertToSessionType(IN MEDIA_CONTENT_TYPE eMediaType);
};

#endif
