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
#include "ISessionDescriptor.h"
#include "offeranswer/SdpAvCodec.h"

#include "MediaManager.h"
#include "MediaProfileFactory.h"
#include "MediaResourceManager.h"
#include "audio/AudioNego.h"
#include "audio/AudioNegoAmr.h"
#include "audio/AudioNegoEvs.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaConfigUtil.h"

#define MODESET_MAX_AMR      7
#define MODESET_MAX_AMRWB    8
#define EVS_NEGO_RETRY_COUNT 2
#define MAX_OAMODEL_SIZE     6
#define RETURN_MODE_MATCHED  IMS_FALSE
#define RETURN_MODE_SIMILAR  IMS_TRUE

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
AudioNego::AudioNego(IMS_SINT32 nSlotId) :
        BaseNego(nSlotId),
        m_lstOaModel(ImsList<OaModel*>()),
        m_objBaseProfile(AudioProfile()),
        m_pConfig(IMS_NULL)
{
    IMS_TRACE_I("+AudioNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC
AudioNego::AudioNego(IN const AudioNego& objAudioNego) :
        BaseNego(objAudioNego.GetSlotId())
{
    copy(&objAudioNego);
}

PUBLIC
AudioNego& AudioNego::operator=(IN const AudioNego& obj)
{
    if (this != &obj)
    {
        copy(&obj);
    }
    return (*this);
}

PUBLIC VIRTUAL AudioNego::~AudioNego()
{
    IMS_TRACE_I("~AudioNego() - lstOaModel size[%d]", m_lstOaModel.GetSize(), 0, 0);

    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());

    if (pMediaManager != IMS_NULL)
    {
        MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

        if (pResourceMngr != IMS_NULL)
        {
            if (m_objBaseProfile.nDataPort != 0)
            {
                pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
            }
        }
    }

    while (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pOaModel = m_lstOaModel.GetAt(0);

        if (pOaModel != IMS_NULL)
        {
            delete pOaModel;
        }

        m_lstOaModel.RemoveAt(0);
    }
}

PUBLIC VIRTUAL void AudioNego::CreateProfiles(
        IN MediaEnvironment* pEnvironment, IN AudioConfiguration* pConfig)
{
    if (pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfiles() - invalid configuration", 0, 0, 0);
        return;
    }

    m_pEnvironment = pEnvironment;
    m_pConfig = pConfig;

    IMS_TRACE_I("CreateProfiles()", 0, 0, 0);

    AudioProfile* pProfile =
            static_cast<AudioProfile*>(MediaProfileFactory::GetInstance()->CreateProfile(
                    pEnvironment, m_pConfig, GetSlotId(), MEDIA_TYPE_AUDIO));

    if (pProfile != IMS_NULL)
    {
        m_objBaseProfile = *pProfile;
        delete pProfile;
    }
}

PUBLIC VIRTUAL IMS_BOOL AudioNego::FormSdp(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
        IN MEDIA_DIRECTION eDir, IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_D("FormSdp() eNegoState[%d], eDir[%d] lstOaModel size[%d]", eNegoState, eDir,
            m_lstOaModel.GetSize());
    IMS_TRACE_D("FormSdp() - EnforceReofferMode[%d]", bEnforceReofferMode, 0, 0);

    switch (eNegoState)
    {
        case STATE_IDLE:
            return FormOffer(pSessionDescriptor, pDescriptor, eDir);
        case STATE_OFFER_RECEIVED:
            return FormAnswer(pSessionDescriptor, pDescriptor, eDir);
        case STATE_NEGOTIATED:
            return FormReoffer(pSessionDescriptor, pDescriptor, eDir, bEnforceReofferMode);
        default:
            IMS_TRACE_E(0, "FormSdp fail eNegoState[%d]", eNegoState, 0, 0);
            return IMS_FALSE;
    }
}

PUBLIC VIRTUAL IMS_BOOL AudioNego::IsMediaCodecFromSdpSupported(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_TYPE_INVALID;
    }

    IMS_TRACE_I("IsMediaCodecFromSdpSupported()", 0, 0, 0);

    OaModel objOaModel;
    objOaModel.pLocalProfile = new AudioProfile(m_objBaseProfile);

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = new AudioProfile();

    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, objOaModel.pPeerProfile) != IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile = new AudioProfile();

    if (MakeNegotiatedProfile(objOaModel.pLocalProfile, objOaModel.pPeerProfile, IMS_TRUE,
                objOaModel.pNegotiatedProfile) != IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    return (objOaModel.pNegotiatedProfile->lstPayload.GetSize() > 0 &&
                   objOaModel.pNegotiatedProfile->nDataPort != 0)
            ? IMS_TRUE
            : IMS_FALSE;
}

PUBLIC VIRTUAL void AudioNego::NegotiateSdp(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
        OUT IMS_SINT32& nDirection)
{
    nDirection = MEDIA_DIRECTION_INVALID;

    switch (eNegoState)
    {
        case STATE_IDLE:
        case STATE_NEGOTIATED:
            nDirection = NegotiateOffer(pSessionDescriptor, pDescriptor);
            break;
        case STATE_OFFER_SENT:
            nDirection = NegotiateAnswer(pSessionDescriptor, pDescriptor);
            break;
        default:
            break;
    }
}

PUBLIC VIRTUAL void AudioNego::FinalizeSdp(
        IN ISessionDescriptor* pSessionDescriptor, IN NEGO_STATE eNegoState)
{
    IMS_BOOL bFoundOaModel = IMS_FALSE;

    // reset confirmed Session check variable
    for (IMS_UINT32 i = 0; i < m_lstOaModel.GetSize(); i++)
    {
        OaModel* pCheckedOaModel = m_lstOaModel.GetAt(i);
        if (pCheckedOaModel != IMS_NULL)
        {
            pCheckedOaModel->bConfirmedSession = IMS_FALSE;
        }
    }

    // check latest OA model
    OaModel* pLatestOaModel = IMS_NULL;

    if (m_lstOaModel.GetSize() > 0)
    {
        pLatestOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize() - 1);
    }

    if (pLatestOaModel != IMS_NULL)
    {
        if ((pLatestOaModel->IsAllProfileExist() &&
                    (eNegoState == STATE_IDLE || eNegoState == STATE_NEGOTIATED)) == IMS_FALSE)
        {
            IMS_TRACE_I("FinalizeSdp() - Incomplete OaModel[%d]. Delete profile",
                    m_lstOaModel.GetSize() - 1, 0, 0);
            delete pLatestOaModel;
            m_lstOaModel.RemoveAt(m_lstOaModel.GetSize() - 1);
        }
    }

    for (IMS_UINT32 i = 0; i < m_lstOaModel.GetSize(); i++)
    {
        // get OaModel
        OaModel* pTempOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize() - 1 - i);

        // find matched SessionDescriptor key
        if (pTempOaModel != IMS_NULL)
        {
            if (pTempOaModel->nSessionDescriptorKey ==
                    reinterpret_cast<IMS_SINTP>(pSessionDescriptor))
            {
                pTempOaModel->bConfirmedSession = IMS_TRUE;
                bFoundOaModel = IMS_TRUE;
                IMS_TRACE_D("FinalizeSdp() - find comfirmed Session OaModel[%d]",
                        m_lstOaModel.GetSize() - i, 0, 0);
                break;
            }
        }
    }

    // SessionDescriptor key mismatch case handling
    // not select OaModel
    if (bFoundOaModel != IMS_TRUE && m_lstOaModel.GetSize() > 0)
    {
        IMS_TRACE_D("FinalizeSdp() - not found comfirmed Session OaModel", 0, 0, 0);
    }
}

