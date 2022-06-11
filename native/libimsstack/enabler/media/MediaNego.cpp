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
#include "ImsCore.h"
#include "ICoreService.h"
#include "ISessionDescriptor.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "config/AudioConfiguration.h"
#include "config/VideoConfiguration.h"
#include "MediaNego.h"

// == DEFINES =============================================================
__IMS_TRACE_TAG_USER_DECL__("MED.MN");

PUBLIC
MediaNego::MediaNego(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_eNegoState(STATE_IDLE),
        m_pAudioNego(IMS_NULL),
        m_pVideoNego(IMS_NULL),
        // m_pTextNego(IMS_NULL), // TODO_MEDIA text
        m_pMediaEnvironment(IMS_NULL),
        m_eSessionType(MEDIA_TYPE_INVALID),
        m_bIsActive(IMS_FALSE),
        m_bForking(IMS_FALSE)
{
    IMS_TRACE_I("MediaNego() - Slot[%d]", nSlotId, 0, 0);
}

PUBLIC
MediaNego::~MediaNego()
{
    IMS_TRACE_I("~MediaNego()", 0, 0, 0);

    if (m_pAudioNego != IMS_NULL)
    {
        delete m_pAudioNego;
    }
    if (m_pVideoNego != IMS_NULL)
    {
        delete m_pVideoNego;
    }
    // if (m_pTextNego != IMS_NULL) // TODO_MEDIA text
    // {
    //     delete m_pTextNego;
    // }
}

PUBLIC
void MediaNego::Create(IN MEDIA_SERVICE_TYPE eServiceType)
{
    IMS_TRACE_D("Create Enter eServiceType[%d]", eServiceType, 0, 0);
    m_pAudioNego = AudioNego::Create(GetSlotId(), eServiceType);
    m_pVideoNego = VideoNego::Create(GetSlotId(), eServiceType);
    //    m_pTextNego = TextNego::Create(GetSlotId(), eServiceType); // TODO_MEDIA text
}

PUBLIC
void MediaNego::SetMediaEnvironment(IN MediaEnvironment* pMediaEnvironment)
{
    IMS_BOOL bNeedToCreateProfile = IMS_FALSE;

    if (pMediaEnvironment == IMS_NULL)
    {
        IMS_TRACE_I("SetMediaEnvironment() pMediaEnvironment is NULL", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("SetMediaEnvironment() -  eNetworkType[%d], eServiceType[%d]",
            (IMS_UINT16)pMediaEnvironment->eNetworkType, pMediaEnvironment->eServiceType, 0);

    if (m_pMediaEnvironment == IMS_NULL ||
            (m_pMediaEnvironment->eServiceType != pMediaEnvironment->eServiceType) ||
            (m_pMediaEnvironment->eNetworkType != pMediaEnvironment->eNetworkType))
    {
        bNeedToCreateProfile = IMS_TRUE;
    }

    m_pMediaEnvironment = pMediaEnvironment;

    if (bNeedToCreateProfile == IMS_TRUE)
    {
        if (m_pAudioNego != IMS_NULL)
        {
            m_pAudioNego->DestroyProfiles();
            m_pAudioNego->SetMediaEnvironment(m_pMediaEnvironment);
            m_pAudioNego->CreateProfiles(m_pMediaEnvironment);
        }
        if (m_pVideoNego != IMS_NULL)
        {
            m_pVideoNego->DestroyProfiles();
            m_pVideoNego->SetMediaEnvironment(m_pMediaEnvironment);
            m_pVideoNego->CreateProfiles(m_pMediaEnvironment);
        }
        // if (m_pTextNego != IMS_NULL) // TODO_MEDIA text
        // {
        //     m_pTextNego->DestroyProfiles();
        //     m_pTextNego->SetMediaEnvironment(m_pMediaEnvironment);
        //     m_pTextNego->CreateProfiles(m_pMediaEnvironment);
        // }
    }
}

PUBLIC
IMS_BOOL MediaNego::Forking(IN MediaNego* pMediaNego)
{
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "Forking() - incomming MediaNego is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("Forking() - nNegoId[%" PFLS_x "]", (IMS_UINTP)pMediaNego, 0, 0);

    m_eNegoState = STATE_OFFER_SENT;
    m_bForking = IMS_TRUE;

    if (m_pAudioNego != IMS_NULL)
    {
        m_pAudioNego->Copy(pMediaNego->GetAudioNego());
    }
    if (m_pVideoNego != IMS_NULL)
    {
        m_pVideoNego->Copy(pMediaNego->GetVideoNego());
    }
    // if (m_pAudioNego != IMS_NULL) // TODO_MEDIA text
    // {
    //     m_pAudioNego->Copy(pMediaNego->GetTextNego());
    // }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MediaNego::FormSDP(OUT ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType,
        IN IMS_SINT32 eAudioDir, IN IMS_SINT32 eVideoDir, IN IMS_SINT32 eTextDir)
{
    IMS_TRACE_I(
            "FormSDP(): eMediaType[%d], pMediaEnvironment[%x]", eMediaType, m_pMediaEnvironment, 0);
    IMS_TRACE_I("FormSDP() - DIR = Audio[%d], Video[%d], Text[%d]", eAudioDir, eVideoDir, eTextDir);

    // -- Step 0. Check the exception case --------------------------------------------------------
    if (m_pMediaEnvironment == IMS_NULL)
    {
        return IMS_FALSE;
    }
    if (m_eNegoState == STATE_OFFER_SENT)
    {
        return IMS_FALSE;
    }

    m_eSessionType = eMediaType;

    if ((MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_AUDIO) && m_pAudioNego == IMS_NULL)
        || (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO) && m_pVideoNego == IMS_NULL)
    /*|| (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT) && m_pTextNego == IMS_NULL)*/)
    // TODO_MEDIA text
    {
        IMS_TRACE_E(0, "FormSDP() - INVALID : eMediaType[%d]", eMediaType, 0, 0);
        return IMS_FALSE;
    }
    MEDIA_CONTENT_TYPE eNeedToMakeMedia = eMediaType;
    if (m_eNegoState == STATE_NEGOTIATED)
    {
        if (!MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_AUDIO) &&
                m_pAudioNego->GetNegotiatedRtpPort() != -1 /*PORT_NONE*/)
        {
            eNeedToMakeMedia =
                    (MEDIA_CONTENT_TYPE)((IMS_SINT32)eNeedToMakeMedia | MEDIA_TYPE_AUDIO);
        }

        if (!MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_VIDEO) &&
                m_pVideoNego->GetNegotiatedRtpPort() != -1 /*PORT_NONE*/)
        {
            eNeedToMakeMedia =
                    (MEDIA_CONTENT_TYPE)((IMS_SINT32)eNeedToMakeMedia | MEDIA_TYPE_VIDEO);
        }

        // TODO_MEDIA text
        // if (!MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_TEXT) &&
        //         m_pTextNego->GetNegotiatedRtpPort() != -1 /*PORT_NONE*/)
        // {
        //     eNeedToMakeMedia = (MEDIA_CONTENT_TYPE)((IMS_SINT32)eNeedToMakeMedia |
        //     MEDIA_TYPE_TEXT);
        // }

        IMS_TRACE_D("FormSDP() - Re-offer case. NeedToMakeMedia[%d]", eNeedToMakeMedia, 0, 0);
    }

    // -- Step 1. Get a list of media line
    IMSList<IMedia*> lstIMedia = GetIMediaListFromSession(pSession, eNeedToMakeMedia);

    // -- Step 2. Determine what descriptor will be used for each media
    IMediaDescriptor* pDescriptorForAudio = IMS_NULL;
    IMediaDescriptor* pDescriptorForVideo = IMS_NULL;
    // IMediaDescriptor* pDescriptorForText = IMS_NULL;  // TODO_MEDIA text

    if (m_eNegoState == STATE_IDLE)
    {
        IMS_UINT32 nMediaLineCounter = 0;
        if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_AUDIO) &&
                (nMediaLineCounter < lstIMedia.GetSize()))
        {
            pDescriptorForAudio = GetMediaDescriptor(lstIMedia.GetAt(nMediaLineCounter++));
        }
        if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_VIDEO) &&
                (nMediaLineCounter < lstIMedia.GetSize()))
        {
            pDescriptorForVideo = GetMediaDescriptor(lstIMedia.GetAt(nMediaLineCounter++));
        }
        // TODO_MEDIA text
        // if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_TEXT) &&
        //         (nMediaLineCounter < lstIMedia.GetSize()))
        // {
        //     pDescriptorForText = GetMediaDescriptor(lstIMedia.GetAt(nMediaLineCounter++));
        // }
    }
    else  // "Already received SDP" or "Re-INVITE" case
    {
        IMS_BOOL bAudioMLineSetted = IMS_FALSE;
        IMS_BOOL bVideoMLineSetted = IMS_FALSE;
        // IMS_BOOL    bTextMLineSetted = IMS_FALSE; // TODO_MEDIA text

        for (IMS_UINT32 i = 0; i < lstIMedia.GetSize(); i++)
        {
            IMediaDescriptor* pDescriptor = GetMediaDescriptor(lstIMedia.GetAt(i));
            if (pDescriptor == IMS_NULL)
            {
                return IMS_FALSE;
            }

            SdpMedia* pSDPMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();
            if (pSDPMedia == IMS_NULL)
            {
                return IMS_FALSE;
            }
            IMS_TRACE_D("FormSDP() - pSDPMedia[%" PFLS_x "], Type[%d]", pSDPMedia,
                    pSDPMedia->GetType(), 0);

            switch (pSDPMedia->GetType())
            {
                case SdpMedia::TYPE_AUDIO:
                    if (bAudioMLineSetted == IMS_FALSE)
                    {
                        pDescriptorForAudio = pDescriptor;
                        // if port 0, replace with another descriptor
                        if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                        {
                            continue;
                        }
                        bAudioMLineSetted = IMS_TRUE;
                    }
                    bAudioMLineSetted = IMS_TRUE;
                    break;
                case SdpMedia::TYPE_VIDEO:
                    if (bVideoMLineSetted == IMS_FALSE)
                    {
                        pDescriptorForVideo = pDescriptor;
                        // if port 0, replace with another descriptor
                        if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                        {
                            continue;
                        }
                        bVideoMLineSetted = IMS_TRUE;
                    }
                    break;
                case SdpMedia::TYPE_TEXT:
                    // TODO_MEDIA text
                    // if (bTextMLineSetted == IMS_FALSE)
                    // {
                    //     pDescriptorForText = pDescriptor;
                    //     // if port 0, replace with another descriptor
                    //     if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                    //     {
                    //         continue;
                    //     }
                    //     bTextMLineSetted = IMS_TRUE;
                    // }
                    break;
                default:
                    if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_AUDIO) &&
                            bAudioMLineSetted == IMS_FALSE)
                    {
                        pDescriptorForAudio = pDescriptor;
                        // if port 0, replace with another descriptor
                        if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                        {
                            pDescriptorForAudio = pDescriptor;
                            // if port 0, replace with another descriptor
                            if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                            {
                                continue;
                            }
                            bAudioMLineSetted = IMS_TRUE;
                            pSDPMedia->SetType(SdpMedia::TYPE_AUDIO);
                        }
                        bAudioMLineSetted = IMS_TRUE;
                        pSDPMedia->SetType(SdpMedia::TYPE_AUDIO);
                    }
                    else if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_VIDEO) &&
                            bVideoMLineSetted == IMS_FALSE)
                    {
                        pDescriptorForVideo = pDescriptor;
                        // if port 0, replace with another descriptor
                        if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                        {
                            continue;
                        }
                        bVideoMLineSetted = IMS_TRUE;
                        pSDPMedia->SetType(SdpMedia::TYPE_VIDEO);
                    }
                    // TODO_MEDIA text
                    // else if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_TEXT) &&
                    //         bTextMLineSetted == IMS_FALSE)
                    // {
                    //     pDescriptorForText = pDescriptor;
                    //     // if port 0, replace with another descriptor
                    //     if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                    //     {
                    //         continue;
                    //     }
                    //     bTextMLineSetted = IMS_TRUE;
                    //     pSDPMedia->SetType(SdpMedia::TYPE_TEXT);
                    // }
                    break;
            }

            IMS_TRACE_D("FormSDP() - m=audio[%d], m=video[%d], m=text[%d]", bAudioMLineSetted,
                    bVideoMLineSetted, 0 /*bTextMLineSetted*/);
            // TODO_MEDIA text
        }
    }

    // Send a "FormSDP" to each session
    IMS_SINT32 nTotalAs = 0;

    if (pDescriptorForAudio != IMS_NULL)
    {
        m_pAudioNego->SetSessionType(m_eSessionType);
        if (m_pAudioNego->FormSDP(GetNegoState(), pSession->GetSessionDescriptor(),
                    pDescriptorForAudio, eMediaType, (MEDIA_DIRECTION)eAudioDir) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "FormSDP() - Forming a m line of audio is failed", 0, 0, 0);
            return IMS_FALSE;
        }
        if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_AUDIO))
        {
            IMS_SINT32 nTmpAs = m_pAudioNego->GetMediaBandwidth();
            if (nTmpAs > 0)
            {
                nTotalAs += nTmpAs;
            }
        }
    }

    if (pDescriptorForVideo != IMS_NULL)
    {
        m_pVideoNego->SetSessionType(m_eSessionType);
        if (m_pVideoNego->FormSDP(GetNegoState(), pSession->GetSessionDescriptor(),
                    pDescriptorForVideo, eMediaType, (MEDIA_DIRECTION)eVideoDir) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "MediaNego::FormSDP() - Forming a m line of video is failed", 0, 0, 0);
            return IMS_FALSE;
        }
        if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO))
        {
            IMS_SINT32 nTmpAS = m_pVideoNego->GetMediaBandwidth();
            if (nTmpAS > 0)
            {
                nTotalAs += nTmpAS;
            }
        }
    }
    // TODO_MEDIA text
    // if (pDescriptorForText != IMS_NULL)
    // {
    //     m_pTextNego->SetSessionType(m_eSessionType);
    //     if (m_pTextNego->FormSDP(GetNegoState(), pSession->GetSessionDescriptor(),
    //             pDescriptorForText, eMediaType, (MEDIA_DIRECTION)eTextDir) == IMS_FALSE)
    //     {
    //         IMS_TRACE_E(0, "MediaNego::FormSDP() - Forming a m line of text is failed", 0, 0, 0);
    //         return IMS_FALSE;
    //     }
    //     if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT))
    //     {
    //         IMS_SINT32  nTmpAS = m_pTextNego->GetMediaBandwidth();
    //         if (nTmpAS > 0)
    //         {
    //             nTotalAs += nTmpAS;
    //         }
    //     }
    // }

    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    GetSlotId(), m_pMediaEnvironment->eServiceType);
    if (pMediaSessionConfig != IMS_NULL)
    {
        if (pMediaSessionConfig->IsSessionLevelBandwidth())
        {
            IMS_TRACE_D("FormSDP() - Session level bandwidth [%d]", nTotalAs, 0, 0);
            // remove session level bandwidth
            pSession->GetSessionDescriptor()->RemoveAllBandwidths();
            // add session level bandwidth
            pSession->GetSessionDescriptor()->AddBandwidth(SdpBandwidth::TYPE_AS, nTotalAs);
        }
    }

    // -- Step 4. Change the negotiation state ----------------------------------------------------
    switch (m_eNegoState)
    {
        case STATE_IDLE:
        case STATE_NEGOTIATED:
            m_eNegoState = STATE_OFFER_SENT;
            break;
        case STATE_OFFER_RECEIVED:
            m_eNegoState = STATE_NEGOTIATED;
            break;
        case STATE_OFFER_SENT:
        default:
            break;
    }
    IMS_TRACE_D("FormSDP() - Done. NegoState[%d]", m_eNegoState, 0, 0);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MediaNego::NegotiateSDP(IN ISession* pSession, OUT IMS_SINT32* eAudioDir,
        OUT IMS_SINT32* eVideoDir, OUT IMS_SINT32* eTextDir, OUT MediaNegoResult& errorReason)
{
    IMS_TRACE_I("NegotiateSDP(): pSession[%" PFLS_x "]", pSession, 0, 0);

    if (m_pMediaEnvironment == IMS_NULL || m_pMediaEnvironment->pIService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SetSessionType(pSession);

    if (m_pAudioNego != IMS_NULL)
    {
        m_pAudioNego->SetSessionType(m_eSessionType);
    }
    if (m_pVideoNego != IMS_NULL)
    {
        m_pVideoNego->SetSessionType(m_eSessionType);
    }
    // TODO_MEDIA text
    // if (m_pTextNego != IMS_NULL)
    // {
    //     m_pTextNego->SetSessionType(m_eSessionType);
    // }

    // -- Step 1. Get a list of media line --------------------------------------------------------
    IMSList<IMedia*> lstIMedia = pSession->GetMedia();

    // Code to support a case of receiving multiple m-line for same media type
    IMediaDescriptor* pNegotiatedAudioDescriptor = IMS_NULL;
    IMediaDescriptor* pNegotiatedVideoDescriptor = IMS_NULL;
    // IMediaDescriptor* pNegotiatedTextDescriptor = IMS_NULL; // TODO_MEDIA text

    IMS_TRACE_I("NegotiateSDP(): lstIMedia.GetSize()[%d]", lstIMedia.GetSize(), 0, 0);

    IMS_BOOL bNegoFailed = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < lstIMedia.GetSize(); i++)
    {
        IMediaDescriptor* pDescriptor = GetMediaDescriptor(lstIMedia.GetAt(i));
        if (pDescriptor == IMS_NULL)
        {
            IMS_TRACE_I("NegotiateSDP() - MediaDescriptor is null", 0, 0, 0);
            errorReason = ERROR_INVALID_DESCRIPTOR;
            bNegoFailed = IMS_TRUE;
            continue;
        }

        SdpMedia* pSDPMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();
        if (pSDPMedia == IMS_NULL)
        {
            IMS_TRACE_I("NegotiateSDP() - MediaDescriptionEx is null", 0, 0, 0);
            errorReason = ERROR_INVALID_DESCRIPTOR;
            bNegoFailed = IMS_TRUE;
            continue;
        }

        IMS_TRACE_I("NegotiateSDP() - pSDPMedia : Type[%d], Port[%d]", pSDPMedia->GetType(),
                pSDPMedia->GetPort(), 0);

        // Reject the peer media line with non-matching IP version
        if (pDescriptor->GetRemoteAddress().GetVersion() !=
                m_pMediaEnvironment->pIService->GetIpAddress().GetVersion())
        {
            if (pDescriptor->GetRemotePort() != 0)
            {
                IMS_TRACE_D("NegotiateSDP() - NOT Matched IP Version [%d / %d]",
                        pDescriptor->GetRemoteAddress().GetVersion(),
                        m_pMediaEnvironment->pIService->GetIpAddress().GetVersion(), 0);
                SetMediaDescriptorAsNotSupported(pDescriptor, pSDPMedia);
                errorReason = ERROR_IP_MISMATCH;
                bNegoFailed = IMS_TRUE;
                continue;
            }
        }

        // Negotiate audio m line
        switch (pSDPMedia->GetType())
        {
            case (SdpMedia::TYPE_AUDIO):
                if (m_pAudioNego == NULL)
                {
                    break;
                }
                if (pNegotiatedAudioDescriptor == NULL)
                {
                    if (m_pAudioNego->NegotiateSDP(GetNegoState(), m_bForking,
                                pSession->GetSessionDescriptor(), pDescriptor,
                                (MEDIA_DIRECTION*)eAudioDir) == IMS_TRUE)
                    {
                        pNegotiatedAudioDescriptor = pDescriptor;
                        errorReason = NO_ERROR;
                        bNegoFailed = IMS_FALSE;
                    }
                }
                else  // Negotiated descriptor is already exist
                {
                    SetMediaDescriptorAsNotSupported(pDescriptor, pSDPMedia);
                }
                break;
            case (SdpMedia::TYPE_VIDEO):
                if (m_pVideoNego == NULL)
                {
                    break;
                }
                if (pNegotiatedVideoDescriptor == NULL)
                {
                    if (m_pVideoNego->NegotiateSDP(GetNegoState(), m_bForking,
                                pSession->GetSessionDescriptor(), pDescriptor,
                                (MEDIA_DIRECTION*)eVideoDir) == IMS_TRUE)
                    {
                        pNegotiatedVideoDescriptor = pDescriptor;
                        errorReason = NO_ERROR;
                        bNegoFailed = IMS_FALSE;
                    }
                }
                else  // Negotiated descriptor is already exist
                {
                    SetMediaDescriptorAsNotSupported(pDescriptor, pSDPMedia);
                }
                break;
            case (SdpMedia::TYPE_TEXT):  // TODO_MEDIA text
                // if (m_pTextNego == NULL)
                // {
                //     break;
                // }
                // if (pNegotiatedTextDescriptor == NULL || (m_pTextNego->GetNegotiatedRtpPort() <=
                // 0
                //         || GetNegotiatedTextQuality() == TEXT_CODEC_NOT_USED))
                // {
                //     if (m_pTextNego->NegotiateSDP(GetNegoState(),
                //     pSession->GetSessionDescriptor(),
                //             pDescriptor, (MEDIA_DIRECTION*)eTextDir) == IMS_TRUE)
                //     {
                //         pNegotiatedTextDescriptor = pDescriptor;
                //         errorReason = NO_ERROR;
                //         bNegoFailed = IMS_FALSE;
                //     }
                // }
                // else    // Negotiated descriptor is already exist
                // {
                //     SetMediaDescriptorAsNotSupported(pDescriptor, pSDPMedia);
                // }
                break;
            default:
                IMS_TRACE_E(0, "NegotiateSDP() - Not supported media type[%d]",
                        pSDPMedia->GetType(), 0, 0);
                break;
        }
    }
    m_bForking = IMS_FALSE;
    // check the result of negitation
    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    GetSlotId(), m_pMediaEnvironment->eServiceType);

    if (pMediaSessionConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Audio nego result
    if (pNegotiatedAudioDescriptor != IMS_NULL)
    {
        if (MEDIA_IS_CONTAINED_THIS_TYPE(m_eSessionType, MEDIA_TYPE_AUDIO) &&
                GetNegotiatedAudioQuality() == AUDIO_CODEC_NOT_USED)
        {
            errorReason = ERROR_NO_CODEC_MATCHED;
            return IMS_FALSE;
        }
    }
    else
    {
        *eAudioDir = MEDIA_DIRECTION_INVALID;

        if (MEDIA_IS_CONTAINED_THIS_TYPE(m_eSessionType, MEDIA_TYPE_AUDIO))
        {
            IMS_TRACE_E(0, "NegotiateSDP() - m line of audio is failed", 0, 0, 0);
            errorReason = ERROR_NO_AUDIO;
            return IMS_FALSE;
        }
    }

    // Video nego result
    if (pNegotiatedVideoDescriptor != IMS_NULL)
    {
        // check "one-way video" or "hold"
        if (CheckOneWayVideoCall() == IMS_TRUE && m_pVideoNego != NULL)
        {
            // m_pVideoNego->SetOneWayVideoCallSetting(); // TODO_MEDIA deleted
        }

        if (m_pVideoNego->GetNegotiatedRtpPort() <= 0 ||
                GetNegotiatedVideoQuality() == VIDEO_RESOLUTION_NOT_USED)
        {
            *eVideoDir = MEDIA_DIRECTION_INVALID;
        }
    }
    else
    {
        *eVideoDir = MEDIA_DIRECTION_INVALID;

        // Check whether video is mandatory for session negotiation
        if (MEDIA_IS_CONTAINED_THIS_TYPE(m_eSessionType, MEDIA_TYPE_VIDEO))
        {
            IMS_TRACE_E(0, "NegotiateSDP() - m line of video is failed", 0, 0, 0);
            errorReason = ERROR_NO_VIDEO;
            return IMS_FALSE;
        }
    }
    // TODO_MEDIA text
    // Text nego result
    // if (pNegotiatedTextDescriptor != IMS_NULL)
    // {
    //     if (m_pTextNego->GetNegotiatedRtpPort() <= 0 ||
    //             GetNegotiatedTextQuality() == TEXT_CODEC_NOT_USED)
    //     {
    //         *eTextDir = MEDIA_DIRECTION_INVALID;
    //     }
    // }
    // else
    // {
    //     *eTextDir = MEDIA_DIRECTION_INVALID;

    //     // Check whether text is mandatory for session negotiation
    //     if (MEDIA_IS_CONTAINED_THIS_TYPE(m_eSessionType, MEDIA_TYPE_TEXT) &&
    //         MEDIA_IS_CONTAINED_THIS_TYPE(pMediaSessionConfig->GetMediaMandatoryNego(),
    //         MEDIA_TYPE_TEXT))
    //     {
    //         IMS_TRACE_E(0, "NegotiateSDP() - Negotiating a m line of text is failed", 0, 0, 0);
    //         errorReason = ERROR_NO_TEXT;
    //         return IMS_FALSE;
    //     }
    // }

    if (*eAudioDir == MEDIA_DIRECTION_INVALID && *eVideoDir == MEDIA_DIRECTION_INVALID &&
            *eTextDir == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_E(0, "NegotiateSDP() - There's NO negotiated media", 0, 0, 0);
        errorReason = ERROR_NO_CODEC_MATCHED;
        return IMS_FALSE;
    }

    // Change the negotiation state
    switch (m_eNegoState)
    {
        case STATE_IDLE:
            m_eNegoState = STATE_OFFER_RECEIVED;
            break;
        case STATE_NEGOTIATED:
            m_eNegoState = STATE_OFFER_RECEIVED;
            break;
        case STATE_OFFER_SENT:
            m_eNegoState = STATE_NEGOTIATED;
            break;
        case STATE_OFFER_RECEIVED:
        default:
            break;
    }

    IMS_TRACE_D("NegotiateSDP() - NegoState[%d]", GetNegoState(), 0, 0);
    IMS_TRACE_D("NegotiateSDP() - AudioDir[%d], VideoDir[%d], TextDir[%d]", *eAudioDir, *eVideoDir,
            *eTextDir);
    IMS_TRACE_D("NegotiateSDP() - AudioQuality[%d], VideoQuality[%d], TextQuality[%d]",
            GetNegotiatedAudioQuality(), GetNegotiatedVideoQuality(),
            0 /*GetNegotiatedTextQuality()*/);  // TODO_MEDIA text

    errorReason = NO_ERROR;
    return IMS_TRUE;
}

