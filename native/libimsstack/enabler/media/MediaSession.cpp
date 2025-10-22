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

#include "MediaSession.h"

#include "IMediaSessionClientListener.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MediaEnvironment.h"
#include "MediaNego.h"
#include "MediaNegoHandler.h"
#include "MediaManager.h"
#include "MediaNegoUtil.h"
#include "MediaNetworkConnectionWatcher.h"
#include "ServiceTrace.h"
#include "audio/AudioController.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaConfigUtil.h"
#include "text/TextController.h"
#include "video/VideoController.h"

__IMS_TRACE_TAG_MEDIA__;

#define MTU_MOBILE     1500
#define MTU_EPDG       1280
#define SIZE_OF_IP_SEC 60
#define SIZE_OF_IPV6   60
#define SIZE_OF_IPV4   40
#define SIZE_OF_RTP    20 + 8  // rtp + header extension (cvo)

using namespace android::telephony::imsmedia;

PUBLIC
MediaSession::MediaSession(MEDIA_NETWORK_TYPE eNetwork, MEDIA_SERVICE_TYPE eServiceType,
        IService* pIService, IN IMS_SINTP nCallKey, IN IMS_UINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_nCallKey(nCallKey),
        m_pClientListener(IMS_NULL),
        m_pEnvironment(std::make_shared<MediaEnvironment>(eNetwork, eServiceType, pIService)),
        m_pMediaNegoHandler(
                std::make_shared<MediaNegoHandler>(m_nSlotId, m_pEnvironment, IMS_NULL)),
        m_pAudioController(std::make_shared<AudioController>()),
        m_pVideoController(std::make_shared<VideoController>()),
        m_pTextController(std::make_shared<TextController>()),
        m_bSessionConfirmed(IMS_FALSE),
        m_eCurMediaType(MEDIA_TYPE_INVALID),
        m_bIsConference(IMS_FALSE)
{
    IMS_TRACE_D(
            "+MediaSession() - ServiceType[%" PFLS_u "], CallKey[%d]", eServiceType, nCallKey, 0);

    CreateMediaConfig(eServiceType);

    m_pNetworkConnectionWatcher = std::make_shared<MediaNetworkConnectionWatcher>(
            m_pEnvironment->pIService->GetIpAddress());
    m_pNetworkConnectionWatcher->SetListener(this);
    m_nCurrentAccessNetwork = m_pNetworkConnectionWatcher->GetNetworkType();
    IMS_TRACE_D("+MediaSession() - Network[%d], Mtu[%d]", m_nCurrentAccessNetwork,
            m_pNetworkConnectionWatcher->GetMtu(), 0);
}

PUBLIC VIRTUAL MediaSession::~MediaSession()
{
    IMS_TRACE_I("~MediaSession() - CallKey[%d]", m_nCallKey, 0, 0);
    std::lock_guard<std::mutex> guard(m_objMutex);

    if (m_pMediaNegoHandler != IMS_NULL)
    {
        m_pMediaNegoHandler->ClearAllMediaNego();
    }

    CloseMediaSessions(MEDIA_TYPE_AUDIOVIDEOTEXT);
    ClearQosParam();
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

PUBLIC void MediaSession::SetMediaNegoHandler(std::shared_ptr<MediaNegoHandler> pMediaNegoHandler)
{
    m_pMediaNegoHandler = pMediaNegoHandler;
}

PUBLIC void MediaSession::SetAudioController(std::shared_ptr<AudioController> pAudioController)
{
    m_pAudioController = pAudioController;
}

PUBLIC void MediaSession::SetVideoController(std::shared_ptr<VideoController> pVideoController)
{
    m_pVideoController = pVideoController;
}

PUBLIC void MediaSession::SetTextController(std::shared_ptr<TextController> pTextController)
{
    m_pTextController = pTextController;
}

PUBLIC VIRTUAL void MediaSession::SetMtcListener(
        IN IMediaSessionClientListener* piMediaSessionListener)
{
    m_pClientListener = piMediaSessionListener;
}

PUBLIC VIRTUAL IMS_UINTP MediaSession::CreateProfile(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("CreateProfile() - NegoId[%" PFLS_x "], MediaType[%d]", nNegoId, eType, 0);

    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfile() - invalid MediaNegoHandler", 0, 0, 0);
        return UNDEFINED_NEGO_ID;
    }

    IMS_UINTP nNewNegoId = m_pMediaNegoHandler->CreateMediaNego(nNegoId);

    if (nNewNegoId == 0)
    {
        IMS_TRACE_E(0, "CreateProfile() - invalid media nego id", 0, 0, 0);
        return UNDEFINED_NEGO_ID;
    }

    CreateMediaSessions(nNewNegoId, eType);
    return nNewNegoId;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::DestroyProfile(IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DestroyProfile() - NegoId[%" PFLS_x "]", nNegoId, 0, 0);

    if (nNegoId == UNDEFINED_NEGO_ID || m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "DestroyProfile() - invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bRet = IMS_TRUE;
    bRet &= m_pMediaNegoHandler->DeleteMediaNego(nNegoId);
    bRet &= m_pAudioController->DeleteSession(nNegoId);
    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::FormSdp(IN IMS_UINTP nNegoId, OUT ISession* pSession,
        IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eAudioDirection,
        IN MEDIA_DIRECTION eVideoDirection, IN MEDIA_DIRECTION eTextDirection,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormSdp() - NegoId[%" PFLS_x "], pSession[%" PFLS_x "], Type[%d]", nNegoId,
            pSession, eType);
    IMS_TRACE_I("FormSdp() - DIR = Audio[%d], Video[%d], Text[%d]", eAudioDirection,
            eVideoDirection, eTextDirection);
    IMS_TRACE_D("FormSdp() - Type[%d], EnforceReofferMode[%d]", eType, bEnforceReofferMode, 0);

    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormSdp() - invalid MediaNegoHandler", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pMediaNegoHandler->FormSdp(nNegoId, pSession, eType, eAudioDirection, eVideoDirection,
                eTextDirection, bEnforceReofferMode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "FormSdp() - FormSdp Failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (eType & MEDIA_TYPE_VIDEO)
    {
        // open the video session for preview
        OpenMediaSessions(nNegoId, m_pMediaNegoHandler->FindMediaNego(nNegoId), MEDIA_TYPE_VIDEO);
    }
    else
    {
        CloseMediaSessions(MEDIA_TYPE_VIDEO);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL MEDIA_CONTENT_TYPE MediaSession::GetSupportedMediaTypesFromSdp(
        IN IMS_UINTP nNegoId, IN ISession* pSession)
{
    IMS_TRACE_I("GetSupportedMediaTypesFromSdp() - NegoId[%" PFLS_x "], pSession[%" PFLS_x "]",
            nNegoId, pSession, 0);

    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetSupportedMediaTypesFromSdp() - invalid MediaNegoHandler", 0, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    return m_pMediaNegoHandler->GetSupportedMediaTypesFromSdp(nNegoId, pSession);
}

PUBLIC VIRTUAL SdpNegotiationResult MediaSession::NegotiateSdp(
        IN IMS_UINTP nNegoId, IN ISession* pSession)
{
    IMS_TRACE_I(
            "NegotiateSdp() - NegoId[%" PFLS_x "], pSession[%" PFLS_x "]", nNegoId, pSession, 0);

    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateSdp() - invalid MediaNegoHandler", 0, 0, 0);
        return SdpNegotiationResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    }

    SdpNegotiationResult objResult = m_pMediaNegoHandler->NegotiateSdp(nNegoId, pSession);

    std::shared_ptr<MediaNego> pMediaNego = m_pMediaNegoHandler->FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateSdp() - Can't find NegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return SdpNegotiationResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    }

    OpenMediaSessions(nNegoId, pMediaNego, objResult.eNegotiatedType);

    if (!(objResult.eNegotiatedType & MEDIA_TYPE_VIDEO))
    {
        CloseMediaSessions(MEDIA_TYPE_VIDEO);
    }

    if (!(objResult.eNegotiatedType & MEDIA_TYPE_TEXT))
    {
        CloseMediaSessions(MEDIA_TYPE_TEXT);
    }

    IMS_TRACE_I("NegotiateSdp() - Audio[%d], Video[%d], Text[%d]", objResult.eAudioDirection,
            objResult.eVideoDirection, objResult.eTextDirection);
    return objResult;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::RequestQos(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("RequestQos() - NegoId[%" PFLS_x "], Type[%d] CurMediaType[%d]", nNegoId, eType,
            m_eCurMediaType);

    if ((eType & MEDIA_TYPE_AUDIO))
    {
        RequestQosParam(nNegoId, MEDIA_TYPE_AUDIO);
    }

    if ((eType & MEDIA_TYPE_VIDEO))
    {
        RequestQosParam(nNegoId, MEDIA_TYPE_VIDEO);
    }
    else if (m_eCurMediaType & MEDIA_TYPE_VIDEO)
    {
        ReleaseQosParam(MEDIA_TYPE_VIDEO);
    }

    if ((eType & MEDIA_TYPE_TEXT))
    {
        RequestQosParam(nNegoId, MEDIA_TYPE_TEXT);
    }
    else if (m_eCurMediaType & MEDIA_TYPE_TEXT)
    {
        ReleaseQosParam(MEDIA_TYPE_TEXT);
    }

    m_eCurMediaType = eType;
    return IMS_TRUE;
}

PROTECTED VIRTUAL void MediaSession::RequestQosParam(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("RequestQosParam() - NegoId[%" PFLS_x "], Type[%d]", nNegoId, eType, 0);

    QosRequestParam* pParam = createQosParam(nNegoId, eType);

    if (pParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "RequestQosParam() - invalid param", 0, 0, 0);
        return;
    }

    // check whether qos for the remote address already requested
    QosRequestParam* pQosParams = FindQosParam(pParam);

    if (pQosParams != IMS_NULL)
    {
        IMS_TRACE_D("RequestQosParam() - Type[%d] found, port[%d]", eType, pQosParams->m_nPort, 0);
        pQosParams->AddNegoId(nNegoId);

        if (pQosParams->m_bResult)  // The qos already acquired
        {
            IMS_TRACE_D("RequestQosParam() - Qos already acquired", 0, 0, 0);
            MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);
            if (pMediaManager != nullptr)
            {
                ImsMediaMsgQosParam* pQosInfoParam = new ImsMediaMsgQosParam(
                        pQosParams->m_eMediaType, pQosParams->m_objIpAddress, pQosParams->m_nPort);
                pQosInfoParam->m_bResult = pQosParams->m_bResult;

                pMediaManager->PostMessage(
                        IJniMedia::NOTIFY_QOS_INFO, m_nCallKey, (IMS_UINTP)pQosInfoParam);
            }
            else
            {
                IMS_TRACE_D("RequestQosParam() - MediaManager is invalid", 0, 0, 0);
            }
        }
        else  // request again
        {
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_QOS,
                    new ImsMediaMsgQosParam(
                            pParam->m_eMediaType, pParam->m_objIpAddress, pParam->m_nPort));
        }

        delete pParam;
    }
    else  // new request
    {
        pParam->AddNegoId(nNegoId);
        MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_QOS,
                new ImsMediaMsgQosParam(
                        pParam->m_eMediaType, pParam->m_objIpAddress, pParam->m_nPort));
        m_objListQosParams.Append(pParam);
    }
}

PROTECTED VIRTUAL void MediaSession::ReleaseQosParam(IN MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("ReleaseQosParam() - Type[%d]", eType, 0, 0);

    for (IMS_SINT32 nIndex = 0; nIndex < m_objListQosParams.GetSize(); nIndex++)
    {
        QosRequestParam* qosParam = m_objListQosParams.GetAt(nIndex);

        if (eType == qosParam->m_eMediaType)
        {
            m_objListQosParams.RemoveAt(nIndex);
            delete qosParam;
        }
    }
}

PUBLIC VIRTUAL void MediaSession::FinalizeSdp(IN IMS_UINTP nNegoId, IN ISession* pSession)
{
    IMS_TRACE_I("FinalizeSdp() - NegoId[%" PFLS_x "], pSession[%" PFLS_x "]", nNegoId, pSession, 0);

    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "FinalizeSdp() - invalid MediaNegoHandler", 0, 0, 0);
        return;
    }

    m_pMediaNegoHandler->FinalizeSdp(nNegoId, pSession);
    m_pMediaNegoHandler->FinalizeNegotiation(nNegoId);
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::Run(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("Run() - NegoId[%" PFLS_x "]", nNegoId, 0, 0);

    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "Run() - invalid MediaNegoHandler", 0, 0, 0);
        return IMS_FALSE;
    }

    UpdateMediaSessions(
            nNegoId, m_pMediaNegoHandler->FindMediaNego(nNegoId), GetNegotiatedMediaType(nNegoId));
    return IMS_TRUE;
}

