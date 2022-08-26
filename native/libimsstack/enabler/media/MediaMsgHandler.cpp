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

#include "ServiceTrace.h"
#include "MediaMsgHandler.h"

__IMS_TRACE_TAG_USER_DECL__("MED.MH");

PUBLIC
MediaMsgHandler::MediaMsgHandler() :
        m_strListenerThread(AString::ConstNull()),
        m_pThread(IMS_NULL)
{
    IMS_TRACE_I("+MediaMsgHandler()", 0, 0, 0);
}

PUBLIC
MediaMsgHandler::~MediaMsgHandler()
{
    IMS_TRACE_I("~MediaMsgHandler()", 0, 0, 0);
}

PUBLIC
void MediaMsgHandler::SetListener(IN CONST AString& strName)
{
    m_strListenerThread = strName;
}

PUBLIC
void MediaMsgHandler::SetJniMediaSessionThread(IN IJniMediaSessionThread* pThread)
{
    m_pThread = pThread;
}

PRIVATE
IMS_BOOL MediaMsgHandler::IsAvailableToSend()
{
    if (m_pThread == IMS_NULL)
    {
        IMS_TRACE_I("isAvailableToSend : Not available", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MediaMsgHandler::SendMessageToMediaService(
        IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam)
{
    IMS_TRACE_I("SendMessageToMediaService() eEvent[%d], strListenerThread[%s]", eEvent,
            m_strListenerThread.GetStr(), 0);

    if (!IsAvailableToSend())
    {
        return IMS_FALSE;
    }

    switch (eEvent)
    {
        case IMMedia::REQUEST_OPEN_SESSION:
            return m_pThread->OnOpenSession(static_cast<ImsMediaMsgOpenConfigParam*>(pParam));
        case IMMedia::REQUEST_MODIFY_SESSION:
            return m_pThread->OnModifySession(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IMMedia::REQUEST_CLOSE_SESSION:
            return m_pThread->OnCloseSession(pParam);
        case IMMedia::REQUEST_ADD_CONFIG:
            return m_pThread->OnAddConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IMMedia::REQUEST_DELETE_CONFIG:
            return m_pThread->OnDeleteConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IMMedia::REQUEST_CONFIRM_CONFIG:
            return m_pThread->OnConfirmConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IMMedia::REQUEST_SEND_DTMF:
            return m_pThread->OnSendDtmf(static_cast<ImsMediaMsgDtmfParam*>(pParam));
        case IMMedia::REQUEST_SET_MEDIA_QUALITY:
            return m_pThread->OnSetMediaQualityThreshold(
                    static_cast<ImsMediaMsgSetMediaQualityParam*>(pParam));
        case IMMedia::REQUEST_SET_PREVIEW_SURFACE:
            return m_pThread->OnSetPreviewSurface();
        case IMMedia::REQUEST_SET_DISPLAY_SURFACE:
            return m_pThread->OnSetDisplaySurface();
        default:
            IMS_TRACE_E(0, "SendMessageToMediaService() eEvent[%d], not handled", eEvent, 0, 0);
            return IMS_TRUE;
    }
}