PUBLIC VIRTUAL IMS_BOOL AudioNego::SetPort(IN IMS_UINT32 nPort)
{
    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());
    if (pMediaManager == IMS_NULL)
    {
        return IMS_FALSE;
    }

    MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();
    if (pResourceMngr == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Release Current Port
    if (m_objBaseProfile.nDataPort != 0)
    {
        pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
    }

    IMS_TRACE_I(
            "SetPort() - VoLTE Changed Data Port[%d]->[%d]", m_objBaseProfile.nDataPort, nPort, 0);

    if (nPort != 0)
    {
        // Acquire New Port
        m_objBaseProfile.nDataPort = pResourceMngr->AcquireRtpPort(nPort, nPort);
        m_objBaseProfile.nControlPort = m_objBaseProfile.nDataPort + 1;
    }
    else  // port 0 case
    {
        // Set to Port 0
        m_objBaseProfile.nDataPort = 0;
        m_objBaseProfile.nControlPort = 0;

        IMS_TRACE_I("SetPort() - VoLTE Data Port is 0!!!", 0, 0, 0);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL const IpAddress& AudioNego::GetNegotiatedRemoteAddress()
{
    AudioProfile* pProfile = GetNegotiatedPeerProfile();

    if (pProfile != IMS_NULL)
    {
        return pProfile->objIpAddress;
    }

    return IpAddress::NONE;
}

PUBLIC VIRTUAL IMS_SINT32 AudioNego::GetRemotePort()
{
    AudioProfile* pProfile = GetNegotiatedPeerProfile();

    if (pProfile != IMS_NULL)
    {
        return pProfile->nDataPort;
    }

    return MEDIA_PORT_INVALID;
}

PUBLIC VIRTUAL AudioProfile* AudioNego::GetNegotiatedLocalProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pLocalProfile;
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL AudioProfile* AudioNego::GetNegotiatedNegoProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pNegotiatedProfile;
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL AudioProfile* AudioNego::GetNegotiatedPeerProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pPeerProfile;
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL MEDIA_DIRECTION AudioNego::GetNegotiatedDirection(void)
{
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel();
        if (pLatestOaModel == IMS_NULL)
        {
            return MEDIA_DIRECTION_INVALID;
        }

        if (pLatestOaModel->IsAllProfileExist() == IMS_TRUE)
        {
            return pLatestOaModel->pNegotiatedProfile->eDirection;
        }
    }
    return MEDIA_DIRECTION_INVALID;
}

PUBLIC VIRTUAL AUDIO_CODEC_BITRATE AudioNego::GetNegotiatedAudioCodecRate(void)
{
    OaModel* pLatestOaModel = GetNegotiatedOaModel();
    if (pLatestOaModel == IMS_NULL)
    {
        return AUDIO_CODEC_BITRATE_MAX;
    }

    AudioProfile* pNegotiatedProfile = pLatestOaModel->pNegotiatedProfile;

    if (pNegotiatedProfile == IMS_NULL || pNegotiatedProfile->lstPayload.GetSize() == 0)
    {
        return AUDIO_CODEC_BITRATE_MAX;
    }

    AudioProfile::Payload* pNegotiatedPayload = NULL;

    if (pNegotiatedProfile->nNegotiatedPayloadIndex < 0)
    {
        pNegotiatedPayload = pNegotiatedProfile->lstPayload.GetAt(0);
    }
    else
    {
        pNegotiatedPayload =
                pNegotiatedProfile->lstPayload.GetAt(pNegotiatedProfile->nNegotiatedPayloadIndex);
    }

    if (pNegotiatedPayload == NULL)
    {
        return AUDIO_CODEC_BITRATE_MAX;
    }

    if (pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
            pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
    {
        IMS_SINT32 nLargestModeSet = -1;

        if (pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            nLargestModeSet =
                    AudioProfileUtil::GetLargestModesetInFmtp("AMR-WB", pNegotiatedPayload) +
                    AUDIO_CODEC_BITRATE_AMR_WB_660;
            return (AUDIO_CODEC_BITRATE)nLargestModeSet;
        }
        else  // AMR case
        {
            nLargestModeSet = AudioProfileUtil::GetLargestModesetInFmtp("AMR", pNegotiatedPayload) +
                    AUDIO_CODEC_BITRATE_AMR_475;
            return (AUDIO_CODEC_BITRATE)nLargestModeSet;
        }
    }
    else if (pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pNegotiatedPayload->pFmtp;
        if (pEvsFmtp == IMS_NULL)
        {
            return AUDIO_CODEC_BITRATE_INVALID;
        }

        IMS_SINT32 nLargestModeSet = -1;
        nLargestModeSet = AudioProfileUtil::GetLargestModesetInFmtp("EVS", pNegotiatedPayload);
        // primary mode
        if (pEvsFmtp->nEvsModeSwitch != 1)
        {
            nLargestModeSet = nLargestModeSet + AUDIO_CODEC_BITRATE_EVS_590;
            // check channel aware mode case
            if (nLargestModeSet == AUDIO_CODEC_BITRATE_EVS_1320)
            {
                if (pEvsFmtp->nChAwRecv == 2)
                {
                    nLargestModeSet = AUDIO_CODEC_BITRATE_EVS_1320_CHA_2;
                }
                else if (pEvsFmtp->nChAwRecv == 3)
                {
                    nLargestModeSet = AUDIO_CODEC_BITRATE_EVS_1320_CHA_3;
                }
                else if (pEvsFmtp->nChAwRecv == 5)
                {
                    nLargestModeSet = AUDIO_CODEC_BITRATE_EVS_1320_CHA_5;
                }
                else if (pEvsFmtp->nChAwRecv == 7)
                {
                    nLargestModeSet = AUDIO_CODEC_BITRATE_EVS_1320_CHA_7;
                }
            }
        }
        else
        {  // EVS AMR IO Mode
            nLargestModeSet = nLargestModeSet + AUDIO_CODEC_BITRATE_EVS_IO_660;
        }

        return (AUDIO_CODEC_BITRATE)nLargestModeSet;
    }

    return AUDIO_CODEC_BITRATE_INVALID;
}

PUBLIC VIRTUAL AUDIO_CODEC AudioNego::GetNegotiatedCodec(void)
{
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel();
        if (pLatestOaModel == IMS_NULL)
        {
            return AUDIO_CODEC_NONE;
        }
        if (pLatestOaModel->IsAllProfileExist() == IMS_FALSE)
        {
            return AUDIO_CODEC_NONE;
        }
        // if (pLatestOaModel->pNegotiatedProfile->nDataPort == 0) return AUDIO_CODEC_NONE;
        if (pLatestOaModel->pNegotiatedProfile->lstPayload.GetSize() == 0)
        {
            return AUDIO_CODEC_NONE;
        }

        AudioProfile::Payload* pPayload;

        if (pLatestOaModel->pNegotiatedProfile->nNegotiatedPayloadIndex < 0)
        {
            pPayload = pLatestOaModel->pNegotiatedProfile->lstPayload.GetAt(0);
        }
        else
        {
            pPayload = pLatestOaModel->pNegotiatedProfile->lstPayload.GetAt(
                    pLatestOaModel->pNegotiatedProfile->nNegotiatedPayloadIndex);
        }

        if (pPayload == IMS_NULL)
        {
            return AUDIO_CODEC_NONE;
        }

        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            return AUDIO_CODEC_AMRWB;
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR"))
        {
            return AUDIO_CODEC_AMR;
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
        {
            AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pPayload->pFmtp;

            if (pEvsFmtp == IMS_NULL)
            {
                return AUDIO_CODEC_EVS;
            }
            // EVS AMR WB IO Mode case
            if (pEvsFmtp->nEvsModeSwitch == 1)
            {
                return AUDIO_CODEC_EVS_WB;
            }
            // Primary SWB case
            if ((pEvsFmtp->nBwList & 0x04) != 0)
            {
                return AUDIO_CODEC_EVS_SWB;
            }
            else if ((pEvsFmtp->nBwList & 0x02) != 0)
            {  // Primary WB case
                return AUDIO_CODEC_EVS_WB;
            }
            else if ((pEvsFmtp->nBwList & 0x01) != 0)
            {  // Primary NB case
                return AUDIO_CODEC_EVS_NB;
            }
            return AUDIO_CODEC_EVS;
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU"))
        {
            return AUDIO_CODEC_G711_PCMU;
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMA"))
        {
            return AUDIO_CODEC_G711_PCMA;
        }
    }

    return AUDIO_CODEC_NONE;
}

PUBLIC VIRTUAL IMS_BOOL AudioNego::HasNegotiatedDtmf(void)
{
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel();
        if (pLatestOaModel == IMS_NULL)
        {
            return IMS_FALSE;
        }
        if (pLatestOaModel->IsAllProfileExist() == IMS_FALSE)
        {
            return IMS_FALSE;
        }

        for (IMS_UINT32 i = 0; i < pLatestOaModel->pNegotiatedProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* pPayload =
                    pLatestOaModel->pNegotiatedProfile->lstPayload.GetAt(i);
            if (pPayload == IMS_NULL)
            {
                continue;
            }

            if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
            {
                IMS_TRACE_I("HasNegotiatedDtmf() - Negotiated DTMF found[%d]", i, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 AudioNego::GetNegotiatedRtpPort(void)
{
    CONST IMS_SINT32 PORT_NONE = -1;
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel();
        if (pLatestOaModel == IMS_NULL)
        {
            return PORT_NONE;
        }
        if (pLatestOaModel->IsAllProfileExist() == IMS_FALSE)
        {
            return PORT_NONE;
        }
        IMS_TRACE_I("GetNegotiatedRtpPort() - Previous negotiated port[%d] found",
                pLatestOaModel->pNegotiatedProfile->nDataPort, 0, 0);
        return (IMS_SINT32)pLatestOaModel->pNegotiatedProfile->nDataPort;
    }

    return PORT_NONE;
}

PUBLIC VIRTUAL IMS_SINT32 AudioNego::GetMediaBandwidth(void)
{
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize() - 1);
        if (pLatestOaModel == IMS_NULL || pLatestOaModel->pLocalProfile == NULL)
        {
            return -1;
        }

        // returned negotiated bandwidth.
        if (pLatestOaModel->pNegotiatedProfile != NULL)
        {
            return (IMS_SINT32)pLatestOaModel->pNegotiatedProfile->nBandwidthAs;
        }

        // if negotiated bandwidth does not exist, then return src profile bandwidth.
        return (IMS_SINT32)pLatestOaModel->pLocalProfile->nBandwidthAs;
    }

    return -1;
}

PRIVATE
void AudioNego::copy(IN const AudioNego* pAudioNego)
{
    if (pAudioNego == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I(
            "copy() - referenced OaModel list size[%d]", pAudioNego->m_lstOaModel.GetSize(), 0, 0);

    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());

    if (pMediaManager == IMS_NULL)
    {
        return;
    }

    MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr != IMS_NULL)
    {
        // To release previous used port
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
        }
    }

    m_objBaseProfile = pAudioNego->m_objBaseProfile;

    if (pResourceMngr != IMS_NULL)
    {
        // To add port (it would be duplicated)
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->AcquireRtpPort(m_objBaseProfile.nDataPort, m_objBaseProfile.nDataPort);
        }
    }

    m_pEnvironment = pAudioNego->m_pEnvironment;

    if (pAudioNego->m_lstOaModel.IsEmpty() == IMS_FALSE)
    {
        OaModel* pNewOaModel = new OaModel();
        pNewOaModel->pLocalProfile = new AudioProfile(m_objBaseProfile);
        m_lstOaModel.Append(pNewOaModel);
    }

    this->m_pConfig = pAudioNego->m_pConfig;
    IMS_TRACE_I("copy() - OA model list size[%d]", m_lstOaModel.GetSize(), 0, 0);
}

PRIVATE
IMS_BOOL AudioNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (eDir == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_E(0, "FormOffer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("FormOffer() - eDir[%d]", eDir, 0, 0);

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();

    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pNewOaModel->pLocalProfile = new AudioProfile(m_objBaseProfile);

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_I("FormOffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pLocalProfile->eDirection = eDir;
    }

    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    GetSlotId(), m_pEnvironment->eServiceType);

    if (pMediaSessionConfig != IMS_NULL && pMediaSessionConfig->IsAnbrSupported())
    {
        pNewOaModel->pLocalProfile->bAnbr = IMS_TRUE;
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    AudioProfileUtil::SetRtcpRsRr(pNewOaModel->pLocalProfile, m_pConfig);
    m_lstOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    IMS_BOOL bSdpMade =
            MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pLocalProfile);

    // Delete Session Level Direction Attribute
    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);

    return bSdpMade;
}

PRIVATE
IMS_BOOL AudioNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir)
{
    IMS_TRACE_D("FormAnswer() - eDir[%d]", eDir, 0, 0);

    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_lstOaModel.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    if (eDir == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_E(0, "FormAnswer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    // Getting OaModel from list
    OaModel* pNewOaModel = GetNegotiatedOaModel();

    if (pNewOaModel == IMS_NULL || pNewOaModel->IsAllProfileExist() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    /** TODO: move this logic to session level
    // Compare the media type between base and requested. If it not matched,
    // re-create a negotiated profile
    MEDIA_CONTENT_TYPE eBaseWithoutText =
            (MEDIA_CONTENT_TYPE)MEDIA_TYPE_WITHOUT_TEXT(m_eSessionType);
    MEDIA_CONTENT_TYPE eRequestedWithoutText = (MEDIA_CONTENT_TYPE)MEDIA_TYPE_WITHOUT_TEXT(eType);

    if (eBaseWithoutText != eRequestedWithoutText)
    {
        IMS_TRACE_I("FormAnswer() Media type doesn't matched Base[%d], Requested[%d]",
                eBaseWithoutText, eRequestedWithoutText, 0);

        *pNewOaModel->pLocalProfile = m_objBaseProfile;

        if (pNewOaModel->pNegotiatedProfile != IMS_NULL)
        {
            delete pNewOaModel->pNegotiatedProfile;
        }

        pNewOaModel->pNegotiatedProfile = new AudioProfile();
        if (MakeNegotiatedProfile(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_TRUE,
                    pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
        {
            delete pNewOaModel;
            return IMS_FALSE;
        }
    }*/

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID)
    {
        pNewOaModel->pNegotiatedProfile->eDirection = eDir;
        IMS_TRACE_I("FormAnswer() - update audio direction[%d]", eDir, 0, 0);
    }

    // Make the SDP from profile
    IMS_BOOL bSDPMade =
            MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pNegotiatedProfile);

    if (pSessionDescriptor->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        // Delete Session Level Direction Attribute
        pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    return bSDPMade;
}

PRIVATE
IMS_BOOL AudioNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormReoffer() pDescriptor[%" PFLS_x "], eDir[%d], m_lstOaModel.GetSize[%d]",
            pDescriptor, eDir, m_lstOaModel.GetSize());
    IMS_TRACE_D("FormReoffer() - EnforceReofferMode[%d]", bEnforceReofferMode, 0, 0);

    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (eDir == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_E(0, "FormReoffer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pConfig == NULL || m_pEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormReoffer() - config is not valid", 0, 0, 0);
        return IMS_FALSE;
    }

    // Make new Offer/Answer model, and copy source profile from previous negotiated profile
    OaModel* pNewOaModel = new OaModel();

    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_lstOaModel.GetSize() == 0)
    {
        pNewOaModel->pLocalProfile = new AudioProfile(m_objBaseProfile);
    }
    else
    {
        OaModel* pPrevOaModel = GetNegotiatedOaModel();

        if (pPrevOaModel == IMS_NULL)
        {
            if (pNewOaModel != NULL)
            {
                delete pNewOaModel;
            }

            return IMS_FALSE;
        }

        MediaSessionConfig* pMediaSessionConfig =
                MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                        GetSlotId(), m_pEnvironment->eServiceType);

        if (pMediaSessionConfig == IMS_NULL)
        {
            return IMS_FALSE;
        }

        // reuse previous profile when negotiated profile data port is 0
        if ((pPrevOaModel->pNegotiatedProfile != IMS_NULL &&
                    pPrevOaModel->pNegotiatedProfile->nDataPort == 0))
        {
            pNewOaModel->pLocalProfile = new AudioProfile(*pPrevOaModel->pNegotiatedProfile);
        }
        else
        {
            if (bEnforceReofferMode == IMS_TRUE)
            {
                pNewOaModel->pLocalProfile = new AudioProfile(m_objBaseProfile);
            }
            else
            {
                IMS_TRACE_I(
                        "FormReoffer() - reuse previous profile, m_bSdpReofferFullCapability[%d]",
                        pMediaSessionConfig->IsSdpReofferFullCapability(), 0, 0);

                if (pMediaSessionConfig->IsSdpReofferFullCapability() == IMS_TRUE)
                {
                    pNewOaModel->pLocalProfile = new AudioProfile(m_objBaseProfile);
                }
                else if (pPrevOaModel->pNegotiatedProfile != IMS_NULL)
                {
                    pNewOaModel->pLocalProfile =
                            new AudioProfile(*pPrevOaModel->pNegotiatedProfile);
                }
            }
        }
    }

    if (pNewOaModel->pLocalProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "create LocalProfile has failed", 0, 0, 0);
        delete pNewOaModel;
        return IMS_FALSE;
    }

    // set default AS value when localProfile AS value is 0 in ReOffer case
    if (pNewOaModel->pLocalProfile->nBandwidthAs <= 0)
    {
        IMS_TRACE_I("FormReoffer() - use default AS value", 0, 0, 0);
        pNewOaModel->pLocalProfile->nBandwidthAs = m_objBaseProfile.nBandwidthAs;
    }

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_I("FormReoffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pLocalProfile->eDirection = eDir;
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    AudioProfileUtil::SetRtcpRsRr(pNewOaModel->pLocalProfile,
            MediaConfigUtil::GetAudioConfig(GetSlotId(), m_pEnvironment->eServiceType));

    pNewOaModel->pLocalProfile->nDataPort = m_objBaseProfile.nDataPort;
    pNewOaModel->pLocalProfile->nControlPort = m_objBaseProfile.nControlPort;

    // when reoffer case - recover rtcpxr to default in sendrecv case
    if (m_objBaseProfile.bSupportRtcpXr == IMS_TRUE &&
            pNewOaModel->pLocalProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        pNewOaModel->pLocalProfile->bSupportRtcpXr = m_objBaseProfile.bSupportRtcpXr;
        pNewOaModel->pLocalProfile->objRtcpXrAttr = m_objBaseProfile.objRtcpXrAttr;
    }

    m_lstOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    IMS_BOOL bSDPMade =
            MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pLocalProfile);

    // Delete Session Level Direction Attribute
    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    return bSDPMade;
}

