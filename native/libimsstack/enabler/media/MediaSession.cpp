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

// == INCLUDES =============================================================
#include "Configuration.h"
#include "config/MediaSessionConfigFactory.h"
#include "EnablerUtils.h"
#include "ICoreService.h"
#include "ISessionDescriptor.h"
#include "MediaDef.h"
#include "MediaManager.h"
#include "MediaNegoUtil.h"
#include "MediaSession.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("MED.SS");

using namespace android::telephony::imsmedia;

PUBLIC
MediaSession::MediaSession(
        IN MEDIA_SERVICE_TYPE eServiceType, IMS_SINTP nCallKey, IN IMS_UINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_nCallKey(nCallKey),
        m_pClientListener(IMS_NULL),
        m_pEnvironment(IMS_NULL),
        m_nRtpTimer(0)
{
    IMS_TRACE_D(
            "+MediaSession() - ServiceType[%" PFLS_u "], CallKey[%d]", eServiceType, nCallKey, 0);

    CreateMediaConfig(eServiceType);
}

PUBLIC VIRTUAL MediaSession::~MediaSession()
{
    IMS_TRACE_I("~MediaSession() - CallKey[%d]", m_nCallKey, 0, 0);
    std::lock_guard<std::mutex> guard(m_objMutex);

    ClearMediaNego();
    m_objAudioController.CloseSession();
    m_objVideoController.CloseSession();

    if (m_pEnvironment != IMS_NULL)
    {
        delete m_pEnvironment;
        m_pEnvironment = IMS_NULL;
    }
}

PUBLIC
MediaEnvironment* MediaSession::GetEnvironment()
{
    return m_pEnvironment;
}

PUBLIC
MEDIA_NETWORK_TYPE MediaSession::GetNetworkType()
{
    if (m_pEnvironment != IMS_NULL)
    {
        return m_pEnvironment->eNetworkType;
    }

    return MEDIA_NETWORK_NONE;
}

PUBLIC
IMS_BOOL MediaSession::GetDTMFEnabled(IN IMS_UINTP nNegoId)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AudioNego* pAudioNego = pMediaNego->GetAudioNego();
    if (pAudioNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pAudioNego->HasNegotiatedDtmf();
}

PUBLIC VIRTUAL void MediaSession::SendImsMediaRequest()
{
    // TODO_MEDIA later
}

PUBLIC VIRTUAL void MediaSession::OnMediaResponse()
{
    // TODO_MEDIA later
}

PUBLIC VIRTUAL void MediaSession::OnNotify(
        IN IMS_UINT32 eReportType, IN MEDIA_CONTENT_TYPE eMediaType)
{
    if (m_pClientListener != IMS_NULL)
    {
        m_pClientListener->MediaSession_Notify(eReportType, eMediaType);
    }
}

PUBLIC
IMS_BOOL MediaSession::IsHoldSession(IN IMS_UINTP nNegoId)
{
    return (m_objAudioController.IsHoldSession(nNegoId) && m_objVideoController.IsHoldSession());
}

PUBLIC
IMS_BOOL MediaSession::HoldSession()
{
    IMS_TRACE_D("HoldSession()", 0, 0, 0);
    m_objAudioController.HoldSession();
    m_objVideoController.HoldSession();
    return IMS_FALSE;
}

PUBLIC
void MediaSession::ReportToClient(IN RtpError eError, IN MEDIA_CONTENT_TYPE eMediaType)
{
    if (m_pClientListener != IMS_NULL)
    {
        if (eError == RtpError::NO_ERROR)
        {
            m_pClientListener->MediaSession_Notify(REPORT_SUCCESS, eMediaType);
        }
        else if (eError > RtpError::NO_ERROR)
        {
            m_pClientListener->MediaSession_NotifyFailures(REPORT_FAILURE, eError, eMediaType);
        }
    }
}

PUBLIC VIRTUAL void MediaSession::SetMtcListener(
        IN IMediaSessionClientListener* piMediaSessionListener)
{
    m_pClientListener = piMediaSessionListener;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::SetEnvironment(IN MediaEnvironment* pEnvironment)
{
    if (pEnvironment == NULL)
    {
        return IMS_FALSE;
    }

    if (pEnvironment->eServiceType == MEDIA_SERVICE_EMERGENCY &&
            pEnvironment->pIService != IMS_NULL)
    {
        IMS_BOOL bIsIPv6 = pEnvironment->pIService->GetIpAddress().IsIPv6Address();
        MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);
        if (pMediaManager != IMS_NULL)
        {
            pMediaManager->GetResourceManager()->UpdatePdnResource(
                    MediaResourceMngr::PDN_EMERGENCY, bIsIPv6);
        }
    }

    // Set the precondition
    IMS_BOOL bNeedToCreateProfile = IMS_FALSE;

    if (m_pEnvironment == IMS_NULL)
    {
        IMS_TRACE_I("SetEnvironment() - CallKey[%d], eServiceType[%d]", m_nCallKey,
                pEnvironment->eServiceType, 0);
        m_pEnvironment = pEnvironment;
        bNeedToCreateProfile = IMS_TRUE;
    }
    else if (m_pEnvironment->eServiceType != pEnvironment->eServiceType ||
            m_pEnvironment->eNetworkType != pEnvironment->eNetworkType)
    {
        IMS_TRACE_I("SetEnvironment() - CallKey[%d], eServiceType[%d]->[%d]", m_nCallKey,
                m_pEnvironment->eServiceType, pEnvironment->eServiceType);
        delete m_pEnvironment;
        m_pEnvironment = pEnvironment;
        bNeedToCreateProfile = IMS_TRUE;
    }

    if (bNeedToCreateProfile == IMS_TRUE)
    {
        for (IMS_UINT32 nIndex = 0; nIndex < m_objMapMediaNego.GetSize(); nIndex++)
        {
            MediaNego* pMediaNego = m_objMapMediaNego.GetValueAt(nIndex);
            pMediaNego->SetMediaEnvironment(m_pEnvironment);
        }
    }
    return IMS_TRUE;  // do it later
}

PUBLIC VIRTUAL IMS_UINTP MediaSession::CreateProfile(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType)
{
    IMS_TRACE_I("CreateProfile() - nNegoId[%" PFLS_x "], MediaType[%d]", nNegoId, eMediaType, 0);

    MediaNego* pMediaNego = CreateMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_UINTP nMediaNego = reinterpret_cast<IMS_UINTP>(pMediaNego);

    AudioNego* pAudioNego = pMediaNego->GetAudioNego();

    if (pAudioNego != NULL)
    {
        m_objAudioController.CreateSession(this, nMediaNego, pAudioNego->GetConfig());
    }

    VideoNego* pVideoNego = pMediaNego->GetVideoNego();

    if (pVideoNego != IMS_NULL)
    {
        m_objVideoController.CreateSession(this, pVideoNego->GetConfig());
    }

    IMS_TRACE_I("CreateProfile() - exit nMediaNego[%" PFLS_x "], Size[%d]", nMediaNego,
            m_objMapMediaNego.GetSize(), 0);

    return nMediaNego;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::DestroyProfile(IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DestroyProfile() - nNegoId[%" PFLS_x "]", nNegoId, 0, 0);

    if (nNegoId == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bRet = IMS_TRUE;
    bRet &= DeleteMediaNego(nNegoId);
    bRet &= m_objAudioController.DeleteSession(nNegoId);
    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::FormSDP(IN IMS_UINTP nNegoId, OUT ISession* pSession,
        IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_SINT32 eAudioDir, IN IMS_SINT32 eVideoDir,
        IN IMS_SINT32 eTextDir)
{
    IMS_TRACE_I("FormSDP() - nNegoId[%" PFLS_x "], pSession[%" PFLS_x "], eMediaType[%d]", nNegoId,
            pSession, eMediaType);
    IMS_TRACE_I("FormSDP() - DIR = Audio[%d], Video[%d], Text[%d]", eAudioDir, eVideoDir, eTextDir);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormSDP() - Can't find nNegoId[%d]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    if (pMediaNego->FormSDP(pSession, eMediaType, eAudioDir, eVideoDir, eTextDir) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "FormSDP() - FormSDP Failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // audio
    if (pMediaNego->GetAudioNego() != IMS_NULL)
    {
        m_objAudioController.UpdateLocalAddress(pMediaNego->GetAudioNego());
        m_objAudioController.OpenSession(nNegoId);
    }

    // video
    if (pMediaNego->GetVideoNego() != IMS_NULL)
    {
        m_objVideoController.CreateSession(this, pMediaNego->GetVideoNego()->GetConfig());
        m_objVideoController.UpdateLocalAddress(pMediaNego->GetVideoNego());
        m_objVideoController.OpenSession();
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::NegotiateSDP(IN IMS_UINTP nNegoId, IN ISession* pSession,
        OUT IMS_SINT32* eAudioDir, OUT IMS_SINT32* eVideoDir, OUT IMS_SINT32* eTextDir,
        OUT MediaNego::MediaNegoResult& errorReason)
{
    IMS_TRACE_I(
            "NegotiateSDP() - nNegoId[%" PFLS_x "], pSession[%" PFLS_x "]", nNegoId, pSession, 0);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateSDP() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    if (pMediaNego->NegotiateSDP(pSession, eAudioDir, eVideoDir, eTextDir, errorReason) == IMS_TRUE)
    {
        // audio
        if (pMediaNego->GetAudioNego() != IMS_NULL)
        {
            m_objAudioController.UpdateRtpConfig(nNegoId, pMediaNego->GetAudioNego());
            m_objAudioController.AddSession(nNegoId, pMediaNego->GetAudioNego());
        }

        // video
        if (pMediaNego->GetVideoNego() != IMS_NULL)
        {
            m_objVideoController.CreateSession(this, pMediaNego->GetVideoNego()->GetConfig());
            m_objVideoController.UpdateLocalAddress(pMediaNego->GetVideoNego());
            m_objVideoController.OpenSession();
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL void MediaSession::FinalizeSDP(IN IMS_UINTP nNegoId, IN ISession* pSession)
{
    IMS_TRACE_I(
            "FinalizeSDP() - nNegoId[%" PFLS_x "], pSession[%" PFLS_x "]", nNegoId, pSession, 0);

    if (pSession == IMS_NULL)
    {
        return;
    }

    MediaNego* pMediaNego = IMS_NULL;
    pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FinalizeSDP() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return;
    }

    pMediaNego->FinalizeSDP(pSession);
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::Run(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("ProcessRun() - nNegoId[%" PFLS_x "]", nNegoId, 0, 0);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "ProcessRun() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    if (IsHoldSession(nNegoId) != IMS_TRUE)
    {
        IMS_TRACE_D(
                "ProcessRun() - Need to hold active Session except callKey[%d]", m_nCallKey, 0, 0);
        MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);

        if (pMediaManager == IMS_NULL)
        {
            IMS_TRACE_E(0, "ProcessRun() - No MediaManager", 0, 0, 0);
            return IMS_FALSE;
        }

        pMediaManager->OtherSessionHold(m_nCallKey);
    }

    m_objAudioController.UpdateQualityThreshold(nNegoId, pMediaNego->GetAudioNego());
    m_objAudioController.UpdateRtpConfig(nNegoId, pMediaNego->GetAudioNego());
    m_objAudioController.UpdateSession(nNegoId);

    m_objVideoController.UpdateQualityThreshold(pMediaNego->GetVideoNego());
    m_objVideoController.UpdateRtpConfig(pMediaNego->GetVideoNego());
    m_objVideoController.UpdateSession();

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::Terminate()
{
    IMS_TRACE_I(
            "Terminate() - CallKey[%d], nego list[%d]", m_nCallKey, m_objMapMediaNego.GetSize(), 0);
    std::lock_guard<std::mutex> guard(m_objMutex);

    if (m_objMapMediaNego.IsEmpty())
    {
        IMS_TRACE_E(0, "Terminate() - No profile to terminate", 0, 0, 0);
        return IMS_FALSE;
    }

    m_objAudioController.CloseSession();
    m_objVideoController.CloseSession();
    ClearMediaNego();

    return IMS_TRUE;
}

PUBLIC VIRTUAL NEGO_STATE MediaSession::GetNegoState(IN IMS_UINTP nNegoId)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        return STATE_NOTUSED;
    }
    else
    {
        return pMediaNego->GetNegoState();
    }
}

PUBLIC VIRTUAL MEDIA_CONTENT_TYPE MediaSession::GetNegotiatedMediaType(IN IMS_UINTP nNegoId)
{
    MEDIA_CONTENT_TYPE eMedia = MEDIA_TYPE_INVALID;
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        return eMedia;
    }

    if (pMediaNego->GetNegotiatedAudioQuality() != AUDIO_CODEC_NOT_USED)
    {
        eMedia = (MEDIA_CONTENT_TYPE)(eMedia | MEDIA_TYPE_AUDIO);
    }

    if (pMediaNego->GetNegotiatedVideoQuality() != VIDEO_RESOLUTION_NOT_USED)
    {
        eMedia = (MEDIA_CONTENT_TYPE)(eMedia | MEDIA_TYPE_VIDEO);
    }
    // TODO_MEDIA: add implementation for TEXT

    return eMedia;
}

PUBLIC VIRTUAL IMS_SINT32 MediaSession::GetNegotiatedQuality(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedQuality() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return 0;
    }

    switch (type)
    {
        case MEDIA_TYPE_AUDIO:
            return (IMS_SINT32)(pMediaNego->GetNegotiatedAudioQuality());
        case MEDIA_TYPE_VIDEO:
            return (IMS_SINT32)(pMediaNego->GetNegotiatedVideoQuality());
        // TODO_MEDIA: add implementation for text
        default:
            break;
    }

    return 0;
}

PUBLIC VIRTUAL IMS_SINT32 MediaSession::GetNegotiatedCodecBitrate(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "GetNegotiatedCodecBitrate() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return 0;
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_AUDIO))
    {
        AudioNego* pAudioNego = pMediaNego->GetAudioNego();
        if (pAudioNego == IMS_NULL)
        {
            return (IMS_SINT32)AUDIO_CODEC_BITRATE_MAX;
        }
        return (IMS_SINT32)pAudioNego->GetNegotiatedAudioCodecRate();
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_VIDEO))
    {
        // VideoNego* pVideoNego = pMediaNego->GetVideoNego();
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_TEXT))
    {
        // TODO_MEDIA: add implementation for TEXT
    }

    return 0;
}

/*
PUBLIC VIRTUAL
IMS_SINT32 MediaSession::GetNegotiatedCodecBandwidth(IN IMS_UINTP nNegoId,
    IN MEDIA_CONTENT_TYPE type) {
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL) {
        IMS_TRACE_E(0, "GetNegotiatedCodecBandwidth() - Can't find nNegoId[%" PFLS_x "]",
            nNegoId, 0, 0);
        return 0;
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_AUDIO)) {
        AudioNego* pAudioNego = pMediaNego->GetAudioNego();
        if (pAudioNego == IMS_NULL) return 0;
        return (IMS_SINT32)pAudioNego->GetMediaBandwidth();
    }
    return 0;
}
*/
PUBLIC VIRTUAL MEDIA_DIRECTION MediaSession::GetNegotiatedDirection(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedMediaDirection() - Can't find nMediaNegoId[%" PFLS_x "]",
                nNegoId, 0, 0);

        return MEDIA_DIRECTION_INVALID;
    }

    switch (type)
    {
        case MEDIA_TYPE_AUDIO:
        {
            AudioNego* pAudioNego = pMediaNego->GetAudioNego();
            if (pAudioNego != IMS_NULL)
            {
                return pAudioNego->GetNegotiatedDirection();
            }
        }
        break;
        case MEDIA_TYPE_VIDEO:
        {
            VideoNego* pVideoNego = pMediaNego->GetVideoNego();
            if (pVideoNego != IMS_NULL)
            {
                return pVideoNego->GetNegotiatedDirection();
            }
        }
        break;
        // TODO_MEDIA text
        // case MEDIA_TYPE_TEXT:
        // {
        //     TextNego* pTextNego = pMediaNego->GetTextNego();
        //     if (pTextNego != IMS_NULL)
        //     {
        //         return pTextNego->GetNegotiatedDirection();
        //     }
        // }
        //     break;
        default:
            break;
    }

    return MEDIA_DIRECTION_INVALID;
}

