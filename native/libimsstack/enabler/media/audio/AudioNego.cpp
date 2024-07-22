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
#include "offeranswer/SdpAvCodec.h"

#include "MediaNegoUtil.h"
#include "MediaProfileFactory.h"
#include "MediaProfileUtil.h"
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
        BaseNego(nSlotId, MEDIA_TYPE_AUDIO),
        m_pProfileExtractor(std::make_unique<AudioProfileExtractor>())
{
    IMS_TRACE_I("+AudioNego() - slot[%d]", nSlotId, 0, 0);
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
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_TYPE_INVALID;
    }

    IMS_TRACE_I("IsMediaCodecFromSdpSupported()", 0, 0, 0);

    OaModel objOaModel;
    objOaModel.pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO, m_pBaseProfile);

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (m_pProfileExtractor->Extract(
                pSessionDescriptor, pDescriptor, GetPeerProfile(&objOaModel)) != IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (MakeNegotiatedProfile(GetLocalProfile(&objOaModel), GetPeerProfile(&objOaModel), IMS_TRUE,
                GetNegotiatedProfile(&objOaModel)) != IMS_TRUE)
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
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_listOaModel.GetSize() == 0)
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
    (MakeNegotiatedProfile(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_TRUE,
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
    IMS_BOOL bSDPMade =
            MakeSdpFromProfile(pSessionDescriptor, pDescriptor, GetNegotiatedProfile(pNewOaModel));

    if (pSessionDescriptor->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        // Delete Session Level Direction Attribute
        pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    return bSDPMade;
}

PROTECTED
IMS_BOOL AudioNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormReoffer() pDescriptor[%" PFLS_x "], eDir[%d], m_listOaModel.GetSize[%d]",
            pDescriptor, eDir, m_listOaModel.GetSize());
    IMS_TRACE_D("FormReoffer() - EnforceReofferMode[%d]", bEnforceReofferMode, 0, 0);

    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
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
                IMS_TRACE_I(
                        "FormReoffer() - reuse previous profile, m_bSdpReofferFullCapability[%d]",
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
    IMS_BOOL bSDPMade =
            MakeSdpFromProfile(pSessionDescriptor, pDescriptor, GetLocalProfile(pNewOaModel));

    // Delete Session Level Direction Attribute
    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    return bSDPMade;
}

PROTECTED
MEDIA_DIRECTION AudioNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
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

    if (m_pProfileExtractor->Extract(
                pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (MakeNegotiatedProfile(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel), IMS_TRUE,
                GetNegotiatedProfile(pNewOaModel)) != IMS_TRUE)
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
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    if (m_listOaModel.GetSize() < 1)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateAnswer() Entered", 0, 0, 0);

    // Get the latest OAmodel from list
    OaModel* pNewOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

    if (pNewOaModel == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (m_pProfileExtractor->Extract(
                pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile with the local, peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_AUDIO);

    if (MakeNegotiatedProfile(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel), IMS_FALSE,
                GetNegotiatedProfile(pNewOaModel)) != IMS_TRUE)
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

PROTECTED
IMS_BOOL AudioNego::MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pBaseProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AudioProfile* pProfile = ProfileCasting(pBaseProfile);

    IMS_TRACE_I("MakeSdpFromProfile() - PayloadSize[%d], AS[%d], port[%d]",
            pProfile->GetPayloadList().GetSize(), pProfile->GetBandwidthAs(),
            pProfile->GetDataPort());

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
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpmap, strFmtp, strPayloadNum;

        AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // set "rtpmap"
        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        strRtpmap.Sprintf("%s/%d", pPayload->GetRtpMap().GetPayloadType().GetStr(),
                pPayload->GetRtpMap().GetSamplingRate());

        if (pPayload->GetRtpMap().GetChannel() > 0)
        {
            AString strChannel;
            strChannel.Sprintf("/%d", pPayload->GetRtpMap().GetChannel());
            strRtpmap.Append(strChannel);
        }

        // set "fmtp"
        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB") ||
                pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR"))
        {
            AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pPayload->GetFmtp();
            if (pAmrFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = AudioNegoAmr::SetSdpFmtpFromAmrFmtp(pAmrFmtp);
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
        {
            AudioProfile::TelephoneEventFmtp* pTEFmtp =
                    (AudioProfile::TelephoneEventFmtp*)pPayload->GetFmtp();
            if (pTEFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = pTEFmtp->GetEvents();
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
        {
            AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pPayload->GetFmtp();
            if (pEvsFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(pEvsFmtp);
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("pcmu") ||
                pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("pcma"))
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
    pDescriptor->SetDirection(pProfile->GetDirection());

    if (pProfile->GetDirection() > MEDIA_DIRECTION_INVALID &&
            pProfile->GetDirection() <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        // Set Session Level Direction Attribute according to the media direction
        // (avoid conflict between media and audio)
        pSessionDescriptor->SetDirection(pProfile->GetDirection());
    }

    // set make ptime & maxptime
    if (pProfile->GetPtime() != AudioProfile::AmrFmtp::DEFAULT_PTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::PTIME, pProfile->GetPtime());
    }

    if (pProfile->GetMaxPtime() != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::MAXPTIME, pProfile->GetMaxPtime());
    }

    // set candidate
    for (IMS_UINT32 nIndex = 0; nIndex < pProfile->GetCandidateAttr().GetSize(); nIndex++)
    {
        AString strCandidateAttr = pProfile->GetCandidateAttr().GetAt(nIndex);
        if (strCandidateAttr.GetLength() != 0)
        {
            strCandidateAttr.Sprintf("%d, %s", nIndex + 1, strCandidateAttr.GetStr());
            pDescriptor->AddAttribute(SdpAttribute::CANDIDATE, strCandidateAttr);
        }
    }

    // set RTCP-XR -- RTCP-XR is for VZW, not a negotiation target by VZW requirement
    if (pProfile->IsRtcpXrSupported() == IMS_TRUE &&
            pProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        if (pProfile->GetRtcpXrAttr().IsStatisticMetricsSupported())
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "stat-summary=loss,dup,jitt,HL");
        }
        if (pProfile->GetRtcpXrAttr().IsVoipMetricsSupported())
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "voip-metrics");
        }
        if (pProfile->GetRtcpXrAttr().IsPacketLossRleSupported())
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "pkt-loss-rle");
        }
        if (pProfile->GetRtcpXrAttr().IsPacketDuplicatedRleSupported())
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "pkt-dup-rle");
        }

        IMS_TRACE_I(
                "MakeSdpFromProfile() - SupportRtcpXr[%d]", pProfile->IsRtcpXrSupported(), 0, 0);
    }

    if (pProfile->IsAnbrSupported())
    {
        pDescriptor->AddAttribute(SdpAttribute::ANBR, AString::ConstNull());
    }
    else
    {
        IMS_TRACE_D("MakeSdpFromProfile() - anbr feature is not supported", 0, 0, 0);
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
    pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());

    IMS_TRACE_D("MakeNegotiatedProfile() - IPAddr nego[%s] src[%s] DestPayloadSize[%d]",
            pNegotiatedProfile->GetIpAddress().ToCharString(),
            pLocalProfile->GetIpAddress().ToCharString(), pPeerProfile->GetPayloadList().GetSize());

    // Setting RTP/RTCP port of mine
    pNegotiatedProfile->SetDataPort(pLocalProfile->GetDataPort());
    pNegotiatedProfile->SetControlPort(pLocalProfile->GetControlPort());

    if (pNegotiatedProfile->GetDataPort() == 0 || pPeerProfile->GetDataPort() == 0)
    {
        // Reset the negotiated profile to local profile
        *pNegotiatedProfile = *pLocalProfile;
        pNegotiatedProfile->SetDataPort(0);
        IMS_TRACE_D("MakeNegotiatedProfile() ZERO Port. DO NOT Use the audio[%d][%d]",
                pNegotiatedProfile->GetDataPort(), pPeerProfile->GetDataPort(), 0);
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

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* pPayload = pPeerProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }
        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0) &&
                    FindAmrInProfile(pLocalProfile, pPayload, bIsOfferReceived, &nNegoModeSetList,
                            &nNegoDefaultRtpModeSet) == IMS_TRUE)
            {
                AudioProfile::Payload* pAMR = new AudioProfile::Payload();
                pAMR->SetRtpMap(pPayload->GetRtpMap());
                pAMR->SetFmtp(new AudioProfile::AmrFmtp(
                        *static_cast<AudioProfile::AmrFmtp*>(pPayload->GetFmtp())));
                templstNegotiatedPayloads.Append(pAMR);
            }
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0) &&
                    FindEvsInProfile(pLocalProfile, pPayload, bIsOfferReceived, &BandwidthNegoList,
                            &BitrateNegoList, &ModeSetNegoList) == IMS_TRUE)
            {
                AudioProfile::Payload* pEVS = new AudioProfile::Payload();
                pEVS->SetRtpMap(pPayload->GetRtpMap());

                pEVS->SetFmtp(new AudioProfile::EvsFmtp(
                        *static_cast<AudioProfile::EvsFmtp*>(pPayload->GetFmtp())));

                templstNegotiatedPayloads.Append(pEVS);
            }
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU") ||
                pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0) &&
                    FindPcmInProfile(pLocalProfile, pPayload) == IMS_TRUE)
            {
                AudioProfile::Payload* pPCM = new AudioProfile::Payload();
                pPCM->SetRtpMap(pPayload->GetRtpMap());

                templstNegotiatedPayloads.Append(pPCM);
            }
        }
    }

    IMS_TRACE_D("MakeNegotiatedProfile() - temp negotiated payload list[%d]",
            templstNegotiatedPayloads.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* pDestPayload = pPeerProfile->GetPayloadAt(i);
        if (pDestPayload == IMS_NULL)
        {
            continue;
        }

        if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            if ((lstNegotiatedPayloads.GetSize() == 0) &&
                    FindAmrInProfile(pLocalProfile, pDestPayload, bIsOfferReceived,
                            &nNegoModeSetList, &nNegoDefaultRtpModeSet) == IMS_TRUE)
            {
                AudioProfile::Payload* pAMR = new AudioProfile::Payload();
                pAMR->SetRtpMap(pDestPayload->GetRtpMap());

                IMS_SINT32 nSrcPayloadIndex =
                        FindPayloadIndexFromProfile(pDestPayload->GetRtpMap().GetPayloadType(),
                                pLocalProfile, pDestPayload, bIsOfferReceived);
                AudioProfile::AmrFmtp* pSrc_Fmtp =
                        (AudioProfile::AmrFmtp*)pLocalProfile->GetPayloadAt(nSrcPayloadIndex)
                                ->GetFmtp();
                AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp(
                        *static_cast<AudioProfile::AmrFmtp*>(pDestPayload->GetFmtp()));
                pAmrFmtp->SetModeSetList(nNegoModeSetList);
                pAmrFmtp->SetDefaultRtpModeSet(nNegoDefaultRtpModeSet);

                pAmrFmtp->SetDtx(pSrc_Fmtp->IsDtxEnabled());

                pAmrFmtp->SetShowModeChangeCapability(pSrc_Fmtp->IsModeChangeCapabilityVisible());
                pAmrFmtp->SetModeChangeCapability(pSrc_Fmtp->GetModeChangeCapability());
                pAmrFmtp->SetShowModeChangeNeighbor(pSrc_Fmtp->IsModeChangeNeighborVisible());
                pAmrFmtp->SetModeChangeNeighbor(pSrc_Fmtp->GetModeChangeNeighbor());
                pAmrFmtp->SetShowModeChangePeriod(pSrc_Fmtp->IsModeChangePeriodVisible());
                pAmrFmtp->SetModeChangePeriod(pSrc_Fmtp->GetModeChangePeriod());

                pAMR->SetFmtp(pAmrFmtp);
                pNegotiatedProfile->GetPayloadList().Append(pAMR);
                lstNegotiatedPayloads.Append(pAMR);

                if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
                {
                    // Set the index of negotiated payload from the list.
                    pPeerProfile->SetNegotiatedPayloadIndex(i);
                    // set nego payload index at src profile
                    pLocalProfile->SetNegotiatedPayloadIndex(nSrcPayloadIndex);
                    IMS_TRACE_D("MakeNegotiatedProfile() - nego payload index[%d]",
                            pLocalProfile->GetNegotiatedPayloadIndex(), 0, 0);

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                            pLocalProfile->GetNegotiatedPayloadIndex() != -1)
                    {
                        AudioProfile::Payload* pTempNegoSrcPayload = pLocalProfile->GetPayloadAt(
                                pLocalProfile->GetNegotiatedPayloadIndex());

                        pTempNegoSrcPayload->GetRtpMap().SetPayloadNumber(
                                pDestPayload->GetRtpMap().GetPayloadNumber());
                    }
                }

                if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile.
                    pNegotiatedProfile->SetNegotiatedPayloadIndex(
                            pNegotiatedProfile->GetPayloadList().GetSize() - 1);
                }
            }
        }
        else if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
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
                pEVS->SetRtpMap(pDestPayload->GetRtpMap());
                IMS_SINT32 nSrcPayloadIndex =
                        FindPayloadIndexFromProfile(pDestPayload->GetRtpMap().GetPayloadType(),
                                pLocalProfile, pDestPayload, bIsOfferReceived);
                AudioProfile::EvsFmtp* pSrc_Fmtp =
                        (AudioProfile::EvsFmtp*)pLocalProfile->GetPayloadAt(nSrcPayloadIndex)
                                ->GetFmtp();
                AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp(
                        *reinterpret_cast<AudioProfile::EvsFmtp*>(pDestPayload->GetFmtp()));
                pEvsFmtp->SetBwList(BandwidthNegoList);
                pEvsFmtp->SetBrList(BitrateNegoList);
                pEvsFmtp->SetModeSetList(ModeSetNegoList);

                if (pEvsFmtp->IsDtxEnabled() != pSrc_Fmtp->IsDtxEnabled())
                {
                    IMS_TRACE_D("MakeNegotiatedProfile() - DTX updated in the destination profile",
                            0, 0, 0);
                }

                pEvsFmtp->SetShowModeChangeCapability(pSrc_Fmtp->IsModeChangeCapabilityVisible());
                pEvsFmtp->SetModeChangeCapability(pSrc_Fmtp->GetModeChangeCapability());
                pEvsFmtp->SetShowModeChangeNeighbor(pSrc_Fmtp->IsModeChangeNeighborVisible());
                pEvsFmtp->SetModeChangeNeighbor(pSrc_Fmtp->GetModeChangeNeighbor());
                pEvsFmtp->SetShowModeChangePeriod(pSrc_Fmtp->IsModeChangePeriodVisible());
                pEvsFmtp->SetModeChangePeriod(pSrc_Fmtp->GetModeChangePeriod());

                // check uni direction attribute
                if (pEvsFmtp->GetBrSend() != 0)
                {
                    pEvsFmtp->SetBrSend(BitrateNegoList);
                }
                if (pEvsFmtp->GetBrRecv() != 0)
                {
                    pEvsFmtp->SetBrRecv(BitrateNegoList);
                }
                if (pEvsFmtp->GetBwSend() != 0)
                {
                    pEvsFmtp->SetBwSend(BandwidthNegoList);
                }
                if (pEvsFmtp->GetBwRecv() != 0)
                {
                    pEvsFmtp->SetBwRecv(BandwidthNegoList);
                }

                pEvsFmtp->SetSendCmr(pSrc_Fmtp->IsSendCmrEnabled());

                // CMR on/off, if bitrate is not range set, disable CMR send option
                IMS_UINT32 nCount = 0;
                IMS_UINT32 nTempBrList = pEvsFmtp->GetBrList();
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
                    pEvsFmtp->SetSendCmr(IMS_FALSE);
                }

                /** fixed for IR92 ver.12 newaly spec as below comment. If the selected EVS
                 * configuration is A1, B0 or B1 then"mode set = 0,1,2" must be included in the SDP
                 * answer.*/
                if (bIsOfferReceived == IMS_TRUE && pEvsFmtp->GetEvsModeSwitch() != 1)
                {
                    // if max BR is 13.2kbps, then set a"mode-set" attribute
                    if (((pEvsFmtp->GetBrList() & 0x10) != 0) &&
                            ((pEvsFmtp->GetBrList() & 0xFFE0) == 0))
                    {
                        pEvsFmtp->SetModeSetList(0x07);  // mode-set = 0,1,2;
                        pEvsFmtp->SetShowModeSet(IMS_TRUE);
                        IMS_TRACE_D("MakeNegotiatedProfile() - add EVS mode-set", 0, 0, 0);
                    }
                }

                pEVS->SetFmtp(pEvsFmtp);
                pNegotiatedProfile->GetPayloadList().Append(pEVS);
                lstNegotiatedPayloads.Append(pEVS);

                if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
                {
                    // Set the index of negotiated payload from the list
                    pPeerProfile->SetNegotiatedPayloadIndex(i);
                    // set nego payload index at src profile
                    pLocalProfile->SetNegotiatedPayloadIndex(nSrcPayloadIndex);

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                            pLocalProfile->GetNegotiatedPayloadIndex() != -1)
                    {
                        AudioProfile::Payload* pTempNegoSrcPayload = pLocalProfile->GetPayloadAt(
                                pLocalProfile->GetNegotiatedPayloadIndex());

                        pTempNegoSrcPayload->GetRtpMap().SetPayloadNumber(
                                pDestPayload->GetRtpMap().GetPayloadNumber());
                    }
                }
                if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile
                    pNegotiatedProfile->SetNegotiatedPayloadIndex(
                            pNegotiatedProfile->GetPayloadList().GetSize() - 1);
                }
            }
        }
        else if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
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
                if (pNegotiatedPayload->GetRtpMap().GetSamplingRate() ==
                        pDestPayload->GetRtpMap().GetSamplingRate())
                {
                    AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                    pTelephoneEvent->SetRtpMap(pDestPayload->GetRtpMap());
                    pTelephoneEvent->SetFmtp(new AudioProfile::TelephoneEventFmtp(
                            *static_cast<AudioProfile::TelephoneEventFmtp*>(
                                    pDestPayload->GetFmtp())));

                    pNegotiatedProfile->GetPayloadList().Append(pTelephoneEvent);
                    bProperNegotiatedTe = IMS_TRUE;
                    break;
                }
            }
        }
        else if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU") ||
                pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"))
        {
            if ((lstNegotiatedPayloads.GetSize() == 0) &&
                    FindPcmInProfile(pLocalProfile, pDestPayload) == IMS_TRUE)
            {
                AudioProfile::Payload* pPCM = new AudioProfile::Payload();
                pPCM->SetRtpMap(pDestPayload->GetRtpMap());
                pNegotiatedProfile->GetPayloadList().Append(pPCM);
                lstNegotiatedPayloads.Append(pPCM);

                if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
                {
                    // Set the index of negotiated payload from the list
                    pPeerProfile->SetNegotiatedPayloadIndex(i);
                    pLocalProfile->SetNegotiatedPayloadIndex(
                            FindPayloadIndexFromProfile(pDestPayload->GetRtpMap().GetPayloadType(),
                                    pLocalProfile, pDestPayload, bIsOfferReceived));
                }

                if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile
                    pNegotiatedProfile->SetNegotiatedPayloadIndex(
                            pNegotiatedProfile->GetPayloadList().GetSize() - 1);
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

    IMS_TRACE_D("MakeNegotiatedProfile() Negotiated Payload Index[%d]",
            pPeerProfile->GetNegotiatedPayloadIndex(), 0, 0);

    // accept 8K DTMF when AMR-WB calling if ther are no proper DTMF payload
    if (bProperNegotiatedTe == IMS_FALSE && pNegotiatedProfile->GetPayloadList().GetSize() > 0 &&
            lstNegotiatedPayloads.GetSize() > 0)
    {
        for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
        {
            AudioProfile::Payload* pDestPayload = pPeerProfile->GetPayloadAt(i);

            if (pDestPayload == IMS_NULL)
            {
                continue;
            }
            if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
            {
                // for acceptable 8K DTMF when AMR-WB calling
                pNegotiatedPayload = lstNegotiatedPayloads.GetAt(0);
                if (pNegotiatedPayload->GetRtpMap().GetSamplingRate() >
                        pDestPayload->GetRtpMap().GetSamplingRate())
                {
                    IMS_TRACE_D("MakeNegotiatedProfile() - Accept sampling rate[%d]->[%d]",
                            pNegotiatedPayload->GetRtpMap().GetSamplingRate(),
                            pDestPayload->GetRtpMap().GetSamplingRate(), 0);
                    AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                    pTelephoneEvent->SetRtpMap(pDestPayload->GetRtpMap());
                    pTelephoneEvent->SetFmtp(new AudioProfile::TelephoneEventFmtp(
                            *static_cast<AudioProfile::TelephoneEventFmtp*>(
                                    pDestPayload->GetFmtp())));
                    pNegotiatedProfile->GetPayloadList().Append(pTelephoneEvent);
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
        if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            AUDIO_CODEC nCurrCodec;

            AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pNegotiatedPayload->GetFmtp();
            if (pNegotiatedPayload->GetRtpMap().GetSamplingRate() == 8000)
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
                    AudioProfileUtil::ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->GetOctetAlign(),
                            pNegotiatedProfile->GetIpAddress().IsIPv6Address(), nModeSet);
        }
        else if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
        {
            AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pNegotiatedPayload->GetFmtp();
            AUDIO_CODEC nCurrCodec = AUDIO_CODEC_EVS;
            nModeSet = AudioProfileUtil::GetLargestModesetInFmtp("EVS", pNegotiatedPayload);
            nAsValueOfNegoticatedCodec = AudioProfileUtil::ConvertToBandwidthAS(nCurrCodec,
                    pNegotiatedProfile->GetIpAddress().IsIPv6Address(),
                    pEvsFmtp->GetEvsModeSwitch(), nModeSet);
        }

        // Setting direction
        pNegotiatedProfile->SetDirection(UpdateDirectionToMine(
                pPeerProfile->GetDirection(), pLocalProfile->GetDirection(), bIsOfferReceived));

        if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
        {
            IMS_TRACE_E(0, "MakeNegotiatedProfile() - invalid direction.", 0, 0, 0);
            return IMS_FALSE;
        }

        // if the case using different interval in live and hold, set here.
        pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
        pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());

        if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
        {
            pNegotiatedProfile->SetRtcpInterval(0);
            IMS_TRACE_D(
                    "MakeNegotiatedProfile() - negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
        }
        else
        {
            pNegotiatedProfile->SetRtcpInterval(m_pConfig->GetRtcpInterval());

            if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE &&
                    m_pConfig->GetRtcpLiveInterval() > 0)
            {
                pNegotiatedProfile->SetRtcpInterval(m_pConfig->GetRtcpLiveInterval());
            }
        }

        AudioProfileUtil::MakeNegotiatedBandwidth(ConfigCasting(m_pConfig), pLocalProfile,
                pPeerProfile, bIsOfferReceived, nAsValueOfNegoticatedCodec, pNegotiatedProfile);

        // RTCP-XR
        if (pLocalProfile->IsRtcpXrSupported() == IMS_TRUE &&
                pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
        {
            pNegotiatedProfile->SetSupportRtcpXr(IMS_TRUE);
            pNegotiatedProfile->SetRtcpXrAttr(pLocalProfile->GetRtcpXrAttr());
        }

        IMS_TRACE_D("MakeNegotiatedProfile()-Rtcp Interval[%d], RTCP-XR support[%d]",
                pNegotiatedProfile->GetRtcpInterval(), pNegotiatedProfile->IsRtcpXrSupported(), 0);

        /** Setting ptime & maxptime
         * [RFC3264]
         * The answerer MAY include a non-zero ptime attribute for any media stream.
         * this indicates the packetization interval that the answerer would like to receive.
         * There is no requirement that the packetization interval be the same in each direction
         * for a particular stream.*/
        if (pLocalProfile->GetPtime() < 20)
        {
            pNegotiatedProfile->SetPtime(20);
        }
        else
        {
            pNegotiatedProfile->SetPtime(pLocalProfile->GetPtime());
        }

        if (pLocalProfile->GetMaxPtime() < 20)
        {
            pNegotiatedProfile->SetMaxPtime(240);
        }
        else
        {
            pNegotiatedProfile->SetMaxPtime(pLocalProfile->GetMaxPtime());
        }

        // Candidate Priority
        pNegotiatedProfile->SetCandidateAttr(pLocalProfile->GetCandidateAttr());

        // ANBR
        pNegotiatedProfile->SetAnbr(IMS_FALSE);
        if (pLocalProfile->IsAnbrSupported() && pPeerProfile->IsAnbrSupported())
        {
            pNegotiatedProfile->SetAnbr(IMS_TRUE);
        }

        IMS_TRACE_D("MakeNegotiatedProfile() - anbr local: %d peer: %d nego: %d",
                pLocalProfile->IsAnbrSupported(), pPeerProfile->IsAnbrSupported(),
                pNegotiatedProfile->IsAnbrSupported());

        return IMS_TRUE;
    }
    else
    {
        return IMS_FALSE;
    }
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
        for (IMS_UINT32 i = 0; i < pLocalProfile->GetPayloadList().GetSize(); i++)
        {
            AudioProfile::Payload* comparedPayload = pLocalProfile->GetPayloadAt(i);

            if (comparedPayload == IMS_NULL)
            {
                continue;
            }

            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
            {
                AudioProfile::EvsFmtp* pCompareFmtp =
                        (AudioProfile::EvsFmtp*)comparedPayload->GetFmtp();
                AudioProfile::EvsFmtp* pReceivedFmtp =
                        (AudioProfile::EvsFmtp*)pDstPayload->GetFmtp();
                if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                            pDstPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
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
                    if (pReceivedFmtp->GetBwList() == 1)
                    {
                        continue;      //vzw req. if offer is nb only, no evs negotiation
                }
                    if (pDstPayload->GetRtpMap().GetPayloadNumber()
                            != comparedPayload->GetRtpMap().GetPayloadNumber())
                    {
                        continue;     //vzw req. payload number based negotiation
                    }
                }*/

                // check channel aware mode received param
                if (pReceivedFmtp->GetReceivedChAwRecv() > 0)
                {
                    IMS_SINT32 tempReceivedChAw = pReceivedFmtp->GetReceivedChAwRecv();

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
                if (((pReceivedFmtp->GetBwList() & 0x06) != 0 &&
                            (pReceivedFmtp->GetBrList() & 0x10) != 0) ||
                        ((*pBandwidthNegoList & 0x06) != 0 && (*pBitrateNegoList & 0x10) != 0))
                {
                    if (bIsOfferReceived != IMS_TRUE)
                    {
                        pReceivedFmtp->SetChAwRecv(pCompareFmtp->GetChAwRecv());
                        pReceivedFmtp->SetShowChannelAwMode(pCompareFmtp->IsChannelAwModeVisible());
                    }
                }

                IMS_TRACE_D("FindEvsInProfile() Found EVS at[%d], BandwidthNegoList[0x%04x], \
                        BitrateNegoList[0x%04x]",
                        i, *pBandwidthNegoList, *pBitrateNegoList);
                IMS_TRACE_D("FindEvsInProfile() EVS ModeSwitch[%d], ModeSetNegoList[0x%04x], \
                        SendCmr[%d]",
                        pReceivedFmtp->GetEvsModeSwitch(), *pModeSetNegoList,
                        pCompareFmtp->IsSendCmrEnabled());
                IMS_TRACE_D("FindEvsInProfile() EVS ChAwMode[%d], opposite ChAwMode[0x%04x], \
                        legacyCheck[%d]",
                        pReceivedFmtp->GetChAwRecv(), pReceivedFmtp->GetReceivedChAwRecv(),
                        legacyCheck);

                return IMS_TRUE;
            }
        }
    }
    return IMS_FALSE;
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

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* comparedPayload = pProfile->GetPayloadAt(i);

        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            AudioProfile::AmrFmtp* pCompareFmtp =
                    (AudioProfile::AmrFmtp*)comparedPayload->GetFmtp();
            AudioProfile::AmrFmtp* pReceivedFmtp = (AudioProfile::AmrFmtp*)pPayload->GetFmtp();

            if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
            {
                continue;
            }
            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
            {
                continue;
            }
            if (comparedPayload->GetRtpMap().GetSamplingRate() !=
                    pPayload->GetRtpMap().GetSamplingRate())
            {
                continue;
            }

            if (pCompareFmtp->GetOctetAlign() != pReceivedFmtp->GetOctetAlign())
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
                            pCompareFmtp->GetModeSetList(), pReceivedFmtp->GetModeSetList(),
                            *pnNegoModeSetList);
                }
                continue;
            }
            else  // exactly matched
            {
                IMS_TRACE_D("FindAmrInProfile() Found AMR at[%d], Codec[%s], OctetAlign[%d]", i,
                        comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                        pCompareFmtp->GetOctetAlign());
                IMS_TRACE_D("FindAmrInProfile() Local/Peer is exactly matched[0x%04x][0x%04x] \
                        =>[0x%04x]",
                        pCompareFmtp->GetModeSetList(), pReceivedFmtp->GetModeSetList(),
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

        IMS_TRACE_D("FindAmrInProfile() Found Similar AMR with ModeSet List[0x%04x], "
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

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* comparedPayload = pProfile->GetPayloadAt(i);

        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU") ||
                comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"))
        {
            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
            {
                continue;
            }
            if (comparedPayload->GetRtpMap().GetSamplingRate() !=
                    pPayload->GetRtpMap().GetSamplingRate())
            {
                continue;
            }

            IMS_TRACE_D("FindPcmInProfile() Found G.711 at[%d], Codec[%s], nSamplingRate[%d]", i,
                    comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                    pPayload->GetRtpMap().GetSamplingRate());

            return IMS_TRUE;
        }
    }

    // IMS_TRACE_D("FindPcmInProfile() Not Found G.711[%s], SamplingRate[%d]",
    // pPayload->GetRtpMap().GetPayloadType().GetStr(), pPayload->GetRtpMap().GetSamplingRate(), 0);

    return IMS_FALSE;
}

PRIVATE
IMS_SINT32 AudioNego::CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
        IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived, IN IMS_BOOL bReturnMode,
        OUT IMS_UINT32* nNegoModeSet, OUT IMS_UINT32* nNegoDefaultRtpModeSet)
{
    IMS_TRACE_I("CompareModeSet() - Src modeSet[0x%04x] Dest modeSet[0x%04x], bIsOfferReceived[%d]",
            pSrcFmtp->GetModeSetList(), pDestFmtp->GetModeSetList(), bIsOfferReceived);
    IMS_TRACE_I("CompareModeSet() - Src defaultmodeSet[0x%04x] Dest defaultmodeSet[0x%04x]",
            pSrcFmtp->GetDefaultRtpModeSet(), pDestFmtp->GetDefaultRtpModeSet(), 0);

    IMS_SINT32 nResult = 1;  // -1 : no matched, 0 : similar, 1 : exactly matched

    if (bIsOfferReceived == IMS_TRUE)  // MT Case
    {
        if (pDestFmtp->GetModeSetList() != 0)
        {
            *nNegoModeSet = pDestFmtp->GetModeSetList();
        }
        else if (pSrcFmtp->GetModeSetList() != 0)
        {
            *nNegoModeSet = pSrcFmtp->GetModeSetList();
        }
        else
        {
            *nNegoModeSet = 0;  // MO, MT both has no mode-set
            *nNegoDefaultRtpModeSet = pSrcFmtp->GetDefaultRtpModeSet();
        }
    }
    else  // MO Case
    {
        if ((pSrcFmtp->GetModeSetList() == 0) && (pDestFmtp->GetModeSetList() == 0))
        {
            *nNegoModeSet = 0;
            *nNegoDefaultRtpModeSet = pSrcFmtp->GetDefaultRtpModeSet();
        }
        else if ((pSrcFmtp->GetModeSetList() != 0) && (pDestFmtp->GetModeSetList() != 0))
        {
            *nNegoModeSet = pSrcFmtp->GetModeSetList() & pDestFmtp->GetModeSetList();

            if (pSrcFmtp->GetModeSetList() != pDestFmtp->GetModeSetList())
            {
                nResult = 0;
            }

            if (*nNegoModeSet == 0)
            {
                IMS_TRACE_D(
                        "CompareModeSet() - ModeSet Not Matched - isFinal: %d", bReturnMode, 0, 0);
                if (bReturnMode == RETURN_MODE_SIMILAR)
                {
                    *nNegoModeSet =
                            pDestFmtp->GetModeSetList();  // Copy Dest Mode-set to Nego Mode-set
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
            *nNegoModeSet = pSrcFmtp->GetModeSetList() | pDestFmtp->GetModeSetList();
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
            pSrcFmtp->GetBwList(), pSrcFmtp->GetBrList(), pSrcFmtp->GetModeSetList());
    IMS_TRACE_D("CompareEvsBwBrMode() - Dest Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pDestFmtp->GetBwList(), pDestFmtp->GetBrList(), pDestFmtp->GetModeSetList());

    if (m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // check EVS ModeSwitch
    if (pDestFmtp->GetEvsModeSwitch() == 1)  // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pSrcFmtp->GetModeSetList() == 0) && (pDestFmtp->GetModeSetList() == 0))
        {
            *nNegoModeList = 0;
        }
        else if ((pSrcFmtp->GetModeSetList() != 0) && (pDestFmtp->GetModeSetList() != 0))
        {
            *nNegoModeList = pSrcFmtp->GetModeSetList() & pDestFmtp->GetModeSetList();

            if (*nNegoModeList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrMode() - AMR IO Mode - ModeSet Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoModeList = pSrcFmtp->GetModeSetList() | pDestFmtp->GetModeSetList();
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        *nNegoBwList = pDestFmtp->GetBwList();
        *nNegoBrList = pDestFmtp->GetBrList();
    }
    else  // Primary Mode
    {
        // Set Bandwidth/Bitrate list.
        // 01. check Bandwidth
        if ((pSrcFmtp->GetBwList() == 0) && (pDestFmtp->GetBwList() == 0))
        {
            *nNegoBwList = 0;
        }
        else if ((pSrcFmtp->GetBwList() != 0) && (pDestFmtp->GetBwList() != 0))
        {
            // IR92 release15 EVS Answer Case.
            if (bIsOfferReceived == IMS_TRUE)
            {
                // check received EVS SWB only case (category B0, B1, B2 case)
                if (pDestFmtp->GetBwList() == 0x04)
                {
                    if (pDestFmtp->GetBrList() == 0x10)  // B0 received case.
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B0 Type Nego", 0,
                                0, 0);
                        *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                    }
                    else if (pSrcFmtp->GetBwList() != 0x04)
                    {  // own EVS category is A. Do not negotiate with received category B.
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Not Config B Type Nego",
                                0, 0, 0);
                        return IMS_FALSE;
                    }
                    else
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B Type Nego", 0,
                                0, 0);
                        *nNegoBwList = pDestFmtp->GetBwList();
                    }
                }
                else  // received EVS category A case
                {
                    // TODO - 20220415 - Need to implement this requirement later
                    /*// except for VZW operator. (only supported category B0 case.)
                    if (IMS_OPERATOR(VZW, GetSlotId()) == IMS_TRUE)
                    {
                        *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                    }
                    else // GSMA IR92 case
                    {
                        // check own EVS SWB only capa. (category B0, B1, B2)
                        if (pSrcFmtp->GetBwList() == 0x04)
                        {
                            // this case, Do not negotiate with own category B. check next paylaod.
                            IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - \
                                    not support B Type Nego", 0, 0, 0);
                            return IMS_FALSE;
                        }
                        else
                        {
                            *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                        }
                    }*/
                    // check own EVS SWB only capa. (category B0, B1, B2)
                    if (pSrcFmtp->GetBwList() == 0x04)
                    {
                        // this case, Do not negotiate with own category B. check next paylaod.
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - \
                                not support B Type Nego",
                                0, 0, 0);
                        return IMS_FALSE;
                    }
                    else
                    {
                        *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                    }
                }
            }
            else
            {
                // TODO - 20220415 - Need to implement this requirement later
                /*// IR92 release15 EVS Answer Received Case.
                if (IMS_OPERATOR(VZW, GetSlotId()) == IMS_TRUE)
                {
                    *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                }
                else
                {
                    if (pSrcFmtp->GetBwList() == 0x04 && pDestFmtp->GetBwList() != 0x04)
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - check next payload", 0, 0, 0);
                        return IMS_FALSE;
                    }
                    *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                }*/
                if (pSrcFmtp->GetBwList() == 0x04 && pDestFmtp->GetBwList() != 0x04)
                {
                    IMS_TRACE_D("CompareEvsBwBrMode() - check next payload", 0, 0, 0);
                    return IMS_FALSE;
                }
                *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
            }

            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bandwidth Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->GetBwList() != 0) &&
                ((pDestFmtp->GetBwRecv() != 0) || (pDestFmtp->GetBwSend() != 0)))
        {
            if (pDestFmtp->GetBwRecv() == 0)
            {
                pDestFmtp->SetBwRecv(0x0f);
            }
            if (pDestFmtp->GetBwSend() == 0)
            {
                pDestFmtp->SetBwSend(0x0f);
            }

            IMS_UINT32 nPeerBWList = pDestFmtp->GetBwRecv() & pDestFmtp->GetBwSend();

            *nNegoBwList = pSrcFmtp->GetBwList() & nPeerBWList;
            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bandwidth Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBwList = pSrcFmtp->GetBwList() | pDestFmtp->GetBwList();
        }

        // 02. check Bitrate

        if ((pSrcFmtp->GetBrList() == 0) && (pDestFmtp->GetBrList() == 0))
        {
            *nNegoBrList = 0;
        }
        else if ((pSrcFmtp->GetBrList() != 0) && (pDestFmtp->GetBrList() != 0))
        {
            // IR92 release15 EVS Answer Case.
            if (bIsOfferReceived == IMS_TRUE)
            {
                *nNegoBrList = pSrcFmtp->GetBrList() & pDestFmtp->GetBrList();
                if ((*nNegoBwList == 0x04) && (*nNegoBrList == 0x10))
                {  // 13.2kbsp swb only case
                    IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B0,B1 Type Nego", 0,
                            0, 0);
                    *nNegoBrList = (pDestFmtp->GetBrList()) & 0x1f;  // negotiated ~13.2kbps
                }
            }
            else
            {
                *nNegoBrList = pSrcFmtp->GetBrList() & pDestFmtp->GetBrList();
            }

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bitrate Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->GetBrList() != 0) &&
                (pDestFmtp->GetBrRecv() != 0 || (pDestFmtp->GetBrSend() != 0)))
        {
            if (pDestFmtp->GetBrRecv() == 0)
            {
                pDestFmtp->SetBrRecv(0x0fff);
            }
            if (pDestFmtp->GetBrSend() == 0)
            {
                pDestFmtp->SetBrSend(0x0fff);
            }

            IMS_UINT32 nPeerBRList = pDestFmtp->GetBrRecv() & pDestFmtp->GetBrSend();

            *nNegoBrList = pSrcFmtp->GetBrList() & nPeerBRList;

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bitrate Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBrList = pSrcFmtp->GetBrList() | pDestFmtp->GetBrList();
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pDestFmtp->GetModeSetList() != 0)
        {
            *nNegoModeList = pDestFmtp->GetModeSetList();
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
            pSrcFmtp->GetBwList(), pSrcFmtp->GetBrList(), pSrcFmtp->GetModeSetList());
    IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Dest BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pDestFmtp->GetBwList(), pDestFmtp->GetBrList(), pDestFmtp->GetModeSetList());

    // check EVS ModeSwitch
    if (pDestFmtp->GetEvsModeSwitch() == 1)  // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pSrcFmtp->GetModeSetList() == 0) && (pDestFmtp->GetModeSetList() == 0))
        {
            *nNegoModeList = 0;
        }
        else if ((pSrcFmtp->GetModeSetList() != 0) && (pDestFmtp->GetModeSetList() != 0))
        {
            *nNegoModeList = pSrcFmtp->GetModeSetList() & pDestFmtp->GetModeSetList();

            if (*nNegoModeList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - AMR IO Mode - ModeSet Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoModeList = pSrcFmtp->GetModeSetList() | pDestFmtp->GetModeSetList();
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        {
            *nNegoBwList = pDestFmtp->GetBwList();
            *nNegoBrList = pDestFmtp->GetBrList();
        }
    }
    else  // Primary Mode
    {
        // Set Bandwidth/Bitrate list.
        // 01. check Bandwidth
        if ((pSrcFmtp->GetBwList() == 0) && (pDestFmtp->GetBwList() == 0))
        {
            *nNegoBwList = 0;
        }
        else if ((pSrcFmtp->GetBwList() != 0) && (pDestFmtp->GetBwList() != 0))
        {
            *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();

            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->GetBwList() != 0) &&
                ((pDestFmtp->GetBwRecv() != 0) || (pDestFmtp->GetBwSend() != 0)))
        {
            if (pDestFmtp->GetBwRecv() == 0)
            {
                pDestFmtp->SetBwRecv(0x0f);
            }
            if (pDestFmtp->GetBwSend() == 0)
            {
                pDestFmtp->SetBwSend(0x0f);
            }

            IMS_UINT32 nPeerBWList = pDestFmtp->GetBwRecv() & pDestFmtp->GetBwSend();

            *nNegoBwList = pSrcFmtp->GetBwList() & nPeerBWList;
            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBwList = pSrcFmtp->GetBwList() | pDestFmtp->GetBwList();
        }

        // 02. check Bitrate

        if ((pSrcFmtp->GetBrList() == 0) && (pDestFmtp->GetBrList() == 0))
        {
            *nNegoBrList = 0;
        }
        else if ((pSrcFmtp->GetBrList() != 0) && (pDestFmtp->GetBrList() != 0))
        {
            *nNegoBrList = pSrcFmtp->GetBrList() & pDestFmtp->GetBrList();

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bitrate Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->GetBrList() != 0) &&
                (pDestFmtp->GetBrRecv() != 0 || (pDestFmtp->GetBrSend() != 0)))
        {
            if (pDestFmtp->GetBrRecv() == 0)
            {
                pDestFmtp->SetBrRecv(0x0fff);
            }
            if (pDestFmtp->GetBrSend() == 0)
            {
                pDestFmtp->SetBrSend(0x0fff);
            }

            IMS_UINT32 nPeerBRList = pDestFmtp->GetBrRecv() & pDestFmtp->GetBrSend();

            *nNegoBrList = pSrcFmtp->GetBrList() & nPeerBRList;

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bitrate Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBrList = pSrcFmtp->GetBrList() | pDestFmtp->GetBrList();
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pDestFmtp->GetModeSetList() != 0)
        {
            *nNegoModeList = pDestFmtp->GetModeSetList();
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
        for (IMS_UINT32 i = 0; i < pLocalProfile->GetPayloadList().GetSize(); i++)
        {
            AudioProfile::Payload* comparedPayload = pLocalProfile->GetPayloadAt(i);

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
                if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                        comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
                {
                    IMS_UINT32 pnNegoModeSetList = 0;
                    IMS_UINT32 pnNegoDefaultRtpModeSet = 0;
                    AudioProfile::AmrFmtp* pCompareFmtp =
                            (AudioProfile::AmrFmtp*)comparedPayload->GetFmtp();
                    AudioProfile::AmrFmtp* pReceivedFmtp =
                            (AudioProfile::AmrFmtp*)pDstPayload->GetFmtp();

                    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                                pDstPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
                    {
                        continue;
                    }
                    if (comparedPayload->GetRtpMap().GetSamplingRate() !=
                            pDstPayload->GetRtpMap().GetSamplingRate())
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
                        if (pCompareFmtp->GetOctetAlign() != pReceivedFmtp->GetOctetAlign() )
                        {
                            continue;
                        }
                    }*/
                    if (pCompareFmtp->GetOctetAlign() != pReceivedFmtp->GetOctetAlign())
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
                                    Codec[%s], OctetAlign[%d]",
                                    i, comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                                    pCompareFmtp->GetOctetAlign());
                            IMS_TRACE_I("FindPayloadIndexFromProfile() Local/Peer is not exactly \
                                    matched[0x%04x][0x%04x] =>[0x%04x]. Try next",
                                    pCompareFmtp->GetModeSetList(), pReceivedFmtp->GetModeSetList(),
                                    pnNegoModeSetList);
                        }
                        continue;
                    }
                    else  // exactly matched
                    {
                        IMS_TRACE_D("FindPayloadIndexFromProfile() Found AMR at[%d], Codec[%s], \
                                OctetAlign[%d]",
                                i, comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                                pCompareFmtp->GetOctetAlign());

                        return i;
                    }
                }
            }
            else if (strCodecName.EqualsIgnoreCase("EVS"))
            {
                if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
                {
                    IMS_UINT32 BandwidthNegoList;
                    IMS_UINT32 BitrateNegoList;
                    IMS_UINT32 ModeSetNegoList;
                    AudioProfile::EvsFmtp* pCompareFmtp =
                            (AudioProfile::EvsFmtp*)comparedPayload->GetFmtp();
                    AudioProfile::EvsFmtp* pReceivedFmtp =
                            (AudioProfile::EvsFmtp*)pDstPayload->GetFmtp();

                    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                                pDstPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
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
                        if (pReceivedFmtp->GetBwList() == 1)
                        {
                            continue;      //vzw req. if offer is nb only, no evs negotiation
                        }
                        if (pDstPayload->GetRtpMap().GetPayloadNumber() !=
                                comparedPayload->GetRtpMap().GetPayloadNumber())
                        {
                            continue;     //vzw req. payload number based negotiation
                        }
                    }*/

                    IMS_TRACE_D("FindPayloadIndexFromProfile() Found EVS at[%d], Codec[%s], \
                            EvsModeSwitch[%d]",
                            i, comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                            pCompareFmtp->GetEvsModeSwitch());

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
                if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                            pDstPayload->GetRtpMap().GetPayloadType()))
                {
                    IMS_TRACE_D("FindPayloadIndexFromProfile() Found G.711(%s) Found at[%d]",
                            comparedPayload->GetRtpMap().GetPayloadType().GetStr(), i, 0);

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

PRIVATE void AudioNego::SetSdpSessionIpAddress(
        OUT ISessionDescriptor* pSessionDescriptor, IN AudioProfile* pProfile)
{
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->GetIpAddress()))
    {
        IMS_TRACE_D("SetSdpSessionIpAddress() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->GetIpAddress().ToCharString(), 0);

        pSessionDescriptor->SetConnectionAddress(pProfile->GetIpAddress().ToString());
        pSessionDescriptor->SetOriginAddress(pProfile->GetIpAddress().ToString());
    }
}

PRIVATE void AudioNego::SetSdpMediaDescription(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    AStringArray objAudioFormat;
    AString strPayloadNum;
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        objAudioFormat.AddElement(strPayloadNum);
    }

    // Set transport type and port number
    pDescriptor->SetMediaDescription(SdpMedia::TYPE_AUDIO, pProfile->GetDataPort(),
            SdpMedia::TRANSPORT_RTP_AVP, objAudioFormat);
}

PRIVATE void AudioNego::SetSdpMediaBandwidth(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pProfile->GetBandwidthAs() > 0)
    {
        pDescriptor->AddBandwidth(SdpBandwidth::TYPE_AS, pProfile->GetBandwidthAs());

        if (pProfile->GetBandwidthRs() >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RS, pProfile->GetBandwidthRs());
        }

        if (pProfile->GetBandwidthRr() >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RR, pProfile->GetBandwidthRr());
        }
    }
}