PRIVATE
MEDIA_DIRECTION AudioNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateOffer() - local port[%d]", m_objBaseProfile.nDataPort, 0, 0);

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile = new AudioProfile(m_objBaseProfile);

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = new AudioProfile();

    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pPeerProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    pNewOaModel->pNegotiatedProfile = new AudioProfile();

    if (MakeNegotiatedProfile(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_TRUE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateOffer() - add session key in NewOaModel[%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_lstOaModel.Append(pNewOaModel);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PRIVATE
MEDIA_DIRECTION AudioNego::NegotiateAnswer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    if (m_lstOaModel.GetSize() < 1)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateAnswer() Entered", 0, 0, 0);

    // Get the latest OAmodel from list
    OaModel* pNewOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize() - 1);

    if (pNewOaModel == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = new AudioProfile();

    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pPeerProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_lstOaModel.RemoveAt(m_lstOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile with the local, peer profile
    pNewOaModel->pNegotiatedProfile = new AudioProfile();

    if (MakeNegotiatedProfile(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_FALSE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_lstOaModel.RemoveAt(m_lstOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateAnswer() - add session key in NewOaModel[%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PRIVATE
IMS_BOOL AudioNego::MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("MakeSdpFromProfile() - PayloadSize[%d], AS[%d], port[%d]",
            pProfile->lstPayload.GetSize(), pProfile->nBandwidthAs, pProfile->nDataPort);

    // clean attr & bandwidth line
    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // make"c" &"o" line of session level if IP does not matched
    SetSdpSessionIpAddress(pSessionDescriptor, pProfile);

    // make"m" line
    // ------"m=audio xxxx RTP/AVP 104 110 105 102 108 100"
    SetSdpMediaDescription(pDescriptor, pProfile);

    // make bandwidth
    // ------"b=AS:xx"
    // ------"b=AS:xx"
    // ------"b=AS:xx"
    SetSdpMediaBandwidth(pDescriptor, pProfile);

    // make each payload
    // ------"a=rtpmap:104 AMR-WB/16000/1"
    // ------"a=fmtp:110 mode-set=2; octet-align=1"
    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AString strRtpmap, strFmtp, strPayloadNum;

        AudioProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // set "rtpmap"
        strPayloadNum.Sprintf("%d", pPayload->objRtpMap.nPayloadNum);
        strRtpmap.Sprintf("%s/%d", pPayload->objRtpMap.strPayloadType.GetStr(),
                pPayload->objRtpMap.nSamplingRate);

        if (pPayload->objRtpMap.nChannel > 0)
        {
            AString strChannel;
            strChannel.Sprintf("/%d", pPayload->objRtpMap.nChannel);
            strRtpmap.Append(strChannel);
        }

        // set "fmtp"
        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB") ||
                pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR"))
        {
            AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pPayload->pFmtp;
            if (pAmrFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = AudioNegoAmr::SetSdpFmtpFromAmrFmtp(pAmrFmtp);
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
        {
            AudioProfile::TelephoneEventFmtp* pTEFmtp =
                    (AudioProfile::TelephoneEventFmtp*)pPayload->pFmtp;
            if (pTEFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = pTEFmtp->strEvents;
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
        {
            AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pPayload->pFmtp;
            if (pEvsFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(pEvsFmtp);
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("pcmu") ||
                pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("pcma"))
        {
            // set rtpmap, not fmtp
            strFmtp = AString::ConstNull();
            pDescriptor->SetMediaFormat(
                    SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpmap, strFmtp);
            continue;
        }
        else
        {
            continue;
        }

        if (strFmtp.GetLength() == 0)
        {
            strFmtp = (m_pConfig != IMS_NULL) ? AString::ConstNull() : AString::ConstEmpty();
        }

        pDescriptor->SetMediaFormat(SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpmap, strFmtp);
    }

    // set make direction
    pDescriptor->SetDirection(pProfile->eDirection);

    if (pProfile->eDirection > MEDIA_DIRECTION_INVALID &&
            pProfile->eDirection <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        // Set Session Level Direction Attribute according to the media direction
        // (avoid conflict between media and audio)
        pSessionDescriptor->SetDirection(pProfile->eDirection);
    }

    // set make ptime & maxptime
    if (pProfile->nPtime != AudioProfile::AmrFmtp::DEFAULT_PTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::PTIME, pProfile->nPtime);
    }

    if (pProfile->nMaxPtime != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::MAXPTIME, pProfile->nMaxPtime);
    }

    // set candidate
    for (IMS_UINT32 nIndex = 0; nIndex < pProfile->objCandidateAttr.GetSize(); nIndex++)
    {
        AString strCandidateAttr = pProfile->objCandidateAttr.GetAt(nIndex);
        if (strCandidateAttr.GetLength() != 0)
        {
            strCandidateAttr.Sprintf("%d, %s", nIndex + 1, strCandidateAttr.GetStr());
            pDescriptor->AddAttribute(SdpAttribute::CANDIDATE, strCandidateAttr);
        }
    }

    // set RTCP-XR -- RTCP-XR is for VZW, not a negotiation target by VZW requirement
    if (pProfile->bSupportRtcpXr == IMS_TRUE &&
            pProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        if (pProfile->objRtcpXrAttr.bSupportStatisticMetrics)
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "stat-summary=loss,dup,jitt,HL");
        }
        if (pProfile->objRtcpXrAttr.bSupportVoipMetrics)
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "voip-metrics");
        }
        if (pProfile->objRtcpXrAttr.bSupportPacketLossRle)
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "pkt-loss-rle");
        }
        if (pProfile->objRtcpXrAttr.bSupportPacketDuplicatedRle)
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "pkt-dup-rle");
        }

        IMS_TRACE_I("MakeSdpFromProfile() - bSupportRtcpXr[%d]", pProfile->bSupportRtcpXr, 0, 0);
    }

    if (pProfile->bAnbr)
    {
        pDescriptor->AddAttribute(SdpAttribute::ANBR, AString::ConstNull());
    }
    else
    {
        IMS_TRACE_D("MakeSdpFromProfile() - anbr feature is not supported", 0, 0, 0);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioNego::MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // IP
    pProfile->objIpAddress = pDescriptor->GetRemoteAddress();

    // data & control port
    pProfile->nDataPort = pDescriptor->GetRemotePort();

    if (pDescriptor->GetAttributeInt(SdpAttribute::RTCP) == IMediaDescriptor::INVALID_VALUE)
    {
        pProfile->nControlPort = pProfile->nDataPort + 1;
    }
    else
    {
        pProfile->nControlPort = pDescriptor->GetAttributeInt(SdpAttribute::RTCP);
    }

    // bandwidth
    pProfile->nBandwidthAs = pDescriptor->GetBandwidth(SdpBandwidth::TYPE_AS);
    pProfile->nBandwidthRs = pDescriptor->GetBandwidth(SdpBandwidth::TYPE_RS);
    pProfile->nBandwidthRr = pDescriptor->GetBandwidth(SdpBandwidth::TYPE_RR);

    IMS_TRACE_I("MakeProfileFromSdp() - AS[%d], RS[%d], RR[%d]", pProfile->nBandwidthAs,
            pProfile->nBandwidthRs, pProfile->nBandwidthRr);

    // read CapaNego profile From SDP
    MakeCapaNegoProfileFromSdp(pDescriptor, &(pProfile->objCapaNego));

    // payload
    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));

        if (pSdpCodec == IMS_NULL)
        {
            return IMS_FALSE;
        }

        AString strCodecName = pSdpCodec->GetName();
        AString strChannel = pSdpCodec->GetEncodingParameters();
        IMS_UINT32 nChannel;
        IMS_SINT32 nPayloadTypeNumber = pSdpCodec->GetPayloadType();

        if ((strChannel != IMS_NULL && strChannel.EqualsIgnoreCase("1")) &&
                (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR") ||
                        strCodecName.EqualsIgnoreCase("EVS")))
        {
            nChannel = 1;
        }
        else
        {
            nChannel = 0;
        }

        IMS_TRACE_D("MakeProfileFromSdp() - At[%d], PayloadType[%d], ClockRate[%d]", i,
                pSdpCodec->GetPayloadType(), pSdpCodec->GetClockRate());

        AudioProfile::Payload* pPayload = new AudioProfile::Payload();
        pPayload->SetRtpMap(
                pSdpCodec->GetPayloadType(), strCodecName, pSdpCodec->GetClockRate(), nChannel);

        if (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR"))
        {
            // Create AMR fmtp
            AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp();
            GetFmtpFromString(pSdpCodec->GetFormatSpecificParameter(), pAmrFmtp);
            pPayload->pFmtp = pAmrFmtp;
        }
        else if (strCodecName.EqualsIgnoreCase("telephone-event"))
        {
            // Create Telephone event fmtp
            AudioProfile::TelephoneEventFmtp* pTeFmtp = new AudioProfile::TelephoneEventFmtp();

            //[RFC4733] For backward compatibility, if no"events" parameter is received,
            // the sender SHOULD assume support for the DTMF events 0-15 but for no other events.
            if (pSdpCodec->GetFormatSpecificParameter() != IMS_NULL &&
                    pSdpCodec->GetFormatSpecificParameter().GetLength() > 0)
            {
                pTeFmtp->strEvents = pSdpCodec->GetFormatSpecificParameter();
            }
            else
            {
                pTeFmtp->strEvents = "0-15";  // default value
            }

            pPayload->pFmtp = pTeFmtp;
        }
        else if (strCodecName.EqualsIgnoreCase("EVS"))
        {
            // Create EVS fmtp
            AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp();
            // check and get EVS fmtp
            GetFmtpFromString(pSdpCodec->GetFormatSpecificParameter(), pEvsFmtp);
            // set Fmpt to payload
            pPayload->pFmtp = pEvsFmtp;
        }
        else if (nPayloadTypeNumber == 0 || nPayloadTypeNumber == 8)
        {  // PCMU or PCMA case
            // do nothing.
            IMS_TRACE_D("MakeProfileFromSdp() - do nothing codec[%s]", strCodecName.GetStr(), 0, 0);
        }
        else
        {
            IMS_TRACE_E(0, "MakeProfileFromSdp() - NOT SUPPORTED audio codec[%s]",
                    strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        if (pPayload != IMS_NULL)
        {
            pProfile->lstPayload.Append(pPayload);
        }
    }

    // direction
    pProfile->eDirection = (MEDIA_DIRECTION)pDescriptor->GetDirection();

    if (pProfile->eDirection == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_D("MakeProfileFromSdp() - Audio Direction does not exist", 0, 0, 0);
        // check session level attribute Direction
        pProfile->eDirection = (MEDIA_DIRECTION)pSessionDescriptor->GetDirection();

        if (pProfile->eDirection == MEDIA_DIRECTION_INVALID)
        {
            pProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
        }
    }

    // ptime & max_ptime
    pProfile->nPtime = pDescriptor->GetAttributeInt(SdpAttribute::PTIME);
    pProfile->nMaxPtime = pDescriptor->GetAttributeInt(SdpAttribute::MAXPTIME);

    // RTCP-XR
    ImsList<AString> lstRtcpXrAttr = pDescriptor->GetAttributes(SdpAttribute::RTCP_XR);

    if (lstRtcpXrAttr.GetSize() > 0)
    {
        pProfile->bSupportRtcpXr = IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < lstRtcpXrAttr.GetSize(); i++)
    {
        AString xrAttr = lstRtcpXrAttr.GetAt(i);

        if (xrAttr.Contains("stat-summary"))
        {
            pProfile->objRtcpXrAttr.bSupportStatisticMetrics = IMS_TRUE;
        }
        if (xrAttr.Contains("voip-metrics"))
        {
            pProfile->objRtcpXrAttr.bSupportVoipMetrics = IMS_TRUE;
        }
        if (xrAttr.Contains("pkt-loss-rle"))
        {
            pProfile->objRtcpXrAttr.bSupportPacketLossRle = IMS_TRUE;
        }
        if (xrAttr.Contains("pkt-dup-rle"))
        {
            pProfile->objRtcpXrAttr.bSupportPacketDuplicatedRle = IMS_TRUE;
        }
    }

    // ANBR
    pProfile->bAnbr = IMS_FALSE;
    if (pDescriptor->GetAttribute(SdpAttribute::ANBR) == IMS_SUCCESS)
    {
        pProfile->bAnbr = IMS_TRUE;
    }

    return IMS_TRUE;
}

IMS_BOOL AudioNego::MakeNegotiatedProfile(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedProfile() invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Setting IP of mine
    pNegotiatedProfile->objIpAddress = pLocalProfile->objIpAddress;

    IMS_TRACE_D("MakeNegotiatedProfile() - IPAddr nego[%s] src[%s] DestPayloadSize[%d]",
            pNegotiatedProfile->objIpAddress.ToCharString(),
            pLocalProfile->objIpAddress.ToCharString(), pPeerProfile->lstPayload.GetSize());

    // Setting RTP/RTCP port of mine
    pNegotiatedProfile->nDataPort = pLocalProfile->nDataPort;
    pNegotiatedProfile->nControlPort = pLocalProfile->nControlPort;

    if (pNegotiatedProfile->nDataPort == 0 || pPeerProfile->nDataPort == 0)
    {
        // Reset the negotiated profile to local profile
        *pNegotiatedProfile = *pLocalProfile;
        pNegotiatedProfile->nDataPort = 0;
        IMS_TRACE_D("MakeNegotiatedProfile() ZERO Port. DO NOT Use the audio[%d][%d]",
                pNegotiatedProfile->nDataPort, pPeerProfile->nDataPort, 0);
    }

    // Compare each payload based destination's profile
    AudioProfile::Payload* pNegotiatedPayload = IMS_NULL;
    ImsList<AudioProfile::Payload*> lstNegotiatedPayloads;

    IMS_BOOL bProperNegotiatedTe = IMS_FALSE;
    IMS_UINT32 nNegoModeSetList = 0;
    IMS_UINT32 nNegoDefaultRtpModeSet = 0;
    IMS_UINT32 BandwidthNegoList;
    IMS_UINT32 BitrateNegoList;
    IMS_UINT32 ModeSetNegoList;

    // find negotiation aduioCodec, because of telephonyEvent negotiation
    ImsList<AudioProfile::Payload*> templstNegotiatedPayloads;

    for (IMS_UINT32 i = 0; i < pPeerProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* pPayload = pPeerProfile->lstPayload.GetAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }
        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
                pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0) &&
                    FindAmrInProfile(pLocalProfile, pPayload, bIsOfferReceived, &nNegoModeSetList,
                            &nNegoDefaultRtpModeSet) == IMS_TRUE)
            {
                AudioProfile::Payload* pAMR = new AudioProfile::Payload();
                pAMR->SetRtpMap(pPayload->objRtpMap);
                pAMR->pFmtp = new AudioProfile::AmrFmtp(
                        *static_cast<AudioProfile::AmrFmtp*>(pPayload->pFmtp));
                templstNegotiatedPayloads.Append(pAMR);
            }
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0) &&
                    FindEvsInProfile(pLocalProfile, pPayload, bIsOfferReceived, &BandwidthNegoList,
                            &BitrateNegoList, &ModeSetNegoList) == IMS_TRUE)
            {
                AudioProfile::Payload* pEVS = new AudioProfile::Payload();
                pEVS->SetRtpMap(pPayload->objRtpMap);

                pEVS->pFmtp = new AudioProfile::EvsFmtp(
                        *static_cast<AudioProfile::EvsFmtp*>(pPayload->pFmtp));

                templstNegotiatedPayloads.Append(pEVS);
            }
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU") ||
                pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMA"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0) &&
                    FindPcmInProfile(pLocalProfile, pPayload) == IMS_TRUE)
            {
                AudioProfile::Payload* pPCM = new AudioProfile::Payload();
                pPCM->SetRtpMap(pPayload->objRtpMap);

                templstNegotiatedPayloads.Append(pPCM);
            }
        }
    }

    IMS_TRACE_D("MakeNegotiatedProfile() - temp negotiated payload list[%d]",
            templstNegotiatedPayloads.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < pPeerProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* pDestPayload = pPeerProfile->lstPayload.GetAt(i);
        if (pDestPayload == IMS_NULL)
        {
            continue;
        }

        if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
                pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            if ((lstNegotiatedPayloads.GetSize() == 0) &&
                    FindAmrInProfile(pLocalProfile, pDestPayload, bIsOfferReceived,
                            &nNegoModeSetList, &nNegoDefaultRtpModeSet) == IMS_TRUE)
            {
                AudioProfile::Payload* pAMR = new AudioProfile::Payload();
                pAMR->SetRtpMap(pDestPayload->objRtpMap);

                IMS_SINT32 nSrcPayloadIndex =
                        FindPayloadIndexFromProfile(pDestPayload->objRtpMap.strPayloadType,
                                pLocalProfile, pDestPayload, bIsOfferReceived);
                AudioProfile::AmrFmtp* pSrc_Fmtp =
                        (AudioProfile::AmrFmtp*)pLocalProfile->lstPayload.GetAt(nSrcPayloadIndex)
                                ->pFmtp;
                AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp(
                        *static_cast<AudioProfile::AmrFmtp*>(pDestPayload->pFmtp));
                pAmrFmtp->nModeSetList = nNegoModeSetList;
                pAmrFmtp->nDefaultRtpModeSet = nNegoDefaultRtpModeSet;

                pAmrFmtp->bDtx = pSrc_Fmtp->bDtx;

                pAmrFmtp->bShowModeChangeCapability = pSrc_Fmtp->bShowModeChangeCapability;
                pAmrFmtp->nModeChangeCapability = pSrc_Fmtp->nModeChangeCapability;
                pAmrFmtp->bShowModeChangeNeighbor = pSrc_Fmtp->bShowModeChangeNeighbor;
                pAmrFmtp->nModeChangeNeighbor = pSrc_Fmtp->nModeChangeNeighbor;
                pAmrFmtp->bShowModeChangePeriod = pSrc_Fmtp->bShowModeChangePeriod;
                pAmrFmtp->nModeChangePeriod = pSrc_Fmtp->nModeChangePeriod;

                pAMR->pFmtp = pAmrFmtp;
                pNegotiatedProfile->lstPayload.Append(pAMR);
                lstNegotiatedPayloads.Append(pAMR);

                if (pPeerProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list.
                    pPeerProfile->nNegotiatedPayloadIndex = i;
                    // set nego payload index at src profile
                    pLocalProfile->nNegotiatedPayloadIndex = nSrcPayloadIndex;
                    IMS_TRACE_D("MakeNegotiatedProfile() - nego payload index[%d]",
                            pLocalProfile->nNegotiatedPayloadIndex, 0, 0);

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                            pLocalProfile->nNegotiatedPayloadIndex != -1)
                    {
                        AudioProfile::Payload* pTempNegoSrcPayload =
                                pLocalProfile->lstPayload.GetAt(
                                        pLocalProfile->nNegotiatedPayloadIndex);
                        pTempNegoSrcPayload->objRtpMap.nPayloadNum =
                                pDestPayload->objRtpMap.nPayloadNum;
                    }
                }

                if (pNegotiatedProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile.
                    pNegotiatedProfile->nNegotiatedPayloadIndex =
                            pNegotiatedProfile->lstPayload.GetSize() - 1;
                }
            }
        }
        else if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
        {
            BandwidthNegoList = 0;
            BitrateNegoList = 0;
            ModeSetNegoList = 0;
            // need to modify FindEvsInProfile() func..
            if ((lstNegotiatedPayloads.GetSize() == 0) &&
                    FindEvsInProfile(pLocalProfile, pDestPayload, bIsOfferReceived,
                            &BandwidthNegoList, &BitrateNegoList, &ModeSetNegoList) == IMS_TRUE)
            {
                AudioProfile::Payload* pEVS = new AudioProfile::Payload();
                pEVS->SetRtpMap(pDestPayload->objRtpMap);
                IMS_SINT32 nSrcPayloadIndex =
                        FindPayloadIndexFromProfile(pDestPayload->objRtpMap.strPayloadType,
                                pLocalProfile, pDestPayload, bIsOfferReceived);
                AudioProfile::EvsFmtp* pSrc_Fmtp =
                        (AudioProfile::EvsFmtp*)pLocalProfile->lstPayload.GetAt(nSrcPayloadIndex)
                                ->pFmtp;
                AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp(
                        *reinterpret_cast<AudioProfile::EvsFmtp*>(pDestPayload->pFmtp));
                pEvsFmtp->nBwList = BandwidthNegoList;
                pEvsFmtp->nBrList = BitrateNegoList;
                pEvsFmtp->nModeSetList = ModeSetNegoList;

                if (pEvsFmtp->bDtx != pSrc_Fmtp->bDtx)
                {
                    IMS_TRACE_D("MakeNegotiatedProfile() - DTX updated in the destination profile",
                            0, 0, 0);
                }

                pEvsFmtp->bShowModeChangeCapability = pSrc_Fmtp->bShowModeChangeCapability;
                pEvsFmtp->nModeChangeCapability = pSrc_Fmtp->nModeChangeCapability;
                pEvsFmtp->bShowModeChangeNeighbor = pSrc_Fmtp->bShowModeChangeNeighbor;
                pEvsFmtp->nModeChangeNeighbor = pSrc_Fmtp->nModeChangeNeighbor;
                pEvsFmtp->bShowModeChangePeriod = pSrc_Fmtp->bShowModeChangePeriod;
                pEvsFmtp->nModeChangePeriod = pSrc_Fmtp->nModeChangePeriod;

                // check uni direction attribute
                if (pEvsFmtp->nBrSend != 0)
                {
                    pEvsFmtp->nBrSend = BitrateNegoList;
                }
                if (pEvsFmtp->nBrRecv != 0)
                {
                    pEvsFmtp->nBrRecv = BitrateNegoList;
                }
                if (pEvsFmtp->nBwSend != 0)
                {
                    pEvsFmtp->nBwSend = BandwidthNegoList;
                }
                if (pEvsFmtp->nBwRecv != 0)
                {
                    pEvsFmtp->nBwRecv = BandwidthNegoList;
                }

                pEvsFmtp->bSendCmr = pSrc_Fmtp->bSendCmr;

                // CMR on/off, if bitrate is not range set, disable CMR send option
                IMS_UINT32 nCount = 0;
                IMS_UINT32 nTempBrList = pEvsFmtp->nBrList;
                for (IMS_UINT32 l = 0; l < 16; l++)
                {
                    if (nTempBrList & 0x01)
                    {
                        nCount++;
                    }

                    nTempBrList = nTempBrList >> 1;

                    if (nTempBrList == 0)
                    {
                        break;
                    }
                }

                if (nCount <= 1)
                {
                    pEvsFmtp->bSendCmr = IMS_FALSE;
                }

                /** fixed for IR92 ver.12 newaly spec as below comment. If the selected EVS
                 * configuration is A1, B0 or B1 then"mode set = 0,1,2" must be included in the SDP
                 * answer.*/
                if (bIsOfferReceived == IMS_TRUE && pEvsFmtp->nEvsModeSwitch != 1)
                {
                    // if max BR is 13.2kbps, then set a"mode-set" attribute
                    if (((pEvsFmtp->nBrList & 0x10) != 0) && ((pEvsFmtp->nBrList & 0xFFE0) == 0))
                    {
                        pEvsFmtp->nModeSetList = 0x07;  // mode-set = 0,1,2;
                        pEvsFmtp->bShowModeSet = IMS_TRUE;
                        IMS_TRACE_D("MakeNegotiatedProfile() - add EVS mode-set", 0, 0, 0);
                    }
                }

                pEVS->pFmtp = pEvsFmtp;
                pNegotiatedProfile->lstPayload.Append(pEVS);
                lstNegotiatedPayloads.Append(pEVS);

                if (pPeerProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list
                    pPeerProfile->nNegotiatedPayloadIndex = i;
                    // set nego payload index at src profile
                    pLocalProfile->nNegotiatedPayloadIndex = nSrcPayloadIndex;

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                            pLocalProfile->nNegotiatedPayloadIndex != -1)
                    {
                        AudioProfile::Payload* pTempNegoSrcPayload =
                                pLocalProfile->lstPayload.GetAt(
                                        pLocalProfile->nNegotiatedPayloadIndex);
                        pTempNegoSrcPayload->objRtpMap.nPayloadNum =
                                pDestPayload->objRtpMap.nPayloadNum;
                    }
                }
                if (pNegotiatedProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile
                    pNegotiatedProfile->nNegotiatedPayloadIndex =
                            pNegotiatedProfile->lstPayload.GetSize() - 1;
                }
            }
        }
        else if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
        {
            if (templstNegotiatedPayloads.GetSize() == 0)
            {
                IMS_TRACE_D("MakeNegotiatedProfile() Telephone-event cannot be a priority payload",
                        0, 0, 0);
                continue;
            }

            for (IMS_UINT32 j = 0; j < templstNegotiatedPayloads.GetSize(); j++)
            {
                pNegotiatedPayload = templstNegotiatedPayloads.GetAt(j);
                if (pNegotiatedPayload->objRtpMap.nSamplingRate ==
                        pDestPayload->objRtpMap.nSamplingRate)
                {
                    AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                    pTelephoneEvent->SetRtpMap(pDestPayload->objRtpMap);
                    pTelephoneEvent->pFmtp = new AudioProfile::TelephoneEventFmtp(
                            *static_cast<AudioProfile::TelephoneEventFmtp*>(pDestPayload->pFmtp));

                    pNegotiatedProfile->lstPayload.Append(pTelephoneEvent);
                    bProperNegotiatedTe = IMS_TRUE;
                    break;
                }
            }
        }
        else if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU") ||
                pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMA"))
        {
            if ((lstNegotiatedPayloads.GetSize() == 0) &&
                    FindPcmInProfile(pLocalProfile, pDestPayload) == IMS_TRUE)
            {
                AudioProfile::Payload* pPCM = new AudioProfile::Payload();
                pPCM->SetRtpMap(pDestPayload->objRtpMap);
                pNegotiatedProfile->lstPayload.Append(pPCM);
                lstNegotiatedPayloads.Append(pPCM);

                if (pPeerProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list
                    pPeerProfile->nNegotiatedPayloadIndex = i;
                    pLocalProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pDestPayload->objRtpMap.strPayloadType,
                                    pLocalProfile, pDestPayload, bIsOfferReceived);
                }

                if (pNegotiatedProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile
                    pNegotiatedProfile->nNegotiatedPayloadIndex =
                            pNegotiatedProfile->lstPayload.GetSize() - 1;
                }
            }
        }
    }

    // free allocated temp memory
    while (templstNegotiatedPayloads.GetSize() > 0)
    {
        AudioProfile::Payload* pDestPayload = templstNegotiatedPayloads.GetAt(0);
        if (pDestPayload != IMS_NULL)
        {
            delete pDestPayload;
        }
        templstNegotiatedPayloads.RemoveAt(0);
    }

    IMS_TRACE_D("MakeNegotiatedProfile() nNegotiatedPayloadIndex[%d]",
            pPeerProfile->nNegotiatedPayloadIndex, 0, 0);

    // accept 8K DTMF when AMR-WB calling if ther are no proper DTMF payload
    if (bProperNegotiatedTe == IMS_FALSE && pNegotiatedProfile->lstPayload.GetSize() > 0 &&
            lstNegotiatedPayloads.GetSize() > 0)
    {
        for (IMS_UINT32 i = 0; i < pPeerProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* pDestPayload = pPeerProfile->lstPayload.GetAt(i);
            if (pDestPayload == IMS_NULL)
            {
                continue;
            }
            if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
            {
                // for acceptable 8K DTMF when AMR-WB calling
                pNegotiatedPayload = lstNegotiatedPayloads.GetAt(0);
                if (pNegotiatedPayload->objRtpMap.nSamplingRate >
                        pDestPayload->objRtpMap.nSamplingRate)
                {
                    IMS_TRACE_D("MakeNegotiatedProfile() - Accept sampling rate[%d]->[%d]",
                            pNegotiatedPayload->objRtpMap.nSamplingRate,
                            pDestPayload->objRtpMap.nSamplingRate, 0);
                    AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                    pTelephoneEvent->SetRtpMap(pDestPayload->objRtpMap);
                    pTelephoneEvent->pFmtp = new AudioProfile::TelephoneEventFmtp(
                            *static_cast<AudioProfile::TelephoneEventFmtp*>(pDestPayload->pFmtp));
                    pNegotiatedProfile->lstPayload.Append(pTelephoneEvent);
                }
            }
        }
    }

    if (lstNegotiatedPayloads.GetSize() > 0)
    {
        pNegotiatedPayload = lstNegotiatedPayloads.GetAt(0);
    }

    if (pNegotiatedPayload != IMS_NULL)
    {
        // Setting bandwidth AS/RS/RR
        IMS_SINT32 nAsValueOfNegoticatedCodec = 0;
        IMS_SINT32 nModeSet;

        // find largest AS value..
        if (pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
                pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            AUDIO_CODEC nCurrCodec;

            AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pNegotiatedPayload->pFmtp;
            if (pNegotiatedPayload->objRtpMap.nSamplingRate == 8000)
            {
                nCurrCodec = AUDIO_CODEC_AMR;
                nModeSet = AudioProfileUtil::GetLargestModesetInFmtp("AMR", pNegotiatedPayload);
            }
            else
            {
                nCurrCodec = AUDIO_CODEC_AMRWB;
                nModeSet = AudioProfileUtil::GetLargestModesetInFmtp("AMR-WB", pNegotiatedPayload);
            }

            nAsValueOfNegoticatedCodec =
                    AudioProfileUtil::ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->nOctetAlign,
                            pNegotiatedProfile->objIpAddress.IsIPv6Address(), nModeSet);
        }
        else if (pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
        {
            AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pNegotiatedPayload->pFmtp;
            AUDIO_CODEC nCurrCodec = AUDIO_CODEC_EVS;
            nModeSet = AudioProfileUtil::GetLargestModesetInFmtp("EVS", pNegotiatedPayload);
            nAsValueOfNegoticatedCodec = AudioProfileUtil::ConvertToBandwidthAS(nCurrCodec,
                    pNegotiatedProfile->objIpAddress.IsIPv6Address(), pEvsFmtp->nEvsModeSwitch,
                    nModeSet);
        }

        // Setting direction
        pNegotiatedProfile->eDirection = UpdateDirectionToMine(
                pPeerProfile->eDirection, pLocalProfile->eDirection, bIsOfferReceived);

        if (pNegotiatedProfile->eDirection == MEDIA_DIRECTION_INVALID)
        {
            IMS_TRACE_E(0, "MakeNegotiatedProfile() - invalid direction.", 0, 0, 0);
            return IMS_FALSE;
        }

        // if the case using different interval in live and hold, set here.
        pNegotiatedProfile->nBandwidthRs = pPeerProfile->nBandwidthRs;
        pNegotiatedProfile->nBandwidthRr = pPeerProfile->nBandwidthRr;

        if (pNegotiatedProfile->nBandwidthRs == 0 && pNegotiatedProfile->nBandwidthRr == 0)
        {
            pNegotiatedProfile->nRtcpInterval = 0;
            IMS_TRACE_D(
                    "MakeNegotiatedProfile() - negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
        }
        else
        {
            pNegotiatedProfile->nRtcpInterval = m_pConfig->GetRtcpInterval();

            if (pNegotiatedProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE &&
                    m_pConfig->GetRtcpLiveInterval() > 0)
            {
                pNegotiatedProfile->nRtcpInterval = m_pConfig->GetRtcpLiveInterval();
            }
        }

        AudioProfileUtil::MakeNegotiatedBandwidth(m_pConfig, pLocalProfile, pPeerProfile,
                bIsOfferReceived, nAsValueOfNegoticatedCodec, pNegotiatedProfile);

        // RTCP-XR
        if (pLocalProfile->bSupportRtcpXr == IMS_TRUE &&
                pNegotiatedProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE)
        {
            pNegotiatedProfile->bSupportRtcpXr = IMS_TRUE;
            pNegotiatedProfile->objRtcpXrAttr = pLocalProfile->objRtcpXrAttr;
        }

        IMS_TRACE_D("MakeNegotiatedProfile()-nRtcpInterval[%d], RTCP-XR support[%d]",
                pNegotiatedProfile->nRtcpInterval, pNegotiatedProfile->bSupportRtcpXr, 0);

        /** Setting ptime & maxptime
         * [RFC3264]
         * The answerer MAY include a non-zero ptime attribute for any media stream.
         * this indicates the packetization interval that the answerer would like to receive.
         * There is no requirement that the packetization interval be the same in each direction
         * for a particular stream.*/
        if (pLocalProfile->nPtime < 20)
        {
            pNegotiatedProfile->nPtime = 20;
        }
        else
        {
            pNegotiatedProfile->nPtime = pLocalProfile->nPtime;
        }

        if (pLocalProfile->nMaxPtime < 20)
        {
            pNegotiatedProfile->nMaxPtime = 240;
        }
        else
        {
            pNegotiatedProfile->nMaxPtime = pLocalProfile->nMaxPtime;
        }

        // Candidate Priority
        pNegotiatedProfile->objCandidateAttr = pLocalProfile->objCandidateAttr;

        // ANBR
        pNegotiatedProfile->bAnbr = IMS_FALSE;
        if (pLocalProfile->bAnbr && pPeerProfile->bAnbr)
        {
            pNegotiatedProfile->bAnbr = IMS_TRUE;
        }

        IMS_TRACE_D("MakeNegotiatedProfile() - anbr local: %d peer: %d nego: %d",
                pLocalProfile->bAnbr, pPeerProfile->bAnbr, pNegotiatedProfile->bAnbr);

        return IMS_TRUE;
    }
    else
    {
        return IMS_FALSE;
    }
}