PUBLIC
void MediaNego::FinalizeSDP(IN ISession* pSession)
{
    if (pSession == IMS_NULL)
    {
        return;
    }

    IMS_BOOL bNegotiated = IMS_FALSE;
    IMS_TRACE_D("FinalizeSDP - enter ISessionDescriptor[%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSession->GetSessionDescriptor()), 0, 0);
    if (m_pAudioNego != IMS_NULL)
    {
        m_pAudioNego->FinalizeSDP(
                reinterpret_cast<IMS_SINTP>(pSession->GetSessionDescriptor()), m_eNegoState);
        if (m_pAudioNego->GetNegotiatedCodec() != AUDIO_CODEC_NONE)
        {
            bNegotiated = IMS_TRUE;
        }
    }
    if (m_pVideoNego != IMS_NULL)
    {
        m_pVideoNego->FinalizeSDP(
                reinterpret_cast<IMS_SINTP>(pSession->GetSessionDescriptor()), m_eNegoState);

        if (m_pVideoNego->GetNegotiatedResolution() != VIDEO_RESOLUTION_INVALID)
            bNegotiated = IMS_TRUE;
    }
    // TODO_MEDIA text
    // if (m_pTextNego != IMS_NULL)
    // {
    //     m_pTextNego->FinalizeSDP(
    //             reinterpret_cast<IMS_SINTP> (pSession->GetSessionDescriptor()), m_eNegoState);

    //     if (m_pTextNego->GetNegotiatedCodec() != TEXT_CODEC_NONE)
    //         bNegotiated = IMS_TRUE;
    // }

    if (bNegotiated == IMS_TRUE)
    {
        SetNegoState(STATE_NEGOTIATED);
    }
    else
    {
        SetNegoState(STATE_IDLE);
    }

    IMSList<IMedia*> lstIMedia = pSession->GetMedia();
    for (IMS_UINT32 i = 0; i < lstIMedia.GetSize(); i++)
    {
        IMedia* pIMedia = lstIMedia.GetAt(i);
        if (pIMedia == IMS_NULL)
        {
            return;
        }
        if (pIMedia->GetState() == IMedia::STATE_DELETED)
        {
            if (pSession->RemoveMedia(pIMedia) == IMS_SUCCESS)
            {
                IMS_TRACE_D("FinalizeSDP() remove IMedia[%d / %" PFLS_x "] SUCCESS", i, pIMedia, 0);
            }
        }
    }
}

PUBLIC
AUDIO_CODEC MediaNego::GetNegotiatedAudioQuality(void)
{
    if (m_pAudioNego != IMS_NULL)
    {
        AUDIO_CODEC eAudioQuality = m_pAudioNego->GetNegotiatedCodec();
        if (eAudioQuality == AUDIO_CODEC_NONE)
        {
            return AUDIO_CODEC_NOT_USED;
        }
        else
        {
            return eAudioQuality;
        }
    }
    return AUDIO_CODEC_NOT_USED;
}

PUBLIC
VIDEO_RESOLUTION MediaNego::GetNegotiatedVideoQuality(void)
{
    if (m_pVideoNego != IMS_NULL)
    {
        VIDEO_RESOLUTION eNegotiatedResolution = m_pVideoNego->GetNegotiatedResolution();
        if (eNegotiatedResolution == VIDEO_RESOLUTION_INVALID)
        {
            return VIDEO_RESOLUTION_NOT_USED;
        }
        // this case is left for keeping legacy I/F with UI. it need to change later
        else if (eNegotiatedResolution == VIDEO_RESOLUTION_QCIF_PR ||
                eNegotiatedResolution == VIDEO_RESOLUTION_QCIF_LS)
        {
            return VIDEO_RESOLUTION_QCIF_LS;
        }
        else
        {
            return eNegotiatedResolution;
        }
    }
    return VIDEO_RESOLUTION_NOT_USED;
}

// TODO_MEDIA text
// PUBLIC
// TEXT_CODEC MediaNego::GetNegotiatedTextQuality(void)
// {
//     if (m_pTextNego != IMS_NULL)
//     {
//         TEXT_CODEC eTextQuality = m_pTextNego->GetNegotiatedCodec();

//         if (eTextQuality == TEXT_CODEC_NONE)
//         {
//             return TEXT_CODEC_NOT_USED;
//         }
//         else
//         {
//             return eTextQuality;
//         }
//     }

//     return TEXT_CODEC_NOT_USED;
// }

PUBLIC
MEDIA_DIRECTION MediaNego::GetNegotiatedAudioDirection(void)
{
    return (m_pAudioNego != IMS_NULL) ? m_pAudioNego->GetNegotiatedDirection()
                                      : MEDIA_DIRECTION_INVALID;
}

PUBLIC
MEDIA_DIRECTION MediaNego::GetNegotiatedVideoDirection(void)
{
    return (m_pVideoNego != IMS_NULL) ? m_pVideoNego->GetNegotiatedDirection()
                                      : MEDIA_DIRECTION_INVALID;
}

// TODO_MEDIA text
// PUBLIC MEDIA_DIRECTION MediaNego::GetNegotiatedTextDirection(void)
// {
//     return (m_pTextNego != IMS_NULL) ? m_pTextNego->GetNegotiatedDirection() :
//             MEDIA_DIRECTION_INVALID;
// }

PUBLIC
IMediaDescriptor* MediaNego::GetMediaDescriptor(IN IMedia* pIMedia)
{
    if (pIMedia == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (pIMedia->GetUpdateState() == IMedia::UPDATE_MODIFIED)
    {
        // After received re-invite
        IMedia* pIMediaProposal = pIMedia->GetProposal();
        if (pIMediaProposal == IMS_NULL)
        {
            return IMS_NULL;
        }

        return pIMediaProposal->GetMediaDescriptor();
    }
    else
    {
        return pIMedia->GetMediaDescriptor();
    }
}

PUBLIC
IMS_BOOL MediaNego::IsForking()
{
    return m_bForking;
}

PRIVATE
IMSList<IMedia*> MediaNego::GetIMediaListFromSession(
        IN ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType)
{
    if (pSession == IMS_NULL || m_pMediaEnvironment == IMS_NULL)
    {
        return IMSList<IMedia*>();
    }

    IMS_TRACE_D("GetIMediaListFromSession() - NegoState[%d], ServiceType[%d], MediaType[%d]",
            m_eNegoState, m_pMediaEnvironment->eServiceType, eMediaType);

    IMS_UINT32 nCountMediaRequired = 0;
    IMSList<IMedia*> objIMediaList = pSession->GetMedia();

    if (GetNegoState() == STATE_IDLE || GetNegoState() == STATE_NEGOTIATED)
    {
        switch (m_pMediaEnvironment->eServiceType)
        {
            case MEDIA_SERVICE_DEFAULT:  // FALL_THROUGH
            case MEDIA_SERVICE_EMERGENCY:
                // Depends on the media type
                if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_AUDIO))
                {
                    nCountMediaRequired++;
                }
                if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO))
                {
                    nCountMediaRequired++;
                }
                if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT))
                {
                    nCountMediaRequired++;
                }
                break;
            default:
                break;
        }

        IMS_TRACE_I("GetIMediaListFromSession()- objIMediaList.GetSize()[%d], \
                nCountMediaRequired[%d], MediaType[%d]",
                objIMediaList.GetSize(), nCountMediaRequired, eMediaType);
    }

    if (nCountMediaRequired > objIMediaList.GetSize())
    {
        for (IMS_UINT32 i = nCountMediaRequired - objIMediaList.GetSize(); i > 0; i--)
        {
            IMS_TRACE_D("GetIMediaListFromSession()- Create Media", 0, 0, 0);
            pSession->CreateMedia(ImsCore::MEDIA_STREAM, IMedia::DIRECTION_SEND_RECEIVE, 1);
        }
    }

    return pSession->GetMedia();
}

