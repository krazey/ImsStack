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
#include <AudioConfig.h>
#include "ServiceMessage.h"
#include "ServiceTrace.h"
#include "IMSCore.h"
#include "ICoreService.h"
#include "ISessionDescriptor.h"
#include "IConfigBuffer.h"
#include "Configuration.h"
#include "MediaDef.h"
#include "EnablerUtils.h"
#include "MediaSession.h"
#include "MediaManager.h"
#include "config/MediaSessionConfigFactory.h"

// == DEFINES =========================================================
__IMS_TRACE_TAG_USER_DECL__("MED.SS");

using namespace android::telephony::imsmedia;

// == Constructor, Destructor, Operator Overloading ========================================
PROTECTED
MediaSession::MediaSession(IN MEDIA_SERVICE_TYPE eServiceType,
        IMS_SINTP nCallKey, IN IMS_UINT32 nSlotId) :
        m_aStrTargetMediaManager(AString::ConstNull()),
        m_nSlotId(nSlotId),
        m_nCallKey(nCallKey),
        m_pClientListener(IMS_NULL),
        m_pEnvironment(IMS_NULL),
        m_nCommandBuffer(IMMedia::MEDIA_MESSAGE_IDX_START),
        m_eSessionState(EARLY_SESSION),
        m_objMapMediaNego(IMSMap<IMS_UINTP, MediaNego*>()),
        m_listAudioSession(IMSList<AudioMediaSession*>()),
        m_nRtpTimer(0),
        m_listMediaSessionTypeNode(IMSList<MediaSessionTypeNode*>())
{
    IMS_TRACE_D("+MediaSession() - ServiceType[%" PFLS_u "], CallKey[%" PFLS_u "]",
        eServiceType, nCallKey, 0);

    CreateMediaConfig(eServiceType);
}

PUBLIC VIRTUAL
MediaSession::~MediaSession()
{
    IMS_TRACE_I("~MediaSession() - CallKey[%" PFLS_u "]", m_nCallKey, 0, 0);

    ClearMediaNego();
    ClearAudioMediaSession();
    ClearMediaSessionTypeNode();
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
    if (pMediaNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pAudioNego->HasNegotiatedDtmf();
}

PUBLIC VIRTUAL
void MediaSession::SendImsMediaRequest()
{
    // do it later
}

PUBLIC VIRTUAL
void MediaSession::OnMediaResponse()
{
    // do it later
}

PUBLIC VIRTUAL
void MediaSession::OnNotify(IN IMS_UINT32 eReportType, IN MEDIA_CONTENT_TYPE eMediaType)
{
    // to UCMediaMngr
    if (m_pClientListener != IMS_NULL)
    {
        m_pClientListener->MediaSession_Notify(eReportType, eMediaType);
    }
}

PUBLIC VIRTUAL
void MediaSession::SetMtcListener(IN IMediaSessionClientListener *piMediaSessionListener)
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
        IMS_BOOL bIsIPv6 = pEnvironment->pIService->GetIPAddress().IsIPv6Address();
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
        IMS_TRACE_I("SetEnvironment() - CallKey[%" PFLS_u "], eServiceType[%d]",
                m_nCallKey, pEnvironment->eServiceType, 0);
        m_pEnvironment = pEnvironment;
        bNeedToCreateProfile = IMS_TRUE;
    }
    else if (m_pEnvironment->eServiceType != pEnvironment->eServiceType ||
        m_pEnvironment->eNetworkType != pEnvironment->eNetworkType)
    {
        IMS_TRACE_I("SetEnvironment() - CallKey[%" PFLS_u "], eServiceType[%d]->[%d]",
                m_nCallKey, m_pEnvironment->eServiceType, pEnvironment->eServiceType);
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
    return IMS_TRUE; // do it later
}