PUBLIC VIRTUAL void MediaSession::SetOptions(
        IN IMS_UINTP nNegoId, OptionType type, IN IMS_SINT32 param1, IN IMS_SINT32 param2)
{
    IMS_TRACE_I("SetOptions() - OptionType[%d], param1[%d], param2[%d]", type, param1, param2);
    MediaNego* pMediaNego = IMS_NULL;
    AudioNego* pAudioNego = IMS_NULL;
    VideoNego* pVideoNego = IMS_NULL;

    switch (type)
    {
        case SET_RTP_PORT:
            pMediaNego = FindMediaNego(nNegoId);
            if (pMediaNego != IMS_NULL)
            {
                if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO)
                {
                    pAudioNego = pMediaNego->GetAudioNego();

                    if (pAudioNego != IMS_NULL)
                    {
                        pAudioNego->SetPort(param2);
                    }
                }
                else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_VIDEO)
                {
                    pVideoNego = pMediaNego->GetVideoNego();

                    if (pVideoNego != IMS_NULL)
                    {
                        pVideoNego->SetPort(param2);
                    }
                }
                else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_TEXT)
                {
                    // TODO_MEDIA: add implementation for text
                }
            }
            break;
        case SET_DIRECTION:
            // TODO_MEDIA: add implementation
            // if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO)
            // {
            //     pAudioNego = pMediaNego->GetAudioNego();
            //     if (pAudioNego != IMS_NULL)
            //     {
            //         pAudioNego->SetDirection(param2);
            //     }
            // } else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_VIDEO) {
            //     //to do
            // } else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_TEXT) {
            //     //to do
            // }
            break;
        case SET_CONFIRMED_SESSION:
            m_objAudioController.SetConfirmSession(param1);
            break;
        case SET_DRA_REPORT_OPTION:  // for Q3 (Analyzer)
        case SET_CONFERENCE_ENABLE:  // TODO: for video conference
        case SET_CVO_SUPPORT:
        case SEND_FAST_VIDEO_UPDATE:  // for video
            break;
        default:
            break;
    }
}