PRIVATE
IMS_BOOL AudioNego::GetFmtpFromString(IN const AString& strFmtp, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objSplitColon = strFmtp.Split(';');

    for (IMS_UINT32 i = 0; i < objSplitColon.GetSize(); i++)
    {
        if (objSplitColon.GetAt(i).GetLength() == 0)
        {
            continue;
        }
        ImsList<AString> objSplitEqual = objSplitColon.GetAt(i).Split('=');

        if (objSplitEqual.GetSize() < 2)
        {
            const AString& strTmp = objSplitColon.GetAt(i);
            IMS_TRACE_D("GetFmtpFromString() - Invalid fmtp parameter(%s) at index[%d]",
                    strTmp.GetStr(), i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (objSplitEqual.GetAt(0).Equals("ptime") == IMS_TRUE)
        {
            pFmtp->nPtime = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowPtime = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("maxptime") == IMS_TRUE)
        {
            pFmtp->nMaxPtime = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowMaxPtime = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("dtx") == IMS_TRUE)
        {
            pFmtp->bDtx = (IMS_BOOL)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowDtx = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("hf-only") == IMS_TRUE)
        {
            pFmtp->nHfOnly = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowHfOnly = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("evs-mode-switch") == IMS_TRUE)
        {
            pFmtp->nEvsModeSwitch = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowEvsModeSwitch = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("max-red") == IMS_TRUE)
        {
            pFmtp->nMaxRed = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowMaxRed = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("br") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBr = 0;
                IMS_UINT32 nLastBr = 0;
                for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BR_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                            IMS_TRUE)
                    {
                        nFirstBr = j;
                    }
                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                            IMS_TRUE)
                    {
                        nLastBr = j;
                        break;
                    }
                }

                for (IMS_UINT32 k = nFirstBr; k <= nLastBr; k++)
                {
                    pFmtp->nBrList = (pFmtp->nBrList | (1 << k));
                }
            }
            else  // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BR_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[k]) ==
                                IMS_TRUE)
                        {
                            pFmtp->nBrList = (pFmtp->nBrList | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("bw") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBw = 0;
                IMS_UINT32 nLastBw = 0;

                for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BW_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                            IMS_TRUE)
                    {
                        nFirstBw = j;
                    }

                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                            IMS_TRUE)
                    {
                        nLastBw = j;
                        break;
                    }
                }

                for (IMS_UINT32 k = nFirstBw; k <= nLastBw; k++)
                {
                    pFmtp->nBwList = (pFmtp->nBwList | (1 << k));
                }
            }
            else  // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BW_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[k]) ==
                                IMS_TRUE)
                        {
                            pFmtp->nBwList = (pFmtp->nBwList | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("cmr") == IMS_TRUE)
        {
            pFmtp->nCmr = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowCmr = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("ch-aw-recv") == IMS_TRUE)
        {
            // pFmtp->nChAwRecv = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->nReceivedChAwRecv = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->nChAwRecv = pFmtp->nReceivedChAwRecv;
            pFmtp->bShowChannelAwMode = IMS_TRUE;
            /*
                        if ((pFmtp->nChAwRecv == 0) || (pFmtp->nChAwRecv == 2) || (pFmtp->nChAwRecv
               == 3)
                            || (pFmtp->nChAwRecv == 5) || (pFmtp->nChAwRecv == 7) ||
               (pFmtp->nChAwRecv == -1)) { pFmtp->bShowChannelAwMode = IMS_TRUE; } else {
                            pFmtp->nChAwRecv = 0;
                            pFmtp->bShowChannelAwMode = IMS_FALSE;
                        }
            */
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-set") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                IMS_UINT32 nModeSet = (IMS_UINT32)objSplitComma.GetAt(j).ToInt32();
                pFmtp->nModeSetList = (pFmtp->nModeSetList | (1 << nModeSet));
            }
            pFmtp->bShowModeSet = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-capability") == IMS_TRUE)
        {
            pFmtp->nModeChangeCapability = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowModeChangeCapability = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-period") == IMS_TRUE)
        {
            pFmtp->nModeChangePeriod = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowModeChangePeriod = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-neighbor") == IMS_TRUE)
        {
            pFmtp->nModeChangeNeighbor = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowModeChangeNeighbor = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("br-send") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBr = 0;
                IMS_UINT32 nLastBr = 0;
                for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BR_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                            IMS_TRUE)
                    {
                        nFirstBr = j;
                    }

                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                            IMS_TRUE)
                    {
                        nLastBr = j;
                        break;
                    }
                }

                for (IMS_UINT32 k = nFirstBr; k <= nLastBr; k++)
                {
                    pFmtp->nBrRecv = (pFmtp->nBrRecv | (1 << k));
                }
            }
            else  // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BR_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[k]) ==
                                IMS_TRUE)
                        {
                            pFmtp->nBrRecv = (pFmtp->nBrRecv | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("br-recv") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBr = 0;
                IMS_UINT32 nLastBr = 0;

                for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BR_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                            IMS_TRUE)
                    {
                        nFirstBr = j;
                    }
                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                            IMS_TRUE)
                    {
                        nLastBr = j;
                        break;
                    }
                }
                for (IMS_UINT32 k = nFirstBr; k <= nLastBr; k++)
                {
                    pFmtp->nBrSend = (pFmtp->nBrSend | (1 << k));
                }
            }
            else  // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BR_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[k]) ==
                                IMS_TRUE)
                        {
                            pFmtp->nBrSend = (pFmtp->nBrSend | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("bw-send") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBw = 0;
                IMS_UINT32 nLastBw = 0;

                for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BW_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                            IMS_TRUE)
                    {
                        nFirstBw = j;
                    }

                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                            IMS_TRUE)
                    {
                        nLastBw = j;
                        break;
                    }
                }

                for (IMS_UINT32 k = nFirstBw; k <= nLastBw; k++)
                {
                    pFmtp->nBwRecv = (pFmtp->nBwRecv | (1 << k));
                }
            }
            else  // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BW_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[k]) ==
                                IMS_TRUE)
                        {
                            pFmtp->nBwRecv = (pFmtp->nBwRecv | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("bw-recv") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBw = 0;
                IMS_UINT32 nLastBw = 0;

                for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BW_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                            IMS_TRUE)
                    {
                        nFirstBw = j;
                    }

                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                            IMS_TRUE)
                    {
                        nLastBw = j;
                        break;
                    }
                }
                for (IMS_UINT32 k = nFirstBw; k <= nLastBw; k++)
                {
                    pFmtp->nBwSend = (pFmtp->nBwSend | (1 << k));
                }
            }
            else  // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BW_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[k]) ==
                                IMS_TRUE)
                        {
                            pFmtp->nBwSend = (pFmtp->nBwSend | (1 << k));
                        }
                    }
                }
            }
        }
    }

    // check br/bw uni direction check
    if ((pFmtp->nBrRecv != 0) && (pFmtp->nBrSend != 0))
    {
        pFmtp->bShowBrList = IMS_FALSE;
    }

    if ((pFmtp->nBwRecv != 0) && (pFmtp->nBwSend != 0))
    {
        pFmtp->bShowBwList = IMS_FALSE;
    }

    IMS_TRACE_D("GetFmtpFromString() - EVS : ModeSet[0x%04x], nBitrate[0x%04x], nBandwidth[0x%04x]",
            pFmtp->nModeSetList, pFmtp->nBrList, pFmtp->nBwList);
    IMS_TRACE_D("GetFmtpFromString() - EVS : nMaxRed[%d], nReceivedChAwRecv[%d]", pFmtp->nMaxRed,
            pFmtp->nReceivedChAwRecv, 0);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioNego::FindEvsInProfile(IN AudioProfile* pLocalProfile,
        IN AudioProfile::Payload* pDstPayload, IN IMS_BOOL bIsOfferReceived,
        OUT IMS_UINT32* pBandwidthNegoList, OUT IMS_UINT32* pBitrateNegoList,
        OUT IMS_UINT32* pModeSetNegoList)
{
    if (pLocalProfile == IMS_NULL || pDstPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 legacyCheck = 0; legacyCheck < EVS_NEGO_RETRY_COUNT; legacyCheck++)
    {
        for (IMS_UINT32 i = 0; i < pLocalProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* comparedPayload = pLocalProfile->lstPayload.GetAt(i);
            if (comparedPayload == IMS_NULL)
            {
                continue;
            }

            if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
            {
                AudioProfile::EvsFmtp* pCompareFmtp =
                        (AudioProfile::EvsFmtp*)comparedPayload->pFmtp;
                AudioProfile::EvsFmtp* pReceivedFmtp = (AudioProfile::EvsFmtp*)pDstPayload->pFmtp;
                if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                            pDstPayload->objRtpMap.strPayloadType) == IMS_FALSE)
                {
                    continue;
                }

                // condition check....
                // need to return nego BW and BR, ModeSet(AMR IO Mode.). also default values...
                if (legacyCheck == 0)
                {
                    // IR92 rel15 EVS Br/Bw check.
                    if (CompareEvsBwBrMode(pCompareFmtp, pReceivedFmtp, bIsOfferReceived,
                                pBandwidthNegoList, pBitrateNegoList,
                                pModeSetNegoList) == IMS_FALSE)
                    {
                        continue;
                    }
                }
                else
                {
                    // legacy EVS BR/BW check
                    if (CompareEvsBwBrModeLegacy(pCompareFmtp, pReceivedFmtp, pBandwidthNegoList,
                                pBitrateNegoList, pModeSetNegoList) == IMS_FALSE)
                    {
                        continue;
                    }
                }

                // TODO - 20220415 - Need to implement this requirement later
                /*if (IMS_OPERATOR(VZW, GetSlotId()) && ((VZWProperty::GetInstance()->GetTestMask()
                        & TEST_MASK_UNLOCK_EVS_NEGO_LIMIT) == 0))
                {
                    //let channel aware mode option to be a negotitation item
                    if (pReceivedFmtp->nBwList == 1)
                    {
                        continue;      //vzw req. if offer is nb only, no evs negotiation
                }
                    if (pDstPayload->objRtpMap.nPayloadNum
                            != comparedPayload->objRtpMap.nPayloadNum)
                    {
                        continue;     //vzw req. payload number based negotiation
                    }
                }*/

                // check channel aware mode received param
                if (pReceivedFmtp->nReceivedChAwRecv > 0)
                {
                    IMS_SINT32 tempReceivedChAw = pReceivedFmtp->nReceivedChAwRecv;

                    if ((tempReceivedChAw != 2) && (tempReceivedChAw != 3) &&
                            (tempReceivedChAw != 5) && (tempReceivedChAw != 7))
                    {
                        continue;
                    }
                }

                // fixed for IR92 ver.12 newaly spec as below comment.
                // If ther SDP parameter ch-aw-recv is present
                // in the corresponding received and accepted SDP offer,
                // then the SDP parameter ch-aw-recv must be included
                // in the SDP answer with the same value as received.
                // In Offered case, set ChAw Mode for mine.
                if (((pReceivedFmtp->nBwList & 0x06) != 0 &&
                            (pReceivedFmtp->nBrList & 0x10) != 0) ||
                        ((*pBandwidthNegoList & 0x06) != 0 && (*pBitrateNegoList & 0x10) != 0))
                {
                    if (bIsOfferReceived != IMS_TRUE)
                    {
                        pReceivedFmtp->nChAwRecv = pCompareFmtp->nChAwRecv;
                        pReceivedFmtp->bShowChannelAwMode = pCompareFmtp->bShowChannelAwMode;
                    }
                }

                IMS_TRACE_D("FindEvsInProfile() Found EVS at[%d], pBandwidthNegoList[0x%04x], \
                        pBitrateNegoList[0x%04x]",
                        i, *pBandwidthNegoList, *pBitrateNegoList);
                IMS_TRACE_D("FindEvsInProfile() EVS ModeSwitch[%d], pModeSetNegoList[0x%04x], \
                        nSendCmr[%d]",
                        pReceivedFmtp->nEvsModeSwitch, *pModeSetNegoList, pCompareFmtp->bSendCmr);
                IMS_TRACE_D("FindEvsInProfile() EVS ChAwMode[%d], opposite ChAwMode[0x%04x], \
                        legacyCheck[%d]",
                        pReceivedFmtp->nChAwRecv, pReceivedFmtp->nReceivedChAwRecv, legacyCheck);

                return IMS_TRUE;
            }
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioNego::GetFmtpFromString(IN const AString& strFmtp, OUT AudioProfile::AmrFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objSplitColon = strFmtp.Split(';');

    for (IMS_UINT32 i = 0; i < objSplitColon.GetSize(); i++)
    {
        if (objSplitColon.GetAt(i).GetLength() == 0)
        {
            continue;
        }

        ImsList<AString> objSplitEqual = objSplitColon.GetAt(i).Split('=');

        if (objSplitEqual.GetSize() < 2)
        {
            const AString& strTmp = objSplitColon.GetAt(i);
            IMS_TRACE_D("GetFmtpFromString() - Invalid fmtp parameter(%s) at index[%d]",
                    strTmp.GetStr(), i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (objSplitEqual.GetAt(0).Equals("mode-set") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');

            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                IMS_UINT32 nModeSet = (IMS_UINT32)objSplitComma.GetAt(j).ToInt32();
                pFmtp->nModeSetList = (pFmtp->nModeSetList | (1 << nModeSet));
                pFmtp->bShowModeSet = IMS_TRUE;
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("octet-align") == IMS_TRUE)
        {
            pFmtp->nOctetAlign = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowOctetAlign = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-capability") == IMS_TRUE)
        {
            pFmtp->nModeChangeCapability = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowModeChangeCapability = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-period") == IMS_TRUE)
        {
            pFmtp->nModeChangePeriod = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowModeChangePeriod = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-neighbor") == IMS_TRUE)
        {
            pFmtp->nModeChangeNeighbor = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowModeChangeNeighbor = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("max-red") == IMS_TRUE)
        {
            pFmtp->nMaxRed = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowMaxRed = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("ptime") == IMS_TRUE)
        {
            pFmtp->nPtime = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowPtime = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("maxptime") == IMS_TRUE)
        {
            pFmtp->nMaxPtime = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShowMaxPtime = IMS_TRUE;
        }
    }

    IMS_TRACE_D("GetFmtpFromString() Ended. ModeSet[0x%04x], nOctetAlign[%d], nModeChangeCapa[%d]",
            pFmtp->nModeSetList, pFmtp->nOctetAlign, pFmtp->nModeChangeCapability);
    IMS_TRACE_D("GetFmtpFromString() Ended. nMaxRed[%d]", pFmtp->nMaxRed, 0, 0);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioNego::FindAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
        IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pnNegoModeSetList,
        OUT IMS_UINT32* pnNegoDefaultRtpModeSet)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bRetModeSetFound = IMS_FALSE;

    bRetModeSetFound = FindMatchedAmrInProfile(pProfile, pPayload, bIsOfferReceived,
            RETURN_MODE_MATCHED, pnNegoModeSetList, pnNegoDefaultRtpModeSet);
    IMS_TRACE_D("FindMatchedAmrInProfile() Ended. the 1st bRetModeSetFound: %d", bRetModeSetFound,
            0, 0);

    if (!bRetModeSetFound)
    {
        bRetModeSetFound = FindMatchedAmrInProfile(pProfile, pPayload, bIsOfferReceived,
                RETURN_MODE_SIMILAR, pnNegoModeSetList, pnNegoDefaultRtpModeSet);
        IMS_TRACE_D("FindMatchedAmrInProfile() Ended. the 2nd bRetModeSetFound: %d",
                bRetModeSetFound, 0, 0);
    }

    return bRetModeSetFound;
}

PRIVATE
IMS_BOOL AudioNego::FindMatchedAmrInProfile(IN AudioProfile* pProfile,
        IN AudioProfile::Payload* pPayload, IN IMS_BOOL bIsOfferReceived, IN IMS_BOOL bReturnMode,
        OUT IMS_UINT32* pnNegoModeSetList, OUT IMS_UINT32* pnNegoDefaultRtpModeSet)
{
    IMS_UINT32 nTempNegoModeSetList = 0;
    IMS_UINT32 nTempDefaultNegoModeSetList = 0;
    IMS_BOOL bModeSetFound = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* comparedPayload = pProfile->lstPayload.GetAt(i);
        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
                comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            AudioProfile::AmrFmtp* pCompareFmtp = (AudioProfile::AmrFmtp*)comparedPayload->pFmtp;
            AudioProfile::AmrFmtp* pReceivedFmtp = (AudioProfile::AmrFmtp*)pPayload->pFmtp;

            if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
            {
                continue;
            }
            if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                        pPayload->objRtpMap.strPayloadType) == IMS_FALSE)
            {
                continue;
            }
            if (comparedPayload->objRtpMap.nSamplingRate != pPayload->objRtpMap.nSamplingRate)
            {
                continue;
            }

            if (pCompareFmtp->nOctetAlign != pReceivedFmtp->nOctetAlign)
            {
                continue;
            }

            /** Fix for AMR Open Offer In case of MO, mode-set from MT could be mismatched Keep
            negotiated mode-set and try to find exact one */
            IMS_SINT32 nCompareResult = CompareModeSet(pCompareFmtp, pReceivedFmtp,
                    bIsOfferReceived, bReturnMode, pnNegoModeSetList, pnNegoDefaultRtpModeSet);

            if (nCompareResult == -1)
            {
                IMS_TRACE_D("FindAmrInProfile() nCompareResult - not match", 0, 0, 0);
                continue;  // mismatched
            }
            else if (nCompareResult == 0)  // similarly matched
            {
                IMS_TRACE_D("FindAmrInProfile() Enter similar match - bReturnMode: %d", bReturnMode,
                        0, 0);
                if (bModeSetFound == IMS_FALSE && bReturnMode == RETURN_MODE_SIMILAR)
                {
                    nTempNegoModeSetList = *pnNegoModeSetList;
                    nTempDefaultNegoModeSetList = *pnNegoDefaultRtpModeSet;
                    bModeSetFound = IMS_TRUE;
                    IMS_TRACE_I("FindAmrInProfile() Local/Peer is not exactly matched\
                            [0x%04x][0x%04x] =>[0x%04x]. Try next",
                            pCompareFmtp->nModeSetList, pReceivedFmtp->nModeSetList,
                            *pnNegoModeSetList);
                }
                continue;
            }
            else  // exactly matched
            {
                IMS_TRACE_D("FindAmrInProfile() Found AMR at[%d], Codec[%s], nOctetAlign[%d]", i,
                        comparedPayload->objRtpMap.strPayloadType.GetStr(),
                        pCompareFmtp->nOctetAlign);
                IMS_TRACE_D("FindAmrInProfile() Local/Peer is exactly matched[0x%04x][0x%04x] \
                        =>[0x%04x]",
                        pCompareFmtp->nModeSetList, pReceivedFmtp->nModeSetList,
                        *pnNegoModeSetList);

                return IMS_TRUE;
            }
        }
    }

    // 20160517 Fix for AMR Open Offer
    if (bModeSetFound == IMS_TRUE)
    {
        *pnNegoModeSetList = nTempNegoModeSetList;
        *pnNegoDefaultRtpModeSet = nTempDefaultNegoModeSetList;

        IMS_TRACE_D("FindAmrInProfile() Found Similar AMR with nModeSetList[0x%04x], "
                    "nDefaultModeSetList[0x%04x]",
                *pnNegoModeSetList, *pnNegoDefaultRtpModeSet, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioNego::FindPcmInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* comparedPayload = pProfile->lstPayload.GetAt(i);
        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU") ||
                comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMA"))
        {
            if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                        pPayload->objRtpMap.strPayloadType) == IMS_FALSE)
            {
                continue;
            }
            if (comparedPayload->objRtpMap.nSamplingRate != pPayload->objRtpMap.nSamplingRate)
            {
                continue;
            }

            IMS_TRACE_D("FindPcmInProfile() Found G.711 at[%d], Codec[%s], nSamplingRate[%d]", i,
                    comparedPayload->objRtpMap.strPayloadType.GetStr(),
                    pPayload->objRtpMap.nSamplingRate);

            return IMS_TRUE;
        }
    }

    // IMS_TRACE_D("FindPcmInProfile() Not Found G.711[%s], nSamplingRate[%d]",
    // pPayload->objRtpMap.strPayloadType.GetStr(), pPayload->objRtpMap.nSamplingRate, 0);

    return IMS_FALSE;
}

