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
#ifndef INTERFACE_JNI_MEDIA_SESSION_THREAD_H_
#define INTERFACE_JNI_MEDIA_SESSION_THREAD_H_

#include "BaseServiceThread.h"
#include "IJniEnablerThread.h"
#include "IJniMedia.h"

class IJniMediaSessionThread : public IJniEnablerThread
{
public:
    virtual ~IJniMediaSessionThread() {}

    /**
     * @brief Called to request open session to each media session
     *
     * @param pParam The parameter set for open session
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnOpenSession(IN ImsMediaMsgOpenConfigParam* pParam);

    /**
     * @brief Called to request modify session to each media session
     *
     * @param pParam The parameter set for modify session
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnModifySession(IN ImsMediaMsgConfigParam* pParam);

    /**
     * @brief Called to request close session to each media session
     *
     * @param pParam The parameter set for close session
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnCloseSession(IN ImsMediaMsgParamBase* pParam);

    /**
     * @brief Called to request add config to the audio session
     *
     * @param pParam The parameter set to request
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnAddConfig(IN ImsMediaMsgConfigParam* pParam);

    /**
     * @brief Called to request delete config to the audio session
     *
     * @param pParam The parameter set for delete config
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnDeleteConfig(IN ImsMediaMsgConfigParam* pParam);

    /**
     * @brief Called to request confirm config to the audio session
     *
     * @param pParam The parameter set for open session
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnConfirmConfig(IN ImsMediaMsgConfigParam* pParam);

    /**
     * @brief Called to request send dtmf to the audio session
     *
     * @param pParam The dtmf parameter set
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnSendDtmf(IN ImsMediaMsgDtmfParam* pParam);

    /**
     * @brief Called to request SetMediaQualityThreshold to each media session
     *
     * @param pParam The qualty threshold parameter set
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnSetMediaQualityThreshold(IN ImsMediaMsgSetMediaQualityParam* pParam);

    /**
     * @brief Called to request register qos callback to each media session
     *
     * @param pParam The qos parameter set includes media type
     * @return IMS_BOOL Returns false when the parameter is invalid
     */
    virtual IMS_BOOL OnRequestQos(IN ImsMediaMsgQosParam* pParam);

    /**
     * @brief Called to inform the preview surface is ready
     */
    virtual void OnSetPreviewSurface();

    /**
     * @brief Called to inform the display surface is ready
     */
    virtual void OnSetDisplaySurface();
};

#endif
