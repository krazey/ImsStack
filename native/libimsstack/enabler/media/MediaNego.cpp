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
#include "ISdpReader.h"
#include "ISessionDescriptor.h"

#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaConfigUtil.h"
#include "config/VideoConfiguration.h"
#include "config/TextConfiguration.h"
#include "MediaNego.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
MediaNego::MediaNego(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_eNegoState(STATE_IDLE),
        m_pAudioNego(IMS_NULL),
        m_pVideoNego(IMS_NULL),
        m_pTextNego(IMS_NULL),
        m_pMediaEnvironment(IMS_NULL),
        m_eSessionType(MEDIA_TYPE_INVALID),
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

    if (m_pTextNego != IMS_NULL)
    {
        delete m_pTextNego;
    }
}

PUBLIC
void MediaNego::CreateProfile(IN MediaEnvironment* pMediaEnvironment)
{
    if (pMediaEnvironment == IMS_NULL)
    {
        return;
    }

    m_pMediaEnvironment = pMediaEnvironment;

    IMS_TRACE_D("CreateProfile() - eServiceType[%d]", pMediaEnvironment->eServiceType, 0, 0);
    m_pAudioNego = new AudioNego(GetSlotId());
    m_pVideoNego = new VideoNego(GetSlotId());
    m_pTextNego = new TextNego(GetSlotId());

    m_pAudioNego->CreateProfiles(m_pMediaEnvironment,
            MediaConfigUtil::GetAudioConfig(GetSlotId(), m_pMediaEnvironment->eServiceType));

    m_pVideoNego->CreateProfiles(m_pMediaEnvironment,
            MediaConfigUtil::GetVideoConfig(GetSlotId(), m_pMediaEnvironment->eServiceType));

    m_pTextNego->CreateProfiles(m_pMediaEnvironment,
            MediaConfigUtil::GetTextConfig(GetSlotId(), m_pMediaEnvironment->eServiceType));
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
        *m_pAudioNego = *pMediaNego->GetAudioNego();
    }

    if (m_pVideoNego != IMS_NULL)
    {
        *m_pVideoNego = *pMediaNego->GetVideoNego();
    }

    if (m_pTextNego != IMS_NULL)
    {
        *m_pTextNego = *pMediaNego->GetTextNego();
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MediaNego::FormSdp(OUT ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType,
        IN IMS_SINT32 nAudioDirection, IN IMS_SINT32 nVideoDirection, IN IMS_SINT32 nTextDirection,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormSdp(): eMediaType[%d], eNegoState[%x]", eMediaType, m_eNegoState, 0);
    IMS_TRACE_I("FormSdp() - DIR = Audio[%d], Video[%d], Text[%d]", nAudioDirection,
            nVideoDirection, nTextDirection);
    IMS_TRACE_D("FormSdp() - eMediaType [%d], bEnforceReofferMode[%d]", eMediaType,
            bEnforceReofferMode, 0);

    if (m_pMediaEnvironment == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_eNegoState == STATE_OFFER_SENT)
    {
        return IMS_FALSE;
    }

    m_eSessionType = eMediaType;

    if ((MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_AUDIO) && m_pAudioNego == IMS_NULL) ||
            (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO) &&
                    m_pVideoNego == IMS_NULL) ||
            (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT) && m_pTextNego == IMS_NULL))
    {
        IMS_TRACE_E(0, "FormSdp() - INVALID : eMediaType[%d]", eMediaType, 0, 0);
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

        if (!MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_TEXT) &&
                m_pTextNego->GetNegotiatedRtpPort() != -1 /*PORT_NONE*/)
        {
            eNeedToMakeMedia = (MEDIA_CONTENT_TYPE)((IMS_SINT32)eNeedToMakeMedia | MEDIA_TYPE_TEXT);
        }

        IMS_TRACE_D("FormSdp() - Re-offer case. NeedToMakeMedia[%d]", eNeedToMakeMedia, 0, 0);
    }

    // Get a list of media line
    ImsList<IMedia*> lstIMedia = GetIMediaListFromSession(pSession, eNeedToMakeMedia);

    // Determine what descriptor will be used for each media
    IMediaDescriptor* pDescriptorForAudio = IMS_NULL;
    IMediaDescriptor* pDescriptorForVideo = IMS_NULL;
    IMediaDescriptor* pDescriptorForText = IMS_NULL;

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

        if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_TEXT) &&
                (nMediaLineCounter < lstIMedia.GetSize()))
        {
            pDescriptorForText = GetMediaDescriptor(lstIMedia.GetAt(nMediaLineCounter++));
        }
    }
    else  // "Already received SDP" or "Re-INVITE" case
    {
        IMS_BOOL bAudioMLineSetted = IMS_FALSE;
        IMS_BOOL bVideoMLineSetted = IMS_FALSE;
        IMS_BOOL bTextMLineSetted = IMS_FALSE;

        for (IMS_UINT32 i = 0; i < lstIMedia.GetSize(); i++)
        {
            IMediaDescriptor* pDescriptor = GetMediaDescriptor(lstIMedia.GetAt(i));

            if (pDescriptor == IMS_NULL)
            {
                return IMS_FALSE;
            }

            SdpMedia* pSDPMedia = const_cast<SdpMedia*>(pDescriptor->GetMediaDescriptionEx());

            if (pSDPMedia == IMS_NULL)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("FormSdp() - pSDPMedia[%" PFLS_x "], Type[%d]", pSDPMedia,
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
                    if (bTextMLineSetted == IMS_FALSE)
                    {
                        pDescriptorForText = pDescriptor;

                        // if port 0, replace with another descriptor
                        if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                        {
                            continue;
                        }

                        bTextMLineSetted = IMS_TRUE;
                    }
                    break;
                default:
                    if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_AUDIO) &&
                            bAudioMLineSetted == IMS_FALSE)
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

                    if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_VIDEO) &&
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

                    if (MEDIA_IS_CONTAINED_THIS_TYPE(eNeedToMakeMedia, MEDIA_TYPE_TEXT) &&
                            bTextMLineSetted == IMS_FALSE)
                    {
                        pDescriptorForText = pDescriptor;
                        // if port 0, replace with another descriptor
                        if (GetNegoState() == STATE_OFFER_RECEIVED && pSDPMedia->GetPort() == 0)
                        {
                            continue;
                        }

                        bTextMLineSetted = IMS_TRUE;
                        pSDPMedia->SetType(SdpMedia::TYPE_TEXT);
                    }
                    break;
            }

            IMS_TRACE_D("FormSdp() - m=audio[%d], m=video[%d], m=text[%d]", bAudioMLineSetted,
                    bVideoMLineSetted, bTextMLineSetted);
        }
    }

    // Send a "FormSdp" to each session
    IMS_SINT32 nTotalAs = 0;

    if (pDescriptorForAudio != IMS_NULL)
    {
        if (m_pAudioNego->FormSdp(GetNegoState(), pSession->GetSessionDescriptor(),
                    pDescriptorForAudio, (MEDIA_DIRECTION)nAudioDirection,
                    MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_AUDIO) == IMS_FALSE
                            ? IMS_TRUE
                            : IMS_FALSE,
                    bEnforceReofferMode) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "FormSdp() - Forming a m line of audio is failed", 0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            IMS_SINT32 nTmpAs = m_pAudioNego->GetNegotiatedBandwidth();

            if (nTmpAs > 0)
            {
                nTotalAs += nTmpAs;
            }
        }
    }

    if (pDescriptorForVideo != IMS_NULL)
    {
        if (m_pVideoNego->FormSdp(GetNegoState(), pSession->GetSessionDescriptor(),
                    pDescriptorForVideo, (MEDIA_DIRECTION)nVideoDirection,
                    MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO) == IMS_FALSE
                            ? IMS_TRUE
                            : IMS_FALSE,
                    bEnforceReofferMode) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "MediaNego::FormSdp() - Forming a m line of video is failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO))
        {
            IMS_SINT32 nTmpAS = m_pVideoNego->GetNegotiatedBandwidth();

            if (nTmpAS > 0)
            {
                nTotalAs += nTmpAS;
            }
        }
    }

    if (pDescriptorForText != IMS_NULL)
    {
        if (m_pTextNego->FormSdp(GetNegoState(), pSession->GetSessionDescriptor(),
                    pDescriptorForText, (MEDIA_DIRECTION)nTextDirection,
                    MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT) == IMS_FALSE
                            ? IMS_TRUE
                            : IMS_FALSE,
                    bEnforceReofferMode) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "MediaNego::FormSdp() - Forming a m line of text is failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT))
        {
            IMS_SINT32 nTmpAS = m_pTextNego->GetNegotiatedBandwidth();

            if (nTmpAS > 0)
            {
                nTotalAs += nTmpAS;
            }
        }
    }

    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    GetSlotId(), m_pMediaEnvironment->eServiceType);

    if (pMediaSessionConfig != IMS_NULL)
    {
        if (pMediaSessionConfig->IsSessionLevelBandwidth())
        {
            IMS_TRACE_D("FormSdp() - Session level bandwidth [%d]", nTotalAs, 0, 0);
            // remove session level bandwidth
            pSession->GetSessionDescriptor()->RemoveAllBandwidths();
            // add session level bandwidth
            pSession->GetSessionDescriptor()->AddBandwidth(SdpBandwidth::TYPE_AS, nTotalAs);
        }
    }

    // Change the negotiation state
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
    IMS_TRACE_D("FormSdp() - Done. NegoState[%d]", m_eNegoState, 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL MEDIA_CONTENT_TYPE MediaNego::GetSupportedMediaTypesFromSdp(IN ISession* pSession)
{
    IMS_TRACE_I("GetSupportedMediaTypesFromSdp(): pSession[%" PFLS_x "]", pSession, 0, 0);

    if (pSession == IMS_NULL)
    {
        return MEDIA_TYPE_INVALID;
    }

    MEDIA_CONTENT_TYPE eSupportedMediaType = MEDIA_TYPE_INVALID;

    ISdpReader* piSdpReader = pSession->GetRemoteMediaCapabilities();
    if (piSdpReader == IMS_NULL)
    {
        return MEDIA_TYPE_INVALID;
    }

    if (piSdpReader->GetMediaDescriptors().IsEmpty())
    {
        return MEDIA_TYPE_INVALID;
    }
    const ImsList<IMediaDescriptor*>& objMediaDescriptors = piSdpReader->GetMediaDescriptors();

    IMS_TRACE_I("GetSupportedMediaTypesFromSdp(): objMediaDescriptors size[%d]",
            objMediaDescriptors.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < objMediaDescriptors.GetSize(); i++)
    {
        IMediaDescriptor* pDescriptor = objMediaDescriptors.GetAt(i);

        if (pDescriptor == IMS_NULL)
        {
            IMS_TRACE_I("GetSupportedMediaTypesFromSdp() - MediaDescriptor is null", 0, 0, 0);
            continue;
        }

        const SdpMedia* pSDPMedia = pDescriptor->GetMediaDescriptionEx();

        if (pSDPMedia == IMS_NULL)
        {
            IMS_TRACE_I("GetSupportedMediaTypesFromSdp() - MediaDescriptionEx is null", 0, 0, 0);
            continue;
        }

        IMS_TRACE_I("GetSupportedMediaTypesFromSdp() - pSDPMedia : Type[%d], Port[%d]",
                pSDPMedia->GetType(), pSDPMedia->GetPort(), 0);

        // Negotiate audio m line
        switch (pSDPMedia->GetType())
        {
            case (SdpMedia::TYPE_AUDIO):
                if (m_pAudioNego == IMS_NULL)
                {
                    break;
                }

                if (!MEDIA_IS_CONTAINED_THIS_TYPE(eSupportedMediaType, MEDIA_TYPE_AUDIO))
                {
                    if (m_pAudioNego->IsMediaCodecFromSdpSupported(
                                piSdpReader->GetSessionDescriptor(), pDescriptor))
                    {
                        eSupportedMediaType =
                                (MEDIA_CONTENT_TYPE)(eSupportedMediaType | MEDIA_TYPE_AUDIO);
                    }
                }
                break;
            case (SdpMedia::TYPE_VIDEO):
                if (m_pVideoNego == IMS_NULL)
                {
                    break;
                }

                if (!MEDIA_IS_CONTAINED_THIS_TYPE(eSupportedMediaType, MEDIA_TYPE_VIDEO))
                {
                    if (m_pVideoNego->IsMediaCodecFromSdpSupported(
                                piSdpReader->GetSessionDescriptor(), pDescriptor))
                    {
                        eSupportedMediaType =
                                (MEDIA_CONTENT_TYPE)(eSupportedMediaType | MEDIA_TYPE_VIDEO);
                    }
                }
                break;
            case (SdpMedia::TYPE_TEXT):
                if (m_pTextNego == IMS_NULL)
                {
                    break;
                }

                if (!MEDIA_IS_CONTAINED_THIS_TYPE(eSupportedMediaType, MEDIA_TYPE_TEXT))
                {
                    if (m_pTextNego->IsMediaCodecFromSdpSupported(
                                piSdpReader->GetSessionDescriptor(), pDescriptor))
                    {
                        eSupportedMediaType =
                                (MEDIA_CONTENT_TYPE)(eSupportedMediaType | MEDIA_TYPE_TEXT);
                    }
                }
                break;
            default:
                IMS_TRACE_D("GetSupportedMediaTypesFromSdp() - Not supported media type", 0, 0, 0);
                break;
        }
    }

    IMS_TRACE_I("GetSupportedMediaTypesFromSdp() - return[%d]", eSupportedMediaType, 0, 0);
    return eSupportedMediaType;
}

PUBLIC
IMS_BOOL MediaNego::NegotiateSdp(IN ISession* pSession, OUT IMS_SINT32& nAudioDirection,
        OUT IMS_SINT32& nVideoDirection, OUT IMS_SINT32& nTextDirection,
        OUT MediaNegoResult& errorReason)
{
    IMS_TRACE_I("NegotiateSdp(): pSession[%" PFLS_x "]", pSession, 0, 0);

    if (m_pMediaEnvironment == IMS_NULL || m_pMediaEnvironment->pIService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SetSessionType(pSession);

    // get a list of media line
    ImsList<IMedia*> lstIMedia = pSession->GetMedia();

    // Code to support a case of receiving multiple m-line for same media type
    IMediaDescriptor* pNegotiatedAudioDescriptor = IMS_NULL;
    IMediaDescriptor* pNegotiatedVideoDescriptor = IMS_NULL;
    IMediaDescriptor* pNegotiatedTextDescriptor = IMS_NULL;

    IMS_TRACE_I("NegotiateSdp(): lstIMedia.GetSize()[%d]", lstIMedia.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < lstIMedia.GetSize(); i++)
    {
        IMediaDescriptor* pDescriptor = GetMediaDescriptor(lstIMedia.GetAt(i));

        if (pDescriptor == IMS_NULL)
        {
            IMS_TRACE_I("NegotiateSdp() - MediaDescriptor is null", 0, 0, 0);
            errorReason = ERROR_INVALID_DESCRIPTOR;
            continue;
        }

        SdpMedia* pSDPMedia = const_cast<SdpMedia*>(pDescriptor->GetMediaDescriptionEx());

        if (pSDPMedia == IMS_NULL)
        {
            IMS_TRACE_I("NegotiateSdp() - MediaDescriptionEx is null", 0, 0, 0);
            errorReason = ERROR_INVALID_DESCRIPTOR;
            continue;
        }

        IMS_TRACE_I("NegotiateSdp() - pSDPMedia : Type[%d], Port[%d]", pSDPMedia->GetType(),
                pSDPMedia->GetPort(), 0);

        // Reject the peer media line with non-matching IP version
        if (pDescriptor->GetRemoteAddress().GetVersion() !=
                m_pMediaEnvironment->pIService->GetIpAddress().GetVersion())
        {
            if (pDescriptor->GetRemotePort() != 0)
            {
                IMS_TRACE_D("NegotiateSdp() - NOT Matched IP Version [%d / %d]",
                        pDescriptor->GetRemoteAddress().GetVersion(),
                        m_pMediaEnvironment->pIService->GetIpAddress().GetVersion(), 0);
                SetMediaDescriptorAsNotSupported(pDescriptor, pSDPMedia);
                errorReason = ERROR_IP_MISMATCH;
                continue;
            }
        }

        // Negotiate audio m line
        switch (pSDPMedia->GetType())
        {
            case (SdpMedia::TYPE_AUDIO):
                if (m_pAudioNego == IMS_NULL)
                {
                    break;
                }

                if (pNegotiatedAudioDescriptor == IMS_NULL)
                {
                    m_pAudioNego->NegotiateSdp(GetNegoState(), pSession->GetSessionDescriptor(),
                            pDescriptor, nAudioDirection);
                    pNegotiatedAudioDescriptor = pDescriptor;
                }
                else  // Negotiated descriptor is already exist
                {
                    SetMediaDescriptorAsNotSupported(pDescriptor, pSDPMedia);
                }
                break;
            case (SdpMedia::TYPE_VIDEO):
                if (m_pVideoNego == IMS_NULL)
                {
                    break;
                }

                if (pNegotiatedVideoDescriptor == IMS_NULL)
                {
                    m_pVideoNego->NegotiateSdp(GetNegoState(), pSession->GetSessionDescriptor(),
                            pDescriptor, nVideoDirection);
                    pNegotiatedVideoDescriptor = pDescriptor;
                }
                else  // Negotiated descriptor is already exist
                {
                    SetMediaDescriptorAsNotSupported(pDescriptor, pSDPMedia);
                }
                break;
            case (SdpMedia::TYPE_TEXT):
                if (m_pTextNego == IMS_NULL)
                {
                    break;
                }

                if (pNegotiatedTextDescriptor == IMS_NULL)
                {
                    m_pTextNego->NegotiateSdp(GetNegoState(), pSession->GetSessionDescriptor(),
                            pDescriptor, nTextDirection);
                    pNegotiatedTextDescriptor = pDescriptor;
                }
                else  // Negotiated descriptor is already exist
                {
                    SetMediaDescriptorAsNotSupported(pDescriptor, pSDPMedia);
                }
                break;
            default:
                IMS_TRACE_E(0, "NegotiateSdp() - Not supported media type[%d]",
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
        nAudioDirection = MEDIA_DIRECTION_INVALID;

        if (MEDIA_IS_CONTAINED_THIS_TYPE(m_eSessionType, MEDIA_TYPE_AUDIO))
        {
            IMS_TRACE_E(0, "NegotiateSdp() - m line of audio is failed", 0, 0, 0);
            errorReason = ERROR_NO_AUDIO;
            return IMS_FALSE;
        }
    }

    if (nAudioDirection == MEDIA_DIRECTION_INVALID && nVideoDirection == MEDIA_DIRECTION_INVALID &&
            nTextDirection == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_E(0, "NegotiateSdp() - There's NO negotiated media", 0, 0, 0);
        errorReason = ERROR_NO_CODEC_MATCHED;
        return IMS_FALSE;
    }

    // Change the negotiation state
    switch (m_eNegoState)
    {
        case STATE_IDLE:  // FALL-THROUGH
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

    IMS_TRACE_D("NegotiateSdp() - NegoState[%d]", GetNegoState(), 0, 0);
    IMS_TRACE_D("NegotiateSdp() - AudioDir[%d], VideoDir[%d], TextDir[%d]", nAudioDirection,
            nVideoDirection, nTextDirection);
    IMS_TRACE_D("NegotiateSdp() - AudioQuality[%d], VideoQuality[%d], TextQuality[%d]",
            GetNegotiatedAudioQuality(), GetNegotiatedVideoQuality(), GetNegotiatedTextQuality());

    errorReason = NO_ERROR;
    return IMS_TRUE;
}

PUBLIC
void MediaNego::FinalizeSdp(IN ISession* pSession)
{
    if (pSession == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("FinalizeSdp - enter ISessionDescriptor[%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSession->GetSessionDescriptor()), 0, 0);

    IMS_BOOL bNegotiated = IMS_FALSE;

    if (m_pAudioNego != IMS_NULL)
    {
        m_pAudioNego->FinalizeSdp(pSession->GetSessionDescriptor(), m_eNegoState);

        if (m_pAudioNego->GetNegotiatedCodec() != AUDIO_CODEC_NONE)
        {
            bNegotiated = IMS_TRUE;
        }
    }

    if (m_pVideoNego != IMS_NULL)
    {
        m_pVideoNego->FinalizeSdp(pSession->GetSessionDescriptor(), m_eNegoState);

        if (m_pVideoNego->GetNegotiatedResolution() != VIDEO_RESOLUTION_INVALID)
            bNegotiated = IMS_TRUE;
    }

    if (m_pTextNego != IMS_NULL)
    {
        m_pTextNego->FinalizeSdp(pSession->GetSessionDescriptor(), m_eNegoState);

        if (m_pTextNego->GetNegotiatedCodec() != TEXT_CODEC_NONE)
        {
            bNegotiated = IMS_TRUE;
        }
    }

    if (bNegotiated == IMS_TRUE)
    {
        SetNegoState(STATE_NEGOTIATED);
    }
    else
    {
        SetNegoState(STATE_IDLE);
    }

    ImsList<IMedia*> lstIMedia = pSession->GetMedia();

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
                IMS_TRACE_D("FinalizeSdp() remove IMedia[%d / %" PFLS_x "] SUCCESS", i, pIMedia, 0);
            }
        }
    }
}

PUBLIC
AUDIO_CODEC MediaNego::GetNegotiatedAudioQuality()
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
VIDEO_RESOLUTION MediaNego::GetNegotiatedVideoQuality()
{
    if (m_pVideoNego == IMS_NULL || m_pVideoNego->GetRemotePort() <= 0)
    {
        return VIDEO_RESOLUTION_NOT_USED;
    }

    VIDEO_RESOLUTION eNegotiatedResolution = m_pVideoNego->GetNegotiatedResolution();

    if (eNegotiatedResolution == VIDEO_RESOLUTION_INVALID)
    {
        return VIDEO_RESOLUTION_NOT_USED;
    }

    return eNegotiatedResolution;
}

PUBLIC
TEXT_CODEC MediaNego::GetNegotiatedTextQuality()
{
    if (m_pTextNego != IMS_NULL)
    {
        TEXT_CODEC eQuality = m_pTextNego->GetNegotiatedCodec();

        if (eQuality == TEXT_CODEC_NONE)
        {
            return TEXT_CODEC_NOT_USED;
        }
        else
        {
            return eQuality;
        }
    }

    return TEXT_CODEC_NOT_USED;
}

PUBLIC
MEDIA_DIRECTION MediaNego::GetNegotiatedAudioDirection()
{
    return (m_pAudioNego != IMS_NULL) ? m_pAudioNego->GetNegotiatedDirection()
                                      : MEDIA_DIRECTION_INVALID;
}

PUBLIC
MEDIA_DIRECTION MediaNego::GetNegotiatedVideoDirection()
{
    return (m_pVideoNego != IMS_NULL) ? m_pVideoNego->GetNegotiatedDirection()
                                      : MEDIA_DIRECTION_INVALID;
}

PUBLIC
MEDIA_DIRECTION MediaNego::GetNegotiatedTextDirection()
{
    return (m_pTextNego != IMS_NULL) ? m_pTextNego->GetNegotiatedDirection()
                                     : MEDIA_DIRECTION_INVALID;
}

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
ImsList<IMedia*> MediaNego::GetIMediaListFromSession(
        IN ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType)
{
    if (pSession == IMS_NULL || m_pMediaEnvironment == IMS_NULL)
    {
        return ImsList<IMedia*>();
    }

    IMS_TRACE_D("GetIMediaListFromSession() - NegoState[%d], ServiceType[%d], MediaType[%d]",
            m_eNegoState, m_pMediaEnvironment->eServiceType, eMediaType);

    IMS_UINT32 nCountMediaRequired = 0;
    ImsList<IMedia*> objIMediaList = pSession->GetMedia();

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
    ImsList<IMedia*> lstIMedia = pSession->GetMedia();
    IMS_UINT32 eMediaType = 0;

    for (IMS_UINT32 i = 0; i < lstIMedia.GetSize(); i++)
    {
        IMediaDescriptor* pDescriptor = GetMediaDescriptor(lstIMedia.GetAt(i));
        if (pDescriptor == IMS_NULL)
        {
            return;
        }

        SdpMedia* pSDPMedia = const_cast<SdpMedia*>(pDescriptor->GetMediaDescriptionEx());
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
    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // set RTP Port to zero
    pSDPMedia->SetPort(0);
    IMS_TRACE_I("SetMediaDescriptorAsNotSupported() - SetPort [0]", 0, 0, 0);

    pDescriptor->SetMediaDescription(
            pSDPMedia->GetType(), 0, pSDPMedia->GetTransportProtocol(), pSDPMedia->GetFormats());
}