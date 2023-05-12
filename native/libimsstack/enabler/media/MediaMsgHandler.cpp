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
#include "IJniMediaSessionThread.h"
#include "IJniEnabler.h"

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
IMS_BOOL MediaMsgHandler::SendMessageToJava(IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam)
{
    IMS_TRACE_I("SendMessageToJava() - eEvent[%d], strListenerThread[%s]", eEvent,
            m_strListenerThread.GetStr(), 0);

    IJniEnabler* piJniMediaSession = JniEnablerConnector::GetInstance().GetJniEnabler(
            m_nSlotId, EnablerType::MEDIA_SESSION, m_nCallKey);

    if (piJniMediaSession == IMS_NULL)
    {
        IMS_TRACE_D("SendMessageToJava() - piJniMediaSession is null", 0, 0, 0);
        return IMS_NULL;
    }

    IJniMediaSessionThread* piThread =
            static_cast<IJniMediaSessionThread*>(piJniMediaSession->GetJniThread());

    if (piThread == IMS_NULL)
    {
        IMS_TRACE_D("SendMessageToJava() - piThread is null", 0, 0, 0);
        return IMS_FALSE;
    }

    switch (eEvent)
    {
        case IJniMedia::REQUEST_OPEN_SESSION:
            return piThread->OnOpenSession(static_cast<ImsMediaMsgOpenConfigParam*>(pParam));
        case IJniMedia::REQUEST_MODIFY_SESSION:
            return piThread->OnModifySession(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IJniMedia::REQUEST_CLOSE_SESSION:
            return piThread->OnCloseSession(pParam);
        case IJniMedia::REQUEST_ADD_CONFIG:
            return piThread->OnAddConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IJniMedia::REQUEST_DELETE_CONFIG:
            return piThread->OnDeleteConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IJniMedia::REQUEST_CONFIRM_CONFIG:
            return piThread->OnConfirmConfig(static_cast<ImsMediaMsgConfigParam*>(pParam));
        case IJniMedia::REQUEST_SEND_DTMF:
            return piThread->OnSendDtmf(static_cast<ImsMediaMsgDtmfParam*>(pParam));
        case IJniMedia::REQUEST_SET_MEDIA_QUALITY:
            return piThread->OnSetMediaQualityThreshold(
                    static_cast<ImsMediaMsgSetMediaQualityParam*>(pParam));
        case IJniMedia::REQUEST_QOS:
            return piThread->OnRequestQos(static_cast<ImsMediaMsgQosParam*>(pParam));
        case IJniMedia::REQUEST_SET_PREVIEW_SURFACE:
            piThread->OnSetPreviewSurface();
            break;
        case IJniMedia::REQUEST_SET_DISPLAY_SURFACE:
            piThread->OnSetDisplaySurface();
            break;
        default:
            IMS_TRACE_E(0, "SendMessageToJava() - eEvent[%d], not handled", eEvent, 0, 0);
            return IMS_FALSE;
    }

    return IMS_TRUE;
}