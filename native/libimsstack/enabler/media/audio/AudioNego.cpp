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

#include "ISessionDescriptor.h"
#include "ImsTypeDef.h"
#include "MediaEnvironment.h"
#include "MediaProfileFactory.h"
#include "MediaProfileUtil.h"
#include "ServiceTrace.h"
#include "audio/AudioNego.h"
#include "audio/AudioProfileUtil.h"
#include "audio/AudioSdpGenerator.h"
#include "audio/AudioProfileGenerator.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaConfigUtil.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
AudioNego::AudioNego(IMS_SINT32 nSlotId) :
        BaseNego(nSlotId, MEDIA_TYPE_AUDIO),
        m_pSdpParser(std::make_shared<AudioSdpParser>()),
        m_pProfileNegotiator(std::make_shared<AudioProfileNegotiator>())
{
    IMS_TRACE_I("+AudioNego(): slot[%d]", nSlotId, 0, 0);

    m_pSdpGenerator = std::make_shared<AudioSdpGenerator>();
    m_pProfileGenerator = std::make_shared<AudioProfileGenerator>();
}

PUBLIC
AudioNego::AudioNego(IN const AudioNego& obj) :
        BaseNego(obj),
        m_pSdpParser(std::make_shared<AudioSdpParser>()),
        m_pProfileNegotiator(std::make_shared<AudioProfileNegotiator>())
{
    IMS_TRACE_I("+AudioNego(): slot[%d]", GetSlotId(), 0, 0);

    m_pSdpGenerator = std::make_shared<AudioSdpGenerator>();
    m_pProfileGenerator = std::make_shared<AudioProfileGenerator>();
    Copy(&obj);
}

PUBLIC
AudioNego& AudioNego::operator=(IN const AudioNego& obj)
{
    if (this != &obj)
    {
        BaseNego::operator=(obj);
        m_pSdpParser = std::make_shared<AudioSdpParser>();
        m_pSdpGenerator = std::make_shared<AudioSdpGenerator>();
        m_pProfileNegotiator = std::make_shared<AudioProfileNegotiator>();
        m_pProfileGenerator = std::make_shared<AudioProfileGenerator>();
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
            m_pProfileNegotiator == IMS_NULL || m_pSdpParser == IMS_NULL)
    {
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): invalid arguments", 0, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    OaModel objOaModel;
    objOaModel.pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile.get());

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(objOaModel)))
    {
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): failed to parse SDP", 0, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(objOaModel), GetPeerProfile(objOaModel),
                IMS_TRUE, GetNegotiatedProfile(objOaModel), m_pConfig))
    {
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): failed to negotiate profile", 0, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    return (objOaModel.pNegotiatedProfile != IMS_NULL &&
                   objOaModel.pNegotiatedProfile->GetPayloadList().GetSize() > 0 &&
                   objOaModel.pNegotiatedProfile->GetDataPort() != 0)
            ? IMS_TRUE
            : IMS_FALSE;
}