PRIVATE
IMS_SINT32 AudioNego::CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
        IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived, IN IMS_BOOL bReturnMode,
        OUT IMS_UINT32* nNegoModeSet, OUT IMS_UINT32* nNegoDefaultRtpModeSet)
{
    IMS_TRACE_I("CompareModeSet() - Src modeSet[0x%04x] Dest modeSet[0x%04x], bIsOfferReceived[%d]",
            pSrcFmtp->nModeSetList, pDestFmtp->nModeSetList, bIsOfferReceived);
    IMS_TRACE_I("CompareModeSet() - Src defaultmodeSet[0x%04x] Dest defaultmodeSet[0x%04x]",
            pSrcFmtp->nDefaultRtpModeSet, pDestFmtp->nDefaultRtpModeSet, 0);

    IMS_SINT32 nResult = 1;  // -1 : no matched, 0 : similar, 1 : exactly matched

    if (bIsOfferReceived == IMS_TRUE)  // MT Case
    {
        if (pDestFmtp->nModeSetList != 0)
        {
            *nNegoModeSet = pDestFmtp->nModeSetList;
        }
        else if (pSrcFmtp->nModeSetList != 0)
        {
            *nNegoModeSet = pSrcFmtp->nModeSetList;
        }
        else
        {
            *nNegoModeSet = 0;  // MO, MT both has no mode-set
            *nNegoDefaultRtpModeSet = pSrcFmtp->nDefaultRtpModeSet;
        }
    }
    else  // MO Case
    {
        if ((pSrcFmtp->nModeSetList == 0) && (pDestFmtp->nModeSetList == 0))
        {
            *nNegoModeSet = 0;
            *nNegoDefaultRtpModeSet = pSrcFmtp->nDefaultRtpModeSet;
        }
        else if ((pSrcFmtp->nModeSetList != 0) && (pDestFmtp->nModeSetList != 0))
        {
            *nNegoModeSet = pSrcFmtp->nModeSetList & pDestFmtp->nModeSetList;

            if (pSrcFmtp->nModeSetList != pDestFmtp->nModeSetList)
            {
                nResult = 0;
            }

            if (*nNegoModeSet == 0)
            {
                IMS_TRACE_D(
                        "CompareModeSet() - ModeSet Not Matched - isFinal: %d", bReturnMode, 0, 0);
                if (bReturnMode == RETURN_MODE_SIMILAR)
                {
                    *nNegoModeSet = pDestFmtp->nModeSetList;  // Copy Dest Mode-set to Nego Mode-set
                    IMS_TRACE_D("CompareModeSet() - Copy Dest Mode-set", 0, 0, 0);
                }
                else
                {
                    IMS_TRACE_E(0, "CompareModeSet() - ModeSet Not Matched...", 0, 0, 0);
                    return -1;
                }
            }
        }
        else  // one of two has no modeset
        {
            *nNegoModeSet = pSrcFmtp->nModeSetList | pDestFmtp->nModeSetList;
        }
    }

    IMS_TRACE_I(
            "CompareModeSet() Result[%d] Negotiated modeSet[0x%04x] nNegoDefaultRtpModeSet[0x%04x]",
            nResult, *nNegoModeSet, *nNegoDefaultRtpModeSet);

    return nResult;
}