PRIVATE
void MediaNego::SetSessionType(IN ISession* pSession)
{
    if (pSession == IMS_NULL)
    {
        return;
    }

    // Check the requested media type
    IMSList<IMedia*> lstIMedia = pSession->GetMedia();
    IMS_UINT32 eMediaType = 0;

    for (IMS_UINT32 i = 0; i < lstIMedia.GetSize(); i++)
    {
        IMediaDescriptor* pDescriptor = GetMediaDescriptor(lstIMedia.GetAt(i));
        if (pDescriptor == IMS_NULL)
        {
            return;
        }

        SdpMedia* pSDPMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();
        if (pSDPMedia != IMS_NULL && pSDPMedia->GetType() == SdpMedia::TYPE_AUDIO &&
                pSDPMedia->GetPort() != -1)
        {
            eMediaType |= (IMS_UINT32)MEDIA_TYPE_AUDIO;
        }
        else if (pSDPMedia != IMS_NULL && pSDPMedia->GetType() == SdpMedia::TYPE_VIDEO &&
                pSDPMedia->GetPort() != -1)
        {
            eMediaType |= (IMS_UINT32)MEDIA_TYPE_VIDEO;
        }
        else if (pSDPMedia != IMS_NULL && pSDPMedia->GetType() == SdpMedia::TYPE_TEXT &&
                pSDPMedia->GetPort() != -1)
        {
            eMediaType |= (IMS_UINT32)MEDIA_TYPE_TEXT;
        }
    }

    m_eSessionType = (MEDIA_CONTENT_TYPE)eMediaType;

    IMS_TRACE_I("SetNegoRequestedMediaType()-RESULT[%d]", m_eSessionType, 0, 0);
}

