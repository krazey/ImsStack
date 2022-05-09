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

// == INCLUDES =========================================================
#include "ServiceTrace.h"
#include "MediaMsgHandler.h"

// == DEFINES =========================================================
__IMS_TRACE_TAG_USER_DECL__("MED.MH");

// == Constructor, Destructor, Operator Overloading ========================================
PUBLIC
MediaMsgHandler::MediaMsgHandler(IMS_SINT32 _nAppId) :
        nAppId(_nAppId),
        strListenerThread(AString::ConstNull()),
        m_pThread(IMS_NULL)
{
    (void)nAppId;
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
    strListenerThread = strName;
}

PUBLIC
void MediaMsgHandler::SetJniMediaSessionThread(IN JniMediaSessionThread* pThread)
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
            strListenerThread.GetStr(), 0);

    if (!IsAvailableToSend())
    {
        return IMS_FALSE;
    }

    switch (eEvent)
    {
        case IMMedia::REQUEST_OPEN_SESSION:
            return m_pThread->OnOpenSession((ImsMediaMsgOpenConfigParam*)pParam);
        case IMMedia::REQUEST_MODIFY_SESSION:
            return m_pThread->OnModifySession((ImsMediaMsgConfigParam*)pParam);
        case IMMedia::REQUEST_CLOSE_SESSION:
            return m_pThread->OnCloseSession((ImsMediaMsgParamBase*)pParam);
        case IMMedia::REQUEST_ADD_CONFIG:
            return m_pThread->OnAddConfig((ImsMediaMsgConfigParam*)pParam);
        case IMMedia::REQUEST_DELETE_CONFIG:
            return m_pThread->OnDeleteConfig((ImsMediaMsgConfigParam*)pParam);
        case IMMedia::REQUEST_CONFIRM_CONFIG:
            return m_pThread->OnConfirmConfig((ImsMediaMsgConfigParam*)pParam);
        case IMMedia::REQUEST_SEND_DTMF:
            return m_pThread->OnSendDtmf((ImsMediaMsgDtmfParam*)pParam);
        case IMMedia::REQUEST_SET_MEDIA_QUALITY:
            return m_pThread->OnSetMediaQualityThreshold((ImsMediaMsgSetMediaQualityParam*)pParam);
        default:
            return IMS_TRUE;
    }
}

/*
PUBLIC
VIRTUAL void MediaMsgHandler::SendMessageToUi(
        IN IMS_UINTP nCallKey, IMS_SINT32 eEvent, IN IMS_UINT32 eResult)
{
    IMS_TRACE_I("SendMessageToUi() nCallKey[%d], eEvent[%d], strListenerThread[%s]",
            nCallKey, eEvent, strListenerThread.GetStr());
    IUMediaResultIndParam* pParam = new IUMediaResultIndParam();
    pParam->nAppId = nAppId;
    pParam->nEventName = eEvent;
    pParam->nCallKey = nCallKey;
    pParam->nResult = eResult;
    IMS_MSG_CreateNPostThreadMessageByName(strListenerThread, eEvent, 0,
            reinterpret_cast<IMS_UINTP>(pParam));
}

PUBLIC
VIRTUAL void MediaMsgHandler::OnRttReceivedInd(IN IMS_UINTP nCallKey, IMS_SINT32 eEvent,
        IN IUMediaRttDataParam* pParam)
{
    IMS_TRACE_I( "OnRttReceivedInd() nCallKey[%d], eEvent[%d], strListenerThread[%s]",
            nCallKey, eEvent, strListenerThread.GetStr());

    pParam->nAppId = nAppId;
    pParam->nEventName = eEvent;
    pParam->nCallKey = nCallKey;

    IMS_MSG_CreateNPostThreadMessageByName(strListenerThread, eEvent, 0, pParam);
}

PUBLIC
void MediaMsgHandler::OnRttAudioIndicator(IN IMS_UINTP nCallKey, IN IMS_SINT32 eEvent,
        IN IMS_UINT32 eRttAudioInd)
{
    IMS_TRACE_I( "OnRttAudioIndicator() nCallKey[%d], eEvent[%d], eRttAudioInd[%d]",
            nCallKey, eEvent, eRttAudioInd);

    IUMediaResultIndParam* pParam = new IUMediaResultIndParam();
    pParam->nAppId = nAppId;
    pParam->nEventName = eEvent;

    pParam->nCallKey = nCallKey;
    pParam->nResult = eRttAudioInd;

    IMS_MSG_CreateNPostThreadMessageByName(strListenerThread, eEvent, 0,
            reinterpret_cast<IMS_UINTP>(pParam));
}

PUBLIC
void MediaMsgHandler::OnDataUsageChanged(IN IMS_UINTP nCallKey, IMS_SINT32 eEvent,
        IN IUMediaDataUsageInfoParam* pParam)
{
    IMS_TRACE_I( "OnDataUsageChanged() nCallKey[%d], eEvent[%d], strListenerThread[%s]",
            nCallKey, eEvent, strListenerThread.GetStr());

    IUMediaDataUsageInfoParam* param = new IUMediaDataUsageInfoParam();

    param->nAppId = nAppId;
    param->nEventName = eEvent;

    param->nCallKey = nCallKey;

    param->nDataUsageRx = pParam->nDataUsageRx;
    param->nDataUsageTx = pParam->nDataUsageTx;

    IMS_MSG_CreateNPostThreadMessageByName(strListenerThread, eEvent, 0, param);
}
*/