PRIVATE
IMS_BOOL AudioNego::CompareEvsBwBrMode(IN AudioProfile::EvsFmtp* pSrcFmtp,
        IN AudioProfile::EvsFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
        OUT IMS_UINT32* nNegoBwList, OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList)
{
    IMS_TRACE_D("CompareEvsBwBrMode() - Src Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pSrcFmtp->nBwList, pSrcFmtp->nBrList, pSrcFmtp->nModeSetList);
    IMS_TRACE_D("CompareEvsBwBrMode() - Dest Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pDestFmtp->nBwList, pDestFmtp->nBrList, pDestFmtp->nModeSetList);

    if (m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // check EVS ModeSwitch
    if (pDestFmtp->nEvsModeSwitch == 1)  // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pSrcFmtp->nModeSetList == 0) && (pDestFmtp->nModeSetList == 0))
        {
            *nNegoModeList = 0;
        }
        else if ((pSrcFmtp->nModeSetList != 0) && (pDestFmtp->nModeSetList != 0))
        {
            *nNegoModeList = pSrcFmtp->nModeSetList & pDestFmtp->nModeSetList;

            if (*nNegoModeList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrMode() - AMR IO Mode - ModeSet Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoModeList = pSrcFmtp->nModeSetList | pDestFmtp->nModeSetList;
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        *nNegoBwList = pDestFmtp->nBwList;
        *nNegoBrList = pDestFmtp->nBrList;
    }
    else  // Primary Mode
    {
        // Set Bandwidth/Bitrate list.
        // 01. check Bandwidth
        if ((pSrcFmtp->nBwList == 0) && (pDestFmtp->nBwList == 0))
        {
            *nNegoBwList = 0;
        }
        else if ((pSrcFmtp->nBwList != 0) && (pDestFmtp->nBwList != 0))
        {
            // IR92 release15 EVS Answer Case.
            if (bIsOfferReceived == IMS_TRUE)
            {
                // check received EVS SWB only case (category B0, B1, B2 case)
                if (pDestFmtp->nBwList == 0x04)
                {
                    if (pDestFmtp->nBrList == 0x10)  // B0 received case.
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B0 Type Nego", 0,
                                0, 0);
                        *nNegoBwList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                    }
                    else if (pSrcFmtp->nBwList != 0x04)
                    {  // own EVS category is A. Do not negotiate with received category B.
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Not Config B Type Nego",
                                0, 0, 0);
                        return IMS_FALSE;
                    }
                    else
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B Type Nego", 0,
                                0, 0);
                        *nNegoBwList = pDestFmtp->nBwList;
                    }
                }
                else  // received EVS category A case
                {
                    // TODO - 20220415 - Need to implement this requirement later
                    /*// except for VZW operator. (only supported category B0 case.)
                    if (IMS_OPERATOR(VZW, GetSlotId()) == IMS_TRUE)
                    {
                        *nNegoBwList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                    }
                    else // GSMA IR92 case
                    {
                        // check own EVS SWB only capa. (category B0, B1, B2)
                        if (pSrcFmtp->nBwList == 0x04)
                        {
                            // this case, Do not negotiate with own category B. check next paylaod.
                            IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - \
                                    not support B Type Nego", 0, 0, 0);
                            return IMS_FALSE;
                        }
                        else
                        {
                            *nNegoBwList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                        }
                    }*/
                    // check own EVS SWB only capa. (category B0, B1, B2)
                    if (pSrcFmtp->nBwList == 0x04)
                    {
                        // this case, Do not negotiate with own category B. check next paylaod.
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - \
                                not support B Type Nego",
                                0, 0, 0);
                        return IMS_FALSE;
                    }
                    else
                    {
                        *nNegoBwList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                    }
                }
            }
            else
            {
                // TODO - 20220415 - Need to implement this requirement later
                /*// IR92 release15 EVS Answer Received Case.
                if (IMS_OPERATOR(VZW, GetSlotId()) == IMS_TRUE)
                {
                    *nNegoBwList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                }
                else
                {
                    if (pSrcFmtp->nBwList == 0x04 && pDestFmtp->nBwList != 0x04)
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - check next payload", 0, 0, 0);
                        return IMS_FALSE;
                    }
                    *nNegoBwList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                }*/
                if (pSrcFmtp->nBwList == 0x04 && pDestFmtp->nBwList != 0x04)
                {
                    IMS_TRACE_D("CompareEvsBwBrMode() - check next payload", 0, 0, 0);
                    return IMS_FALSE;
                }
                *nNegoBwList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
            }

            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bandwidth Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->nBwList != 0) &&
                ((pDestFmtp->nBwRecv != 0) || (pDestFmtp->nBwSend != 0)))
        {
            if (pDestFmtp->nBwRecv == 0)
            {
                pDestFmtp->nBwRecv = 0x0f;
            }
            if (pDestFmtp->nBwSend == 0)
            {
                pDestFmtp->nBwSend = 0x0f;
            }

            IMS_UINT32 nPeerBWList = pDestFmtp->nBwRecv & pDestFmtp->nBwSend;

            *nNegoBwList = pSrcFmtp->nBwList & nPeerBWList;
            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bandwidth Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBwList = pSrcFmtp->nBwList | pDestFmtp->nBwList;
        }

        // 02. check Bitrate

        if ((pSrcFmtp->nBrList == 0) && (pDestFmtp->nBrList == 0))
        {
            *nNegoBrList = 0;
        }
        else if ((pSrcFmtp->nBrList != 0) && (pDestFmtp->nBrList != 0))
        {
            // IR92 release15 EVS Answer Case.
            if (bIsOfferReceived == IMS_TRUE)
            {
                *nNegoBrList = pSrcFmtp->nBrList & pDestFmtp->nBrList;
                if ((*nNegoBwList == 0x04) && (*nNegoBrList == 0x10))
                {  // 13.2kbsp swb only case
                    IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B0,B1 Type Nego", 0,
                            0, 0);
                    *nNegoBrList = (pDestFmtp->nBrList) & 0x1f;  // negotiated ~13.2kbps
                }
            }
            else
            {
                *nNegoBrList = pSrcFmtp->nBrList & pDestFmtp->nBrList;
            }

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bitrate Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->nBrList != 0) && (pDestFmtp->nBrRecv != 0 || (pDestFmtp->nBrSend != 0)))
        {
            if (pDestFmtp->nBrRecv == 0)
            {
                pDestFmtp->nBrRecv = 0x0fff;
            }
            if (pDestFmtp->nBrSend == 0)
            {
                pDestFmtp->nBrSend = 0x0fff;
            }

            IMS_UINT32 nPeerBRList = pDestFmtp->nBrRecv & pDestFmtp->nBrSend;

            *nNegoBrList = pSrcFmtp->nBrList & nPeerBRList;

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bitrate Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBrList = pSrcFmtp->nBrList | pDestFmtp->nBrList;
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pDestFmtp->nModeSetList != 0)
        {
            *nNegoModeList = pDestFmtp->nModeSetList;
        }
    }
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioNego::CompareEvsBwBrModeLegacy(IN AudioProfile::EvsFmtp* pSrcFmtp,
        IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
        OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList)
{
    IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Src BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pSrcFmtp->nBwList, pSrcFmtp->nBrList, pSrcFmtp->nModeSetList);
    IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Dest BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pDestFmtp->nBwList, pDestFmtp->nBrList, pDestFmtp->nModeSetList);

    // check EVS ModeSwitch
    if (pDestFmtp->nEvsModeSwitch == 1)  // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pSrcFmtp->nModeSetList == 0) && (pDestFmtp->nModeSetList == 0))
        {
            *nNegoModeList = 0;
        }
        else if ((pSrcFmtp->nModeSetList != 0) && (pDestFmtp->nModeSetList != 0))
        {
            *nNegoModeList = pSrcFmtp->nModeSetList & pDestFmtp->nModeSetList;

            if (*nNegoModeList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - AMR IO Mode - ModeSet Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoModeList = pSrcFmtp->nModeSetList | pDestFmtp->nModeSetList;
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        {
            *nNegoBwList = pDestFmtp->nBwList;
            *nNegoBrList = pDestFmtp->nBrList;
        }
    }
    else  // Primary Mode
    {
        // Set Bandwidth/Bitrate list.
        // 01. check Bandwidth
        if ((pSrcFmtp->nBwList == 0) && (pDestFmtp->nBwList == 0))
        {
            *nNegoBwList = 0;
        }
        else if ((pSrcFmtp->nBwList != 0) && (pDestFmtp->nBwList != 0))
        {
            *nNegoBwList = pSrcFmtp->nBwList & pDestFmtp->nBwList;

            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->nBwList != 0) &&
                ((pDestFmtp->nBwRecv != 0) || (pDestFmtp->nBwSend != 0)))
        {
            if (pDestFmtp->nBwRecv == 0)
            {
                pDestFmtp->nBwRecv = 0x0f;
            }
            if (pDestFmtp->nBwSend == 0)
            {
                pDestFmtp->nBwSend = 0x0f;
            }

            IMS_UINT32 nPeerBWList = pDestFmtp->nBwRecv & pDestFmtp->nBwSend;

            *nNegoBwList = pSrcFmtp->nBwList & nPeerBWList;
            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBwList = pSrcFmtp->nBwList | pDestFmtp->nBwList;
        }

        // 02. check Bitrate

        if ((pSrcFmtp->nBrList == 0) && (pDestFmtp->nBrList == 0))
        {
            *nNegoBrList = 0;
        }
        else if ((pSrcFmtp->nBrList != 0) && (pDestFmtp->nBrList != 0))
        {
            *nNegoBrList = pSrcFmtp->nBrList & pDestFmtp->nBrList;

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bitrate Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->nBrList != 0) && (pDestFmtp->nBrRecv != 0 || (pDestFmtp->nBrSend != 0)))
        {
            if (pDestFmtp->nBrRecv == 0)
            {
                pDestFmtp->nBrRecv = 0x0fff;
            }
            if (pDestFmtp->nBrSend == 0)
            {
                pDestFmtp->nBrSend = 0x0fff;
            }

            IMS_UINT32 nPeerBRList = pDestFmtp->nBrRecv & pDestFmtp->nBrSend;

            *nNegoBrList = pSrcFmtp->nBrList & nPeerBRList;

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bitrate Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBrList = pSrcFmtp->nBrList | pDestFmtp->nBrList;
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pDestFmtp->nModeSetList != 0)
        {
            *nNegoModeList = pDestFmtp->nModeSetList;
        }
    }
    return IMS_TRUE;
}

