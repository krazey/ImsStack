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
#include "JniEnablerConnector.h"
#include "IJniEnabler.h"
#include "IJniMediaSessionThread.h"

__IMS_TRACE_TAG_USER_DECL__("MED.MH");

PUBLIC
MediaMsgHandler::MediaMsgHandler(IN IMS_SINT32 nSlotId, IN IMS_SINTP nCallKey) :
        m_nSlotId(nSlotId),
        m_strListenerThread(AString::ConstNull()),
        m_nCallKey(nCallKey)
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
IMS_BOOL MediaMsgHandler::SendMessageToMediaService(
        IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam)
{
    IMS_TRACE_I("SendMessageToMediaService() eEvent[%d], strListenerThread[%s]", eEvent,
            m_strListenerThread.GetStr(), 0);

    IJniMediaSessionThread* piThread = GetJniThread();
    if (piThread == IMS_NULL)
    {
        IMS_TRACE_D("SendMessageToMediaService piThread is null", 0, 0, 0);
        return IMS_FALSE;
    }

    switch (eEvent)
    {
        case IMMedia::REQUEST_OPEN_SESSION:
            return piThread->OnOpenSession(static_cast<ImsMediaMsgOpenConfigParam*>(pParam));
        case IMMedia::REQUEST_MODIFY_SESSION:
            return piThread->OnModifySession(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IMMedia::REQUEST_CLOSE_SESSION:
            return piThread->OnCloseSession(pParam);
        case IMMedia::REQUEST_ADD_CONFIG:
            return piThread->OnAddConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IMMedia::REQUEST_DELETE_CONFIG:
            return piThread->OnDeleteConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IMMedia::REQUEST_CONFIRM_CONFIG:
            return piThread->OnConfirmConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IMMedia::REQUEST_SEND_DTMF:
            return piThread->OnSendDtmf(static_cast<ImsMediaMsgDtmfParam*>(pParam));
        case IMMedia::REQUEST_SET_MEDIA_QUALITY:
            return piThread->OnSetMediaQualityThreshold(
                    static_cast<ImsMediaMsgSetMediaQualityParam*>(pParam));
        case IMMedia::REQUEST_SET_PREVIEW_SURFACE:
            return piThread->OnSetPreviewSurface();
        case IMMedia::REQUEST_SET_DISPLAY_SURFACE:
            return piThread->OnSetDisplaySurface();
        default:
            IMS_TRACE_E(0, "SendMessageToMediaService() eEvent[%d], not handled", eEvent, 0, 0);
            return IMS_TRUE;
    }
}

PRIVATE
IJniMediaSessionThread* MediaMsgHandler::GetJniThread()
{
    IJniEnabler* piJniMediaSession = JniEnablerConnector::GetInstance().GetJniEnabler(
            m_nSlotId, EnablerType::MEDIA_SESSION, m_nCallKey);
    if (piJniMediaSession == IMS_NULL)
    {
        return IMS_NULL;
    }
    return reinterpret_cast<IJniMediaSessionThread*>(piJniMediaSession->GetJniThread());
}