PUBLIC VIRTUAL
IMS_UINTP MediaSession::CreateProfile(IN IMS_UINTP nNegoId)
{
    IMS_UINTP nMediaNego = CreateMediaNego(nNegoId);
    if (nMediaNego == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (CreateAudioMediaSession(nMediaNego) == IMS_NULL)
    {
        return IMS_NULL;
    }

    ProcessOfferSdp(nMediaNego, nNegoId);

    IMS_TRACE_I("CreateProfile() - nMediaNego[%" PFLS_x "], Size[%d]",
            nMediaNego, m_objMapMediaNego.GetSize(), 0);

    return nMediaNego;
}

PUBLIC VIRTUAL
IMS_BOOL MediaSession::DestroyProfile(IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DestroyProfile() - nNegoId[%" PFLS_x "], m_eSessionState[%d]",
            nNegoId, m_eSessionState, 0);

    if (nNegoId == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bRet = IMS_TRUE;

    if (m_eSessionState == EARLY_SESSION)
    {
        m_nCommandBuffer = IMMedia::REQUEST_DELETE_CONFIG;
        if (ProcessRun(nNegoId) == IMS_TRUE)
        {
            IMS_TRACE_I("DestroyProfile() - got the answer then delete this profile", 0, 0, 0);
            bRet &= DeleteMediaNego(nNegoId);
            bRet &= DeleteAudioMediaSession(nNegoId);
            bRet &= DeleteMediaSessionTypeNode(nNegoId);
        }
        else
        {
            bRet = IMS_FALSE;
        }
    }
    else
    {
        // TODO::MEDIA - Need to discuss with MTC if this case is need
        bRet &= DeleteMediaNego(nNegoId);
        bRet &= DeleteAudioMediaSession(nNegoId);
        bRet &= DeleteMediaSessionTypeNode(nNegoId);
    }

    return bRet;
}

PUBLIC VIRTUAL
IMS_BOOL MediaSession::FormSDP(IN IMS_UINTP nNegoId, OUT ISession* pSession,
        IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_SINT32 eAudioDir, IN IMS_SINT32 eVideoDir,
        IN IMS_SINT32 eTextDir)
{
    IMS_TRACE_I("FormSDP() - nNegoId[%" PFLS_x "], pSession[%" PFLS_x "], eMediaType[%d]",
            nNegoId, pSession, eMediaType);
    IMS_TRACE_I("FormSDP() - DIR = Audio[%d], Video[%d], Text[%d]", eAudioDir, eVideoDir, eTextDir);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormSDP() - Can't find nNegoId[%d]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    if (pMediaNego->FormSDP(pSession, eMediaType, eAudioDir, eVideoDir, eTextDir) == IMS_TRUE)
    {
        if(pMediaNego->GetNegoState() == STATE_NEGOTIATED)
        {
            ProcessAnswerSdp(nNegoId);
        }
    }
    else
    {
        IMS_TRACE_E(0, "FormSDP() - FormSDP Failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL MediaSession::NegotiateSDP(IN IMS_UINTP nNegoId, IN ISession* pSession,
        OUT IMS_SINT32* eAudioDir, OUT IMS_SINT32* eVideoDir, OUT IMS_SINT32* eTextDir,
        OUT MediaNego::MediaNegoResult& errorReason)
{
    IMS_TRACE_I("NegotiateSDP() - nNegoId[%" PFLS_x "], pSession[%" PFLS_x "]",
            nNegoId, pSession, 0);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateSDP() - Can't find nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    if (pMediaNego->NegotiateSDP(
            pSession, eAudioDir, eVideoDir, eTextDir, errorReason) == IMS_TRUE)
    {
        CreateMediaSessionTypeNode(nNegoId, pSession);

        if (pMediaNego->GetNegoState() == STATE_NEGOTIATED)
        {
            ProcessAnswerSdp(nNegoId);
        }
        return IMS_TRUE;
    }
    else
    {
        return IMS_FALSE;
    }
}

PUBLIC VIRTUAL
void MediaSession::FinalizeSDP(IN IMS_UINTP nNegoId, IN ISession* pSession)
{
    IMS_TRACE_I("FinalizeSDP() - nNegoId[%" PFLS_x "], pSession[%" PFLS_x "]",
            nNegoId, pSession, 0);

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

PUBLIC VIRTUAL
IMS_BOOL MediaSession::Run(IN IMS_UINTP nNegoId)
{
    ProcessRun(nNegoId);
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL MediaSession::Terminate()
{
    IMS_TRACE_I("Terminate() - CallKey[%" PFLS_u "]",m_nCallKey, 0, 0);

    m_nCommandBuffer = IMMedia::REQUEST_CLOSE_SESSION;

    if (m_objMapMediaNego.IsEmpty())
    {
        IMS_TRACE_E(0, "Terminate() - No profile to terminate", 0, 0, 0);
        return IMS_FALSE;
    }
    MediaNego* pMediaNego = m_objMapMediaNego.GetValueAt(0);
    ProcessRun((IMS_UINTP)pMediaNego);

    return IMS_TRUE;
}

PUBLIC VIRTUAL
NEGO_STATE MediaSession::GetNegoState(IN IMS_UINTP nNegoId)
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

PUBLIC VIRTUAL
MEDIA_CONTENT_TYPE MediaSession::GetNegotiatedMediaType(IN IMS_UINTP nNegoId)
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
/*
    if (pMediaNego->GetNegotiatedVideoQuality() != VIDEO_RESOLUTION_NOT_USED)
    {
        eMedia = (MEDIA_CONTENT_TYPE)(eMedia | MEDIA_TYPE_VIDEO);
    }
    if (pMediaNego->GetNegotiatedTextQuality() != TEXT_CODEC_NOT_USED)
    {
        eMedia = (MEDIA_CONTENT_TYPE)(eMedia | MEDIA_TYPE_TEXT);
    }
*/ // do it later for text, video
    return eMedia;
}

/*
PUBLIC VIRTUAL
AString MediaSession::GetNegotiatedCodec(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL) {
        IMS_TRACE_E(0, "GetNegotiatedCodec() - Can't find nNegoId[%" PFLS_x "]",
            nNegoId, 0, 0);
        return IMS_NULL;
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_AUDIO)) {
        // do it later
    } else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_VIDEO)) {
        // do it later
    } else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_TEXT)) {
        // do it later
    }

    return IMS_NULL;
}
*/

PUBLIC VIRTUAL
IMS_SINT32 MediaSession::GetNegotiatedQuality(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedQuality() - Can't find nNegoId[%" PFLS_x "]",
                nNegoId, 0, 0);
        return 0;
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_AUDIO))
    {
        return (IMS_SINT32)(pMediaNego->GetNegotiatedAudioQuality());
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_VIDEO))
    {
        // return pMediaNego->GetNegotiatedVideoQuality();
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_TEXT))
    {
        // return pMediaNego->GetNegotiatedTextQuality();
    } // do it later
    return 0;
}

PUBLIC VIRTUAL
IMS_SINT32 MediaSession::GetNegotiatedCodecBitrate(IN IMS_UINTP nNegoId,
        IN MEDIA_CONTENT_TYPE type)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBitrate() - Can't find nNegoId[%" PFLS_x "]",
                nNegoId, 0, 0);
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
        //VideoNego* pVideoNego = pMediaNego->GetVideoNego();
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_TEXT))
    {
        //TextNego* pTextNego = pMediaNego->GetTextNego();
    } // for video (Phase3) do it later
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
PUBLIC VIRTUAL
MEDIA_DIRECTION MediaSession::GetNegotiatedDirection(IN IMS_UINTP nNegoId,
        IN MEDIA_CONTENT_TYPE type)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedDirection() - Can't find nNegoId[%" PFLS_x "]",
                nNegoId, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_AUDIO))
    {
        AudioNego* pAudioNego = pMediaNego->GetAudioNego();
        if (pAudioNego == IMS_NULL) return MEDIA_DIRECTION_INVALID;
        return pAudioNego->GetNegotiatedDirection();
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_VIDEO))
    {
        //do it later
        //return (IMS_SINT32)AUDIO_CODEC_BITRATE_MAX;
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(type, MEDIA_TYPE_TEXT))
    {
        //do it later
        //return (IMS_SINT32)AUDIO_CODEC_BITRATE_MAX;
    }
    return MEDIA_DIRECTION_INVALID;
}