PUBLIC
void MediaSession::OnNetworkConnectionChanged(IN const IMS_SINT32 nRatType)
{
    IMS_TRACE_D(
            "OnNetworkConnectionChanged(): CallKey[%d], NetworkType[%d]", m_nCallKey, nRatType, 0);

    if (m_pAudioController == IMS_NULL || m_pVideoController == IMS_NULL ||
            m_pTextController == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "OnChangeNetworkConnection(): CallKey[%d], null parameters", m_nCallKey, 0, 0);
        return;
    }

    IMS_UINT32 nAccessNetwork = nRatType;
    m_nCurrentAccessNetwork = nAccessNetwork;

    if (m_pAudioController->IsSessionOpened())
    {
        m_pAudioController->UpdateAccessNetwork(nAccessNetwork);
    }

    if (m_pVideoController->IsSessionOpened())
    {
        m_pVideoController->UpdateAccessNetwork(nAccessNetwork);
    }

    if (m_pTextController->IsSessionOpened())
    {
        m_pTextController->UpdateAccessNetwork(nAccessNetwork);
    }
}

PUBLIC
void MediaSession::OnMediaMtuChanged(IN const IMS_UINT32 nMtu)
{
    IMS_TRACE_D("OnMediaMtuChanged(): CallKey[%d], Mtu[%d]", m_nCallKey, nMtu, 0);

    if (m_pVideoController->IsSessionOpened())
    {
        m_pVideoController->SetMtu(nMtu);
        m_pVideoController->UpdateSession();
    }
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::Terminate()
{
    IMS_TRACE_I("Terminate() - CallKey[%d]", m_nCallKey, 0, 0);
    std::lock_guard<std::mutex> guard(m_objMutex);

    CloseMediaSessions(MEDIA_TYPE_AUDIOVIDEOTEXT);

    if (m_pMediaNegoHandler != IMS_NULL)
    {
        m_pMediaNegoHandler->ClearAllMediaNego();
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL NEGO_STATE MediaSession::GetNegoState(IN IMS_UINTP nNegoId)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegoState() - invalid MediaNegoHandler", 0, 0, 0);
        return STATE_NOTUSED;
    }

    return m_pMediaNegoHandler->GetNegoState(nNegoId);
}

PUBLIC VIRTUAL MEDIA_CONTENT_TYPE MediaSession::GetNegotiatedMediaType(IN IMS_UINTP nNegoId)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedMediaType() - invalid MediaNegoHandler", 0, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    return m_pMediaNegoHandler->GetNegotiatedMediaType(nNegoId);
}