PRIVATE IMS_SINT32 AudioNego::FindPayloadIndexFromProfile(IN const AString& strCodecName,
        IN AudioProfile* pLocalProfile, IN AudioProfile::Payload* pDstPayload,
        IN IMS_BOOL bIsOfferReceived)
{
    if (pLocalProfile == IMS_NULL || pDstPayload == IMS_NULL)
    {
        return -1;
    }

    IMS_SINT32 nRetIndex = -1;

    IMS_TRACE_D("FindPayloadIndexFromProfile()", 0, 0, 0);

    nRetIndex = FindMatchedPayloadIndexFromProfile(
            strCodecName, pLocalProfile, pDstPayload, bIsOfferReceived, RETURN_MODE_MATCHED);

    IMS_TRACE_D("FindPayloadIndexFromProfile() the 1st nRetIndex: %d", nRetIndex, 0, 0);

    if (nRetIndex == -1)
    {
        nRetIndex = FindMatchedPayloadIndexFromProfile(
                strCodecName, pLocalProfile, pDstPayload, bIsOfferReceived, RETURN_MODE_SIMILAR);
        IMS_TRACE_D("FindPayloadIndexFromProfile() the 2nd nRetIndex: %d", nRetIndex, 0, 0);
    }

    return nRetIndex;
}