PUBLIC VIRTUAL
void MediaSession::SetOptions(IN IMS_UINTP nNegoId, OptionType type,
        IN IMS_SINT32 param1, IN IMS_SINT32 param2)
{
    IMS_TRACE_I("SetOptions() - OptionType[%d], param1[%d], param2[%d]", type, param1, param2);
    MediaNego* pMediaNego = IMS_NULL;
    AudioNego* pAudioNego = IMS_NULL;
//    VideoNego* pVideoNego = IMS_NULL; // for video (Phase3) do it later
//    TextNego* pTextNego = IMS_NULL;   // for text (Phase2) do it later

    pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetOptions() - Can't find MediaNego[%" PFLS_x "]", nNegoId, 0, 0);
        return;
    }

    switch (type)
    {
        case SET_RTP_PORT:
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
                //pVideoNego = pMediaNego->GetVideoNego();
                //if (pVideoNego != IMS_NULL) pVideoNego->SetPort(param2);
            }
            else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_TEXT)
            {
                //pTextNego = pMediaNego->GetTextNego();
                //if (pTextNego != IMS_NULL) pTextNego->SetPort(param2);
            } // for video (Phase3) do it later
            break;
        case SET_DIRECTION:
            if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO)
            {
                pAudioNego = pMediaNego->GetAudioNego();
                //if (pAudioNego != IMS_NULL) pAudioNego->SetDirection(param2);
                // do it later (setDirection)
            } /*else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_VIDEO) {
                //to do
            } else if (param1 == (IMS_SINT32)MEDIA_CONTENT_TYPE::MEDIA_TYPE_TEXT) {
                //to do
            }*/ // for video (Phase3) do it later
            break;
        case SET_CONFIRMED_SESSION:
            if (m_eSessionState == EARLY_SESSION && param1 == IMS_TRUE)
            {
                m_eSessionState = READY_TO_CONFIRM;
            }
            break;
        case SET_DRA_REPORT_OPTION: // for Q3 (Analyzer)
        case SET_CONFERENCE_ENABLE: //for video
        case SET_CVO_SUPPORT:   //for video
        case SEND_FAST_VIDEO_UPDATE:    //for video
            break;
        default:
            break;
    }
}

PUBLIC VIRTUAL
void MediaSession::SetNetworkToneRTPTimer(MEDIA_CONTENT_TYPE eMediaType, IN IMS_UINT32 nRtpTimer)
{
    (void)eMediaType; // do it later
    IMS_TRACE_I("SetNetworkToneRTPTimer() - CallKey[%" PFLS_u "], eMediaType[%d], nRtpTimer[%d]",
            m_nCallKey, eMediaType, nRtpTimer);

    m_nRtpTimer = nRtpTimer;
}