PUBLIC VIRTUAL IMS_SINT32 MediaSession::GetNegotiatedQuality(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedQuality() - invalid MediaNegoHandler", 0, 0, 0);
        return 0;
    }

    return m_pMediaNegoHandler->GetNegotiatedQuality(nNegoId, eType);
}

PUBLIC VIRTUAL IMS_SINT32 MediaSession::GetNegotiatedCodecBitrate(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBitrate() - invalid MediaNegoHandler", 0, 0, 0);
        return 0;
    }

    return m_pMediaNegoHandler->GetNegotiatedCodecBitrate(nNegoId, eType);
}

PUBLIC VIRTUAL IMS_FLOAT MediaSession::GetNegotiatedCodecBitrateKbps(IN IMS_UINTP nNegoId)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBitrateKbps() - invalid MediaNegoHandler", 0, 0, 0);
        return 0;
    }

    return m_pMediaNegoHandler->GetNegotiatedCodecBitrateKbps(nNegoId, MEDIA_TYPE_AUDIO);
}

PUBLIC VIRTUAL IMS_FLOAT MediaSession::GetNegotiatedCodecBandwidthKhz(IN IMS_UINTP nNegoId)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBandwidthKhz() - invalid MediaNegoHandler", 0, 0, 0);
        return 0;
    }

    return m_pMediaNegoHandler->GetNegotiatedCodecBandwidthKhz(nNegoId, MEDIA_TYPE_AUDIO);
}

PUBLIC VIRTUAL void MediaSession::GetNegotiatedCodecBitrateRange(
        IN IMS_UINTP nNegoId, OUT IMS_FLOAT& nBitrateStart, OUT IMS_FLOAT& nBitrateEnd)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBitrateRange() - invalid MediaNegoHandler", 0, 0, 0);
        return;
    }
    m_pMediaNegoHandler->GetNegotiatedCodecBitrateRange(
            nNegoId, MEDIA_TYPE_AUDIO, nBitrateStart, nBitrateEnd);
}