PRIVATE IMS_SINT32 AudioNego::FindMatchedPayloadIndexFromProfile(IN const AString& strCodecName,
        IN AudioProfile* pLocalProfile, IN AudioProfile::Payload* pDstPayload,
        IN IMS_BOOL bIsOfferReceived, IN IMS_BOOL bReturnMode)
{
    IMS_SINT32 nTempIndex = -1;

    IMS_TRACE_D("FindMatchedPayloadIndexFromProfile()", 0, 0, 0);

    for (IMS_UINT32 legacyCheck = 0; legacyCheck < EVS_NEGO_RETRY_COUNT; legacyCheck++)
    {
        for (IMS_UINT32 i = 0; i < pLocalProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* comparedPayload = pLocalProfile->lstPayload.GetAt(i);
            if (comparedPayload == IMS_NULL)
            {
                continue;
            }

            if (strCodecName.EqualsIgnoreCase("AMR") || strCodecName.EqualsIgnoreCase("AMR-WB"))
            {
                if (legacyCheck >= 1)
                {
                    continue;
                }
                if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
                        comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
                {
                    IMS_UINT32 pnNegoModeSetList = 0;
                    IMS_UINT32 pnNegoDefaultRtpModeSet = 0;
                    AudioProfile::AmrFmtp* pCompareFmtp =
                            (AudioProfile::AmrFmtp*)comparedPayload->pFmtp;
                    AudioProfile::AmrFmtp* pReceivedFmtp =
                            (AudioProfile::AmrFmtp*)pDstPayload->pFmtp;

                    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                                pDstPayload->objRtpMap.strPayloadType) == IMS_FALSE)
                    {
                        continue;
                    }
                    if (comparedPayload->objRtpMap.nSamplingRate !=
                            pDstPayload->objRtpMap.nSamplingRate)
                    {
                        continue;
                    }

                    // TODO - 20220415 - Need to implement this requirement later
                    /*if (IMS_OPERATOR(SKT, GetSlotId()) && bIsOfferReceived == IMS_TRUE)
                    {
                        IMS_TRACE_D("FindPayloadIndexFromProfile - ignore AMR octet align case",
                                0, 0, 0);
                    }
                    else
                    {
                        if (pCompareFmtp->nOctetAlign != pReceivedFmtp->nOctetAlign )
                        {
                            continue;
                        }
                    }*/
                    if (pCompareFmtp->nOctetAlign != pReceivedFmtp->nOctetAlign)
                    {
                        continue;
                    }

                    // 20160517 Fix for AMR Open Offer
                    // In case of MO, mode-set from MT could be mismatched
                    // => Keep negotiated mode-set and try to find exact one

                    IMS_SINT32 nCompareResult =
                            CompareModeSet(pCompareFmtp, pReceivedFmtp, bIsOfferReceived,
                                    bReturnMode, &pnNegoModeSetList, &pnNegoDefaultRtpModeSet);
                    if (nCompareResult == -1)
                    {
                        continue;  // mismatched
                    }
                    else if (nCompareResult == 0)  // similarly matched
                    {
                        IMS_TRACE_D("isFinal: %d", bReturnMode, 0, 0);
                        if (nTempIndex == -1 && bReturnMode == RETURN_MODE_SIMILAR)
                        {
                            nTempIndex = i;
                            IMS_TRACE_I("FindPayloadIndexFromProfile() Found Similar AMR at[%d], \
                                    Codec[%s], nOctetAlign[%d]",
                                    i, comparedPayload->objRtpMap.strPayloadType.GetStr(),
                                    pCompareFmtp->nOctetAlign);
                            IMS_TRACE_I("FindPayloadIndexFromProfile() Local/Peer is not exactly \
                                    matched[0x%04x][0x%04x] =>[0x%04x]. Try next",
                                    pCompareFmtp->nModeSetList, pReceivedFmtp->nModeSetList,
                                    pnNegoModeSetList);
                        }
                        continue;
                    }
                    else  // exactly matched
                    {
                        IMS_TRACE_D("FindPayloadIndexFromProfile() Found AMR at[%d], Codec[%s], \
                                nOctetAlign[%d]",
                                i, comparedPayload->objRtpMap.strPayloadType.GetStr(),
                                pCompareFmtp->nOctetAlign);

                        return i;
                    }
                }
            }
            else if (strCodecName.EqualsIgnoreCase("EVS"))
            {
                if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
                {
                    IMS_UINT32 BandwidthNegoList;
                    IMS_UINT32 BitrateNegoList;
                    IMS_UINT32 ModeSetNegoList;
                    AudioProfile::EvsFmtp* pCompareFmtp =
                            (AudioProfile::EvsFmtp*)comparedPayload->pFmtp;
                    AudioProfile::EvsFmtp* pReceivedFmtp =
                            (AudioProfile::EvsFmtp*)pDstPayload->pFmtp;

                    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                                pDstPayload->objRtpMap.strPayloadType) == IMS_FALSE)
                    {
                        continue;
                    }
                    // need to return nego BW and BR, ModeSet(AMR IO Mode.). also default values...
                    if (legacyCheck == 0)
                    {
                        // IR92 rel15 EVS Br/Bw check.
                        if (CompareEvsBwBrMode(pCompareFmtp, pReceivedFmtp, bIsOfferReceived,
                                    &BandwidthNegoList, &BitrateNegoList,
                                    &ModeSetNegoList) == IMS_FALSE)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        // legacy EVS BR/BW check
                        if (CompareEvsBwBrModeLegacy(pCompareFmtp, pReceivedFmtp,
                                    &BandwidthNegoList, &BitrateNegoList,
                                    &ModeSetNegoList) == IMS_FALSE)
                        {
                            continue;
                        }
                    }

                    // TODO - 20220415 - Need to implement this requirement later
                    /*if (IMS_OPERATOR(VZW, GetSlotId()) &&
                            ((VZWProperty::GetInstance()->GetTestMask()
                            & TEST_MASK_UNLOCK_EVS_NEGO_LIMIT) == 0))
                    {
                        //let channel aware mode option to be a negotitation item
                        if (pReceivedFmtp->nBwList == 1)
                        {
                            continue;      //vzw req. if offer is nb only, no evs negotiation
                        }
                        if (pDstPayload->objRtpMap.nPayloadNum !=
                                comparedPayload->objRtpMap.nPayloadNum)
                        {
                            continue;     //vzw req. payload number based negotiation
                        }
                    }*/

                    IMS_TRACE_D("FindPayloadIndexFromProfile() Found EVS at[%d], Codec[%s], \
                            nEvsModeSwitch[%d]",
                            i, comparedPayload->objRtpMap.strPayloadType.GetStr(),
                            pCompareFmtp->nEvsModeSwitch);

                    return i;
                }
            }
            //[G711]
            else if (strCodecName.EqualsIgnoreCase("PCMU") || strCodecName.EqualsIgnoreCase("PCMA"))
            {
                if (legacyCheck >= 1)
                {
                    continue;
                }
                if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                            pDstPayload->objRtpMap.strPayloadType))
                {
                    IMS_TRACE_D("FindPayloadIndexFromProfile() Found G.711(%s) Found at[%d]",
                            comparedPayload->objRtpMap.strPayloadType.GetStr(), i, 0);

                    return i;
                }
            }
        }
    }

    // Fix for AMR Open Offer
    if (nTempIndex > -1)
    {
        IMS_TRACE_D("FindPayloadIndexFromProfile() Found Similar AMR at[%d]", nTempIndex, 0, 0);
        return nTempIndex;
    }

    return -1;
}

PRIVATE
MEDIA_DIRECTION AudioNego::UpdateDirectionToMine(
        IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase)
{
    if (ePeerDir < MEDIA_DIRECTION_INACTIVE || ePeerDir > MEDIA_DIRECTION_SEND_RECEIVE)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_D("UpdateDirectionToMine() ePeerDir[%d], eSrcDir[%d], bIsMtCase[%d]", ePeerDir,
            eSrcDir, bIsMtCase);

    MEDIA_DIRECTION eNegotiatedDir = MEDIA_DIRECTION_INVALID;

    switch (ePeerDir)
    {
        case MEDIA_DIRECTION_INACTIVE:
        case MEDIA_DIRECTION_SEND_RECEIVE:
            eNegotiatedDir = ePeerDir;
            break;
        case MEDIA_DIRECTION_RECEIVE:
            eNegotiatedDir = MEDIA_DIRECTION_SEND;
            break;
        case MEDIA_DIRECTION_SEND:
            eNegotiatedDir = MEDIA_DIRECTION_RECEIVE;
            break;
        default:
            return MEDIA_DIRECTION_INVALID;
    }

    if (bIsMtCase == IMS_FALSE)
    {
        // direction check strictly
        if ((eSrcDir == MEDIA_DIRECTION_SEND &&
                    (ePeerDir == MEDIA_DIRECTION_SEND ||
                            ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE)) ||
                (eSrcDir == MEDIA_DIRECTION_RECEIVE &&
                        (ePeerDir == MEDIA_DIRECTION_RECEIVE ||
                                ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE)) ||
                (eSrcDir == MEDIA_DIRECTION_INACTIVE && (ePeerDir != MEDIA_DIRECTION_INACTIVE)))
        {
            return MEDIA_DIRECTION_INVALID;
        }
    }

    return eNegotiatedDir;
}

PRIVATE
IMS_BOOL AudioNego::MakeCapaNegoProfileFromSdp(
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile::CapaNego* pObjCapaNego)
{
    if (pDescriptor == IMS_NULL || pObjCapaNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> lstTcapAttr = pDescriptor->GetAttributes(SdpAttribute::TCAP);
    ImsList<AString> lstAcapAttr = pDescriptor->GetAttributes(SdpAttribute::ACAP);
    ImsList<AString> lstAcfgAttr = pDescriptor->GetAttributes(SdpAttribute::ACFG);

    if (lstAcfgAttr.GetSize() > 0)
    {
        pObjCapaNego->strNegotiatedAcfg = lstAcfgAttr.GetAt(0);
        IMS_TRACE_I("GetCapaNegoValueFromSdp() - ACFG[%s]",
                pObjCapaNego->strNegotiatedAcfg.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    // Get Potential configuration list (pcfg) -"'prio #' SP"t=Tcap #' SP 'a=Acap #'" pair
    pObjCapaNego->lstPotentialConfig = pDescriptor->GetAttributes(SdpAttribute::PCFG);

    IMS_TRACE_I("GetCapaNegoValueFromSdp() - PcfgSize[%d], TcapSize[%d], AcapSize[%d]",
            pObjCapaNego->lstPotentialConfig.GetSize(), pObjCapaNego->mapTransportCapa.GetSize(),
            pObjCapaNego->mapAttributeCapa.GetSize());

    // Get transport capability(TCAP) list -"'number' SP 'Tcap'" pair
    for (IMS_UINT32 i = 0; i < lstTcapAttr.GetSize(); i++)
    {
        AString strTcapline = lstTcapAttr.GetAt(i);
        if (strTcapline.GetLength() == 0)
        {
            continue;
        }

        ImsList<AString> lstSplitSpace = strTcapline.Split(' ');
        IMS_SINT32 nTcapInitNum = 0;

        // save Tcap String to CapaNego Obj
        for (IMS_UINT32 j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            if (j == 0)
            {
                nTcapInitNum = lstSplitSpace.GetAt(j).ToInt32();
                IMS_TRACE_I("GetCapaNegoValueFromSdp() - nTcapInitNum[%d]", nTcapInitNum, 0, 0);
            }
            else
            {
                AString strTcap = "";
                // mapped - key : 'number' value:'Tcap'
                strTcap.Sprintf("%s", lstSplitSpace.GetAt(j).GetStr());
                pObjCapaNego->mapTransportCapa.Add(nTcapInitNum, strTcap);
                nTcapInitNum++;
            }
        }
    }

    // Get attribute capability(ACAP) list -"'number' SP 'Acap'" pair
    for (IMS_UINT32 i = 0; i < lstAcapAttr.GetSize(); i++)
    {
        IMS_SINT32 nAcapNum = 0;
        AString strAcap = "";

        AString strAcapline = lstAcapAttr.GetAt(i);
        if (strAcapline.GetLength() == 0)
        {
            continue;
        }

        ImsList<AString> lstSplitSpace = strAcapline.Split(' ');

        // save Acap String to CapaNego Obj
        for (IMS_UINT32 j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            if (j == 0)
            {
                nAcapNum = lstSplitSpace.GetAt(j).ToInt32();
            }
            else
            {
                if (strAcap.GetLength() == 0)
                {
                    strAcap.Append(lstSplitSpace.GetAt(j));
                }
                else
                {
                    strAcap.Append("" + lstSplitSpace.GetAt(j));
                }
            }
        }
        // save Acap String...
        if (strAcap.GetLength() != 0)
        {
            IMS_TRACE_I(
                    "GetCapaNegoValueFromSdp() - add map[%d - %s]", nAcapNum, strAcap.GetStr(), 0);
            pObjCapaNego->mapAttributeCapa.Add(nAcapNum, strAcap);
        }
    }
    if (lstAcapAttr.GetSize() > 0)
    {
        pObjCapaNego->bIsAttCapaInPcfg = IMS_TRUE;
    }

    if (pObjCapaNego->mapTransportCapa.GetSize() == 0)
    {
        IMS_TRACE_I("GetCapaNegoValueFromSdp() - Therer are no Tcap value in SDP", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pObjCapaNego->lstPotentialConfig.GetSize() == 0)
    {
        IMS_TRACE_I("GetCapaNegoValueFromSdp() - Therer are no PCFG value in SDP", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
AudioNego::OaModel* AudioNego::GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed)
{
    IMS_UINT32 nOaModelCount = m_lstOaModel.GetSize();
    IMS_UINT32 nTempOaModelCount = nOaModelCount;
    while (nTempOaModelCount > 0)
    {
        OaModel* pLatestOaModel = m_lstOaModel.GetAt(nTempOaModelCount - 1);
        if (pLatestOaModel != IMS_NULL)
        {
            if ((pLatestOaModel->IsAllProfileExist() == IMS_TRUE && bCheckConfirmed == IMS_FALSE) ||
                    (pLatestOaModel->bConfirmedSession == IMS_TRUE && bCheckConfirmed == IMS_TRUE))
            {
                return pLatestOaModel;
            }

            IMS_TRACE_I("GetNegotiatedOaModel() - [%d/%d]th is not perfect. Try next",
                    nTempOaModelCount, nOaModelCount, 0);
        }
        nTempOaModelCount--;
    }

    return IMS_NULL;
}

PRIVATE void AudioNego::SetSdpSessionIpAddress(
        OUT ISessionDescriptor* pSessionDescriptor, IN AudioProfile* pProfile)
{
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->objIpAddress))
    {
        IMS_TRACE_D("SetSdpSessionIpAddress() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->objIpAddress.ToCharString(), 0);

        pSessionDescriptor->SetConnectionAddress(pProfile->objIpAddress.ToString());
        pSessionDescriptor->SetOriginAddress(pProfile->objIpAddress.ToString());
    }
}

PRIVATE void AudioNego::SetSdpMediaDescription(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    AStringArray objAudioFormat;
    AString strPayloadNum;
    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->objRtpMap.nPayloadNum);
        objAudioFormat.AddElement(strPayloadNum);
    }

    // Set transport type and port number
    pDescriptor->SetMediaDescription(
            SdpMedia::TYPE_AUDIO, pProfile->nDataPort, SdpMedia::TRANSPORT_RTP_AVP, objAudioFormat);
}

PRIVATE void AudioNego::SetSdpMediaBandwidth(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pProfile->nBandwidthAs > 0)
    {
        pDescriptor->AddBandwidth(SdpBandwidth::TYPE_AS, pProfile->nBandwidthAs);

        if (pProfile->nBandwidthRs >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RS, pProfile->nBandwidthRs);
        }

        if (pProfile->nBandwidthRr >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RR, pProfile->nBandwidthRr);
        }
    }
}