PUBLIC VIRTUAL
IMS_BOOL MediaSession::SendMessage(IN IMSMSG &objMsg)
{
    IMS_TRACE_I("SendMessage() - nMSG[%d], LParam[%d], CallKey[%" PFLS_u "]",
            objMsg.nMSG, objMsg.nLparam, m_nCallKey);

    if (m_pClientListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendMessage() - No ClientListener", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bRet = IMS_TRUE;

    switch (objMsg.nMSG)
    {
        case IMMedia::RESPONSE_OPEN_SESSION:
            bRet = OnResponseOpenSession(objMsg);
            break;
        case IMMedia::RESPONSE_MODIFY_SESSION:
            bRet = OnResponseModifySession(objMsg);
            break;
        case IMMedia::RESPONSE_ADD_CONFIG:
            bRet = OnResponseAddConfig(objMsg);
            break;
        case IMMedia::RESPONSE_CONFIRM_CONFIG:
            bRet = OnResponseConfirmConfig(objMsg);
            break;
        case IMMedia::NOTIFY_FIRST_PACKET:
            bRet = OnNotifyFirstPacket(objMsg);
            break;
        case IMMedia::NOTIFY_MEDIA_INACTIVITY:
            bRet = OnNotifyMediaInactivity(objMsg);
            break;
        case IMMedia::NOTIFY_PACKET_LOSS:
            bRet = OnNofityPacketLoss(objMsg);
            break;
        case IMMedia::NOTIFY_JITTER:
            bRet = OnNofityJitter(objMsg);
            break;
        case IMMedia::NOTIFY_MEDIA_QUALITY_CHANGE:
            bRet = OnNofityMediaQualityChange(objMsg);
            break;
        case IMMedia::RESPONSE_SESSION_CHANGED:
            bRet = OnResponseSessionChanged(objMsg);
            break;
        case IMMedia::NOTIFY_HEADER_EXTENSION:
            bRet = OnNofityHeaderExtension(objMsg);
            break;
        case IMMedia::NOTIFY_QOS_INFO:
            bRet = OnNotifyQosInfo(objMsg);
            break;
        default:
            break;
    }

    return bRet;
}

PUBLIC VIRTUAL
IMS_BOOL MediaSession::SendDtmf(IN IMS_UINTP nNegoId, IN IMS_CHAR cDtmfCode,
    IN IMS_SINT32 nDuration)
{
    if (GetDTMFEnabled(nNegoId) == IMS_TRUE)
    {
        AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);
        if (pAudioSession != IMS_NULL)
        {
            return pAudioSession->SendDtmf(cDtmfCode, nDuration);
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MediaSession::CreateMediaConfig(IN MEDIA_SERVICE_TYPE eServiceType)
{
    AString strMediaRef = "gims.com.media";

    IMS_TRACE_D("CreateMediaConfig() media_ref=%s", strMediaRef.GetStr(), 0, 0);

    MediaSessionConfigFactory::GetInstance()->CreateMediaSessionConfig(m_nSlotId, eServiceType);

    MediaSessionConfig* pMediaSessionConfig = MediaSessionConfigFactory::GetInstance()->
            FindMediaSessionConfig(m_nSlotId, eServiceType);
    if (pMediaSessionConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateMediaConfig() - fail to  CreateMediaConfig", 0, 0, 0);
        return IMS_FALSE;
    }

    IConfigBuffer *piBuffer = Configuration::GetInstance()->CreateConfig(strMediaRef, m_nSlotId);
    if (piBuffer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pMediaSessionConfig->Create(m_nSlotId, piBuffer, 0))
    {
        piBuffer->Destroy();
        return IMS_FALSE;
    }

    piBuffer->Destroy();
    return IMS_TRUE;
}

// == PROTECTED METHOD ==========================================================
PROTECTED
IMS_UINTP MediaSession::CreateMediaNego(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_I( "CreateMediaNego()", 0, 0, 0);

    // Create new MediaNego
    MediaNego* pMediaNego = new MediaNego(m_nSlotId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfile() - fail to create MediaNego", 0, 0, 0);
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
            IMS_TRACE_I("CreateProfile() - invalid negoId", 0, 0, 0);
            return IMS_NULL;
        }
        pMediaNego->Forking(objExistingNego);
    }

    m_objMapMediaNego.Add((IMS_UINTP)pMediaNego, pMediaNego);

    return (IMS_UINTP)pMediaNego;
}

PROTECTED
IMS_UINTP MediaSession::CreateAudioMediaSession(IN IMS_UINTP nNegoId)
{
    AudioMediaSession* pAudioSession = new AudioMediaSession();
    if (pAudioSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateAudioMediaSession() - fail to create AudioMediaSession", 0, 0, 0);
        return IMS_NULL;
    }
    pAudioSession->SetNegoId(nNegoId);
    pAudioSession->SetMediaSessionListener(this);

    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego != IMS_NULL)
    {
        AudioNego* pAudioNego = pMediaNego->GetAudioNego();
        if (pAudioNego != IMS_NULL)
        {
            pAudioSession->SetConfig(pAudioNego->GetConfig());
        }
    }

    m_listAudioSession.Append(pAudioSession);

    return IMS_TRUE;
}

PROTECTED
MEDIA_CONTENT_TYPE MediaSession::GetSessionTypeFromMsg(IN IMS_SINT32 nMsg)
{
    switch (nMsg)
    {
        // Message of audio from UI
        case IUMedia::AUDIO_STOP_CMD:
        case IUMedia::AUDIO_START_CMD:
            return MEDIA_TYPE_AUDIO;
        // Message of video from UI
        case IUMedia::SETSURFACE_CMD :
        case IUMedia::PAUSE_CMD :
        case IUMedia::RESUME_CMD :
        case IUMedia::SELECT_CAMERA_CMD :
        case IUMedia::CHANGE_CAMERA_ZOOM_CMD :
        case IUMedia::CHANGE_CAMERA_BRIGHTNESS_CMD :
        case IUMedia::CAPTURE_CMD :
        case IUMedia::START_ALTERNATE_IMAGE_CMD :
        case IUMedia::STOP_ALTERNATE_IMAGE_CMD :
        case IUMedia::UPDATE_DISPLAY_CMD :
        case IUMedia::CHANGE_VIEW_SIZE_CMD :
        case IUMedia::SWAP_DISPLAY_CMD :
        case IUMedia::CHANGE_ORIENTATION_CMD :
        case IUMedia::START_PREVIEW_CAMERA_CMD :
        case IUMedia::STOP_PREVIEW_CAMERA_CMD :
        case IUMedia::VIDEO_DATA_USAGE_CMD :
        case IUMedia::VIDEO_CALL_EFFECT_CMD:
            return MEDIA_TYPE_VIDEO;
        case IUMedia::RTT_TEXT_SEND_CMD:
            return MEDIA_TYPE_TEXT;
        case IUMedia::START_RECODING_CMD:
        case IUMedia::STOP_RECODING_CMD:
        default:
            return MEDIA_TYPE_INVALID;
    }
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
AudioMediaSession* MediaSession::FindAudioSession(IN IMS_UINTP nNegoId)
{
    AudioMediaSession* pAudioSession = IMS_NULL;
    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        pAudioSession = m_listAudioSession.GetAt(nIndex);
        if (pAudioSession != IMS_NULL && pAudioSession->IsSameNegoId(nNegoId) == IMS_TRUE)
        {
            return pAudioSession;
        }
    }
    return IMS_NULL;
}

PROTECTED
void MediaSession::ConfirmAudioSession(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("ConfirmAudioSession() - nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
    if (nNegoId == IMS_NULL)
    {
        return;
    }

    ConfirmMediaNego(nNegoId);
    ConfirmAudioMediaSession(nNegoId);
    ConfirmMediaSessionTypeNode(nNegoId);
}

PROTECTED
void MediaSession::ConfirmMediaNego(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("ConfirmMediaNego() - nNegoId[%" PFLS_x "], Size[%d]",
            nNegoId, m_objMapMediaNego.GetSize(), 0);

    if (m_objMapMediaNego.GetSize() <= 1)
    {
        return;
    }

    MediaNego* pMediaNego = IMS_NULL;
    IMS_SINT32 nIndex = 0;

    while (m_objMapMediaNego.GetSize() > nIndex)
    {
        pMediaNego = m_objMapMediaNego.GetValueAt(nIndex);
        if (pMediaNego == IMS_NULL)
        {
            IMS_TRACE_E(0, "ConfirmMediaNego() - invalid pMediaNego", 0, 0, 0);
            m_objMapMediaNego.RemoveAt(nIndex);
        }
        else if ((IMS_UINTP)pMediaNego != nNegoId)
        {
            delete pMediaNego;
            pMediaNego = IMS_NULL;
            m_objMapMediaNego.RemoveAt(nIndex);
        }
        else
        {
            nIndex++;
        }
    }
}

PROTECTED
void MediaSession::ConfirmAudioMediaSession(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("ConfirmAudioMediaSession() - nNegoId[%" PFLS_x "], Size[%d]",
            nNegoId, m_listAudioSession.GetSize(), 0);

    if (m_listAudioSession.GetSize() <= 1)
    {
        return;
    }

    AudioMediaSession* pAudioSession = IMS_NULL;
    IMS_SINT32 nIndex = 0;

    while (m_listAudioSession.GetSize() > nIndex)
    {
        pAudioSession = m_listAudioSession.GetAt(nIndex);
        if (pAudioSession == IMS_NULL)
        {
            IMS_TRACE_E(0, "ConfirmAudioMediaSession() - invalid pAudioSession", 0, 0, 0);
            m_listAudioSession.RemoveAt(nIndex);
        }
        else if (pAudioSession->IsSameNegoId(nNegoId) == IMS_FALSE)
        {
            delete pAudioSession;
            pAudioSession = IMS_NULL;
            m_listAudioSession.RemoveAt(nIndex);
        }
        else
        {
            nIndex++;
        }
    }
}

PROTECTED
void MediaSession::ConfirmMediaSessionTypeNode(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("ConfirmMediaSessionTypeNode() - nNegoId[%" PFLS_x "], Size[%d]",
            nNegoId, m_listMediaSessionTypeNode.GetSize(), 0);

    if (m_listMediaSessionTypeNode.GetSize() <= 1)
    {
        return;
    }

    MediaSessionTypeNode* pMediaSessionTypeNode = IMS_NULL;
    IMS_SINT32 nIndex = 0;

    while (m_listMediaSessionTypeNode.GetSize() > nIndex)
    {
        pMediaSessionTypeNode = m_listMediaSessionTypeNode.GetAt(nIndex);
        if (pMediaSessionTypeNode == IMS_NULL)
        {
            IMS_TRACE_E(0, "ConfirmMediaSessionTypeNode() - invalid pMediaSessionTypeNode",
                    0, 0, 0);
            m_listMediaSessionTypeNode.RemoveAt(nIndex);
        }
        else if (pMediaSessionTypeNode->m_nNegoId != nNegoId)
        {
            delete pMediaSessionTypeNode;
            pMediaSessionTypeNode = IMS_NULL;
            m_listMediaSessionTypeNode.RemoveAt(nIndex);
        }
        else
        {
            nIndex++;
        }
    }
}

PROTECTED
IMS_BOOL MediaSession::DeleteMediaNego(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DeleteMediaNego() - nNegoId[%" PFLS_x "], Size[%d]",
            nNegoId, m_objMapMediaNego.GetSize(), 0);

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
IMS_BOOL MediaSession::DeleteAudioMediaSession(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DeleteAudioMediaSession() - nNegoId[%" PFLS_x "], Size[%d]",
            nNegoId, m_listAudioSession.GetSize(), 0);

    if (nNegoId == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AudioMediaSession* pAudioSession = IMS_NULL;
    for (IMS_UINT32 nIndex = 0; nIndex < m_listAudioSession.GetSize(); nIndex++)
    {
        pAudioSession = m_listAudioSession.GetAt(nIndex);
        if (pAudioSession != IMS_NULL && pAudioSession->IsSameNegoId(nNegoId) == IMS_TRUE)
        {
            delete pAudioSession;
            pAudioSession = IMS_NULL;
            m_listAudioSession.RemoveAt(nIndex);
            return IMS_TRUE;
        }
    }
    IMS_TRACE_E(0, "DeleteAudioMediaSession() - Nothing matched with this NegoId", 0, 0, 0);
    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MediaSession::DeleteMediaSessionTypeNode(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DeleteMediaSessionTypeNode() - nNegoId[%" PFLS_x "], Size[%d]",
            nNegoId, m_listMediaSessionTypeNode.GetSize(), 0);

    if (nNegoId == IMS_NULL)
    {
        return IMS_FALSE;
    }

    MediaSessionTypeNode* pMediaSessionTypeNode = IMS_NULL;
    IMS_BOOL bRet = IMS_FALSE;
    IMS_UINT32 nIndex = 0;

    while (m_listMediaSessionTypeNode.GetSize() > nIndex)
    {
        if (pMediaSessionTypeNode != IMS_NULL && pMediaSessionTypeNode->m_nNegoId == nNegoId)
        {
            delete pMediaSessionTypeNode;
            pMediaSessionTypeNode = IMS_NULL;
            m_listMediaSessionTypeNode.RemoveAt(nIndex);
            bRet = IMS_TRUE;
        }
        else
        {
            nIndex++;
        }
    }

    if (bRet == IMS_FALSE)
    {
        IMS_TRACE_E(0, "DeleteMediaSessionTypeNode() - Nothing matched with this NegoId", 0, 0, 0);
    }

    return bRet;
}

PROTECTED
void MediaSession::ClearMediaNego()
{
    IMS_TRACE_D("ClearMediaNego() m_objMapMediaNego size=%d", m_objMapMediaNego.GetSize(), 0, 0);

    MediaNego* pMediaNego = IMS_NULL;

    while(m_objMapMediaNego.GetSize() > 0)
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
void MediaSession::ClearAudioMediaSession()
{
    IMS_TRACE_D("ClearAudioMediaSession() m_listAudioSession size=%d",
            m_listAudioSession.GetSize(), 0, 0);

    AudioMediaSession* pAudioSession = IMS_NULL;

    while(m_listAudioSession.GetSize() > 0)
    {
        pAudioSession = m_listAudioSession.GetValueAt(0);

        if (pAudioSession != IMS_NULL)
        {
            delete pAudioSession;
            pAudioSession = IMS_NULL;
        }
        m_listAudioSession.RemoveAt(0);
    }
    m_listAudioSession.Clear();
}

PROTECTED
void MediaSession::ClearMediaSessionTypeNode()
{
    IMS_TRACE_D("ClearMediaSessionTypeNode() m_listMediaSessionTypeNode size=%d",
            m_listMediaSessionTypeNode.GetSize(), 0, 0);

    MediaSessionTypeNode* pMediaSessionTypeNode = IMS_NULL;

    while(m_listMediaSessionTypeNode.GetSize() > 0)
    {
        pMediaSessionTypeNode = m_listMediaSessionTypeNode.GetValueAt(0);
        if (pMediaSessionTypeNode != IMS_NULL)
        {
            delete pMediaSessionTypeNode;
            pMediaSessionTypeNode = IMS_NULL;
        }
        m_listMediaSessionTypeNode.RemoveAt(0);
    }
    m_listMediaSessionTypeNode.Clear();
}

// TODO:Media - deprecated function
PROTECTED
void MediaSession::SetAllMediaNegoActiveProfile(IN IMS_BOOL bIsActive)
{
    MediaNego* pMediaNego = IMS_NULL;

    for (IMS_UINT32 nIndex = 0; nIndex < m_objMapMediaNego.GetSize(); nIndex++)
    {
        pMediaNego = m_objMapMediaNego.GetValueAt(nIndex);

        if (pMediaNego != IMS_NULL) {
            pMediaNego->SetActiveProfile(bIsActive);
        }
    }
}

// TODO:Media - deprecated function
PROTECTED
IMS_BOOL MediaSession::SetActiveProfile(IMS_UINTP nNegoId)
{
    MediaNego* pActiveMediaNego;
    pActiveMediaNego = FindMediaNego(nNegoId);
    if (pActiveMediaNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pActiveMediaNego->SetActiveProfile(IMS_TRUE);

    for (IMS_UINT32 nIndex = 0; nIndex < m_objMapMediaNego.GetSize(); nIndex++)
    {
        MediaNego* pMediaNego;
        pMediaNego = m_objMapMediaNego.GetValueAt(nIndex);
        if (pMediaNego == pActiveMediaNego)
        {
            continue;
        }
        pMediaNego->SetActiveProfile(IMS_FALSE);
    }

    return IMS_TRUE;
}

PROTECTED
void MediaSession::ProcessOfferSdp(IN IMS_UINTP nNegoId, IN IMS_UINT32 nReUsed)
{
    IMS_TRACE_I("ProcessOfferSdp() - nNegoId[%" PFLS_x "], nReUsed[%d]", nNegoId, nReUsed, 0);

    if (m_eSessionState == EARLY_SESSION && nReUsed == 0)
    {
        m_nCommandBuffer = IMMedia::REQUEST_OPEN_SESSION;
        m_bSessionOpened = IMS_TRUE;
        ProcessRun(nNegoId);
    }
}

PROTECTED
void MediaSession::ProcessAnswerSdp(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_I("ProcessAnswerSdp() - nNegoId[%" PFLS_x "]", nNegoId, 0, 0);

    if (m_listAudioSession.IsEmpty() == IMS_TRUE)
    {
        return;
    }

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);

    // create session based on the session state : if remote ip/port changed..
    if (m_eSessionState == EARLY_SESSION && m_bSessionOpened == IMS_TRUE
            && pAudioSession == IMS_NULL)
    {
        pAudioSession = new AudioMediaSession();
        pAudioSession->SetNegoId(nNegoId);
        m_listAudioSession.Append(pAudioSession);
        m_nCommandBuffer = IMMedia::REQUEST_ADD_CONFIG;
        IMS_TRACE_D("ProcessAnswerSdp() - create session nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
    }
    else
    {
        m_nCommandBuffer = IMMedia::REQUEST_MODIFY_SESSION;
    }
}

PROTECTED
IMS_BOOL MediaSession::ProcessRun(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_D("ProcessRun() - nNegoId[%" PFLS_x "], m_nCommandBuffer[%d]",
            nNegoId, m_nCommandBuffer, 0);
    if (m_nCommandBuffer != IMMedia::MEDIA_MESSAGE_IDX_START)
    {
        AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);
        if (pAudioSession == IMS_NULL)
        {
            IMS_TRACE_E(0, "ProcessRun() - No AudioSession : nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
            return IMS_FALSE;
        }

        IMS_BOOL isJustConfirmedWithMultiConfig =
                (m_eSessionState == READY_TO_CONFIRM) && (m_listAudioSession.GetSize() > 1);

        if (m_nCommandBuffer == IMMedia::REQUEST_OPEN_SESSION)
        {
            UpdateLocalAddress(nNegoId);
            pAudioSession->Open();
        }
        else if (m_nCommandBuffer == IMMedia::REQUEST_MODIFY_SESSION
                && isJustConfirmedWithMultiConfig == IMS_FALSE)
        {
            UpdateRtpConfig(nNegoId);
            pAudioSession->Modify();
        }
        else if (m_nCommandBuffer == IMMedia::REQUEST_ADD_CONFIG)
        {
            UpdateRtpConfig(nNegoId);
            pAudioSession->Add();
        }
        else if (m_nCommandBuffer == IMMedia::REQUEST_DELETE_CONFIG)
        {
            pAudioSession->Delete();
        }
        else if (m_nCommandBuffer == IMMedia::REQUEST_CLOSE_SESSION)
        {
            pAudioSession->Close();
        }
        m_nCommandBuffer = IMMedia::MEDIA_MESSAGE_IDX_START;

        if (isJustConfirmedWithMultiConfig == IMS_TRUE)
        {
            pAudioSession->Confirm();
            this->ConfirmAudioSession(nNegoId);
        }
    }
    if (m_eSessionState == READY_TO_CONFIRM)
    {
        m_eSessionState = CONFIRMED_SESSION;
    }
    return IMS_TRUE;
}

PROTECTED
void MediaSession::UpdateLocalAddress(IN IMS_UINTP nNegoId)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        return;
    }

    AudioNego* pAudioNego = pMediaNego->GetAudioNego();
    if (pAudioNego == IMS_NULL)
    {
        return;
    }

    IPAddress objLocalAddr = pAudioNego->GetLocalAddr();
    IMS_UINT32 nPort = pAudioNego->GetLocalPort();

    AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);
    if (pAudioSession != IMS_NULL)
    {
        pAudioSession->UpdateLocalEndPoint(objLocalAddr, nPort);
    }
}

PROTECTED
void MediaSession::UpdateRtpConfig(IN IMS_UINTP nNegoId)
{
    UpdateAudioRtpConfig(nNegoId);
}

PROTECTED
void MediaSession::UpdateAudioRtpConfig(IN IMS_UINTP nNegoId)
{
    IMS_TRACE_I("UpdateAudioRtpConfig() - nNegoId[%" PFLS_x "]", nNegoId, 0, 0);
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        return;
    }

    AudioNego* pAudioNego = pMediaNego->GetAudioNego();
    if (pAudioNego == IMS_NULL)
    {
        return;
    }

    AudioProfile* pSrcProfile = IMS_NULL;
    AudioProfile* pDestProfile = IMS_NULL;
    AudioProfile* pNegoProfile = IMS_NULL;

    if (pAudioNego->GetNegotiatedProfileSet(
            pSrcProfile, pDestProfile, pNegoProfile) == IMS_TRUE)
    {
        AudioMediaSession* pAudioSession = FindAudioSession(nNegoId);
        if (pAudioSession != IMS_NULL)
        {
            pAudioSession->UpdateRtpConfig(pSrcProfile, pDestProfile, pNegoProfile);
        }
    }
}

