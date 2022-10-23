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

#include "Configuration.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaConfigUtil.h"
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
    m_objTextController.CloseSession();

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

PUBLIC VIRTUAL void MediaSession::SetMtcListener(
        IN IMediaSessionClientListener* piMediaSessionListener)
{
    m_pClientListener = piMediaSessionListener;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::SetEnvironment(IN MediaEnvironment* pEnvironment)
{
    if (pEnvironment == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("SetEnvironment() - CallKey[%d], eServiceType[%d]", m_nCallKey,
            pEnvironment->eServiceType, 0);

    // update pdn
    MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);

    if (pMediaManager != IMS_NULL && pEnvironment->pIService != IMS_NULL)
    {
        if (pMediaManager->GetResourceManager()->UpdatePdn(
                    pEnvironment->eServiceType == MEDIA_SERVICE_EMERGENCY
                            ? MediaResourceManager::PDN_EMERGENCY
                            : MediaResourceManager::PDN_IMS,
                    pEnvironment->pIService->GetIpAddress()) == IMS_FALSE)
        {
            return IMS_FALSE;
        }
    }

    if (m_pEnvironment != IMS_NULL)
    {
        delete m_pEnvironment;
    }

    m_pEnvironment = pEnvironment;
    return IMS_TRUE;
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

    if (pAudioNego != IMS_NULL)
    {
        m_objAudioController.CreateSession(this, nMediaNego,
                MediaConfigUtil::GetAudioConfig(m_nSlotId, m_pEnvironment->eServiceType));
    }

    VideoNego* pVideoNego = pMediaNego->GetVideoNego();

    if (pVideoNego != IMS_NULL && MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO))
    {
        m_objVideoController.CreateSession(
                this, MediaConfigUtil::GetVideoConfig(m_nSlotId, m_pEnvironment->eServiceType));
    }

    TextNego* pTextNego = pMediaNego->GetTextNego();

    if (pTextNego != IMS_NULL && MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT))
    {
        m_objTextController.CreateSession(
                this, MediaConfigUtil::GetTextConfig(m_nSlotId, m_pEnvironment->eServiceType));
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
        IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_SINT32 nAudioDirection,
        IN IMS_SINT32 nVideoDirection, IN IMS_SINT32 nTextDirection,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormSDP() - nNegoId[%" PFLS_x "], pSession[%" PFLS_x "], eMediaType[%d]", nNegoId,
            pSession, eMediaType);
    IMS_TRACE_I("FormSDP() - DIR = Audio[%d], Video[%d], Text[%d]", nAudioDirection,
            nVideoDirection, nTextDirection);
    IMS_TRACE_D("FormSDP() - eMediaType [%d], EnforceReofferMode[%d]", eMediaType,
            bEnforceReofferMode, 0);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormSDP() - Can't find nNegoId[%d]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    if (pMediaNego->FormSDP(pSession, eMediaType, nAudioDirection, nVideoDirection, nTextDirection,
                bEnforceReofferMode) == IMS_FALSE)
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
    if (pMediaNego->GetVideoNego() != IMS_NULL && IS_VALID_MEDIA_DIRECTION(nVideoDirection))
    {
        m_objVideoController.CreateSession(
                this, MediaConfigUtil::GetVideoConfig(m_nSlotId, m_pEnvironment->eServiceType));
        m_objVideoController.UpdateLocalAddress(pMediaNego->GetVideoNego());
        m_objVideoController.OpenSession();
    }

    // text
    if (pMediaNego->GetTextNego() != IMS_NULL && IS_VALID_MEDIA_DIRECTION(nTextDirection))
    {
        m_objTextController.CreateSession(
                this, MediaConfigUtil::GetTextConfig(m_nSlotId, m_pEnvironment->eServiceType));
        m_objTextController.UpdateLocalAddress(pMediaNego->GetTextNego());
        m_objTextController.OpenSession();
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::NegotiateSDP(IN IMS_UINTP nNegoId, IN ISession* pSession,
        OUT IMS_SINT32* nAudioDirection, OUT IMS_SINT32* nVideoDirection,
        OUT IMS_SINT32* nTextDirection, OUT MediaNego::MediaNegoResult& errorReason)
{
    IMS_TRACE_I(
            "NegotiateSDP() - nNegoId[%" PFLS_x "], pSession[%" PFLS_x "]", nNegoId, pSession, 0);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateSDP() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    if (pMediaNego->NegotiateSDP(pSession, *nAudioDirection, *nVideoDirection, *nTextDirection,
                errorReason) == IMS_TRUE)
    {
        IMS_TRACE_I("NegotiateSDP() - DIR = Audio[%d], Video[%d], Text[%d]", *nAudioDirection,
                *nVideoDirection, *nTextDirection);

        // set Access Network
        MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);

        IMS_SINT32 nAccessNetwork = 0;

        if (pMediaManager != IMS_NULL)
        {
            nAccessNetwork = pMediaManager->GetResourceManager()->GetNetworkType();
        }

        // audio
        if (pMediaNego->GetAudioNego() != IMS_NULL)
        {
            m_objAudioController.AddSession(nNegoId, nAccessNetwork, pMediaNego->GetAudioNego());
        }

        // video
        if (pMediaNego->GetVideoNego() != IMS_NULL && IS_VALID_MEDIA_DIRECTION(*nVideoDirection))
        {
            m_objVideoController.CreateSession(
                    this, MediaConfigUtil::GetVideoConfig(m_nSlotId, m_pEnvironment->eServiceType));
            m_objVideoController.UpdateLocalAddress(pMediaNego->GetVideoNego());
            m_objVideoController.OpenSession();
        }

        // text
        if (pMediaNego->GetTextNego() != IMS_NULL && IS_VALID_MEDIA_DIRECTION(*nTextDirection))
        {
            m_objTextController.CreateSession(
                    this, MediaConfigUtil::GetTextConfig(m_nSlotId, m_pEnvironment->eServiceType));
            m_objTextController.UpdateLocalAddress(pMediaNego->GetTextNego());
            m_objTextController.OpenSession();
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::RequestQos(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType)
{
    IMS_TRACE_I("RequestQos() - nNegoId[%" PFLS_x "], eMediaType[%d]", nNegoId, eMediaType, 0);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "RequestQos() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_AUDIO))
    {
        /** TODO: add implementation */
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO))
    {
        /** TODO: add implementation */
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT))
    {
        /** TODO: add implementation */
    }

    return IMS_TRUE;
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
    IMS_TRACE_D("Run() - nNegoId[%" PFLS_x "]", nNegoId, 0, 0);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "Run() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    // set Access Network
    MediaManager* pMediaManager = MediaManager::GetInstance(m_nSlotId);

    IMS_SINT32 nAccessNetwork = 0;

    if (pMediaManager != IMS_NULL)
    {
        nAccessNetwork = pMediaManager->GetResourceManager()->GetNetworkType();
    }

    m_objAudioController.UpdateSession(nNegoId, nAccessNetwork, pMediaNego->GetAudioNego());

    if (m_objVideoController.IsSessionOpened() == IMS_TRUE)
    {
        m_objVideoController.UpdateRtpConfig(pMediaNego->GetVideoNego());
        m_objVideoController.UpdateAccessNetwork(nAccessNetwork);
        m_objVideoController.UpdateQualityThreshold(pMediaNego->GetVideoNego());
        m_objVideoController.UpdateSession();
    }

    m_objTextController.UpdateRtpConfig(pMediaNego->GetTextNego());
    m_objTextController.UpdateAccessNetwork(nAccessNetwork);
    m_objTextController.UpdateQualityThreshold(pMediaNego->GetTextNego());
    m_objTextController.UpdateSession();

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnChangeNetworkConnection(IN IMS_UINT32 nAccessNetwork)
{
    m_objAudioController.UpdateAccessNetwork(nAccessNetwork);

    if (m_objVideoController.IsSessionOpened() == IMS_TRUE)
    {
        m_objVideoController.UpdateAccessNetwork(nAccessNetwork);
        m_objVideoController.UpdateSession();
    }

    m_objTextController.UpdateAccessNetwork(nAccessNetwork);
    m_objTextController.UpdateSession();

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
    m_objTextController.CloseSession();
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

    if (pMediaNego->GetNegotiatedTextQuality() != TEXT_CODEC_NOT_USED)
    {
        eMedia = (MEDIA_CONTENT_TYPE)(eMedia | MEDIA_TYPE_TEXT);
    }

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
        case MEDIA_TYPE_TEXT:
            return (IMS_SINT32)(pMediaNego->GetNegotiatedTextQuality());
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
        VideoNego* pVideoNego = pMediaNego->GetVideoNego();

        if (pVideoNego == IMS_NULL)
        {
            return (IMS_SINT32)VIDEO_RESOLUTION_INVALID;
        }

        return (IMS_SINT32)pVideoNego->GetNegotiatedResolution();
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_TEXT))
    {
        return 1000;
    }

    return 0;
}