PUBLIC VIRTUAL AUDIO_CODEC_BITRATE AudioNego::GetNegotiatedAudioCodecRate(void)
{
    MediaBaseProfile::BasePayload* pNegotiatedPayload = GetNegotiatedPayload();

    if (pNegotiatedPayload == IMS_NULL)
    {
        return AUDIO_CODEC_BITRATE_MAX;
    }

    if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
            pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
    {
        IMS_SINT32 nLargestModeSet = -1;

        if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            nLargestModeSet = AudioProfileUtil::GetLargestModesetInFmtp("AMR-WB",
                                      static_cast<AudioProfile::Payload*>(pNegotiatedPayload)) +
                    AUDIO_CODEC_BITRATE_AMR_WB_660;
            return (AUDIO_CODEC_BITRATE)nLargestModeSet;
        }
        else  // AMR case
        {
            nLargestModeSet = AudioProfileUtil::GetLargestModesetInFmtp("AMR",
                                      static_cast<AudioProfile::Payload*>(pNegotiatedPayload)) +
                    AUDIO_CODEC_BITRATE_AMR_475;
            return (AUDIO_CODEC_BITRATE)nLargestModeSet;
        }
    }
    else if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
    {
        auto pEvsFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(
                static_cast<AudioProfile::Payload*>(pNegotiatedPayload)->GetFmtp());
        if (pEvsFmtp == IMS_NULL)
        {
            return AUDIO_CODEC_BITRATE_INVALID;
        }

        IMS_SINT32 nLargestModeSet = -1;
        nLargestModeSet = AudioProfileUtil::GetLargestModesetInFmtp(
                "EVS", static_cast<AudioProfile::Payload*>(pNegotiatedPayload));
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
    auto pPayload = static_cast<AudioProfile::Payload*>(GetNegotiatedPayload());

    if (pPayload == IMS_NULL)
    {
        return AUDIO_CODEC_NONE;
    }

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
        auto pEvsFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pPayload->GetFmtp());

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
        std::shared_ptr<OaModel> pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel();
        if (pLatestOaModel == IMS_NULL)
        {
            IMS_TRACE_E(0, "FormAnswer(): invalid OA model", 0, 0, 0);
            return IMS_FALSE;
        }

        if (!pLatestOaModel->IsAllProfileExist())
        {
            IMS_TRACE_E(0, "FormAnswer(): invalid OA model", 0, 0, 0);
            return IMS_FALSE;
        }

        for (IMS_UINT32 i = 0; i < pLatestOaModel->pNegotiatedProfile->GetPayloadList().GetSize();
                i++)
        {
            AudioProfile::Payload* pPayload =
                    GetNegotiatedProfile(*pLatestOaModel)->GetPayloadAt(i);

            if (pPayload == IMS_NULL)
            {
                continue;
            }

            if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED AudioProfile* AudioNego::GetLocalProfile(IN const OaModel& objOaModel)
{
    return static_cast<AudioProfile*>(BaseNego::GetLocalProfile(objOaModel));
}

PROTECTED AudioProfile* AudioNego::GetPeerProfile(IN const OaModel& objOaModel)
{
    return static_cast<AudioProfile*>(BaseNego::GetPeerProfile(objOaModel));
}

PROTECTED AudioProfile* AudioNego::GetNegotiatedProfile(IN const OaModel& objOaModel)
{
    return static_cast<AudioProfile*>(BaseNego::GetNegotiatedProfile(objOaModel));
}

PROTECTED
IMS_BOOL AudioNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    if (CheckArgument(pSessionDescriptor, pDescriptor, eDirection) && m_pSdpGenerator)
    {
        // Make the SDP from profile
        IMS_BOOL bSdpMade = m_pSdpGenerator->Generate(pSessionDescriptor, pDescriptor,
                GetLocalProfile(*CreateOaModel(eDirection, bDisable)));

        // Remove the session level direction
        pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
        return bSdpMade;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    if (!CheckArgument(pSessionDescriptor, pDescriptor, eDirection) || m_pSdpGenerator == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormAnswer(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_listOaModel.IsEmpty())
    {
        IMS_TRACE_E(0, "FormAnswer(): empty OA model list", 0, 0, 0);
        return IMS_FALSE;
    }

    // Getting OaModel from list
    std::shared_ptr<OaModel> pNewOaModel = GetNegotiatedOaModel();

    if (pNewOaModel == IMS_NULL || !pNewOaModel->IsAllProfileExist())
    {
        IMS_TRACE_E(0, "FormAnswer(): no valid negotiated model", 0, 0, 0);
        return IMS_FALSE;
    }

    // Modify a direction by Enabler
    if (eDirection > MEDIA_DIRECTION_INVALID)
    {
        pNewOaModel->pNegotiatedProfile->SetDirection(eDirection);
    }

    if (bDisable)
    {
        pNewOaModel->pNegotiatedProfile->SetDataPort(0);
        pNewOaModel->pNegotiatedProfile->SetControlPort(0);
    }

    IMS_TRACE_D("FormAnswer(): direction[%d], disable[%d]", eDirection, bDisable, 0);

    // Make the SDP from profile
    IMS_BOOL bSdpMade = m_pSdpGenerator->Generate(
            pSessionDescriptor, pDescriptor, GetNegotiatedProfile(*pNewOaModel));

    // Remove the session level direction
    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    return bSdpMade;
}

PROTECTED
IMS_BOOL AudioNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    if (!CheckArgument(pSessionDescriptor, pDescriptor, eDirection) || m_pSdpGenerator == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormReoffer(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pConfig == IMS_NULL || m_pEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormReoffer(): config is not valid", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("FormReoffer(): direction[%d], OA model[%d], reOffer[%d]", eDirection,
            m_listOaModel.GetSize(), bEnforceReofferMode);

    // Make new Offer/Answer model, and copy source profile from previous negotiated profile
    std::shared_ptr<OaModel> pNewOaModel = std::make_shared<OaModel>();

    if (m_listOaModel.IsEmpty())
    {
        pNewOaModel->pLocalProfile =
                MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile.get());
    }
    else
    {
        std::shared_ptr<OaModel> pPrevOaModel = GetNegotiatedOaModel();

        if (pPrevOaModel == IMS_NULL)
        {
            return IMS_FALSE;
        }

        const MediaSessionConfig* pMediaSessionConfig =
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
                    m_eType, GetNegotiatedProfile(*pPrevOaModel));
        }
        else
        {
            if (bEnforceReofferMode)
            {
                pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                        m_eType, m_pBaseProfile.get());
            }
            else
            {
                IMS_TRACE_I("FormReoffer(): reuse previous profile, capability[%d]",
                        pMediaSessionConfig->IsSdpReofferFullCapability(), 0, 0);

                if (pMediaSessionConfig->IsSdpReofferFullCapability())
                {
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            m_eType, m_pBaseProfile.get());
                }
                else if (pPrevOaModel->pNegotiatedProfile != IMS_NULL)
                {
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            m_eType, GetNegotiatedProfile(*pPrevOaModel));
                }
            }
        }
    }

    if (pNewOaModel->pLocalProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "create LocalProfile has failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // set default AS value when localProfile AS value is 0 in ReOffer case
    if (pNewOaModel->pLocalProfile->GetBandwidthAs() <= 0)
    {
        IMS_TRACE_I("FormReoffer(): use default AS value", 0, 0, 0);
        pNewOaModel->pLocalProfile->SetBandwidthAs(m_pBaseProfile->GetBandwidthAs());
    }

    // Modify a direction by Enabler
    if (eDirection > MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_I("FormReoffer(): direction[%d]", eDirection, 0, 0);
        pNewOaModel->pLocalProfile->SetDirection(eDirection);
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    MediaProfileUtil::SetRtcpRsRr(GetLocalProfile(*pNewOaModel),
            MediaConfigUtil::GetAudioConfig(GetSlotId(), m_pEnvironment->eServiceType),
            MEDIA_DIRECTION_IS_AUDIO_HOLD(eDirection));

    if (bDisable)
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
    auto pAudioBaseProfile = std::static_pointer_cast<AudioProfile>(m_pBaseProfile);
    if (pAudioBaseProfile && pAudioBaseProfile->IsRtcpXrSupported() &&
            pNewOaModel->pLocalProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        auto pLocalAudioProfile = GetLocalProfile(*pNewOaModel);
        if (pLocalAudioProfile)
        {
            pLocalAudioProfile->SetSupportRtcpXr(pAudioBaseProfile->IsRtcpXrSupported());
            pLocalAudioProfile->SetRtcpXrAttr(pAudioBaseProfile->GetRtcpXrAttr());
        }
    }

    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    IMS_BOOL bSdpMade = m_pSdpGenerator->Generate(
            pSessionDescriptor, pDescriptor, GetLocalProfile(*pNewOaModel));

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
            m_pProfileNegotiator == IMS_NULL || m_pSdpParser == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateOffer(): invalid arguments", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateOffer(): local port[%d]", m_pBaseProfile->GetDataPort(), 0, 0);

    // Make new Offer/Answer model, and copy source profile
    std::shared_ptr<OaModel> pNewOaModel = std::make_shared<OaModel>();
    pNewOaModel->pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile.get());

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(*pNewOaModel)))
    {
        IMS_TRACE_E(0, "NegotiateOffer(): failed to parse SDP", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    pNewOaModel->pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(*pNewOaModel),
                GetPeerProfile(*pNewOaModel), IMS_TRUE, GetNegotiatedProfile(*pNewOaModel),
                m_pConfig))
    {
        IMS_TRACE_E(0, "NegotiateOffer(): failed to negotiate SDP", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key
    m_listOaModel.Append(pNewOaModel);

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
        IMS_TRACE_E(0, "NegotiateAnswer(): invalid arguments", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    if (m_listOaModel.IsEmpty())
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): empty OA model", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // Get the latest OAmodel from list
    std::shared_ptr<OaModel> pNewOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

    if (pNewOaModel == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(*pNewOaModel)))
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): failed to parse SDP", 0, 0, 0);
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile with the local, peer profile
    pNewOaModel->pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(*pNewOaModel),
                GetPeerProfile(*pNewOaModel), IMS_FALSE, GetNegotiatedProfile(*pNewOaModel),
                m_pConfig))
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): failed to negotiate SDP", 0, 0, 0);
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}