PROTECTED
VIRTUAL void MediaSession::MediaSession_SendEventToUi(IMS_SINT32 nEvent, IMS_SINT32 nResult)
{
    IMS_TRACE_D("MediaSession_SendEventToUi() : CallKey[%" PFLS_u "] nEvent[%d], nResult[%d]",
            m_nCallKey, nEvent, nResult);
    IMSMSG objMsg(nEvent, m_nCallKey, nResult);
    MSGService::PostMessage(MediaManager::GetThreadName(m_nSlotId), objMsg);
}

PROTECTED VIRTUAL
IMS_BOOL MediaSession::MediaSession_SendMsgToMediaManager(IN IMS_SINT32 nEvent,
        IN ImsMediaMsgParamBase* param)
{
    IMS_TRACE_D("MediaSession_SendMsgToMediaManager() : CallKey[%" PFLS_u "] nEvent[%d]",
            m_nCallKey, nEvent, 0);
    IMSMSG objMsg(nEvent, m_nCallKey, reinterpret_cast<IMS_UINTP>(param));
    return MSGService::PostMessage(MediaManager::GetThreadName(m_nSlotId), objMsg);
}

PROTECTED
void MediaSession::CreateMediaSessionTypeNode(IN IMS_UINTP nNegoId, IN ISession* pSession)
{
    if (pSession == IMS_NULL)
    {
        return;
    }

    // Check the requested media type
    IMSList<IMedia*> lstIMedia = pSession->GetMedia();
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < lstIMedia.GetSize(); nIndex++)
    {
        IMediaDescriptor* pDescriptor = pMediaNego->GetMediaDescriptor(lstIMedia.GetAt(nIndex));
        if (pDescriptor == IMS_NULL)
        {
            return;
        }

        SdpMedia* pSDPMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();
        if (pSDPMedia == IMS_NULL)
        {
            return;
        }

        IMS_SINT32 nPort = pSDPMedia->GetPort();

        if (nPort != -1)
        {
            if (IsExistingTypeNode(pDescriptor->GetRemoteAddress().ToString(), nPort) == IMS_FALSE)
            {
                MediaSessionTypeNode* pMediaSessionTypeNode = new MediaSessionTypeNode();

                pMediaSessionTypeNode->m_strIpAddr = pDescriptor->GetRemoteAddress().ToString();
                pMediaSessionTypeNode->m_nPort = nPort;
                pMediaSessionTypeNode->m_nNegoId = nNegoId;

                IMS_SINT32 nType = pSDPMedia->GetType();
                if (nType == SdpMedia::TYPE_AUDIO)
                {
                    pMediaSessionTypeNode->m_eMediaType = MEDIA_TYPE_AUDIO;
                }
                else if (nType == SdpMedia::TYPE_VIDEO)
                {
                    pMediaSessionTypeNode->m_eMediaType = MEDIA_TYPE_VIDEO;
                }
                else if (nType == SdpMedia::TYPE_TEXT)
                {
                    pMediaSessionTypeNode->m_eMediaType = MEDIA_TYPE_TEXT;
                }

                m_listMediaSessionTypeNode.Append(pMediaSessionTypeNode);
            }
        }
    }
}