PUBLIC VIRTUAL void MediaSession::SetNetworkToneRTPTimer(
        MEDIA_CONTENT_TYPE eMediaType, IN IMS_UINT32 nRtpTimer)
{
    (void)eMediaType;  // do it later
    IMS_TRACE_I("SetNetworkToneRTPTimer() - CallKey[%d], eMediaType[%d], nRtpTimer[%d]", m_nCallKey,
            eMediaType, nRtpTimer);

    m_nRtpTimer = nRtpTimer;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::SendMessage(IN IMSMSG& objMsg)
{
    (void)objMsg;
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    IMS_TRACE_I(
            "SendMessage() - Msg[%d, %s], CallKey[%d]", nMsg, IMMedia::PrintMsg(nMsg), m_nCallKey);

    IMS_BOOL bRet = IMS_TRUE;
    IMMedia::MessageType nMsgType = IMMedia::CategorizeMessageType(nMsg);

    switch (nMsgType)
    {
        case IMMedia::MSG_RESPONSE:
            bRet = OnResponse(nMsg, pParam);
            break;
        case IMMedia::MSG_VIDEO_NOTIFICATION:
            bRet = m_objVideoController.SendMessage(nMsg, pParam);
            break;
        default:
            break;
    }

    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::SendDtmf(
        IN IMS_UINTP nNegoId, IN IMS_CHAR cDtmfCode, IN IMS_SINT32 nDuration)
{
    if (GetDTMFEnabled(nNegoId) == IMS_TRUE)
    {
        return m_objAudioController.SendDtmf(nNegoId, cDtmfCode, nDuration);
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MediaSession::CreateMediaConfig(IN MEDIA_SERVICE_TYPE eServiceType)
{
    AString strMediaRef = "gims.com.media";

    IMS_TRACE_D("CreateMediaConfig() media_ref=%s", strMediaRef.GetStr(), 0, 0);

    MediaSessionConfigFactory::GetInstance()->CreateMediaSessionConfig(m_nSlotId, eServiceType);

    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    m_nSlotId, eServiceType);
    if (pMediaSessionConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateMediaConfig() - fail to  CreateMediaConfig", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!pMediaSessionConfig->Create(m_nSlotId))  // todo
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL MediaSession::OnResponse(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    IMS_TRACE_I("OnResponse() - nMsg[%d, %s]", nMsg, IMMedia::PrintMsg(nMsg), 0);

    if (m_pClientListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "OnResponse() - No ClientListener", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bRet = IMS_TRUE;

    switch (nMsg)
    {
        case IMMedia::RESPONSE_OPEN_SESSION:
            bRet = OnResponseOpenSession(pParam);
            break;
        case IMMedia::RESPONSE_MODIFY_SESSION:
            bRet = OnResponseModifySession(pParam);
            break;
        case IMMedia::RESPONSE_ADD_CONFIG:
            bRet = OnResponseAddConfig(pParam);
            break;
        case IMMedia::RESPONSE_CONFIRM_CONFIG:
            bRet = OnResponseConfirmConfig(pParam);
            break;
        case IMMedia::NOTIFY_FIRST_PACKET:
            bRet = OnNotifyFirstPacket(pParam);
            break;
        case IMMedia::NOTIFY_MEDIA_INACTIVITY:
            bRet = OnNotifyMediaInactivity(pParam);
            break;
        case IMMedia::NOTIFY_PACKET_LOSS:
            bRet = OnNofityPacketLoss(pParam);
            break;
        case IMMedia::NOTIFY_JITTER:
            bRet = OnNofityJitter(pParam);
            break;
        case IMMedia::NOTIFY_CALL_QUALITY_CHANGE:
            bRet = OnNofityCallQualityChange(pParam);
            break;
        case IMMedia::RESPONSE_SESSION_CHANGED:
            bRet = OnResponseSessionChanged(pParam);
            break;
        case IMMedia::NOTIFY_HEADER_EXTENSION:
            bRet = OnNofityHeaderExtension(pParam);
            break;
        case IMMedia::NOTIFY_QOS_INFO:
            bRet = OnNotifyQosInfo(pParam);
            break;
        case IMMedia::NOTIFY_MEDIA_DETACH:
            bRet = OnNotifyMediaDetach();
            break;
        default:
            break;
    }

    return bRet;
}

PROTECTED
MediaNego* MediaSession::CreateMediaNego(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_I("CreateMediaNego() nNegoId[%d]", nNegoId, 0, 0);

    // Create new MediaNego
    MediaNego* pMediaNego = new MediaNego(m_nSlotId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateMediaNego() - fail to create MediaNego", 0, 0, 0);
        return IMS_NULL;
    }

    pMediaNego->Create(m_pEnvironment->eServiceType);
    pMediaNego->SetMediaEnvironment(m_pEnvironment);

    // Copy Existed Media Nego with nego id
    if (nNegoId != 0)
    {
        MediaNego* objExistingNego = FindMediaNego(nNegoId);

        if (objExistingNego == IMS_NULL)
        {
            IMS_TRACE_I("CreateMediaNego() - invalid negoId", 0, 0, 0);
            return IMS_NULL;
        }

        pMediaNego->Forking(objExistingNego);
    }

    m_objMapMediaNego.Add(reinterpret_cast<IMS_UINTP>(pMediaNego), pMediaNego);
    return pMediaNego;
}

PROTECTED
MediaNego* MediaSession::FindMediaNego(IN IMS_UINTP nNegoId)
{
    MediaNego* pMediaNego = IMS_NULL;
    IMS_SLONG nIndex = m_objMapMediaNego.GetIndexOfKey(nNegoId);

    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "FindMediaNego -- invalid nNegoId[%d]", nNegoId, 0, 0);
        return IMS_NULL;
    }

    pMediaNego = m_objMapMediaNego.GetValueAt(nIndex);

    if (pMediaNego == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pMediaNego;
}

PROTECTED
IMS_BOOL MediaSession::DeleteMediaNego(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DeleteMediaNego() - nNegoId[%" PFLS_x "], Size[%d]", nNegoId,
            m_objMapMediaNego.GetSize(), 0);

    if (nNegoId == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SLONG nIndex = m_objMapMediaNego.GetIndexOfKey(nNegoId);
    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "DeleteMediaNego() - invalid nNegoId[%d]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    MediaNego* pMediaNego = m_objMapMediaNego.GetValueAt(nIndex);
    m_objMapMediaNego.RemoveAt(nIndex);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "DeleteMediaNego() - pMediaNego is NULL", 0, 0, 0);
        return IMS_FALSE;
    }
    delete pMediaNego;
    pMediaNego = IMS_NULL;

    return IMS_TRUE;
}

PROTECTED
void MediaSession::ClearMediaNego()
{
    IMS_TRACE_D("ClearMediaNego() m_objMapMediaNego size[%d]", m_objMapMediaNego.GetSize(), 0, 0);

    MediaNego* pMediaNego = IMS_NULL;

    while (m_objMapMediaNego.GetSize() > 0)
    {
        pMediaNego = m_objMapMediaNego.GetValueAt(0);

        if (pMediaNego != IMS_NULL)
        {
            delete pMediaNego;
            pMediaNego = IMS_NULL;
        }

        m_objMapMediaNego.RemoveAt(0);
    }

    m_objMapMediaNego.Clear();
}

PROTECTED
VIRTUAL void MediaSession::MediaSession_SendEventToUi(IMS_SINT32 nEvent, IMS_SINT32 nResult)
{
    IMS_TRACE_D("MediaSession_SendEventToUi() : CallKey[%d] nEvent[%d], nResult[%d]", m_nCallKey,
            nEvent, nResult);
    IMSMSG objMsg(nEvent, m_nCallKey, nResult);
    MessageService::PostMessage(MediaManager::GetThreadName(m_nSlotId), objMsg);
}

PROTECTED VIRTUAL IMS_BOOL MediaSession::MediaSession_SendMsgToMediaManager(
        IN IMS_SINT32 nEvent, IN ImsMediaMsgParamBase* param)
{
    IMS_TRACE_D(
            "MediaSession_SendMsgToMediaManager() : CallKey[%d] nEvent[%d]", m_nCallKey, nEvent, 0);
    MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);
    if (pMediaManager != IMS_NULL)
    {
        return pMediaManager->handleRequestMsg(nEvent, m_nCallKey, param);
    }
    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseOpenSession(IN IMS_UINTP pParam_)
{
    ImsMediaResponseConfigParam* pParam = reinterpret_cast<ImsMediaResponseConfigParam*>(pParam_);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    RtpError eResult = pParam->m_eResult;

    IMS_TRACE_I("OnResponseOpenSession() - CallKey[%d], eMediaType[%d], eResult[%d]", m_nCallKey,
            eMediaType, eResult);

    ReportToClient(eResult, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseModifySession(IN IMS_UINTP pParam_)
{
    ImsMediaResponseConfigParam* pParam = reinterpret_cast<ImsMediaResponseConfigParam*>(pParam_);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    RtpError eResult = pParam->m_eResult;
    AudioConfig objAudioConfig = pParam->m_objAudioConfig;
    (void)objAudioConfig;

    IMS_TRACE_I("OnResponseModifySession() - CallKey[%d], eMediaType[%d], eResult[%d]", m_nCallKey,
            eMediaType, eResult);

    ReportToClient(eResult, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseAddConfig(IN IMS_UINTP pParam_)
{
    ImsMediaResponseConfigParam* pParam = reinterpret_cast<ImsMediaResponseConfigParam*>(pParam_);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    RtpError eResult = pParam->m_eResult;
    AudioConfig objAudioConfig = pParam->m_objAudioConfig;
    (void)objAudioConfig;

    IMS_TRACE_I("OnResponseAddConfig() - CallKey[%d], eMediaType[%d], eResult[%d]", m_nCallKey,
            eMediaType, eResult);

    ReportToClient(eResult, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseConfirmConfig(IN IMS_UINTP pParam_)
{
    ImsMediaResponseConfigParam* pParam = reinterpret_cast<ImsMediaResponseConfigParam*>(pParam_);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    RtpError eResult = pParam->m_eResult;
    AudioConfig objAudioConfig = pParam->m_objAudioConfig;
    (void)objAudioConfig;

    IMS_TRACE_I("OnResponseConfirmConfig() - CallKey[%d], eMediaType[%d], eResult[%d]", m_nCallKey,
            eMediaType, eResult);

    ReportToClient(eResult, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotifyFirstPacket(IN IMS_UINTP pParam_)
{
    ImsMediaResponseConfigParam* pParam = reinterpret_cast<ImsMediaResponseConfigParam*>(pParam_);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;

    IMS_TRACE_I(
            "OnResponseConfirmConfig() - CallKey[%d], eMediaType[%d]", m_nCallKey, eMediaType, 0);

    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_STARTED, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotifyMediaInactivity(IN IMS_UINTP pParam_)
{
    ImsMediaNotifyInactivityParam* pParam =
            reinterpret_cast<ImsMediaNotifyInactivityParam*>(pParam_);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType =
            pParam->m_eMediaProtocolType == RTP ? MEDIA_PROTOCOL_RTP : MEDIA_PROTOCOL_RTCP;

    IMS_TRACE_I("OnNotifyMediaInactivity() - eMediaProtocolType[%d], eMediaType[%d]",
            eMediaProtocolType, eMediaType, 0);

    m_pClientListener->MediaSession_Notify(
            REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType, eMediaProtocolType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNofityPacketLoss(IN IMS_UINTP pParam_)
{
    ImsMediaNotifyPacketParam* pParam = reinterpret_cast<ImsMediaNotifyPacketParam*>(pParam_);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    IMS_SINT32 nPacketLossPercentage = pParam->m_nResponse;

    IMS_TRACE_I("OnNofityPacketLoss() - CallKey[%d], eMediaType[%d], nPacketLoss[%d]", m_nCallKey,
            eMediaType, nPacketLossPercentage);

    // do it later :
    // TBD : need to discuss how to use this info

    //    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNofityJitter(IN IMS_UINTP pParam_)
{
    ImsMediaNotifyPacketParam* pParam = reinterpret_cast<ImsMediaNotifyPacketParam*>(pParam_);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    IMS_SINT32 nJitter = pParam->m_nResponse;

    IMS_TRACE_I("OnNofityJitter() - CallKey[%d], eMediaType[%d], nJitter[%d]", m_nCallKey,
            eMediaType, nJitter);

    // do it later :
    // TBD : need to discuss how to use this info

    //    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNofityCallQualityChange(IN IMS_UINTP pParam_)
{
    (void)pParam_;
    /* TODO_MEDIA later
        ImsMediaNotifyQualityParam* pParam =
                reinterpret_cast<ImsMediaNotifyQualityParam*>(pParam_);
        MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;

        IMS_TRACE_I("OnNofityCallQualityChange() - CallKey[%d], eMediaType[%d]",
                m_nCallKey, eMediaType, 0);

        // do it later
        // need to get CallQuality from ImsMediaNotifyQualityParam

        // do it later :
        // TBD : need to discuss how to use this info

    //    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);
        delete pParam;
    */
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseSessionChanged(IN IMS_UINTP pParam_)
{
    (void)pParam_;
    /* TODO_MEDIA later
        ImsMediaSessionChangedParam* pParam =
                reinterpret_cast<ImsMediaSessionChangedParam*>(pParam_);
        MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;

        IMS_TRACE_I("OnResponseSessionChanged() - CallKey[%d], eMediaType[%d]",
                m_nCallKey, eMediaType, 0);

        // need to get RtpSession from ImsMediaSessionChangedParam

        // TBD : need to discuss how to use this info

    //    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);
        delete pParam;
    */ // NEXT_ITEM :: OnSessionChanged
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNofityHeaderExtension(IN IMS_UINTP pParam_)
{
    (void)pParam_;
    /*  TODO_MEDIA later
        ImsMediaHeaderExtensionParam* pParam =
                reinterpret_cast<ImsMediaHeaderExtensionParam*>(pParam_);
        MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;

        IMS_TRACE_I("OnNofityHeaderExtension() - CallKey[%d], eMediaType[%d]",
                m_nCallKey, eMediaType, 0);

        // need to get RtpHeaderExtension from ImsMediaSessionChangedParam

        // TBD : need to discuss how to use this info

    //    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);
        delete pParam;
    */ // NEXT_ITEM :: OnNofityHeaderExtension
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotifyQosInfo(IN IMS_UINTP pParam_)
{
    ImsMediaNotifyQosParam* pParam = reinterpret_cast<ImsMediaNotifyQosParam*>(pParam_);
    IPAddress objIpAddr = pParam->m_objIpAddr;
    IMS_SINT32 nPort = pParam->m_nPort;
    IMS_BOOL bResult = pParam->m_bResult;

    IMS_TRACE_I(
            "OnNotifyQosInfo() - CallKey[%d], nPort[%d], bResult[%d]", m_nCallKey, nPort, bResult);

    ImsMediaBasicSessionInfoParam* pBasicSessionInfo =
            GetBasicSessionInfofromRemoteArress(objIpAddr.ToString(), nPort);

    if (pBasicSessionInfo != IMS_NULL)
    {
        m_pClientListener->MediaSession_NotifyQos(
                pBasicSessionInfo->m_nNegoId, bResult, pParam->m_eMediaType);
        delete pBasicSessionInfo;
    }

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotifyMediaDetach()
{
    IMS_TRACE_I("OnNotifyMediaDetach()", 0, 0, 0);

    m_pClientListener->MediaSession_Notify(REPORT_MEDIA_DETACH);

    return IMS_TRUE;
}

PROTECTED
ImsMediaBasicSessionInfoParam* MediaSession::GetBasicSessionInfofromRemoteArress(
        IN AString strIpAddr, IN IMS_SINT32 nPort)
{
    IMS_UINTP nNegoId;
    MEDIA_CONTENT_TYPE eMediaType;

    if (MediaNegoUtil::GetMediaNegoInfo(
                &m_objMapMediaNego, strIpAddr, nPort, nNegoId, eMediaType) == IMS_TRUE)
    {
        ImsMediaBasicSessionInfoParam* pBasicSessionInfo = new ImsMediaBasicSessionInfoParam();
        pBasicSessionInfo->m_nNegoId = nNegoId;
        pBasicSessionInfo->m_eMediaType = eMediaType;
        return pBasicSessionInfo;
    }

    return IMS_NULL;
}