/* TODO: add implementation
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
        case MEDIA_TYPE_TEXT:
        {
            TextNego* pTextNego = pMediaNego->GetTextNego();
            if (pTextNego != IMS_NULL)
            {
                return pTextNego->GetNegotiatedDirection();
            }
        }
        break;
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

    switch (type)
    {
        case SET_RTP_PORT:
            pMediaNego = FindMediaNego(nNegoId);
            if (pMediaNego != IMS_NULL)
            {
                if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO)
                {
                    AudioNego* pAudioNego = pMediaNego->GetAudioNego();

                    if (pAudioNego != IMS_NULL)
                    {
                        pAudioNego->SetPort(param2);
                    }
                }
                else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_VIDEO)
                {
                    VideoNego* pVideoNego = pMediaNego->GetVideoNego();

                    if (pVideoNego != IMS_NULL)
                    {
                        pVideoNego->SetPort(param2);
                    }
                }
                else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_TEXT)
                {
                    TextNego* pTextNego = pMediaNego->GetTextNego();

                    if (pTextNego != IMS_NULL)
                    {
                        pTextNego->SetPort(param2);
                    }
                }
            }
            break;
        case SET_CONFIRMED_SESSION:
            m_objAudioController.SetConfirmSession(param1);
            break;
        case SET_DIRECTION:
        case SET_CONFERENCE_ENABLE:
        case SEND_FAST_VIDEO_UPDATE:
            /** TODO: add implementation*/
            break;
        default:
            break;
    }
}

