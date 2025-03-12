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

#include "ISessionDescriptor.h"
#include "ServiceTrace.h"

#include "MediaProfileFactory.h"
#include "MediaProfileUtil.h"
#include "audio/AudioNego.h"
#include "audio/AudioProfileGenerator.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaConfigUtil.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
AudioNego::AudioNego(IMS_SINT32 nSlotId) :
        BaseNego(nSlotId, MEDIA_TYPE_AUDIO),
        m_pSdpParser(std::make_unique<AudioSdpParser>())
{
    IMS_TRACE_I("+AudioNego() - slot[%d]", nSlotId, 0, 0);
    m_pSdpGenerator = std::make_shared<AudioSdpGenerator>();
    m_pProfileNegotiator = std::make_shared<AudioProfileNegotiator>();
    m_pProfileGenerator = std::make_shared<AudioProfileGenerator>();
}

PUBLIC
AudioNego::AudioNego(IN const AudioNego& objAudioNego) :
        BaseNego(objAudioNego.GetSlotId())
{
    Copy(&objAudioNego);
}

PUBLIC
AudioNego& AudioNego::operator=(IN const AudioNego& obj)
{
    if (this != &obj)
    {
        Copy(&obj);
    }
    return (*this);
}

PUBLIC VIRTUAL AudioNego::~AudioNego()
{
    IMS_TRACE_I("~AudioNego()", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL AudioNego::IsMediaCodecFromSdpSupported(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        return MEDIA_TYPE_INVALID;
    }

    IMS_TRACE_I("IsMediaCodecFromSdpSupported()", 0, 0, 0);

    OaModel objOaModel;
    objOaModel.pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO, m_pBaseProfile);

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(&objOaModel)) !=
            IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (std::static_pointer_cast<AudioProfileNegotiator>(m_pProfileNegotiator)
                    ->Negotiate(GetLocalProfile(&objOaModel), GetPeerProfile(&objOaModel), IMS_TRUE,
                            GetNegotiatedProfile(&objOaModel), m_pConfig) != IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    return (objOaModel.pNegotiatedProfile->GetPayloadList().GetSize() > 0 &&
                   objOaModel.pNegotiatedProfile->GetDataPort() != 0)
            ? IMS_TRUE
            : IMS_FALSE;
}