PRIVATE
void MediaNego::SetMediaDescriptorAsNotSupported(
        IN IMediaDescriptor* pDescriptor, IN SdpMedia* pSDPMedia)
{
    IMS_TRACE_I("SetMediaDescriptorAsNotSupported() - Clear descriptor[%" PFLS_x "]", pDescriptor,
            0, 0);

    // clean attr & bandwidth lines
    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    IMSList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // set RTP Port to zero
    pSDPMedia->SetPort(0);
    IMS_TRACE_I("SetMediaDescriptorAsNotSupported() - SetPort [0]", 0, 0, 0);

    pDescriptor->SetMediaDescription(
            pSDPMedia->GetType(), 0, pSDPMedia->GetTransportProtocol(), pSDPMedia->GetFormats());
}

PRIVATE
IMS_BOOL MediaNego::CheckOneWayVideoCall()
{
    IMS_TRACE_I("CheckOneWayVideoCall() Oneway video call check", 0, 0, 0);

    MEDIA_DIRECTION eAudioDir = GetNegotiatedAudioDirection();
    MEDIA_DIRECTION eVideoDir = GetNegotiatedVideoDirection();

    // check one-way video call case..
    if ((eAudioDir == MEDIA_DIRECTION_SEND_RECEIVE) &&
            ((eVideoDir == MEDIA_DIRECTION_RECEIVE) || (eVideoDir == MEDIA_DIRECTION_SEND)))
    {
        IMS_TRACE_I("CheckOneWayVideoCall() Oneway video call - VideoDir[%d]", eVideoDir, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