PUBLIC VIRTUAL void MediaSession::SetNetworkToneRTPTimer(
        MEDIA_CONTENT_TYPE eMediaType, IN IMS_UINT32 nRtpTimer)
{
    (void)eMediaType; /** TODO do it later */
    IMS_TRACE_I("SetNetworkToneRTPTimer() - CallKey[%d], eMediaType[%d], nRtpTimer[%d]", m_nCallKey,
            eMediaType, nRtpTimer);

    m_nRtpTimer = nRtpTimer;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::NotifySrvccStatus(IN MEDIA_SRVCC_STATUS nStatus)
{
    IMS_TRACE_I("NotifySrvccStatus() - nStatus[%d]", nStatus, 0, 0);

    switch (nStatus)
    {
        default:
        case MEDIA_SRVCC_IDLE:
            break;
        case MEDIA_SRVCC_STARTED:
            return m_objAudioController.UpdateMediaDirection(MEDIA_DIRECTION_INVALID);
        case MEDIA_SRVCC_SUCCEED:
            return m_objAudioController.CloseSession();
        case MEDIA_SRVCC_FAILED:
        case MEDIA_SRVCC_CANCELED:
            return m_objAudioController.UpdateMediaDirection(MEDIA_DIRECTION_INVALID, IMS_TRUE);
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL MediaSession::SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    return OnMessage(nMsg, pParam);
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

    pMediaNego->CreateProfile(m_pEnvironment);

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
        IMS_TRACE_E(0, "DeleteMediaNego() - invalid nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
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

PROTECTED
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

PROTECTED
IMS_BOOL MediaSession::OnMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam)
{
    IMS_TRACE_I(
            "OnMessage() - CallKey[%d], nMsg[%d, %s]", m_nCallKey, nMsg, IMMedia::PrintMsg(nMsg));

    if (m_pClientListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "OnMessage() - null listener", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bRet = IMS_TRUE;

    switch (nMsg)
    {
        case IMMedia::RESPONSE_OPEN_SESSION:
        case IMMedia::RESPONSE_MODIFY_SESSION:
        case IMMedia::RESPONSE_ADD_CONFIG:
        case IMMedia::RESPONSE_CONFIRM_CONFIG:
            bRet = OnResponse(pParam);
            break;
        case IMMedia::NOTIFY_FIRST_PACKET:
        case IMMedia::NOTIFY_MEDIA_INACTIVITY:
        case IMMedia::NOTIFY_PACKET_LOSS:
        case IMMedia::NOTIFY_JITTER:
        case IMMedia::NOTIFY_MEDIA_DETACH:
        case IMMedia::NOTIFY_QOS_INFO:
            bRet = OnNotify(nMsg, pParam);
            break;
        case IMMedia::SEND_DTMF:
            bRet = OnSendDtmf(pParam);
            break;
        case IMMedia::NOTIFY_CALL_QUALITY_CHANGE:
        case IMMedia::NOTIFY_HEADER_EXTENSION:
            // TODO: add implementation
            break;
        case IMMedia::SETSURFACE_CMD:
        case IMMedia::SELECT_CAMERA_CMD:
        case IMMedia::CHANGE_CAMERA_ZOOM_CMD:
        case IMMedia::SET_PAUSE_IMAGE_CMD:
        case IMMedia::CHANGE_ORIENTATION_CMD:
            bRet = m_objVideoController.SendMessage(nMsg, pParam);
            break;
        case IMMedia::CHANGE_NETWORK_CONNECTION:
            bRet = OnChangeNetworkConnection(pParam);
            break;
        case IMMedia::CHANGE_MTU:
            /** TODO: add implementation */
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
        IMS_TRACE_I("OnResponse() - eMediaType[%d], eResult[%d]", pParam->m_eMediaType,
                pParam->m_eResult, 0);
        ReportToClient(pParam->m_eResult, pParam->m_eMediaType);
        delete pParam;
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
        case IMMedia::NOTIFY_FIRST_PACKET:
        {
            ImsMediaResponseConfigParam* pParam =
                    reinterpret_cast<ImsMediaResponseConfigParam*>(nParam);

            if (pParam != IMS_NULL)
            {
                IMS_TRACE_I("OnNotify() - eMediaType[%d]", pParam->m_eMediaType, 0, 0);
                m_pClientListener->MediaSession_Notify(
                        REPORT_DATA_RECEIVE_STARTED, pParam->m_eMediaType);
                delete pParam;
                return IMS_TRUE;
            }
        }
        break;
        case IMMedia::NOTIFY_MEDIA_INACTIVITY:
        {
            ImsMediaNotifyInactivityParam* pParam =
                    reinterpret_cast<ImsMediaNotifyInactivityParam*>(nParam);

            if (pParam != IMS_NULL)
            {
                IMS_TRACE_I("OnNotify() - eMediaProtocolType[%d], eMediaType[%d]",
                        pParam->m_eMediaType, pParam->m_eMediaProtocolType, 0);
                m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED,
                        pParam->m_eMediaType,
                        pParam->m_eMediaProtocolType == RTP ? MEDIA_PROTOCOL_RTP
                                                            : MEDIA_PROTOCOL_RTCP);

                delete pParam;
                return IMS_TRUE;
            }
        }
        break;
        case IMMedia::NOTIFY_PACKET_LOSS:
        case IMMedia::NOTIFY_JITTER:
            /** TODO: add implementation */
            break;
        case IMMedia::NOTIFY_MEDIA_DETACH:
            m_pClientListener->MediaSession_Notify(REPORT_MEDIA_DETACH);
            break;
        case IMMedia::NOTIFY_QOS_INFO:
        {
            ImsMediaNotifyQosParam* pParam = reinterpret_cast<ImsMediaNotifyQosParam*>(nParam);

            if (pParam != IMS_NULL)
            {
                IPAddress objIpAddress = pParam->m_objIpAddr;
                IMS_SINT32 nPort = pParam->m_nPort;
                IMS_BOOL bResult = pParam->m_bResult;

                IMS_TRACE_I("OnNotify() QOS - CallKey[%d], nPort[%d], bResult[%d]", m_nCallKey,
                        nPort, bResult);

                ImsMediaBasicSessionInfoParam* pBasicSessionInfo =
                        GetBasicSessionInfofromRemoteArress(objIpAddress.ToString(), nPort);

                if (pBasicSessionInfo == IMS_NULL)
                {
                    delete pParam;
                    return IMS_FALSE;
                }

                m_pClientListener->MediaSession_NotifyQos(
                        pBasicSessionInfo->m_nNegoId, bResult, pParam->m_eMediaType);
                delete pBasicSessionInfo;
                delete pParam;
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
    ImsMediaMsgDtmfParam* pParam = reinterpret_cast<ImsMediaMsgDtmfParam*>(nParam);

    if (pParam != IMS_NULL)
    {
        m_objAudioController.SendDtmf(pParam->m_dtmfCode);
        delete pParam;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
void MediaSession::ReportToClient(IN IMS_SINT32 eError, IN MEDIA_CONTENT_TYPE eMediaType)
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

PRIVATE IpAddress MediaSession::GetAndroidIP()
{
    if (m_pEnvironment != IMS_NULL && m_pEnvironment->pIService != IMS_NULL)
    {
        return m_pEnvironment->pIService->GetIpAddress();
    }

    return IpAddress();
}