PROTECTED
IMS_BOOL MediaSession::IsExistingTypeNode(IN AString strIpAddr, IN IMS_UINT32 nPort)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_listMediaSessionTypeNode.GetSize(); nIndex++)
    {
        if (m_listMediaSessionTypeNode.GetAt(nIndex)->m_strIpAddr == strIpAddr &&
                m_listMediaSessionTypeNode.GetAt(nIndex)->m_nPort == nPort)
        {
            IMS_TRACE_I("checkExistingTypeNode() - Found existing one", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseOpenSession(IN IMSMSG &objMsg)
{
    ImsMediaResponseConfigParam* pParam =
            reinterpret_cast<ImsMediaResponseConfigParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    RtpError eResult = pParam->m_eResult;

    IMS_TRACE_I("OnResponseOpenSession() - CallKey[%" PFLS_u "], eMediaType[%d], eResult[%d]",
            m_nCallKey, eMediaType, eResult);

    if (eResult == NO_ERROR)
    {
        m_pClientListener->MediaSession_Notify(REPORT_SUCCESS, pParam->m_eMediaType);
    }
    else if (eResult > NO_ERROR)
    {
        m_pClientListener->MediaSession_NotifyFailures(REPORT_FAILURE, eResult,
                pParam->m_eMediaType);
    }

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseModifySession(IN IMSMSG &objMsg)
{
    ImsMediaResponseConfigParam* pParam =
            reinterpret_cast<ImsMediaResponseConfigParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    RtpError eResult = pParam->m_eResult;
    AudioConfig objAudioConfig = pParam->m_objAudioConfig;
    (void) objAudioConfig;

    IMS_TRACE_I("OnResponseModifySession() - CallKey[%" PFLS_u "], eMediaType[%d], eResult[%d]",
            m_nCallKey, eMediaType, eResult);

    if (eResult == NO_ERROR)
    {
        m_pClientListener->MediaSession_Notify(REPORT_SUCCESS, pParam->m_eMediaType);
    }
    else if (eResult > NO_ERROR)
    {
        m_pClientListener->MediaSession_NotifyFailures(REPORT_FAILURE, eResult,
                pParam->m_eMediaType);
    }

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseAddConfig(IN IMSMSG &objMsg)
{
    ImsMediaResponseConfigParam* pParam =
            reinterpret_cast<ImsMediaResponseConfigParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    RtpError eResult = pParam->m_eResult;
    AudioConfig objAudioConfig = pParam->m_objAudioConfig;
    (void) objAudioConfig;

    IMS_TRACE_I("OnResponseAddConfig() - CallKey[%" PFLS_u "], eMediaType[%d], eResult[%d]",
            m_nCallKey, eMediaType, eResult);

    if (eResult == NO_ERROR)
    {
        m_pClientListener->MediaSession_Notify(REPORT_SUCCESS, pParam->m_eMediaType);
    }
    else if (eResult > NO_ERROR)
    {
        m_pClientListener->MediaSession_NotifyFailures(REPORT_FAILURE, eResult,
                pParam->m_eMediaType);
    }

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnResponseConfirmConfig(IN IMSMSG &objMsg)
{
    ImsMediaResponseConfigParam* pParam =
            reinterpret_cast<ImsMediaResponseConfigParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    RtpError eResult = pParam->m_eResult;
    AudioConfig objAudioConfig = pParam->m_objAudioConfig;
    (void) objAudioConfig;

    IMS_TRACE_I("OnResponseConfirmConfig() - CallKey[%" PFLS_u "], eMediaType[%d], eResult[%d]",
            m_nCallKey, eMediaType, eResult);

    if (eResult == NO_ERROR)
    {
        m_pClientListener->MediaSession_Notify(REPORT_SUCCESS, pParam->m_eMediaType);
    }
    else if (eResult > NO_ERROR)
    {
        m_pClientListener->MediaSession_NotifyFailures(REPORT_FAILURE, eResult,
                pParam->m_eMediaType);
    }

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotifyFirstPacket(IN IMSMSG &objMsg)
{
    ImsMediaResponseConfigParam* pParam =
            reinterpret_cast<ImsMediaResponseConfigParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;

    IMS_TRACE_I("OnResponseConfirmConfig() - CallKey[%" PFLS_u "], eMediaType[%d]",
            m_nCallKey, eMediaType, 0);

    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_STARTED, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotifyMediaInactivity(IN IMSMSG &objMsg)
{
    ImsMediaNotifyInactivityParam* pParam =
            reinterpret_cast<ImsMediaNotifyInactivityParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType = pParam->m_eMediaProtocolType;

    IMS_TRACE_I("OnNotifyMediaInactivity() - eMediaProtocolType[%d], eMediaType[%d]",
            eMediaProtocolType, eMediaType, 0);

    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNofityPacketLoss(IN IMSMSG &objMsg)
{
    ImsMediaNotifyPacketParam* pParam =
            reinterpret_cast<ImsMediaNotifyPacketParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    IMS_SINT32 nPacketLossPercentage = pParam->m_nResponse;

    IMS_TRACE_I("OnNofityPacketLoss() - CallKey[%" PFLS_u "], eMediaType[%d], nPacketLoss[%d]",
            m_nCallKey, eMediaType, nPacketLossPercentage);

    // do it later :
    // TBD : need to discuss how to use this info

//    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNofityJitter(IN IMSMSG &objMsg)
{
    ImsMediaNotifyPacketParam* pParam =
            reinterpret_cast<ImsMediaNotifyPacketParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;
    IMS_SINT32 nJitter = pParam->m_nResponse;

    IMS_TRACE_I("OnNofityJitter() - CallKey[%" PFLS_u "], eMediaType[%d], nJitter[%d]",
            m_nCallKey, eMediaType, nJitter);

    // do it later :
    // TBD : need to discuss how to use this info

//    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);

    delete pParam;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNofityMediaQualityChange(IN IMSMSG &objMsg)
{
    (void) objMsg;
/*
    ImsMediaNotifyQualityParam* pParam =
            reinterpret_cast<ImsMediaNotifyQualityParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;

    IMS_TRACE_I("OnNofityMediaQualityChange() - CallKey[%" PFLS_u "], eMediaType[%d]",
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
IMS_BOOL MediaSession::OnResponseSessionChanged(IN IMSMSG &objMsg)
{
    (void) objMsg;
/*
    ImsMediaSessionChangedParam* pParam =
            reinterpret_cast<ImsMediaSessionChangedParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;

    IMS_TRACE_I("OnResponseSessionChanged() - CallKey[%" PFLS_u "], eMediaType[%d]",
            m_nCallKey, eMediaType, 0);

    // need to get RtpSession from ImsMediaSessionChangedParam

    // TBD : need to discuss how to use this info

//    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);
    delete pParam;
*/ // NEXT_ITEM :: OnSessionChanged
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNofityHeaderExtension(IN IMSMSG &objMsg)
{
    (void) objMsg;
/*
    ImsMediaHeaderExtensionParam* pParam =
            reinterpret_cast<ImsMediaHeaderExtensionParam*>(objMsg.nLparam);
    MEDIA_CONTENT_TYPE eMediaType = pParam->m_eMediaType;

    IMS_TRACE_I("OnNofityHeaderExtension() - CallKey[%" PFLS_u "], eMediaType[%d]",
            m_nCallKey, eMediaType, 0);

    // need to get RtpHeaderExtension from ImsMediaSessionChangedParam

    // TBD : need to discuss how to use this info

//    m_pClientListener->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, pParam->m_eMediaType);
    delete pParam;
*/ // NEXT_ITEM :: OnNofityHeaderExtension
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSession::OnNotifyQosInfo(IN IMSMSG &objMsg)
{
    ImsMediaNotifyQosParam* pParam =
            reinterpret_cast<ImsMediaNotifyQosParam*>(objMsg.nLparam);
    IPAddress objIpAddr = pParam->m_objIpAddr;
    IMS_SINT32 nPort = pParam->m_nPort;
    IMS_BOOL bResult = pParam->m_bResult;

    IMS_TRACE_I("OnNotifyQosInfo() - CallKey[%" PFLS_u "], nPort[%d], bResult[%d]",
            m_nCallKey, nPort, bResult);

    ImsMediaBasicSessionInfoParam* pBasicSessionInfo =
            GetBasicSessionInfofromRemoteArress(objIpAddr.ToString(), nPort);

    m_pClientListener->MediaSession_NotifyQos(pBasicSessionInfo->m_nNegoId, bResult,
            pParam->m_eMediaType);

    delete pParam;
    delete pBasicSessionInfo;
    return IMS_TRUE;
}

PROTECTED
ImsMediaBasicSessionInfoParam* MediaSession::GetBasicSessionInfofromRemoteArress(
        IN AString strIpAddr, IN IMS_SINT32 nPort)
{
    MediaSessionTypeNode* pMediaSessionTypeNode = IMS_NULL;
    for (IMS_UINT32 nIndex = 0; nIndex < m_listMediaSessionTypeNode.GetSize(); nIndex++)
    {
        pMediaSessionTypeNode = m_listMediaSessionTypeNode.GetAt(nIndex);
        if (pMediaSessionTypeNode->m_strIpAddr == strIpAddr &&
                pMediaSessionTypeNode->m_nPort == nPort)
        {
            ImsMediaBasicSessionInfoParam* pBasicSessionInfo = new ImsMediaBasicSessionInfoParam();
            pBasicSessionInfo->m_nNegoId = pMediaSessionTypeNode->m_nNegoId;
            pBasicSessionInfo->m_eMediaType = pMediaSessionTypeNode->m_eMediaType;

            return pBasicSessionInfo;
        }
    }
    return IMS_NULL;
}

PROTECTED
IMS_UINTP MediaSession::GetNegoIdfromRemoteAddress(IN AString strIpAddr, IN IMS_SINT32 nPort)
{
    MediaSessionTypeNode* pMediaSessionTypeNode = IMS_NULL;
    for (IMS_UINT32 nIndex = 0; nIndex < m_listMediaSessionTypeNode.GetSize(); nIndex++)
    {
        pMediaSessionTypeNode = m_listMediaSessionTypeNode.GetAt(nIndex);
        if (pMediaSessionTypeNode->m_strIpAddr == strIpAddr &&
                pMediaSessionTypeNode->m_nPort == nPort)
        {
            return pMediaSessionTypeNode->m_nNegoId;
        }
    }
    return IMS_NULL;
}