PUBLIC VIRTUAL void MediaSession::GetNegotiatedCodecBandwidthRange(
        IN IMS_UINTP nNegoId, OUT IMS_FLOAT& nBandwidthStart, OUT IMS_FLOAT& nBandwidthEnd)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBandwidthRange() - invalid MediaNegoHandler", 0, 0, 0);
        return;
    }
    m_pMediaNegoHandler->GetNegotiatedCodecBandwidthRange(
            nNegoId, MEDIA_TYPE_AUDIO, nBandwidthStart, nBandwidthEnd);
}

PUBLIC VIRTUAL IMS_SINT32 MediaSession::GetRemotePort(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetRemotePort() - invalid MediaNegoHandler", 0, 0, 0);
        return -1;
    }

    return m_pMediaNegoHandler->GetRemotePort(nNegoId, eType);
}

PUBLIC VIRTUAL MEDIA_DIRECTION MediaSession::GetNegotiatedDirection(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetRemotePort() - invalid MediaNegoHandler", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    return m_pMediaNegoHandler->GetNegotiatedDirection(nNegoId, eType);
}

PUBLIC VIRTUAL void MediaSession::SetOptions(
        IN IMS_UINTP nNegoId, OptionType type, IN IMS_SINT32 param1, IN IMS_SINT32 param2)
{
    IMS_TRACE_I("SetOptions() - OptionType[%d], param1[%d], param2[%d]", type, param1, param2);

    switch (type)
    {
        case SET_RTP_PORT:
            if (m_pMediaNegoHandler != IMS_NULL)
            {
                m_pMediaNegoHandler->SetRtpPort(
                        nNegoId, static_cast<MEDIA_CONTENT_TYPE>(param1), param2);
            }
            break;
        case SET_CONFIRMED_SESSION:
            if (m_pMediaNegoHandler != IMS_NULL)
            {
                std::shared_ptr<MediaNego> pMediaNego = m_pMediaNegoHandler->FindMediaNego(nNegoId);

                if (pMediaNego != IMS_NULL)
                {
                    pMediaNego->SetPreviewMode(IMS_FALSE);
                }
            }

            if (m_pAudioController != IMS_NULL)
            {
                m_pAudioController->SetCallSessionState(param1);
            }

            if (m_pVideoController != IMS_NULL)
            {
                m_pVideoController->SetCallSessionState(param1);
            }

            m_bSessionConfirmed = (param1 > 0);
            IMS_TRACE_I("SetOptions() - Confirmed flag[%d]", m_bSessionConfirmed, 0, 0);
            break;
        case SET_CONFERENCE_ENABLE:
            if (m_pVideoController != IMS_NULL)
            {
                m_bIsConference = IMS_TRUE;
                m_pVideoController->ApplyQualityThreshold(m_bIsConference);
            }
            break;
        case SET_DIRECTION:
        case SEND_FAST_VIDEO_UPDATE:
            /** TODO: add implementation*/
        default:
            break;
    }
}