PUBLIC VIRTUAL AUDIO_CODEC_BITRATE AudioNego::GetNegotiatedAudioCodecRate(void)
{
    MediaBaseProfile::BasePayload* pNegotiatedPayload = GetNegotiatedPayload();

    if (pNegotiatedPayload == NULL)
    {
        return AUDIO_CODEC_BITRATE_MAX;
    }

    if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
            pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
    {
        IMS_SINT32 nLargestModeSet = -1;

        if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            nLargestModeSet = AudioProfileUtil::GetLargestModesetInFmtp(
                                      "AMR-WB", PayloadCasting(pNegotiatedPayload)) +
                    AUDIO_CODEC_BITRATE_AMR_WB_660;
            return (AUDIO_CODEC_BITRATE)nLargestModeSet;
        }
        else  // AMR case
        {
            nLargestModeSet = AudioProfileUtil::GetLargestModesetInFmtp(
                                      "AMR", PayloadCasting(pNegotiatedPayload)) +
                    AUDIO_CODEC_BITRATE_AMR_475;
            return (AUDIO_CODEC_BITRATE)nLargestModeSet;
        }
    }
    else if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp =
                (AudioProfile::EvsFmtp*)PayloadCasting(pNegotiatedPayload)->GetFmtp();
        if (pEvsFmtp == IMS_NULL)
        {
            return AUDIO_CODEC_BITRATE_INVALID;
        }

        IMS_SINT32 nLargestModeSet = -1;
        nLargestModeSet = AudioProfileUtil::GetLargestModesetInFmtp(
                "EVS", PayloadCasting(pNegotiatedPayload));
        // primary mode
        if (pEvsFmtp->GetEvsModeSwitch() != 1)
        {
            nLargestModeSet = nLargestModeSet + AUDIO_CODEC_BITRATE_EVS_590;
            // check channel aware mode case
            if (nLargestModeSet == AUDIO_CODEC_BITRATE_EVS_1320)
            {
                if (pEvsFmtp->GetChAwRecv() == 2)
                {
                    nLargestModeSet = AUDIO_CODEC_BITRATE_EVS_1320_CHA_2;
                }
                else if (pEvsFmtp->GetChAwRecv() == 3)
                {
                    nLargestModeSet = AUDIO_CODEC_BITRATE_EVS_1320_CHA_3;
                }
                else if (pEvsFmtp->GetChAwRecv() == 5)
                {
                    nLargestModeSet = AUDIO_CODEC_BITRATE_EVS_1320_CHA_5;
                }
                else if (pEvsFmtp->GetChAwRecv() == 7)
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
    MediaBaseProfile::BasePayload* pPayload = GetNegotiatedPayload();

    if (pPayload == IMS_NULL)
    {
        return AUDIO_CODEC_NONE;
    }

    IMS_TRACE_D("GetNegotiatedCodec() - Negotiated Payload Type is [%s]",
            pPayload->GetRtpMap().GetPayloadType().GetStr(), 0, 0);

    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
    {
        return AUDIO_CODEC_AMRWB;
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR"))
    {
        return AUDIO_CODEC_AMR;
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pPayload->GetFmtp();

        if (pEvsFmtp == IMS_NULL)
        {
            return AUDIO_CODEC_EVS;
        }
        if (pEvsFmtp->GetEvsModeSwitch() == 1)
        {
            return AUDIO_CODEC_EVS_WB;
        }
        if ((pEvsFmtp->GetBwList() & 0x04) != 0)
        {
            return AUDIO_CODEC_EVS_SWB;
        }
        else if ((pEvsFmtp->GetBwList() & 0x02) != 0)
        {
            return AUDIO_CODEC_EVS_WB;
        }
        else if ((pEvsFmtp->GetBwList() & 0x01) != 0)
        {
            return AUDIO_CODEC_EVS_NB;
        }
        return AUDIO_CODEC_EVS;
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU"))
    {
        return AUDIO_CODEC_G711_PCMU;
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"))
    {
        return AUDIO_CODEC_G711_PCMA;
    }

    return AUDIO_CODEC_NONE;
}

PUBLIC VIRTUAL IMS_BOOL AudioNego::HasNegotiatedDtmf(void)
{
    if (m_listOaModel.GetSize() > 0)
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

        for (IMS_UINT32 i = 0; i < pLatestOaModel->pNegotiatedProfile->GetPayloadList().GetSize();
                i++)
        {
            AudioProfile::Payload* pPayload = GetNegotiatedProfile(pLatestOaModel)->GetPayloadAt(i);

            if (pPayload == IMS_NULL)
            {
                continue;
            }

            if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
            {
                IMS_TRACE_I("HasNegotiatedDtmf() - Negotiated DTMF found[%d]", i, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC AudioConfiguration* AudioNego::ConfigCasting(IN MediaConfiguration* pConfig)
{
    return (pConfig != IMS_NULL) ? static_cast<AudioConfiguration*>(pConfig) : IMS_NULL;
}

PUBLIC AudioProfile* AudioNego::ProfileCasting(IN MediaBaseProfile* pProfile)
{
    return (pProfile != IMS_NULL) ? static_cast<AudioProfile*>(pProfile) : IMS_NULL;
}

PUBLIC AudioProfile::Payload* AudioNego::PayloadCasting(IN MediaBaseProfile::BasePayload* pPayload)
{
    return (pPayload != IMS_NULL) ? static_cast<AudioProfile::Payload*>(pPayload) : IMS_NULL;
}

PROTECTED AudioProfile* AudioNego::GetLocalProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetLocalProfile(pOaModel));
}

PROTECTED AudioProfile* AudioNego::GetPeerProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetPeerProfile(pOaModel));
}

PROTECTED AudioProfile* AudioNego::GetNegotiatedProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetNegotiatedProfile(pOaModel));
}

PROTECTED
IMS_BOOL AudioNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable)
{
    IMS_TRACE_D("FormAnswer() - eDir[%d]", eDir, 0, 0);

    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_listOaModel.GetSize() == 0 ||
            m_pSdpGenerator == IMS_NULL)
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

        *pNewOaModel->pLocalProfile = m_pBaseProfile;

        if (pNewOaModel->pNegotiatedProfile != IMS_NULL)
        {
            delete pNewOaModel->pNegotiatedProfile;
        }

        pNewOaModel->pNegotiatedProfile =
    MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO); if
    (Negotiate(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_TRUE,
                    pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
        {
            delete pNewOaModel;
            return IMS_FALSE;
        }
    }*/

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID)
    {
        pNewOaModel->pNegotiatedProfile->SetDirection(eDir);
        IMS_TRACE_I("FormAnswer() - update audio direction[%d]", eDir, 0, 0);
    }

    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pNegotiatedProfile->SetDataPort(0);
        pNewOaModel->pNegotiatedProfile->SetControlPort(0);
    }

    // Make the SDP from profile
    IMS_BOOL bSdpMade = IMS_FALSE;

    bSdpMade =
            std::static_pointer_cast<AudioSdpGenerator>(m_pSdpGenerator)
                    ->Generate(pSessionDescriptor, pDescriptor, GetNegotiatedProfile(pNewOaModel));

    if (pSessionDescriptor->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        // Delete Session Level Direction Attribute
        pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    return bSdpMade;
}