PUBLIC VIRTUAL void MediaSession::SetNetworkToneRtpTimer(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType, IN IMS_UINT32 nRtpTimer)
{
    IMS_TRACE_I("SetNetworkToneRtpTimer() - NegoId[%" PFLS_x "], Type[%d], nRtpTimer[%d]", nNegoId,
            eType, nRtpTimer);

    if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_AUDIO) && m_pAudioController != IMS_NULL)
    {
        m_pAudioController->SetNetworkToneTimer(nNegoId, nRtpTimer);
    }
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::NotifySrvccStatus(IN MEDIA_SRVCC_STATUS nStatus)
{
    IMS_TRACE_I("NotifySrvccStatus() - nStatus[%d]", nStatus, 0, 0);

    if (m_pAudioController == IMS_NULL)
    {
        IMS_TRACE_E(0, "NotifySrvccStatus() - invalid AudioController", 0, 0, 0);
        return IMS_FALSE;
    }

    switch (nStatus)
    {
        default:
        case MEDIA_SRVCC_IDLE:
            break;
        case MEDIA_SRVCC_STARTED:
            return m_pAudioController->UpdateMediaDirection(MEDIA_DIRECTION_INVALID);
        case MEDIA_SRVCC_SUCCEED:
            return m_pAudioController->CloseSession();
        case MEDIA_SRVCC_FAILED:
        case MEDIA_SRVCC_CANCELED:
            return m_pAudioController->UpdateMediaDirection(MEDIA_DIRECTION_INVALID, IMS_TRUE);
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    return OnMessage(nMsg, pParam);
}

PUBLIC
VIRTUAL void MediaSession::SetMediaPemType(IN IMS_UINTP nNegoId, IN MEDIA_PEM_TYPE ePemType)
{
    std::shared_ptr<MediaNego> pMediaNego = m_pMediaNegoHandler->FindMediaNego(nNegoId);

    // audio
    if (pMediaNego != IMS_NULL && pMediaNego->GetAudioNego() != IMS_NULL)
    {
        m_pAudioController->SetMediaPemType(nNegoId, ePemType);
    }

    // video
    if (pMediaNego != IMS_NULL && pMediaNego->GetVideoNego() != IMS_NULL)
    {
        m_pVideoController->SetMediaPemType(ePemType);
    }
}

IMS_BOOL MediaSession::IsPreviewMode(IMS_UINTP nNegoId)
{
    if (m_pMediaNegoHandler != IMS_NULL)
    {
        std::shared_ptr<MediaNego> pMediaNego = m_pMediaNegoHandler->FindMediaNego(nNegoId);

        if (pMediaNego != IMS_NULL)
        {
            return pMediaNego->IsPreviewMode();
        }
    }

    IMS_TRACE_E(0, "IsPreviewMode() - invalid negoId[%d]", nNegoId, 0, 0);
    return IMS_FALSE;
}

PROTECTED
QosRequestParam* MediaSession::FindQosParam(const QosRequestParam* targetParam)
{
    for (IMS_SINT32 nIndex = 0; nIndex < m_objListQosParams.GetSize(); nIndex++)
    {
        QosRequestParam* param = m_objListQosParams.GetAt(nIndex);

        if (param != IMS_NULL && *param == *targetParam)
        {
            return param;
        }
    }

    return IMS_NULL;
}

PROTECTED QosRequestParam* MediaSession::createQosParam(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eType)
{
    if (m_pMediaNegoHandler == IMS_NULL)
    {
        IMS_TRACE_E(0, "createQosParam() - MediaNegoHandler is null", 0, 0, 0);
        return IMS_NULL;
    }

    if (eType == MEDIA_TYPE_INVALID)
    {
        IMS_TRACE_E(0, "createQosParam() - invalid media type", 0, 0, 0);
        return IMS_NULL;
    }

    IMS_SINT32 nPort = m_pMediaNegoHandler->GetRemotePort(nNegoId, eType);

    IMS_TRACE_I(
            "createQosParam() - type[%s], Port[%d]", IJniMedia::PrintMediaType(eType), nPort, 0);
    return new QosRequestParam(
            eType, m_pMediaNegoHandler->GetNegotiatedRemoteAddress(nNegoId, eType), nPort);
}

PROTECTED void MediaSession::ClearQosParam()
{
    IMS_TRACE_D("ClearQosParam() - list size[%d]", m_objListQosParams.GetSize(), 0, 0);

    while (!m_objListQosParams.IsEmpty())
    {
        QosRequestParam* param = m_objListQosParams.GetAt(0);
        m_objListQosParams.RemoveAt(0);

        if (param != IMS_NULL)
        {
            delete param;
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL MediaSession::MediaSession_SendMsgToMediaManager(
        IN IMS_SINT32 nEvent, IN ImsMediaMsgParamBase* param)
{
    IMS_TRACE_D("MediaSession_SendMsgToMediaManager() : MediaType[%s], CallKey[%d] nEvent[%d]",
            IJniMedia::PrintMediaType(param->m_eMediaType), m_nCallKey, nEvent);

    MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);

    if (pMediaManager != IMS_NULL)
    {
        return pMediaManager->HandleRequestMsg(nEvent, m_nCallKey, param);
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL MediaSession::MediaSession_NotifyToClient(IMS_UINT32 eReportType,
        MEDIA_CONTENT_TYPE eType, MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType)
{
    IMS_TRACE_D("MediaSession_NotifyToClient() : ReportType[%d], MediaType[%d] ProtocolType[%d]",
            eReportType, eType, eMediaProtocolType);

    if (m_pClientListener != IMS_NULL)
    {
        m_pClientListener->MediaSession_Notify(eReportType, eType, eMediaProtocolType);
    }

    return IMS_TRUE;
}

PROTECTED IMS_BOOL MediaSession::CreateMediaConfig(IN MEDIA_SERVICE_TYPE eServiceType)
{
    IMS_TRACE_D("CreateMediaConfig()", 0, 0, 0);
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

PROTECTED
IMS_BOOL MediaSession::OnMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    IMS_TRACE_I(
            "OnMessage() - CallKey[%d], nMsg[%d, %s]", m_nCallKey, nMsg, IJniMedia::PrintMsg(nMsg));

    if (m_pClientListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "OnMessage() - null listener", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bRet = IMS_TRUE;

    switch (nMsg)
    {
        case IJniMedia::RESPONSE_OPEN_SESSION:
        case IJniMedia::RESPONSE_MODIFY_SESSION:
        case IJniMedia::RESPONSE_ADD_CONFIG:
        case IJniMedia::RESPONSE_CONFIRM_CONFIG:
            bRet = OnResponse(pParam);
            break;
        case IJniMedia::NOTIFY_FIRST_PACKET:
        case IJniMedia::NOTIFY_MEDIA_INACTIVITY:
        case IJniMedia::NOTIFY_PACKET_LOSS:
        case IJniMedia::NOTIFY_JITTER:
        case IJniMedia::NOTIFY_MEDIA_DETACH:
        case IJniMedia::NOTIFY_QOS_INFO:
        case IJniMedia::NOTIFY_VIDEO_BITRATE:
            bRet = OnNotify(nMsg, pParam);
            break;
        case IJniMedia::SEND_DTMF:
            bRet = OnSendDtmf(pParam);
            break;
        case IJniMedia::NOTIFY_CALL_QUALITY_CHANGE:
        case IJniMedia::NOTIFY_HEADER_EXTENSION:
            // TODO: add implementation
            break;
        case IJniMedia::SETSURFACE_CMD:
        case IJniMedia::SELECT_CAMERA_CMD:
        case IJniMedia::CHANGE_CAMERA_ZOOM_CMD:
        case IJniMedia::SET_PAUSE_IMAGE_CMD:
        case IJniMedia::CHANGE_ORIENTATION_CMD:
            bRet = m_pVideoController->SendMessage(nMsg, pParam);
            break;
        case IJniMedia::NOTIFY_ANBR_RECEIVED:
            bRet = OnNotifyAnbrReceived(pParam);
            break;
        default:
            break;
    }

    return bRet;
}

PROTECTED
IMS_BOOL MediaSession::OnResponse(IN IMS_UINTP nParam)
{
    ImsMediaResponseConfigParam* pParam = reinterpret_cast<ImsMediaResponseConfigParam*>(nParam);

    if (pParam != IMS_NULL)
    {
        IMS_TRACE_I(
                "OnResponse() - Type[%d], eResult[%d]", pParam->m_eMediaType, pParam->m_eResult, 0);
        ReportToClient(pParam->m_eResult, pParam->m_eMediaType);
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotify(IN IMS_SINT32 nMsg, IN IMS_UINTP nParam)
{
    if (m_pClientListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    switch (nMsg)
    {
        case IJniMedia::NOTIFY_FIRST_PACKET:
        {
            ImsMediaResponseConfigParam* pParam =
                    reinterpret_cast<ImsMediaResponseConfigParam*>(nParam);

            if (pParam != IMS_NULL)
            {
                IMS_TRACE_I("OnNotify() - Type[%d]", pParam->m_eMediaType, 0, 0);
                m_pClientListener->MediaSession_Notify(
                        REPORT_DATA_RECEIVE_STARTED, pParam->m_eMediaType);

                if (MEDIA_IS_CONTAINED_THIS_TYPE(pParam->m_eMediaType, MEDIA_TYPE_AUDIO))
                {
                    if (m_pAudioController->GetInactivityTimer(
                                NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID) > 0)
                    {
                        m_pAudioController->SetNetworkToneTimer(UNDEFINED_NEGO_ID, 0);
                        m_pClientListener->MediaSession_Notify(
                                REPORT_NW_TONE_RTP_RECEIVE_STARTED, pParam->m_eMediaType);
                    }
                }

                return IMS_TRUE;
            }
        }
        break;
        case IJniMedia::NOTIFY_MEDIA_INACTIVITY:
        {
            IMS_TRACE_I("OnNotify() - media Inactivity timer", 0, 0, 0);
            return HandleNotifyMediaInactivity(nParam);
        }
        case IJniMedia::NOTIFY_PACKET_LOSS:
        case IJniMedia::NOTIFY_JITTER:
            /** TODO: add implementation */
            break;
        case IJniMedia::NOTIFY_MEDIA_DETACH:
            m_pClientListener->MediaSession_Notify(REPORT_MEDIA_DETACH);
            break;
        case IJniMedia::NOTIFY_QOS_INFO:
        {
            const ImsMediaMsgQosParam* pParam = reinterpret_cast<ImsMediaMsgQosParam*>(nParam);

            if (pParam != IMS_NULL)
            {
                IpAddress objIpAddress = pParam->m_objIpAddress;
                IMS_SINT32 nPort = pParam->m_nPort;
                IMS_BOOL bResult = pParam->m_bResult;

                IMS_TRACE_I("OnNotify() - QOS - CallKey[%d], nPort[%d], bResult[%d]", m_nCallKey,
                        nPort, bResult);

                QosRequestParam tempParam(pParam->m_eMediaType, objIpAddress, nPort);
                QosRequestParam* pMatchedParam = FindQosParam(&tempParam);

                if (pMatchedParam != IMS_NULL)
                {
                    pMatchedParam->m_bResult = bResult;

                    for (const auto& negoId : pMatchedParam->m_objListNegoId)
                    {
                        m_pClientListener->MediaSession_NotifyQos(
                                negoId, bResult, pMatchedParam->m_eMediaType);
                    }
                }

                return IMS_TRUE;
            }
        }
        break;
        case IJniMedia::NOTIFY_VIDEO_BITRATE:
        {
            const ImsMediaVideoParam* pParam = reinterpret_cast<ImsMediaVideoParam*>(nParam);

            if (pParam != IMS_NULL)
            {
                if (pParam->nValue > 0)
                {
                    m_pClientListener->MediaSession_Notify(REPORT_VIDEO_LOWEST_BITRATE);
                }

                return IMS_TRUE;
            }
        }
        break;
        default:
            IMS_TRACE_I("OnNotify() - unhandled case", 0, 0, 0);
            break;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MediaSession::OnSendDtmf(IN IMS_UINTP nParam)
{
    const ImsMediaMsgDtmfParam* pParam = reinterpret_cast<ImsMediaMsgDtmfParam*>(nParam);

    if (pParam != IMS_NULL)
    {
        m_pAudioController->SendDtmf(pParam->m_dtmfCode);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotifyAnbrReceived(IN IMS_UINTP nParam)
{
    const ImsMediaMsgAnbrReceivedParam* pParam =
            reinterpret_cast<ImsMediaMsgAnbrReceivedParam*>(nParam);

    if (pParam != IMS_NULL)
    {
        if (pParam->m_nAnbrMediaType == MEDIA_TYPE_AUDIO)
        {
            m_pAudioController->NotifyAnbrReceived(
                    pParam->m_nAnbrMediaType, pParam->m_nAnbrDirection, pParam->m_nAnbrBitrate);
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
void MediaSession::ReportToClient(IN IMS_SINT32 eError, IN MEDIA_CONTENT_TYPE eType)
{
    if (m_pClientListener != IMS_NULL)
    {
        if (eError == RtpError::NO_ERROR)
        {
            m_pClientListener->MediaSession_Notify(REPORT_SUCCESS, eType);
        }
        else if (eError > RtpError::NO_ERROR)
        {
            m_pClientListener->MediaSession_NotifyFailures(REPORT_FAILURE, eError, eType);
        }
    }
}

PRIVATE
IMS_BOOL MediaSession::HandleNotifyMediaInactivity(IN IMS_UINTP nParam)
{
    const ImsMediaMsgParamBase* pTempParam = reinterpret_cast<ImsMediaMsgParamBase*>(nParam);

    if (pTempParam != IMS_NULL)
    {
        if (pTempParam->m_eMediaType == MEDIA_TYPE_AUDIO)
        {
            ImsMediaNotifyQualityStatusParam* pParam =
                    reinterpret_cast<ImsMediaNotifyQualityStatusParam*>(nParam);
            IMS_SINT32 nLocalNetworkToneTimer = m_pAudioController->GetInactivityTimer(
                    NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID);
            IMS_SINT32 nLocalRtpTimer =
                    m_pAudioController->GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID);
            IMS_SINT32 nLocalRtcpTimer =
                    m_pAudioController->GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID);

            IMS_TRACE_I(
                    "HandleNotifyMediaInactivity() - LocalNetworkToneTimer[%d], LocalRtpTimer[%d], "
                    "LocalRtcpTimer[%d]",
                    nLocalNetworkToneTimer, nLocalRtpTimer, nLocalRtcpTimer);
            IMS_TRACE_I("HandleNotifyMediaInactivity() - rtp inactivity[%d], rtcp "
                        "inactivity[%d]",
                    pParam->m_nRtpInactivityTimerMillis, pParam->m_nRtcpInactivityTimerMillis, 0);

            if (nLocalNetworkToneTimer > 0)
            {
                if (IsInactivityTimerExpired(
                            pParam->m_nRtpInactivityTimerMillis, nLocalNetworkToneTimer))
                {
                    IMS_TRACE_I(
                            "HandleNotifyMediaInactivity() - Notify netwok tone timeout", 0, 0, 0);
                    m_pAudioController->SetNetworkToneTimer(UNDEFINED_NEGO_ID, 0);
                    m_pClientListener->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_FAILED,
                            pParam->m_eMediaType, MEDIA_PROTOCOL_RTP);
                    return IMS_TRUE;
                }
            }
            else if ((nLocalRtpTimer > 0 && nLocalRtcpTimer > 0) &&
                    m_pAudioController->GetMediaDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
            {
                if (IsInactivityTimerExpired(pParam->m_nRtpInactivityTimerMillis, nLocalRtpTimer))
                {
                    IMS_TRACE_I("HandleNotifyMediaInactivity() - Notify rtp inactivity timeout in "
                                "the active call state",
                            0, 0, 0);
                    m_pClientListener->MediaSession_Notify(
                            REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType, MEDIA_PROTOCOL_RTP);
                    return IMS_TRUE;
                }
                else
                {
                    IMS_TRACE_I("HandleNotifyMediaInactivity() - No notifications in the active "
                                "call state",
                            0, 0, 0);
                }
            }
            else
            {
                if (nLocalRtpTimer > 0)
                {
                    if (IsInactivityTimerExpired(
                                pParam->m_nRtpInactivityTimerMillis, nLocalRtpTimer))
                    {
                        IMS_TRACE_I("HandleNotifyMediaInactivity() - Notify rtp inactivity timeout",
                                0, 0, 0);
                        m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED,
                                pParam->m_eMediaType, MEDIA_PROTOCOL_RTP);
                        return IMS_TRUE;
                    }
                }

                if (nLocalRtcpTimer > 0)
                {
                    if (IsInactivityTimerExpired(
                                pParam->m_nRtcpInactivityTimerMillis, nLocalRtcpTimer))
                    {
                        IMS_TRACE_I(
                                "HandleNotifyMediaInactivity() - Notify rtcp inactivity timeout", 0,
                                0, 0);
                        m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED,
                                pParam->m_eMediaType, MEDIA_PROTOCOL_RTCP);
                        return IMS_TRUE;
                    }
                }

                IMS_TRACE_I("HandleNotifyMediaInactivity() - No notifications for rtp/rtcp "
                            "inactivity timer",
                        0, 0, 0);
            }
        }
        else
        {
            const ImsMediaNotifyInactivityParam* pParam =
                    reinterpret_cast<ImsMediaNotifyInactivityParam*>(nParam);
            m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType,
                    pParam->m_eMediaProtocolType == RTP ? MEDIA_PROTOCOL_RTP : MEDIA_PROTOCOL_RTCP);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MediaSession::IsInactivityTimerExpired(
        IN IMS_SINT32 nRunningTimerValue, IN IMS_SINT32 nTimerValue)
{
    if (nRunningTimerValue > 0 && nTimerValue > 0 && nRunningTimerValue >= nTimerValue)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void MediaSession::CreateMediaSessions(IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_D("CreateMediaSessions() - NegoId[%" PFLS_x "], MediaType[%d]", nNegoID, eType, 0);

    if (m_pEnvironment == IMS_NULL || eType == MEDIA_TYPE_INVALID)
    {
        IMS_TRACE_E(0, "CreateMediaSessions() - invalid param", 0, 0, 0);
    }

    if (m_pAudioController != IMS_NULL && eType & MEDIA_TYPE_AUDIO)
    {
        m_pAudioController->CreateSession(this, nNegoID,
                MediaConfigUtil::GetAudioConfig(m_nSlotId, m_pEnvironment->eServiceType),
                m_pEnvironment->eServiceType);
    }

    if (m_pVideoController != IMS_NULL && eType & MEDIA_TYPE_VIDEO)
    {
        m_pVideoController->CreateSession(
                this, MediaConfigUtil::GetVideoConfig(m_nSlotId, m_pEnvironment->eServiceType));
    }

    if (m_pTextController != IMS_NULL && eType & MEDIA_TYPE_TEXT)
    {
        m_pTextController->CreateSession(
                this, MediaConfigUtil::GetTextConfig(m_nSlotId, m_pEnvironment->eServiceType));
    }
}

PRIVATE
void MediaSession::OpenMediaSessions(
        IN IMS_UINTP nNegoId, IN std::shared_ptr<MediaNego> pMediaNego, MEDIA_CONTENT_TYPE eType)
{
    if (pMediaNego == IMS_NULL || eType == MEDIA_TYPE_INVALID)
    {
        IMS_TRACE_E(0, "OpenMediaSessions() - invalid argument", 0, 0, 0);
        return;
    }

    // audio
    if (eType & MEDIA_TYPE_AUDIO && m_pAudioController != IMS_NULL &&
            !m_pAudioController->IsSessionOpened())
    {
        m_pAudioController->UpdateLocalAddress(pMediaNego->GetAudioNego());

        if (!m_pAudioController->OpenSession(nNegoId))
        {
            IMS_TRACE_I("OpenMediaSessions() - Audio OpenSession Failed", 0, 0, 0);
        }
    }

    // video
    if (eType & MEDIA_TYPE_VIDEO && m_pVideoController != IMS_NULL &&
            !m_pVideoController->IsSessionOpened())
    {
        m_pVideoController->CreateSession(
                this, MediaConfigUtil::GetVideoConfig(m_nSlotId, m_pEnvironment->eServiceType));
        m_pVideoController->UpdateLocalAddress(pMediaNego->GetVideoNego());

        if (!m_pVideoController->OpenSession())
        {
            IMS_TRACE_E(0, "OpenMediaSessions() - Video OpenSession Failed", 0, 0, 0);
        }

        m_pVideoController->SetMtu(GetRtpFragmentSize());
    }

    // text
    if (eType & MEDIA_TYPE_TEXT && m_pTextController != IMS_NULL &&
            !m_pTextController->IsSessionOpened())
    {
        m_pTextController->CreateSession(
                this, MediaConfigUtil::GetTextConfig(m_nSlotId, m_pEnvironment->eServiceType));
        m_pTextController->UpdateLocalAddress(pMediaNego->GetTextNego());

        if (!m_pTextController->OpenSession())
        {
            IMS_TRACE_E(0, "OpenMediaSessions() - Text OpenSession Failed", 0, 0, 0);
        }
    }
}

PRIVATE
void MediaSession::UpdateMediaSessions(
        IN IMS_UINTP nNegoId, IN std::shared_ptr<MediaNego> pMediaNego, MEDIA_CONTENT_TYPE eType)
{
    if (pMediaNego == IMS_NULL || eType == MEDIA_TYPE_INVALID)
    {
        IMS_TRACE_E(0, "UpdateMediaSessions() - invalid argument", 0, 0, 0);
        return;
    }

    // set Access Network
    IMS_UINT32 nAccessNetwork = m_nCurrentAccessNetwork;
    IMS_TRACE_D("UpdateMediaSessions() - CurrentAccessNetwork[%d]", m_nCurrentAccessNetwork, 0, 0);

    // Update Audio Session
    if (eType & MEDIA_TYPE_AUDIO && m_pAudioController != IMS_NULL &&
            !m_pAudioController->UpdateSession(nNegoId, nAccessNetwork, pMediaNego->GetAudioNego()))
    {
        IMS_TRACE_E(0, "UpdateMediaSessions() - fail to update audio", 0, 0, 0);
    }

    // Update Video Session
    if (eType & MEDIA_TYPE_VIDEO && m_pVideoController != IMS_NULL &&
            m_pVideoController->IsSessionOpened())
    {
        m_pVideoController->UpdateRtpConfig(pMediaNego->GetVideoNego(),
                pMediaNego->GetAudioNego()->GetNegotiatedDirection() !=
                        MEDIA_DIRECTION_SEND_RECEIVE);
        m_pVideoController->UpdateAccessNetwork(nAccessNetwork);

        if (m_pVideoController->UpdateSession())
        {
            m_pVideoController->ApplyQualityThreshold(m_bIsConference);
        }
    }

    // Update Text Session
    if (eType & MEDIA_TYPE_TEXT && m_pTextController != IMS_NULL &&
            m_pTextController->IsSessionOpened() && m_bSessionConfirmed)
    {
        m_pTextController->UpdateRtpConfig(pMediaNego->GetTextNego());
        m_pTextController->UpdateAccessNetwork(nAccessNetwork);

        if (m_pTextController->UpdateSession())
        {
            m_pTextController->ApplyQualityThreshold();
        }
    }
}

PRIVATE
void MediaSession::CloseMediaSessions(MEDIA_CONTENT_TYPE eType)
{
    if (eType & MEDIA_TYPE_AUDIO && m_pAudioController != IMS_NULL &&
            m_pAudioController->IsSessionOpened())
    {
        if (!m_pAudioController->CloseSession())
        {
            IMS_TRACE_E(0, "CloseMediaSessions() - failed to close audio", 0, 0, 0);
        }
    }

    if (eType & MEDIA_TYPE_VIDEO && m_pVideoController != IMS_NULL &&
            m_pVideoController->IsSessionOpened())
    {
        if (!m_pVideoController->CloseSession())
        {
            IMS_TRACE_E(0, "CloseMediaSessions() - failed to close video", 0, 0, 0);
        }
    }

    if (eType & MEDIA_TYPE_TEXT && m_pTextController != IMS_NULL &&
            m_pTextController->IsSessionOpened())
    {
        if (!m_pTextController->CloseSession())
        {
            IMS_TRACE_E(0, "CloseMediaSessions() - failed to close text", 0, 0, 0);
        }
    }
}

PRIVATE
IMS_SINT32 MediaSession::GetRtpFragmentSize()
{
    IMS_SINT32 nMtu = 0;
    IMS_BOOL bIsIpv6 =
            m_pEnvironment != IMS_NULL && m_pEnvironment->pIService->GetIpAddress().IsIpv6Address()
            ? IMS_TRUE
            : IMS_FALSE;

    if (m_pNetworkConnectionWatcher != IMS_NULL)
    {
        nMtu = m_pNetworkConnectionWatcher->GetMtu();
    }

    if (nMtu == 0)
    {
        nMtu = (GetNetworkType() == MediaNetworkConnectionWatcher::IWLAN) ? MTU_EPDG : MTU_MOBILE;
    }

    nMtu -= SIZE_OF_IP_SEC;
    nMtu -= bIsIpv6 ? SIZE_OF_IPV6 : SIZE_OF_IPV4;
    nMtu -= SIZE_OF_RTP;

    IMS_TRACE_D("GetRtpFragmentSize() - mtu[%d], IsIpv6[%d], network type[%d]", nMtu, bIsIpv6,
            GetNetworkType());

    return nMtu;
}