PROTECTED
IMS_BOOL AudioNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormReoffer() pDescriptor[%" PFLS_x "], eDir[%d], listOaModel Size[%d]",
            pDescriptor, eDir, m_listOaModel.GetSize());
    IMS_TRACE_D("FormReoffer() - EnforceReofferMode[%d]", bEnforceReofferMode, 0, 0);

    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pSdpGenerator == IMS_NULL)
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

    if (m_listOaModel.GetSize() == 0)
    {
        pNewOaModel->pLocalProfile =
                MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO, m_pBaseProfile);
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
                    pPrevOaModel->pNegotiatedProfile->GetDataPort() == 0))
        {
            pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                    MEDIA_TYPE_AUDIO, GetNegotiatedProfile(pPrevOaModel));
        }
        else
        {
            if (bEnforceReofferMode == IMS_TRUE)
            {
                pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                        MEDIA_TYPE_AUDIO, m_pBaseProfile);
            }
            else
            {
                IMS_TRACE_I("FormReoffer() - reuse previous profile, SdpReofferFullCapability[%d]",
                        pMediaSessionConfig->IsSdpReofferFullCapability(), 0, 0);

                if (pMediaSessionConfig->IsSdpReofferFullCapability() == IMS_TRUE)
                {
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            MEDIA_TYPE_AUDIO, m_pBaseProfile);
                }
                else if (pPrevOaModel->pNegotiatedProfile != IMS_NULL)
                {
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            MEDIA_TYPE_AUDIO, GetNegotiatedProfile(pPrevOaModel));
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
    if (pNewOaModel->pLocalProfile->GetBandwidthAs() <= 0)
    {
        IMS_TRACE_I("FormReoffer() - use default AS value", 0, 0, 0);
        pNewOaModel->pLocalProfile->SetBandwidthAs(m_pBaseProfile->GetBandwidthAs());
    }

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_I("FormReoffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pLocalProfile->SetDirection(eDir);
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    MediaProfileUtil::SetRtcpRsRr(GetLocalProfile(pNewOaModel),
            MediaConfigUtil::GetAudioConfig(GetSlotId(), m_pEnvironment->eServiceType));

    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pLocalProfile->SetDataPort(0);
        pNewOaModel->pLocalProfile->SetControlPort(0);
    }
    else
    {
        pNewOaModel->pLocalProfile->SetDataPort(m_pBaseProfile->GetDataPort());
        pNewOaModel->pLocalProfile->SetControlPort(m_pBaseProfile->GetControlPort());
    }

    // when reoffer case - recover rtcpxr to default in sendrecv case
    if (ProfileCasting(m_pBaseProfile)->IsRtcpXrSupported() == IMS_TRUE &&
            pNewOaModel->pLocalProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        GetLocalProfile(pNewOaModel)
                ->SetSupportRtcpXr(ProfileCasting(m_pBaseProfile)->IsRtcpXrSupported());
        GetLocalProfile(pNewOaModel)
                ->SetRtcpXrAttr(ProfileCasting(m_pBaseProfile)->GetRtcpXrAttr());
    }

    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    IMS_BOOL bSdpMade = IMS_FALSE;

    bSdpMade = std::static_pointer_cast<AudioSdpGenerator>(m_pSdpGenerator)
                       ->Generate(pSessionDescriptor, pDescriptor, GetLocalProfile(pNewOaModel));

    // Delete Session Level Direction Attribute
    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    return bSdpMade;
}

PROTECTED
MEDIA_DIRECTION AudioNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateOffer() - local port[%d]", m_pBaseProfile->GetDataPort(), 0, 0);

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO, m_pBaseProfile);

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) !=
            IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (std::static_pointer_cast<AudioProfileNegotiator>(m_pProfileNegotiator)
                    ->Negotiate(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel), IMS_TRUE,
                            GetNegotiatedProfile(pNewOaModel), m_pConfig) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateOffer() - add session key in NewOaModel[%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_listOaModel.Append(pNewOaModel);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}

PROTECTED
MEDIA_DIRECTION AudioNego::NegotiateAnswer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    if (m_listOaModel.GetSize() < 1)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateAnswer()", 0, 0, 0);

    // Get the latest OAmodel from list
    OaModel* pNewOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

    if (pNewOaModel == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) !=
            IMS_TRUE)
    {
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile with the local, peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (std::static_pointer_cast<AudioProfileNegotiator>(m_pProfileNegotiator)
                    ->Negotiate(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel),
                            IMS_FALSE, GetNegotiatedProfile(pNewOaModel), m_pConfig) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateAnswer() - add session key in NewOaModel[%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}
