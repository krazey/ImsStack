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
#include "IMSCore.h"
#include "ISessionDescriptor.h"
#include "offeranswer/SdpAvCodec.h"

#include "audio/AudioNego.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "MediaResourceMngr.h"
#include "MediaManager.h"

#include "operator/VZWProperty.h"

#define MODESET_MAX_AMR     7
#define MODESET_MAX_AMRWB   8
#define EVS_NEGO_RETRY_COUNT  2
#define MAX_OAMODEL_SIZE 6
#define EVS_BR_CNT 12
#define EVS_BW_CNT 4

__IMS_TRACE_TAG_USER_DECL__("MED.AN");
const AString AudioNego::EVS_BR[EVS_BR_CNT] = {
    "5.9","7.2","8","9.6","13.2","16.4","24.4", "32","48","64","96","128"};
const AString AudioNego::EVS_BW[EVS_BW_CNT] = {"nb","wb","swb","fb"};
const AString AudioNego::AUDIO_CODEC_BANDWIDTH_STRING[EVS_BW_CNT] = {"NB","WB","SWB","FB"};
const AString AudioNego::AUDIO_CODEC_BITRATE_STRING[3][9] = {
    {"4.75","5.15","5.90","6.70","7.40","7.95","10.20","12.20","0"},        // AMR NB
    {"6.60","8.85","12.65","14.25","15.85","18.25","19.85","23.05","23.85"},// AMR WB/EVS AMR IO
    {"5.90","7.20","8.00","9.60","13.20","16.40","24.40","0","0"}           // EVS
};

// == Constructor, Destructor, Operator Overloading ========================================
PUBLIC
AudioNego::AudioNego(IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_lstOaModel(IMSList<OaModel*>()),
        m_objBaseProfile(AudioProfile()),
        m_pMediaEnvironment(IMS_NULL),
        m_eSessionType(MEDIA_TYPE_AUDIO),
        m_bMandatoryNego(IMS_FALSE)
{
    IMS_TRACE_I("+AudioNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC
AudioNego::~AudioNego()
{
    IMS_TRACE_I("~AudioNego()", 0, 0, 0);
    DestroyProfiles();

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

GLOBAL PUBLIC
AudioNego* AudioNego::Create(IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType)
{
    (void)eServiceType;
    return new AudioNego(nSlotId);
}

PUBLIC
void AudioNego::Copy(IN AudioNego* pAudioNego)
{
    if (pAudioNego == IMS_NULL)
    {
        return;
    }
    IMS_TRACE_I("Copy() - list size[%d]", pAudioNego->m_lstOaModel.GetSize(), 0, 0);

    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());
    MediaResourceMngr* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr != IMS_NULL)
    {
        // To release previous used port
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
        }
    }

    m_objBaseProfile.Copy(&pAudioNego->m_objBaseProfile);

    if (pResourceMngr != IMS_NULL)
    {
        // To add port (it would be duplicated)
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->AcquireRtpPort(m_objBaseProfile.nDataPort, m_objBaseProfile.nDataPort);
        }
    }

    m_eSessionType = pAudioNego->m_eSessionType;
    m_pMediaEnvironment = pAudioNego->m_pMediaEnvironment;
    m_bMandatoryNego = pAudioNego->m_bMandatoryNego;

    //static const AString EVS_BR[12]; (remove warning)
    //static const AString EVS_BW[4] ; (remove warning)

    if (pAudioNego->m_lstOaModel.GetSize() < 1)
    {
        return;
    }

    OaModel* pNewOaModel = new OaModel();
    OaModel* pOldOaModel = pAudioNego->m_lstOaModel.GetAt(0);
    pNewOaModel->pSrcProfile = new AudioProfile(pOldOaModel->pSrcProfile);
    m_lstOaModel.Append(pNewOaModel);
    IMS_TRACE_I("Copy() - list size[%d]", m_lstOaModel.GetSize(), 0, 0);

    return;
}

PUBLIC
void AudioNego::CreateProfiles(IN MediaEnvironment* pEnvironment)
{
    if (pEnvironment == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("CreateProfiles() svc[%d]", (IMS_SINT32)pEnvironment->eServiceType, 0, 0);

    IMS_UINT32 nNumAudioSession = 0;

    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(GetSlotId(),
            pEnvironment->eServiceType);

    if (pMediaSessionConfig == IMS_NULL)
    {
        return;
    }

    nNumAudioSession = pMediaSessionConfig->GetAudioConfigSessionNum();

    for (IMS_UINT32 nIndex = 0; nIndex < nNumAudioSession; nIndex ++)
    {
        MEDIA_CONTENT_TYPE eSessionType;
        AudioProfile* pProfile;
        AudioConfiguration* pConfig = pMediaSessionConfig->
                GetAudioConfiguration(nIndex, &eSessionType);

        IMS_TRACE_I("CreateProfiles() Sessiontype=0x%x", eSessionType, 0, 0);

        if (pConfig == NULL)
        {
            IMS_TRACE_E(0,"CreateProfiles() pConfig is NULL", 0, 0, 0);
            continue;
        }
        if (!MEDIA_IS_CONTAINED_THIS_TYPE(eSessionType, MEDIA_TYPE_AUDIO))
        {
            IMS_TRACE_E(0, "CreateProfiles(), Invalid SessionType[%d]",
                (IMS_SINT32)eSessionType, 0, 0);
            continue;
        }

        pProfile = &m_objBaseProfile;

        AudioProfileConfigurer::CreateAudioProfile(pProfile, pEnvironment, pConfig, GetSlotId());
    }

    if (pMediaSessionConfig != NULL)
    {
        m_bMandatoryNego = MEDIA_IS_CONTAINED_THIS_TYPE(
                pMediaSessionConfig->GetMediaMandatoryNego(), MEDIA_TYPE_AUDIO);
    }
}

PUBLIC
VIRTUAL void AudioNego::DestroyProfiles()
{
    while (m_objBaseProfile.lstPayload.GetSize() > 0)
    {
        AudioProfile::Payload* pPayload = m_objBaseProfile.lstPayload.GetAt(0);
        if (pPayload != IMS_NULL)
        {
            delete pPayload;
        }

        m_objBaseProfile.lstPayload.RemoveAt(0);
    }

    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());
    MediaResourceMngr* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr != IMS_NULL)
    {
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
        }
    }
}

PUBLIC VIRTUAL
void AudioNego::SetMediaEnvironment(IN MediaEnvironment* pMediaEnvironment)
{
    m_pMediaEnvironment = pMediaEnvironment;
}

PUBLIC
void AudioNego::SetSessionType(IN MEDIA_CONTENT_TYPE eSessionType)
{
    m_eSessionType = eSessionType;
}

// -- Negotiation APIs -------------------------------------------------------------------------
PUBLIC VIRTUAL
IMS_BOOL AudioNego::FormSDP(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
        IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eDir)
{
    IMS_TRACE_D("FormSDP Enter",0, 0, 0);
    switch (eNegoState)
    {
        case STATE_IDLE :
            return FormOffer(pSessionDescriptor, pDescriptor, eType, eDir);
        case STATE_OFFER_RECEIVED :
            return FormAnswer(pSessionDescriptor, pDescriptor, eType, eDir);
        case STATE_NEGOTIATED :
            return FormReoffer(pSessionDescriptor, pDescriptor, eType, eDir);
        default :
            IMS_TRACE_E(0,"FormSDP fail eNegoState[%d]",eNegoState,0, 0);
            return IMS_FALSE;
    }
}

PROTECTED VIRTUAL
IMS_BOOL AudioNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eDir)
{
    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_AUDIO) != IMS_TRUE ||
            eDir == MEDIA_DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("FormOffer() Entered", 0, 0, 0);


    // Step 1. Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pNewOaModel->pSrcProfile = new AudioProfile(&m_objBaseProfile);

    // Step 2. Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_I("FormOffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pSrcProfile->eDirection = eDir;
    }

    // Step 3. Modify a RS/RR by conditions (for RTCP enable/disable)
    AudioProfileConfigurer::SetAudioRsRr(pNewOaModel->pSrcProfile, GetConfig(),
            pNewOaModel->pSrcProfile->eDirection);

    pNewOaModel->pSrcProfile->bIsOfferCase = IMS_TRUE;
    m_lstOaModel.Append(pNewOaModel);

    // Step 5. Make the SDP from profile
    IMS_BOOL bSdpMade = MakeSdpFromProfile(pSessionDescriptor, pDescriptor,
            pNewOaModel->pSrcProfile);

    // Delete Session Level Direction Attribute
    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);

    return bSdpMade;
}

PROTECTED VIRTUAL
IMS_BOOL AudioNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eDir)
{
    IMS_TRACE_D("FormAnswer() enter. eType[%d], eDir[%d]", eType, eDir, 0);

    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }
    if (m_lstOaModel.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    // Do not return even if direction is MEDIA_DIRECTION_INVALID
    if (m_bMandatoryNego && (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_AUDIO) &&
            eDir == MEDIA_DIRECTION_INVALID))
    {
        return IMS_FALSE;
    }

    // Getting OaModel from list
    OaModel* pNewOaModel = GetNegotiatedOaModel();
    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }
    if (pNewOaModel->IsAllProfileExist() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    // Compare the media type between base and requested. If it not matched,
    // re-create a negotiated profile
    MEDIA_CONTENT_TYPE eBaseWithoutText =
            (MEDIA_CONTENT_TYPE)MEDIA_TYPE_WITHOUT_TEXT(m_eSessionType);
    MEDIA_CONTENT_TYPE eRequestedWithoutText = (MEDIA_CONTENT_TYPE)MEDIA_TYPE_WITHOUT_TEXT(eType);

    if (eBaseWithoutText != eRequestedWithoutText)
    {
        IMS_TRACE_I("FormAnswer() Media type doesn't matched Base[%d], Requested[%d]",
            eBaseWithoutText, eRequestedWithoutText, 0);

        pNewOaModel->pSrcProfile->Copy(&m_objBaseProfile);

        if (pNewOaModel->pNegotiatedProfile != IMS_NULL)
        {
            delete pNewOaModel->pNegotiatedProfile;
        }

        pNewOaModel->pNegotiatedProfile = new AudioProfile();
        if (MakeNegotiatedProfile(pNewOaModel->pSrcProfile, pNewOaModel->pDestProfile,
                IMS_TRUE, pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
        {
            delete pNewOaModel;
            return IMS_FALSE;
        }
    }

    // Modify a RTP/RTCP port if audio is not supported
    if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_AUDIO) != IMS_TRUE &&
        pNewOaModel->pSrcProfile->nBandwidthAs > 0)
    {
        pNewOaModel->pNegotiatedProfile->nDataPort = 0;
        pNewOaModel->pNegotiatedProfile->nControlPort= 0;
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    // AudioProfileConfigurer* pConfigurer = m_pMediaSession->GetAudioProfileConfigurer();
    // if (pConfigurer == IMS_NULL) return IMS_FALSE;
    // pConfigurer->SetAudioRsRr(pNewOaModel->pNegotiatedProfile, m_pMediaSession->GetEnvironment(),
    //      eType, eDir);
    // 07062016 - Set RS/RR value ZERO when direction INACTIVE Initial OFFER received

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID)
    {
        pNewOaModel->pNegotiatedProfile->eDirection = eDir;
        pNewOaModel->pNegotiatedProfile->bIsHold =
                AudioProfileConfigurer::CheckHoldDirection(GetConfig(), eDir,
                pNewOaModel->pSrcProfile);

        IMS_TRACE_I("FormAnswer() Enforced Set to direction[%d], bHold[%d]",
                eDir, pNewOaModel->pNegotiatedProfile->bIsHold, 0);
    }

    pNewOaModel->pSrcProfile->bIsOfferCase = IMS_FALSE;

    // Step 6. Make the SDP from profile
    IMS_BOOL bSDPMade = MakeSdpFromProfile(pSessionDescriptor, pDescriptor,
            pNewOaModel->pNegotiatedProfile);

    if (pSessionDescriptor->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        // Delete Session Level Direction Attribute
        pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    return bSDPMade;

}

PROTECTED VIRTUAL
IMS_BOOL AudioNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eDir)
{
    IMS_TRACE_I("FormReoffer() pDescriptor[%" PFLS_x"], eDir[%d], m_lstOaModel.GetSize[%d]",
            pDescriptor, eDir, m_lstOaModel.GetSize());

    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }
    // Do not return even if direction is MEDIA_DIRECTION_INVALID
    if (m_bMandatoryNego && (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_AUDIO) &&
        eDir == MEDIA_DIRECTION_INVALID))
    {
        return IMS_FALSE;
    }

    AudioConfiguration* pConfig = GetConfig();
    if (pConfig == NULL)
    {
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
        pNewOaModel->pSrcProfile = new AudioProfile(&m_objBaseProfile);
    }
    else
    {
        //OaModel* pPrevOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize()-1);
        OaModel* pPrevOaModel = GetNegotiatedOaModel();

        if (pPrevOaModel == IMS_NULL)
        {
            if (pNewOaModel != NULL)
            {
                delete pNewOaModel;
            }
            return IMS_FALSE;
        }

        // reuse previous profile when negotiated profile data port is 0
        if (pPrevOaModel->pNegotiatedProfile != IMS_NULL
                && pPrevOaModel->pNegotiatedProfile->nDataPort == 0)
        {
            pNewOaModel->pSrcProfile = new AudioProfile(pPrevOaModel->pNegotiatedProfile);
        }
        else
        {
            if (pConfig->GetSdpReofferFullcapa() == IMS_TRUE)
            {
                pNewOaModel->pSrcProfile = new AudioProfile(&m_objBaseProfile);
            }
            else
            {
                pNewOaModel->pSrcProfile = new AudioProfile(pPrevOaModel->pNegotiatedProfile);
            }
            //set default AS value when srcProfile AS value is 0 in ReOffer case
            if (pNewOaModel->pSrcProfile->nBandwidthAs <= 0)
            {
                IMS_TRACE_I("FormReoffer() - use default AS value", 0, 0, 0);
                pNewOaModel->pSrcProfile->nBandwidthAs = m_objBaseProfile.nBandwidthAs;
            }
        }
    }

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_I("FormReoffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pSrcProfile->eDirection = eDir;
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    AudioProfileConfigurer::SetAudioRsRr(pNewOaModel->pSrcProfile, GetConfig(),
            pNewOaModel->pSrcProfile->eDirection);

    // Modify a RTP/RTCP port if audio is not supported
    if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_AUDIO) != IMS_TRUE)
    {
        pNewOaModel->pSrcProfile->nDataPort = 0;
        pNewOaModel->pSrcProfile->nControlPort= 0;
    }
    else
    {
        pNewOaModel->pSrcProfile->nDataPort = m_objBaseProfile.nDataPort;
        pNewOaModel->pSrcProfile->nControlPort= m_objBaseProfile.nControlPort;
    }

    // when reoffer case - recover rtcpxr to default in sendrecv case
    if (m_objBaseProfile.bSupportRtcpXr == IMS_TRUE &&
            pNewOaModel->pSrcProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        pNewOaModel->pSrcProfile->bSupportRtcpXr = m_objBaseProfile.bSupportRtcpXr;
        pNewOaModel->pSrcProfile->objRtcpXrAttr = m_objBaseProfile.objRtcpXrAttr;
    }

    pNewOaModel->pSrcProfile->bIsOfferCase = IMS_TRUE;
    m_lstOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    IMS_BOOL bSDPMade = MakeSdpFromProfile(pSessionDescriptor, pDescriptor,
            pNewOaModel->pSrcProfile);

    // Delete Session Level Direction Attribute
    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    return bSDPMade;
}

PUBLIC
IMS_BOOL AudioNego::NegotiateSDP(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
        OUT MEDIA_DIRECTION* eDir)
{
    if (eDir == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("NegotiateSDP() NegoState[%d]", eNegoState, 0, 0);

    *eDir = MEDIA_DIRECTION_INVALID;
    switch (eNegoState)
    {
        case STATE_IDLE :
             *eDir = NegotiateOffer(pSessionDescriptor, pDescriptor);
            break;
        case STATE_OFFER_SENT :
            *eDir = NegotiateAnswer(pSessionDescriptor, pDescriptor);
            break;
        case STATE_NEGOTIATED :
            *eDir = NegotiateReanswer(pSessionDescriptor, pDescriptor);
            break;
        default :
            break;
    }

    if (*eDir != MEDIA_DIRECTION_INVALID)
    {
        return IMS_TRUE;
    }
    else
    {
        return IMS_FALSE;
    }
}

PROTECTED VIRTUAL
MEDIA_DIRECTION AudioNego::NegotiateOffer(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor)
{
    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 1. Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pSrcProfile = new AudioProfile(&m_objBaseProfile);

    // Step 2. Make a destination profile from SDP
    pNewOaModel->pDestProfile = new AudioProfile();
    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pDestProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 3. Make a negotiated profile from Src&Dest profile
    pNewOaModel->pNegotiatedProfile = new AudioProfile();
    if (MakeNegotiatedProfile(pNewOaModel->pSrcProfile, pNewOaModel->pDestProfile,
            IMS_TRUE, pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
        {
        if (m_bMandatoryNego == IMS_TRUE)
        {
            delete pNewOaModel;
            return MEDIA_DIRECTION_INVALID;
        }
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateOffer() - add session key in NewOaModel[%" PFLS_x"]",
            reinterpret_cast<IMS_SINTP> (pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_lstOaModel.Append(pNewOaModel);

    // Step 4. Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PROTECTED VIRTUAL
MEDIA_DIRECTION AudioNego::NegotiateAnswer(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor)
{
    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }
    if (m_lstOaModel.GetSize() < 1)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 1. Get the latest OAmodel from list
    OaModel* pNewOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize()-1);
    if (pNewOaModel == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 2. Make a destination profile from SDP
    pNewOaModel->pDestProfile = new AudioProfile();

    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pDestProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_lstOaModel.RemoveAt(m_lstOaModel.GetSize()-1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 3. Make a negotiated profile from Src&Dest profile
    pNewOaModel->pNegotiatedProfile = new AudioProfile();

    if (MakeNegotiatedProfile(pNewOaModel->pSrcProfile, pNewOaModel->pDestProfile,
            IMS_FALSE, pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_lstOaModel.RemoveAt(m_lstOaModel.GetSize()-1);
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateAnswer() - add session key in NewOaModel[%" PFLS_x"]",
            reinterpret_cast<IMS_SINTP> (pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP> (pSessionDescriptor);
    // Step 4. Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PROTECTED VIRTUAL
MEDIA_DIRECTION AudioNego::NegotiateReanswer(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor)
{
    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }
    if (m_lstOaModel.GetSize() < 1)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 1. Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pSrcProfile = new AudioProfile(&m_objBaseProfile);

    // Step 2. Make a destination profile from SDP
    pNewOaModel->pDestProfile = new AudioProfile();

    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pDestProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 3. Make a negotiated profile from Src&Dest profile
    pNewOaModel->pNegotiatedProfile = new AudioProfile();

    if (MakeNegotiatedProfile(pNewOaModel->pSrcProfile, pNewOaModel->pDestProfile,
            IMS_FALSE, pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }
    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateReanswer() - add session key in NewOaModel[%" PFLS_x"]",
            reinterpret_cast<IMS_SINTP> (pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP> (pSessionDescriptor);
    m_lstOaModel.Append(pNewOaModel);

    // Step 4. Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PUBLIC VIRTUAL
void AudioNego::FinalizeSDP(IN IMS_SINTP nInputSessionDesciptorKey, IN NEGO_STATE eNegoState)
{
    IMS_BOOL bFoundOaModel = IMS_FALSE;
    IMS_UINT32 nOaModelSize = m_lstOaModel.GetSize();

    //reset confirmed Session check variable
    for (IMS_UINT32 i = 0; i < nOaModelSize; i++)
    {
        OaModel* pCheckedOaModel = m_lstOaModel.GetAt(i);
        if (pCheckedOaModel != IMS_NULL)
        {
            pCheckedOaModel->bConfirmedSession = IMS_FALSE;
        }
    }
    // check latest OA model
    OaModel* pLatestOaModel = IMS_NULL;
    if (nOaModelSize > 0)
    {
        pLatestOaModel = m_lstOaModel.GetAt(nOaModelSize-1);
    }
    if (pLatestOaModel != IMS_NULL)
    {
        if ((pLatestOaModel->IsAllProfileExist() &&
                (eNegoState == STATE_IDLE || eNegoState == STATE_NEGOTIATED)) == IMS_FALSE)
        {
            IMS_TRACE_I("FinalizeSDP() - Incomplete OaModel[%d]. Delete profile",
                    nOaModelSize-1, 0, 0);
            delete pLatestOaModel;
            m_lstOaModel.RemoveAt(--nOaModelSize);
        }
    }

    for (IMS_UINT32 i = 0; i < nOaModelSize; i++)
    {
        // get OaModel
        OaModel* pTempOaModel = m_lstOaModel.GetAt(nOaModelSize-1 -i);

        // find matched SessionDescriptor key
        if (pTempOaModel != IMS_NULL)
        {
            if (pTempOaModel->nSessionDescriptorKey == nInputSessionDesciptorKey)
            {
                pTempOaModel->bConfirmedSession = IMS_TRUE;
                bFoundOaModel = IMS_TRUE;
                IMS_TRACE_D("FinalizeSDP() - find comfirmed Session OaModel[%d]",
                        m_lstOaModel.GetSize()-i, 0, 0);
                break;
            }
        }
    }

    // SessionDescriptor key mismatch case handling
    // not select OaModel
    if (bFoundOaModel != IMS_TRUE && nOaModelSize > 0)
    {
        IMS_TRACE_D("FinalizeSDP() - not found comfirmed Session OaModel", 0, 0, 0);
        return;
    }

    // Remove old OaModels excepts confirmed OaModel
    for (IMS_UINT32 i = 0; i < nOaModelSize; i++)
    {
        if ((nOaModelSize - i -1) < MAX_OAMODEL_SIZE)
        {
            break;
        }

        OaModel* pDeleteCheckOaModel = m_lstOaModel.GetAt(0);

        if (pDeleteCheckOaModel != IMS_NULL)
        {
            IMS_TRACE_D("FinalizeSDP() - remove old OaModel", 0, 0, 0);
            if ((pDeleteCheckOaModel->nSessionDescriptorKey ==  nInputSessionDesciptorKey)
                    && (pDeleteCheckOaModel->bConfirmedSession == IMS_TRUE))
            {
                break;
            }
            IMS_TRACE_D("FinalizeSDP() - Delete the oldest[%" PFLS_x"] OaModel",
                    pDeleteCheckOaModel, 0, 0);
            delete pDeleteCheckOaModel;
            m_lstOaModel.RemoveAt(0);
        }
    }
}

PUBLIC
IMS_BOOL AudioNego::SetPort(IN IMS_UINT32 nPort)
{
    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());
    MediaResourceMngr* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Release Current Port
    if (m_objBaseProfile.nDataPort != 0)
    {
        pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
    }

    IMS_TRACE_I("SetPort() - VoLTE Changed Data Port[%d]->[%d]",
            m_objBaseProfile.nDataPort, nPort, 0);

    if (nPort != 0)
    {
        // Acquire New Port
        m_objBaseProfile.nDataPort = pResourceMngr->AcquireRtpPort(nPort, nPort);
        m_objBaseProfile.nControlPort = m_objBaseProfile.nDataPort + 1;
    }
    else    // port 0 case
    {
        // Set to Port 0
        m_objBaseProfile.nDataPort = 0;
        m_objBaseProfile.nControlPort = 0;

        IMS_TRACE_I("SetPort() - VoLTE Data Port is 0!!!", 0, 0, 0);
    }

    return IMS_TRUE;
}

PUBLIC
AudioNego::OaModel* AudioNego::GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed)
{
    OaModel* pLatestOaModel = IMS_NULL;

    IMS_UINT32 nOaModelCount = m_lstOaModel.GetSize();
    IMS_UINT32 nTempOaModelCount = nOaModelCount;
    while (nTempOaModelCount > 0)
    {
        pLatestOaModel = m_lstOaModel.GetAt(nTempOaModelCount - 1);
        if (pLatestOaModel != IMS_NULL)
        {
            if (pLatestOaModel->IsAllProfileExist() == IMS_TRUE && bCheckConfirmed == IMS_FALSE)
            {
                return pLatestOaModel;
            }
            else if (pLatestOaModel->bConfirmedSession == IMS_TRUE && bCheckConfirmed == IMS_TRUE)
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

PUBLIC
IMS_BOOL AudioNego::GetNegotiatedProfileSet(OUT AudioProfile* &pSrcProfile,
            OUT AudioProfile* &pDestProfile, OUT AudioProfile* &pNegotiatedProfile)
{
    OaModel* pOaModel = GetNegotiatedOaModel();
    if (pOaModel != IMS_NULL)
    {
        pSrcProfile = pOaModel->pSrcProfile;
        pDestProfile = pOaModel->pDestProfile;
        pNegotiatedProfile = pOaModel->pNegotiatedProfile;
        return IMS_TRUE;
    }
    else
    {
        return IMS_FALSE;
    }

}

PUBLIC
MEDIA_DIRECTION AudioNego::GetNegotiatedDirection(void)
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

PUBLIC
AUDIO_CODEC_BITRATE AudioNego::GetNegotiatedAudioCodecRate(void)
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
        pNegotiatedPayload = pNegotiatedProfile->lstPayload.GetAt(
                pNegotiatedProfile->nNegotiatedPayloadIndex);
    }

    if (pNegotiatedPayload == NULL)
    {
        return AUDIO_CODEC_BITRATE_MAX;
    }

    if (pNegotiatedPayload->objRtpMap.strPayloadType.Equals("AMR")
            || pNegotiatedPayload->objRtpMap.strPayloadType.Equals("AMR-WB"))
    {
        //AudioProfile::AmrFmtp* pFmtp = (AudioProfile::AmrFmtp*)pNegotiatedPayload->pFmtp;
        //if (pFmtp == IMS_NULL) return AUDIO_CODEC_BITRATE_MAX;

        IMS_SINT32 nLargestModeSet = -1;

        if (pNegotiatedPayload->objRtpMap.strPayloadType.Equals("AMR-WB"))
        {
            nLargestModeSet = AudioProfileConfigurer::GetLargestModesetInFmtp("AMR-WB",
                    pNegotiatedPayload) + AUDIO_CODEC_BITRATE_AMR_WB_660;
            return (AUDIO_CODEC_BITRATE)nLargestModeSet;
        }
        else    // AMR case
        {
            nLargestModeSet = AudioProfileConfigurer::GetLargestModesetInFmtp("AMR",
                    pNegotiatedPayload) + AUDIO_CODEC_BITRATE_AMR_475;
            return (AUDIO_CODEC_BITRATE)nLargestModeSet;
        }
    }
    else if (pNegotiatedPayload->objRtpMap.strPayloadType.Equals("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pNegotiatedPayload->pFmtp;
        if (pEvsFmtp == IMS_NULL)
        {
            return AUDIO_CODEC_BITRATE_INVALID;
        }

        IMS_SINT32 nLargestModeSet = -1;
        nLargestModeSet =
                AudioProfileConfigurer::GetLargestModesetInFmtp("EVS", pNegotiatedPayload);
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
        {    // EVS AMR IO Mode
            nLargestModeSet = nLargestModeSet + AUDIO_CODEC_BITRATE_EVS_IO_660;
        }

        return (AUDIO_CODEC_BITRATE)nLargestModeSet;
    }

    return AUDIO_CODEC_BITRATE_INVALID;
}

PUBLIC AUDIO_CODEC AudioNego::GetNegotiatedCodec(void)
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
        //if (pLatestOaModel->pNegotiatedProfile->nDataPort == 0) return AUDIO_CODEC_NONE;
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
            { // Primary WB case
                return AUDIO_CODEC_EVS_WB;
            }
            else if ((pEvsFmtp->nBwList & 0x01) != 0)
            { // Primary NB case
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

PUBLIC
IMS_BOOL AudioNego::HasNegotiatedDtmf(void)
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

        AudioProfile::Payload* pPayload = IMS_NULL;

        for (IMS_UINT32 i=0; i<pLatestOaModel->pNegotiatedProfile->lstPayload.GetSize(); i++)
        {
            pPayload = pLatestOaModel->pNegotiatedProfile->lstPayload.GetAt(i);
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

PUBLIC
IMS_SINT32 AudioNego::GetMediaBandwidth(void)
{
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize()-1);
        if (pLatestOaModel == IMS_NULL || pLatestOaModel->pSrcProfile == NULL)
        {
            return -1;
        }

        // returned negotiated bandwidth.
        if (pLatestOaModel->pNegotiatedProfile != NULL)
        {
            return (IMS_SINT32)pLatestOaModel->pNegotiatedProfile->nBandwidthAs;
        }

        // if negotiated bandwidth does not exist, then return src profile bandwidth.
        return (IMS_SINT32)pLatestOaModel->pSrcProfile->nBandwidthAs;
    }

    return -1;
}

// == PROTECTED METHOD ==========================================================

PROTECTED
IMS_BOOL AudioNego::MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("MakeSdpFromProfile() - PayloadSize[%d], AS[%d]",
            pProfile->lstPayload.GetSize(), pProfile->nBandwidthAs, 0);

    // clean attr & bandwidth line
    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    IMSList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // Step 0. make"c" &"o" line of session level if IP does not matched
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->objIpAddr))
    {
        IMS_TRACE_D(
                "MakeSdpFromProfile() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->objIpAddr.ToCharString(), 0);

        pSessionDescriptor->SetConnectionAddress(pProfile->objIpAddr.ToString());
        pSessionDescriptor->SetOriginAddress(pProfile->objIpAddr.ToString());
    }

    // Step 1. make"m" line
    // ------"m=audio xxxx RTP/AVP 104 110 105 102 108 100"
    AStringArray objAudioFormat;
    AString strPayloadNum;
    for (IMS_UINT32 i=0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->objRtpMap.nPayloadNum);
        objAudioFormat.AddElement(strPayloadNum);
    }

    // Step 1.1 Check Transport Type
    if (pProfile->strTransportType.Equals("RTP/SAVP"))
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_AUDIO, pProfile->nDataPort,
                SdpMedia::TRANSPORT_RTP_SAVP, objAudioFormat);
    }
    else
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_AUDIO, pProfile->nDataPort,
                SdpMedia::TRANSPORT_RTP_AVP, objAudioFormat);
    }

    // Step 2. make bandwidth
    // ------"b=AS:xx"
    // ------"b=AS:xx"
    // ------"b=AS:xx"
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

    // Step 3. make each payload
    // ------"a=rtpmap:104 AMR-WB/16000/1"
    // ------"a=fmtp:110 mode-set=2; octet-align=1"
    for (IMS_UINT32 i=0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AString strRtpmap, strFmtp;

        AudioProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // Step 4-1. make"rtpmap"
        strPayloadNum.Sprintf("%d", pPayload->objRtpMap.nPayloadNum);
        strRtpmap.Sprintf("%s/%d", pPayload->objRtpMap.strPayloadType.GetStr(),
                pPayload->objRtpMap.nSamplingRate);

        if (pPayload->objRtpMap.nChannel > 0)
        {
            AString strChannel;
            strChannel.Sprintf("/%d", pPayload->objRtpMap.nChannel);
            strRtpmap.Append(strChannel);
        }

        // Step 4-2. make"fmtp"
        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB")
                || pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR"))
        {
            AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pPayload->pFmtp;
            if (pAmrFmtp == IMS_NULL)
            {
                continue;
            }

            // mode-set
            if (pAmrFmtp->nModeSetList != 0)
            {
                AString strTemp, strMode;
                IMS_UINT32 nModeSet;

                for (nModeSet = 0; nModeSet <= MODESET_MAX_AMRWB; nModeSet++)
                {
                    IMS_UINT32 nMatch = (pAmrFmtp->nModeSetList) & (1 << nModeSet);
                    if (nMatch)
                    {
                        if (strTemp.GetLength() > 0)
                        {
                            strTemp.Append(",");
                        }

                        strMode.Sprintf("%d", nModeSet);
                        strTemp.Append(strMode);
                    }
                }

                strFmtp.Append("mode-set=");
                strFmtp.Append(strTemp);
            }

            // octet-align
            if (pAmrFmtp->nOctetAlign != AudioProfile::AmrFmtp::DEFAULT_OCTCTALIGN ||
                    pAmrFmtp->bShow_OctetAlign == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("octet-align=%d", pAmrFmtp->nOctetAlign);
                strFmtp.Append(strTemp);
            }

            // mode-cahnge-capability
            if (pAmrFmtp->nModeChangeCapability == 2 ||
                    pAmrFmtp->bShowModeChangeCapability == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("mode-change-capability=%d", pAmrFmtp->nModeChangeCapability);
                strFmtp.Append(strTemp);
            }

            // mode-change-period
            if (pAmrFmtp->nModeChangePeriod == 2 || pAmrFmtp->bShowModeChangePeriod == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("mode-change-period=%d", pAmrFmtp->nModeChangePeriod);
                strFmtp.Append(strTemp);
            }

            // mode-change-neighbor
            if (pAmrFmtp->nModeChangeNeighbor != AudioProfile::AmrFmtp::DEFAULT_MODECHANGE_NEIGHBOR
                    || pAmrFmtp->bShowModeChangeNeighbor == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("mode-change-neighbor=%d", pAmrFmtp->nModeChangeNeighbor);
                strFmtp.Append(strTemp);
            }

            // robust-sorting
            if (pAmrFmtp->nRobustSorting != AudioProfile::AmrFmtp::DEFAULT_ROBUSTSORTING ||
                    pAmrFmtp->bShow_RobustSorting == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("robust-sorting=%d", pAmrFmtp->nRobustSorting);
                strFmtp.Append(strTemp);
            }

            // max-red
            if (pAmrFmtp->nMaxRed != AudioProfile::AmrFmtp::DEFAULT_MAXRED ||
                    pAmrFmtp->bShowMaxRed == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("max-red=%d", pAmrFmtp->nMaxRed);
                strFmtp.Append(strTemp);
            }

            // maxptime
            if (pAmrFmtp->nPtime != AudioProfile::AmrFmtp::DEFAULT_PTIME ||
                    pAmrFmtp->bShowPtime == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("ptime=%d", pAmrFmtp->nPtime);
                strFmtp.Append(strTemp);
            }

            // maxptime
            if (pAmrFmtp->nMaxPtime != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME ||
                    pAmrFmtp->bShowMaxPtime == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("maxptime=%d", pAmrFmtp->nMaxPtime);
                strFmtp.Append(strTemp);
            }

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

            // ptime
            if (pEvsFmtp->nPtime != AudioProfile::EvsFmtp::DEFAULT_PTIME ||
                    pEvsFmtp->bShowPtime == IMS_TRUE)
            {
                AString strTemp;
                strTemp.Sprintf("ptime=%d", pEvsFmtp->nPtime);
                strFmtp.Append(strTemp);
            }
            // maxptime
            if (pEvsFmtp->nMaxPtime != AudioProfile::EvsFmtp::DEFAULT_MAXPTIME ||
                    pEvsFmtp->bShowMaxPtime == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("maxptime=%d", pEvsFmtp->nMaxPtime);
                strFmtp.Append(strTemp);
            }
            // dtx
            if (pEvsFmtp->nDtx != AudioProfile::EvsFmtp::DEFAULT_DTX ||
                    pEvsFmtp->bShowDtx == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("dtx=%d", pEvsFmtp->nDtx);
                strFmtp.Append(strTemp);
            }
            // dtx
            if (pEvsFmtp->nHfOnly != AudioProfile::EvsFmtp::DEFAULT_HFMODE ||
                    pEvsFmtp->bShowHfOnly == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("hf-only=%d", pEvsFmtp->nHfOnly);
                strFmtp.Append(strTemp);
            }
            // evs mode switch
            if (pEvsFmtp->nEvsModeSwitch != AudioProfile::EvsFmtp::DEFAULT_EVSMODESWITCH ||
                    pEvsFmtp->bShowEvsModeSwitch == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("evs-mode-switch=%d", pEvsFmtp->nEvsModeSwitch);
                strFmtp.Append(strTemp);
            }
            // max-red
            if (pEvsFmtp->nMaxRed != AudioProfile::EvsFmtp::DEFAULT_MAXRED ||
                    pEvsFmtp->bShowMaxRed == IMS_TRUE)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("max-red=%d", pEvsFmtp->nMaxRed);
                strFmtp.Append(strTemp);
            }
            // bandwidth list
            if ((pEvsFmtp->nBwList != 0) && (pEvsFmtp->bShowBwList))
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
                IMS_UINT32 nBandwidthList;
                IMS_UINT32 nBandwidthListTotalCnt = 0;

                for (nBandwidthList = 0; nBandwidthList < EVS_BW_CNT; nBandwidthList++)
                {
                    IMS_UINT32 nMatch = (pEvsFmtp->nBwList) & (1 << nBandwidthList);
                    if (nMatch)
                    {
                        if (strTemp.GetLength() > 0)
                        strTemp.Append(",");
                        strMode.Sprintf("%s", EVS_BW[nBandwidthList].GetStr());
                        strTemp.Append(strMode);
                        nBandwidthListTotalCnt++;

                        if (nBandwidthListTotalCnt == 1) {
                            strFirstBandwidth = EVS_BW[nBandwidthList];
                        }

                        strLastBandwidth = EVS_BW[nBandwidthList];
                    }
                }

                if (nBandwidthListTotalCnt > 1) {
                    strTemp ="";
                    strTemp.Append(strFirstBandwidth);
                    strTemp.Append("-");
                    strTemp.Append(strLastBandwidth);
                }

                strFmtp.Append("bw=");
                strFmtp.Append(strTemp);
            }
            // bitrate list
            if ((pEvsFmtp->nBrList != 0) && (pEvsFmtp->bShowBrList)) {
                if (strFmtp.GetLength() > 0)
                    strFmtp.Append(";");

                AString strTemp, strMode, strFirstBitrate, strLastBitrate;
                IMS_UINT32 nBitrateList;
                IMS_UINT32 nBitrateListTotalCnt = 0;

                for (nBitrateList = 0; nBitrateList < EVS_BR_CNT; nBitrateList++)
                {
                    IMS_UINT32 nMatch = (pEvsFmtp->nBrList) & (1 << nBitrateList);
                    if (nMatch)
                    {
                        if (strTemp.GetLength() > 0)
                        strTemp.Append(",");

                        strMode.Sprintf("%s", EVS_BR[nBitrateList].GetStr());
                        strTemp.Append(strMode);
                        nBitrateListTotalCnt++;

                        if (nBitrateListTotalCnt == 1) strFirstBitrate = EVS_BR[nBitrateList];

                        strLastBitrate = EVS_BR[nBitrateList];
                    }
                }

                if (nBitrateListTotalCnt > 1)
                {
                    strTemp ="";
                    strTemp.Append(strFirstBitrate);
                    strTemp.Append("-");
                    strTemp.Append(strLastBitrate);
                }

                strFmtp.Append("br=");
                strFmtp.Append(strTemp);
            }
            // cmr
            if ((pEvsFmtp->nCmr != AudioProfile::EvsFmtp::DEFAULT_CMR ||
                    pEvsFmtp->bShowCmr == IMS_TRUE) && (pEvsFmtp->nEvsModeSwitch != 1))
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("cmr=%d", pEvsFmtp->nCmr);
                strFmtp.Append(strTemp);
            }
            // channel aware mode
            if ((pEvsFmtp->bShowChannelAwMode == IMS_TRUE) && (pEvsFmtp->nEvsModeSwitch != 1))
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("ch-aw-recv=%d", pEvsFmtp->nChAwRecv);
                strFmtp.Append(strTemp);
            }
            // mode-set (AMR-IO mode)
            if (pEvsFmtp->nModeSetList != 0)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp, strMode;
                IMS_UINT32 nModeSet;

                for (nModeSet = 0; nModeSet <= MODESET_MAX_AMRWB; nModeSet++)
                {
                    IMS_UINT32 nMatch = (pEvsFmtp->nModeSetList) & (1 << nModeSet);
                    if (nMatch)
                    {
                        if (strTemp.GetLength() > 0)
                        {
                            strTemp.Append(",");
                        }

                        strMode.Sprintf("%d", nModeSet);
                        strTemp.Append(strMode);
                    }
                }
                strFmtp.Append("mode-set=");
                strFmtp.Append(strTemp);
            }
            // mode change capa (AMR-IO mode)
            if ((pEvsFmtp->nModeChangeCapability
                    != AudioProfile::EvsFmtp::DEFAULT_MODECHANGE_CAPABILITY
                    || pEvsFmtp->bShowModeChangeCapability == IMS_TRUE)
                    && (pEvsFmtp->nEvsModeSwitch == 1))
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("mode-change-capability=%d", pEvsFmtp->nModeChangeCapability);
                strFmtp.Append(strTemp);
            }
            // mode change periode (AMR-IO mode)
            if ((pEvsFmtp->nModeChangePeriod != AudioProfile::EvsFmtp::DEFAULT_MODECHANGE_PERIOD ||
                    pEvsFmtp->bShowModeChangePeriod == IMS_TRUE) &&
                    (pEvsFmtp->nEvsModeSwitch == 1))
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("mode-change-period=%d", pEvsFmtp->nModeChangePeriod);
                strFmtp.Append(strTemp);
            }
            // mode change neighbor (AMR-IO mode)
            if ((pEvsFmtp->nModeChangeNeighbor !=
                        AudioProfile::EvsFmtp::DEFAULT_MODECHANGE_NEIGHBOR ||
                        pEvsFmtp->bShowModeChangeNeighbor == IMS_TRUE) &&
                        (pEvsFmtp->nEvsModeSwitch == 1))
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp;
                strTemp.Sprintf("mode-change-neighbor=%d", pEvsFmtp->nModeChangeNeighbor);
                strFmtp.Append(strTemp);
            }

            // bandwidth unicast list - send direciton
            if (pEvsFmtp->nBwSend != 0)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
                IMS_UINT32 nBandwidthList;
                IMS_UINT32 nBandwidthListTotalCnt = 0;

                for (nBandwidthList = 0; nBandwidthList < EVS_BW_CNT; nBandwidthList++)
                {
                    IMS_UINT32 nMatch = (pEvsFmtp->nBwSend) & (1 << nBandwidthList);
                    if (nMatch)
                    {
                        if (strTemp.GetLength() > 0)
                        {
                            strTemp.Append(",");
                        }
                        strMode.Sprintf("%s", EVS_BW[nBandwidthList].GetStr());
                        strTemp.Append(strMode);
                        nBandwidthListTotalCnt++;

                        if (nBandwidthListTotalCnt == 1)
                        {
                            strFirstBandwidth = EVS_BW[nBandwidthList];
                        }

                        strLastBandwidth = EVS_BW[nBandwidthList];
                    }
                }

                if (nBandwidthListTotalCnt > 1)
                {
                    strTemp ="";
                    strTemp.Append(strFirstBandwidth);
                    strTemp.Append("-");
                    strTemp.Append(strLastBandwidth);
                }

                strFmtp.Append("bw-send=");
                strFmtp.Append(strTemp);
            }

            // bandwidth unicast list - recv direciton
            if (pEvsFmtp->nBwRecv!= 0)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
                IMS_UINT32 nBandwidthList;
                IMS_UINT32 nBandwidthListTotalCnt = 0;

                for (nBandwidthList = 0; nBandwidthList < EVS_BW_CNT; nBandwidthList++)
                {
                    IMS_UINT32 nMatch = (pEvsFmtp->nBwRecv) & (1 << nBandwidthList);
                    if (nMatch)
                    {
                        if (strTemp.GetLength() > 0)
                        {
                            strTemp.Append(",");
                        }
                        strMode.Sprintf("%s", EVS_BW[nBandwidthList].GetStr());
                        strTemp.Append(strMode);
                        nBandwidthListTotalCnt++;

                        if (nBandwidthListTotalCnt == 1)
                        {
                            strFirstBandwidth = EVS_BW[nBandwidthList];
                        }

                        strLastBandwidth = EVS_BW[nBandwidthList];
                    }
                }

                if (nBandwidthListTotalCnt > 1)
                {
                    strTemp ="";
                    strTemp.Append(strFirstBandwidth);
                    strTemp.Append("-");
                    strTemp.Append(strLastBandwidth);
                }

                strFmtp.Append("bw-recv=");
                strFmtp.Append(strTemp);
            }

            // bitrate uni case list - send direction
            if (pEvsFmtp->nBrSend!= 0)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp, strMode, strFirstBitrate, strLastBitrate;
                IMS_UINT32 nBitrateList;
                IMS_UINT32 nBitrateListTotalCnt = 0;

                for (nBitrateList = 0; nBitrateList < EVS_BR_CNT; nBitrateList++)
                {
                    IMS_UINT32 nMatch = (pEvsFmtp->nBrSend) & (1 << nBitrateList);
                    if (nMatch)
                    {
                        if (strTemp.GetLength() > 0)
                        {
                            strTemp.Append(",");
                        }
                        strMode.Sprintf("%s", EVS_BR[nBitrateList].GetStr());
                        strTemp.Append(strMode);
                        nBitrateListTotalCnt++;

                        if (nBitrateListTotalCnt == 1)
                        {
                            strFirstBitrate = EVS_BR[nBitrateList];
                        }

                        strLastBitrate = EVS_BR[nBitrateList];
                    }
                }

                if (nBitrateListTotalCnt > 1)
                {
                    strTemp ="";
                    strTemp.Append(strFirstBitrate);
                    strTemp.Append("-");
                    strTemp.Append(strLastBitrate);
                }

                strFmtp.Append("br-send=");
                strFmtp.Append(strTemp);
            }

            // bitrate uni case list - recv direction
            if (pEvsFmtp->nBrRecv!= 0)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }

                AString strTemp, strMode, strFirstBitrate, strLastBitrate;
                IMS_UINT32 nBitrateList;
                IMS_UINT32 nBitrateListTotalCnt = 0;

                for (nBitrateList = 0; nBitrateList < EVS_BR_CNT; nBitrateList++)
                {
                    IMS_UINT32 nMatch = (pEvsFmtp->nBrRecv) & (1 << nBitrateList);
                    if (nMatch)
                    {
                        if (strTemp.GetLength() > 0)
                        {
                            strTemp.Append(",");
                        }

                        strMode.Sprintf("%s", EVS_BR[nBitrateList].GetStr());
                        strTemp.Append(strMode);
                        nBitrateListTotalCnt++;

                        if (nBitrateListTotalCnt == 1)
                        {
                            strFirstBitrate = EVS_BR[nBitrateList];
                        }

                        strLastBitrate = EVS_BR[nBitrateList];
                    }
                }

                if (nBitrateListTotalCnt > 1)
                {
                    strTemp ="";
                    strTemp.Append(strFirstBitrate);
                    strTemp.Append("-");
                    strTemp.Append(strLastBitrate);
                }

                strFmtp.Append("br-recv=");
                strFmtp.Append(strTemp);
            }
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("pcmu")
                || pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("pcma"))
        {
            // to setting rtpmap, not fmtp
            strFmtp = AString::ConstNull();
            pDescriptor->SetMediaFormat(SdpMediaFormat::TYPE_RTP,
                    strPayloadNum, strRtpmap, strFmtp);
            continue;
        }
        else
        {
            continue;
        }

        if (strFmtp.GetLength() == 0)
        {
            AudioConfiguration* pConfig = GetConfig();
            if ((pConfig != IMS_NULL) && (!pConfig->IsEmptyFmtpAttr()))
            {
                strFmtp = AString::ConstNull();
            }
            else
            {
                strFmtp = AString::ConstEmpty();
            }
        }

        pDescriptor->SetMediaFormat(SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpmap, strFmtp);
    }

    // Step 4. make direction
    pDescriptor->SetDirection(pProfile->eDirection);

    if (pProfile->eDirection > MEDIA_DIRECTION_INVALID &&
            pProfile->eDirection <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        // Set Session Level Direction Attribute according to the media direction
        // (avoid conflict between media and audio)
        pSessionDescriptor->SetDirection(pProfile->eDirection);
    }

    // Step 5. make ptime & maxptime
    if (pProfile->nPtime != AudioProfile::AmrFmtp::DEFAULT_PTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::PTIME, pProfile->nPtime);
    }

    if (pProfile->nMaxPtime != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::MAXPTIME, pProfile->nMaxPtime);
    }

    // Step 6. Set candidate
    if (pProfile->nCandidatePriority > 0 && pProfile->nDataPort != 0)
    {
        // Add attribute=candidate
        AString strCandidate;

        // form candidate attribute as string
        // a=candidate:<foundation(1)> <component ID(RTP:1)>
        // <transport(UDP)> <Priority(nCandPriority> <private-ip> <Port>
        // <cand-type(typ)> <candidate-types(host)>
        strCandidate.Sprintf("1 1 UDP %d %s %d typ host", pProfile->nCandidatePriority,
        pDescriptor->GetLocalAddress().ToString().GetStr(), pProfile->nDataPort);

        // Add Candidate Attribute to pIMediaDescriptor
        pDescriptor->AddAttribute(SdpAttribute::CANDIDATE, strCandidate);
    }

    // Step 7. RTCP-XR -- RTCP-XR is for VZW, not a negotiation target by VZW requirement
    if (pProfile->bSupportRtcpXr == IMS_TRUE &&
            pProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        if (pProfile->objRtcpXrAttr.bSupportStatisticMetrics)
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR,"stat-summary=loss,dup,jitt,HL");
        }
        if (pProfile->objRtcpXrAttr.bSupportVoipMatircs)
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR,"voip-metrics");
        }
        if (pProfile->objRtcpXrAttr.bSupportPacketLossRle)
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR,"pkt-loss-rle");
        }
        if (pProfile->objRtcpXrAttr.bSupportPacketDuplicatedRle)
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR,"pkt-dup-rle");
        }

        IMS_TRACE_I("MakeSdpFromProfile() - bSupportRtcpXr[%d]", pProfile->bSupportRtcpXr, 0, 0);
    }

    // Step 8. make Capa Nego Attribute
    if (pProfile->bSupportCapaNegoForSrtp == IMS_TRUE)
    {
        // add"ACFG" if it's a initial answer
        if (pProfile->objCapaNego.strNegotiatedAcfg.GetLength() > 0)
        {
            AString strACFG;
            IMS_TRACE_D("MakeSdpFromProfile() - strNegotiatedAcfg[%s]",
                    pProfile->objCapaNego.strNegotiatedAcfg.GetStr(),0, 0);
            strACFG.Sprintf("%s", pProfile->objCapaNego.strNegotiatedAcfg.GetStr());
            pDescriptor->AddAttribute(SdpAttribute::ACFG, strACFG);
        }

        //For Support SRTP Capa Nego
        if (pProfile->bSupportSrtp == IMS_TRUE)
        {
            AString strTcap ="";
            AString strAcap ="";
            AString strPcfg ="";

            IMS_TRACE_I("MakeSdpFromProfile() PcfgSize[%d], TcapSize[%d], AcapSize[%d]",
                    pProfile->objCapaNego.lstPotentialConfig.GetSize(),
                    pProfile->objCapaNego.mapTransportCapa.GetSize(),
                    pProfile->objCapaNego.mapAttributeCapa.GetSize());

            for (IMS_UINT32 i = 0 ; i < pProfile->objCapaNego.mapTransportCapa.GetSize(); i++)
            {
                strTcap ="";
                strTcap.Sprintf("%d %s", i+1,
                        pProfile->objCapaNego.mapTransportCapa.GetValueAt(i).GetStr());
                pDescriptor->AddAttribute(SdpAttribute::TCAP, strTcap);
            }

            if (pProfile->objCapaNego.bIsAttCapaInPcfg == IMS_TRUE)
            {
                for (IMS_UINT32 i = 0 ; i < pProfile->objCapaNego.mapAttributeCapa.GetSize(); i++)
                {
                    strAcap ="";
                    strAcap.Sprintf("%d %s", i+1,
                            pProfile->objCapaNego.mapAttributeCapa.GetValueAt(i).GetStr());
                    pDescriptor->AddAttribute(SdpAttribute::ACAP, strAcap);
                    IMS_TRACE_I(":MakeSdpFromProfile() - Add ACAP strAcap : %s",
                            strAcap.GetStr(),0, 0);
                }
            }

            for (IMS_UINT32 i = 0 ; i < pProfile->objCapaNego.lstPotentialConfig.GetSize(); i++)
            {
                strPcfg ="";
                strPcfg.Sprintf("%d %s", i+1,
                        pProfile->objCapaNego.lstPotentialConfig.GetAt(i).GetStr());
                pDescriptor->AddAttribute(SdpAttribute::PCFG, strPcfg);
            }
        }
    }

    //Step 9. make SRTP Attribute
    if (pProfile->strTransportType.Equals("RTP/SAVP"))
    {
        AString strCrypto;
        strCrypto = MakeCryptoAttributeFromSrtpProfile(pProfile);
        pDescriptor->AddAttribute(SdpAttribute::CRYPTO, strCrypto);

        if (pProfile->objCapaNego.strNegotiatedAcfg.GetLength() > 0)
        {
            pDescriptor->AddAttribute(SdpAttribute::A_3GE2AE,"applied");
        }
        else if (pProfile->objCapaNego.lstPotentialConfig.GetSize() > 0)
        {
            pDescriptor->AddAttribute(SdpAttribute::A_3GE2AE,"requested");
        }
        else
        {
            //Nothing to do.
        }
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AudioNego::MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }
    // IP
    pProfile->objIpAddr = pDescriptor->GetRemoteAddress();
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

    IMS_TRACE_I("MakeProfileFromSdp() - AS[%d], RS[%d], RR[%d]",
            pProfile->nBandwidthAs, pProfile->nBandwidthRs, pProfile->nBandwidthRr);

    // transport type for SRTP -
    SdpMedia* pSDPMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();

    if (pSDPMedia != IMS_NULL)
    {
        pProfile->strTransportType = pSDPMedia->GetTransportProtocolEx();

        if (pProfile->strTransportType.Equals("RTP/AVP") == IMS_TRUE)
        {
            pProfile->bSupportSrtp = IMS_FALSE;
            pProfile->bSupportCapaNegoForSrtp = IMS_FALSE;
        }
        else if (pProfile->strTransportType.Equals("RTP/SAVP") == IMS_TRUE)
        {
            IMS_TRACE_D("MakeProfileFromSdp() SRTP enable", 0, 0, 0);
            pProfile->bSupportSrtp = IMS_TRUE;
            pProfile->bSupportCapaNegoForSrtp = IMS_FALSE;
        }
    }

    // read CapaNego profile From SDP
    if (MakeCapaNegoProfileFromSdp(pDescriptor, &(pProfile->objCapaNego)) == IMS_TRUE)
    {
        //Create Audio capa nego profile from the incoming SDP for SRTP
        if (MakeSrtpProfileFromCapaNego(pProfile) == IMS_TRUE)
        {
            IMS_TRACE_D("MakeProfileFromSdp() SRTP enable", 0, 0, 0);
            pProfile->bSupportCapaNegoForSrtp = IMS_TRUE;
        }
    }

    // payload
    IMSList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSDPCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));
        if (pSDPCodec == IMS_NULL)
        {
            return IMS_FALSE;
        }
        AString strCodecName = pSDPCodec->GetName();
        AString strChannel = pSDPCodec->GetEncodingParameters();
        IMS_UINT32 nChannel;
        IMS_SINT32 nPayloadTypeNumber = pSDPCodec->GetPayloadType();

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

        IMS_TRACE_D("MakeProfileFromSdp() - At[%d], PayloadType[%d], ClockRate[%d]",
                i, pSDPCodec->GetPayloadType(), pSDPCodec->GetClockRate());

        AudioProfile::Payload* pPayload = new AudioProfile::Payload();
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        pPayload->SetRtpMap(pSDPCodec->GetPayloadType(), strCodecName,
                pSDPCodec->GetClockRate(), nChannel);

        if (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR"))
        {
            // Create AMR fmtp
            AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp();
            GetFmtpFromString(pSDPCodec->GetFormatSpecificParameter(), pAmrFmtp);
            pPayload->pFmtp = pAmrFmtp;
        }
        else if (strCodecName.EqualsIgnoreCase("telephone-event"))
        {
            // Create Telephone event fmtp
            AudioProfile::TelephoneEventFmtp* pTeFmtp = new AudioProfile::TelephoneEventFmtp();

            //[RFC4733] For backward compatibility, if no"events" parameter is received,
            // the sender SHOULD assume support for the DTMF events 0-15 but for no other events.
            if (pSDPCodec->GetFormatSpecificParameter() != IMS_NULL &&
                    pSDPCodec->GetFormatSpecificParameter().GetLength() > 0)
            {
                pTeFmtp->strEvents = pSDPCodec->GetFormatSpecificParameter();
            }
            else
            {
                pTeFmtp->strEvents ="0-15";   //default value
            }

            pPayload->pFmtp = pTeFmtp;
        }
        else if (strCodecName.EqualsIgnoreCase("EVS"))
        {
            // Create EVS fmtp
            AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp();
            // check and get EVS fmtp
            GetFmtpFromString(pSDPCodec->GetFormatSpecificParameter(), pEvsFmtp);
            // set Fmpt to payload
            pPayload->pFmtp = pEvsFmtp;
        }
        else if (nPayloadTypeNumber == 0 || nPayloadTypeNumber == 8)
        { // PCMU or PCMA case
            // do nothing.
            IMS_TRACE_D("MakeProfileFromSdp() - do nothing codec[%s]", strCodecName.GetStr(), 0, 0);
        }
        else
        {
            IMS_TRACE_E(0,"MakeProfileFromSdp() - NOT SUPPORTED audio codec[%s]",
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

    if (pProfile->bSupportSrtp)
    {
        IMSList<AString> strCrypto = pDescriptor->GetAttributes(SdpAttribute::CRYPTO);
        for (IMS_UINT32 nIndex = 0; nIndex < strCrypto.GetSize(); nIndex++)
        {
            if (MakeSrtpProfileFromCryptoAttr(pProfile,strCrypto.GetAt(nIndex)) == IMS_TRUE)
            {
                break;
            }
        }
    }

    //RTCP-XR
    IMSList<AString> lstRtcpXrAttr = pDescriptor->GetAttributes(SdpAttribute::RTCP_XR);

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
            pProfile->objRtcpXrAttr.bSupportVoipMatircs = IMS_TRUE;
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

    return IMS_TRUE;
}

IMS_BOOL AudioNego::MakeNegotiatedProfile(IN AudioProfile* pSrcProfile,
        IN AudioProfile* pDestProfile, IN IMS_BOOL bIsOfferReceived,
        OUT AudioProfile* pNegotiatedProfile)
{
    if (pSrcProfile == IMS_NULL || pDestProfile == IMS_NULL  || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0,"MakeNegotiatedProfile() invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    AudioConfiguration* pConfig = GetConfig();
    if (pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Step 1. Setting IP of mine
    pNegotiatedProfile->objIpAddr = pSrcProfile->objIpAddr;

    IMS_TRACE_D("MakeNegotiatedProfile() - IPAddr nego[%s] src[%s] DestPayloadSize[%d]",
            pNegotiatedProfile->objIpAddr.ToCharString(), pSrcProfile->objIpAddr.ToCharString(),
            pDestProfile->lstPayload.GetSize());

    // Step 2. Setting RTP/RTCP port of mine
    pNegotiatedProfile->nDataPort = pSrcProfile->nDataPort;
    pNegotiatedProfile->nControlPort = pSrcProfile->nControlPort;

    if (pNegotiatedProfile->nDataPort == 0 || pDestProfile->nDataPort == 0)
    {
        pNegotiatedProfile->Copy(pSrcProfile);
        pNegotiatedProfile->nDataPort = 0;
        IMS_TRACE_D("MakeNegotiatedProfile() ZERO Port. DO NOT Use the audio[%d][%d]",
                pNegotiatedProfile->nDataPort, pDestProfile->nDataPort, 0);
    }

    // Step 3. Setting profile type
    // m_objProfile.strTransportType ="RTP/AVP"";    // Audio uses a default.
    if (pSrcProfile->bSupportSrtp == IMS_TRUE && pDestProfile->bSupportSrtp == IMS_TRUE)
    {
        pNegotiatedProfile->bSupportSrtp = IMS_TRUE;
        pNegotiatedProfile->bSupportCapaNegoForSrtp = pDestProfile->bSupportCapaNegoForSrtp;
    }

    if (pNegotiatedProfile->bSupportCapaNegoForSrtp == IMS_TRUE)
    {
        if (MakeNegotiatedCapaNegoProfile(&(pSrcProfile->objCapaNego),
                &(pDestProfile->objCapaNego), &(pNegotiatedProfile->objCapaNego)) != IMS_TRUE)
        {
            pNegotiatedProfile->strTransportType ="RTP/AVP";
            pNegotiatedProfile->bSupportSrtp = IMS_FALSE;
            IMS_TRACE_D("MakeNegotiatedProfile() Fail, SRTP Disable", 0, 0, 0);
        }
        else
        {
            pNegotiatedProfile->bSupportSrtp = IMS_TRUE;
        }

        pNegotiatedProfile->objCapaNego.mapTransportCapa.Clear();
        pNegotiatedProfile->objCapaNego.mapAttributeCapa.Clear();
    }
    // Step 3-1  Set SRTP Profile
    if (pNegotiatedProfile->bSupportSrtp == IMS_TRUE)
    {
        pNegotiatedProfile->eSrtpCryptoType = pSrcProfile->eSrtpCryptoType;
        pNegotiatedProfile->nMasterKeyLifeTime = pSrcProfile->nMasterKeyLifeTime;
        IMS_MEM_Memcpy(pNegotiatedProfile->szKey,pSrcProfile->szKey,
                sizeof(pNegotiatedProfile->szKey));
        IMS_TRACE_D("MakeNegotiatedProfile() SRTP enable[%d], eSrtpProfile[%d]",
                pNegotiatedProfile->bSupportSrtp, pNegotiatedProfile->eSrtpCryptoType, 0);
    }

    pNegotiatedProfile->strTransportType ="RTP/AVP";

    if (pNegotiatedProfile->bSupportSrtp == IMS_TRUE &&
            pNegotiatedProfile->bSupportCapaNegoForSrtp == IMS_FALSE)
    {
        pNegotiatedProfile->strTransportType ="RTP/SAVP";
    }

    // Step 4. Compare each payload based destination's profile
    AudioProfile::Payload* pNegotiatedPayload = IMS_NULL;
    IMSList<AudioProfile::Payload*> lstNegotiatedPayloads;

    IMS_BOOL    bProperNegotiatedTe = IMS_FALSE;
    IMS_UINT32  nNegoModeSetList = 0;
    IMS_UINT32  nNegoDefaultRtpModeSet = 0;
    NegoListSet BandwidthNegoList;
    NegoListSet BitrateNegoList;
    NegoListSet ModeSetNegoList;

    // find negotiation aduioCodec, because of telephonyEvent negotiation
    IMSList<AudioProfile::Payload*> templstNegotiatedPayloads;

    for (IMS_UINT32 i=0; i<pDestProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* pPayload = pDestProfile->lstPayload.GetAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }
        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
                pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0 ||
                    pConfig->GetSdpAnswerFullcapa() == IMS_TRUE) &&
                    FindAmrInProfile(pSrcProfile, pPayload, bIsOfferReceived, &nNegoModeSetList,
                    &nNegoDefaultRtpModeSet) == IMS_TRUE)
            {
                AudioProfile::Payload* pAMR = new AudioProfile::Payload();
                pAMR->SetRtpMap(&pPayload->objRtpMap);
                AudioProfile::AmrFmtp* pAmrFmtp =
                        new AudioProfile::AmrFmtp((AudioProfile::AmrFmtp*)pPayload->pFmtp);
                pAMR->pFmtp = (void*)pAmrFmtp;
                templstNegotiatedPayloads.Append(pAMR);
            }
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0 ||
                    pConfig->GetSdpAnswerFullcapa() == IMS_TRUE) &&
                    FindEvsInProfile(pSrcProfile, pPayload, bIsOfferReceived, &BandwidthNegoList,
                    &BitrateNegoList, &ModeSetNegoList) == IMS_TRUE)
            {
                AudioProfile::Payload* pEVS = new AudioProfile::Payload();
                pEVS->SetRtpMap(&pPayload->objRtpMap);

                AudioProfile::EvsFmtp* pEvsFmtp =
                        new AudioProfile::EvsFmtp((AudioProfile::EvsFmtp*)pPayload->pFmtp);
                pEVS->pFmtp = (void*)pEvsFmtp;

                templstNegotiatedPayloads.Append(pEVS);
            }
        } else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU") ||
                pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMA"))
        {
            if ((templstNegotiatedPayloads.GetSize() == 0 ||
                    pConfig->GetSdpAnswerFullcapa() == IMS_TRUE) &&
                    FindPcmInProfile(pSrcProfile, pPayload) == IMS_TRUE)
            {
                AudioProfile::Payload* pPCM = new AudioProfile::Payload();
                pPCM->SetRtpMap(&pPayload->objRtpMap);

                templstNegotiatedPayloads.Append(pPCM);
            }
        }
    }

    IMS_TRACE_D("MakeNegotiatedProfile() - temp negotiated payload list[%d]",
            templstNegotiatedPayloads.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < pDestProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* pDestPayload = pDestProfile->lstPayload.GetAt(i);
        if (pDestPayload == IMS_NULL)
        {
            continue;
        }

        if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
                pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            if ((lstNegotiatedPayloads.GetSize() == 0 ||
                    pConfig->GetSdpAnswerFullcapa() == IMS_TRUE) &&
                    FindAmrInProfile(pSrcProfile, pDestPayload, bIsOfferReceived, &nNegoModeSetList,
                    &nNegoDefaultRtpModeSet) == IMS_TRUE)
            {
                AudioProfile::Payload* pAMR = new AudioProfile::Payload();
                pAMR->SetRtpMap(&pDestPayload->objRtpMap);

                IMS_SINT32 nSrcPayloadIndex =
                        FindPayloadIndexFromProfile(pDestPayload->objRtpMap.strPayloadType,
                        pSrcProfile, pDestPayload, bIsOfferReceived);
                AudioProfile::AmrFmtp* pSrc_Fmtp = (AudioProfile::AmrFmtp*)pSrcProfile->
                        lstPayload.GetAt(nSrcPayloadIndex)->pFmtp;
                AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp(
                        (AudioProfile::AmrFmtp*)pDestPayload->pFmtp);
                pAmrFmtp->nModeSetList = nNegoModeSetList;
                pAmrFmtp->nDefaultRtpModeSet = nNegoDefaultRtpModeSet;
                pAmrFmtp->bSCREnable = pSrc_Fmtp->bSCREnable;

                if (pConfig->GetModeChangeCapaAlwaysAnswer() > 0)
                {
                    pAmrFmtp->bShowModeChangeCapability = pSrc_Fmtp->bShowModeChangeCapability;
                    pAmrFmtp->nModeChangeCapability = pSrc_Fmtp->nModeChangeCapability;
                    pAmrFmtp->bShowModeChangeNeighbor = pSrc_Fmtp->bShowModeChangeNeighbor;
                    pAmrFmtp->nModeChangeNeighbor = pSrc_Fmtp->nModeChangeNeighbor;
                    pAmrFmtp->bShowModeChangePeriod = pSrc_Fmtp->bShowModeChangePeriod;
                    pAmrFmtp->nModeChangePeriod = pSrc_Fmtp->nModeChangePeriod;
                }

                pAMR->pFmtp = (void*)pAmrFmtp;
                pNegotiatedProfile->lstPayload.Append(pAMR);
                lstNegotiatedPayloads.Append(pAMR);

                if (pDestProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list.
                    pDestProfile->nNegotiatedPayloadIndex = i;
                    // set nego payload index at src profile
                    pSrcProfile->nNegotiatedPayloadIndex = nSrcPayloadIndex;
                    IMS_TRACE_D("MakeNegotiatedProfile() - nego payload index[%d]",
                            pSrcProfile->nNegotiatedPayloadIndex, 0, 0);

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE && pSrcProfile->nNegotiatedPayloadIndex != -1)
                    {
                        AudioProfile::Payload* pTempNegoSrcPayload =
                                pSrcProfile->lstPayload.GetAt(pSrcProfile->nNegotiatedPayloadIndex);
                        pTempNegoSrcPayload->objRtpMap.nPayloadNum =
                                pDestPayload->objRtpMap.nPayloadNum;
                    }
                }

                if (pNegotiatedProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile.
                    pNegotiatedProfile->nNegotiatedPayloadIndex =
                            pNegotiatedProfile->lstPayload.GetSize() -1;
                }
            }
        }
        else if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
        {
            BandwidthNegoList.Clear();
            BitrateNegoList.Clear();
            ModeSetNegoList.Clear();
            // need to modify FindEvsInProfile() func..
            if ((lstNegotiatedPayloads.GetSize() == 0 ||
                    pConfig->GetSdpAnswerFullcapa() == IMS_TRUE) &&
                    FindEvsInProfile(pSrcProfile, pDestPayload, bIsOfferReceived,
                    &BandwidthNegoList, &BitrateNegoList, &ModeSetNegoList) == IMS_TRUE)
            {
                AudioProfile::Payload* pEVS = new AudioProfile::Payload();
                pEVS->SetRtpMap(&pDestPayload->objRtpMap);
                IMS_SINT32 nSrcPayloadIndex =
                        FindPayloadIndexFromProfile(pDestPayload->objRtpMap.strPayloadType,
                        pSrcProfile, pDestPayload, bIsOfferReceived);
                AudioProfile::EvsFmtp* pSrc_Fmtp =
                        (AudioProfile::EvsFmtp*)pSrcProfile->lstPayload.GetAt(
                        nSrcPayloadIndex)->pFmtp;
                AudioProfile::EvsFmtp* pEvsFmtp =
                        new AudioProfile::EvsFmtp((AudioProfile::EvsFmtp*)pDestPayload->pFmtp);
                pEvsFmtp->nBwList = BandwidthNegoList.nNegoList;
                pEvsFmtp->nDefaultBandwidthList = BandwidthNegoList.nDefaultNegoList;
                pEvsFmtp->nBrList = BitrateNegoList.nNegoList;
                pEvsFmtp->nDefaultBitrateList = BitrateNegoList.nDefaultNegoList;
                pEvsFmtp->nModeSetList = ModeSetNegoList.nNegoList;
                pEvsFmtp->nDefaultRtpModeSet = ModeSetNegoList.nDefaultNegoList;

                if ( pEvsFmtp->nDtx != pSrc_Fmtp->nDtx)
                {
                    IMS_TRACE_D("MakeNegotiatedProfile() - DTX updated in the destination profile",
                            0, 0, 0);
                }

                if (pConfig->GetModeChangeCapaAlwaysAnswer() > 0)
                {
                    pEvsFmtp->bShowModeChangeCapability = pSrc_Fmtp->bShowModeChangeCapability;
                    pEvsFmtp->nModeChangeCapability = pSrc_Fmtp->nModeChangeCapability;
                    pEvsFmtp->bShowModeChangeNeighbor = pSrc_Fmtp->bShowModeChangeNeighbor;
                    pEvsFmtp->nModeChangeNeighbor = pSrc_Fmtp->nModeChangeNeighbor;
                    pEvsFmtp->bShowModeChangePeriod = pSrc_Fmtp->bShowModeChangePeriod;
                    pEvsFmtp->nModeChangePeriod = pSrc_Fmtp->nModeChangePeriod;
                }

                // check uni direction attribute
                if (pEvsFmtp->nBrSend != 0)
                {
                    pEvsFmtp->nBrSend = BitrateNegoList.nNegoList;
                }
                if (pEvsFmtp->nBrRecv != 0)
                {
                    pEvsFmtp->nBrRecv = BitrateNegoList.nNegoList;
                }
                if (pEvsFmtp->nBwSend != 0)
                {
                    pEvsFmtp->nBwSend = BandwidthNegoList.nNegoList;
                }
                if (pEvsFmtp->nBwRecv != 0)
                {
                    pEvsFmtp->nBwRecv = BandwidthNegoList.nNegoList;
                }

                pEvsFmtp->bSendCmr = pSrc_Fmtp->bSendCmr;

                //CMR on/off, if bitrate is not range set, disable CMR send option
                IMS_UINT32 nCount = 0;
                IMS_UINT32 nTempBrList = pEvsFmtp->nBrList;
                for (int i = 0; i < 16; i++)
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

                // fixed for IR92 ver.12 newaly spec as below comment.
                // If the selected EVS configuration is A1, B0 or B1 then"mode set = 0,1,2"
                // must be included in the SDP answer.
                if (bIsOfferReceived == IMS_TRUE && pEvsFmtp->nEvsModeSwitch != 1)
                {
                    // if max BR is 13.2kbps, then set a"mode-set" attribute
                    if (((pEvsFmtp->nBrList&0x10)!=0) && ((pEvsFmtp->nBrList&0xFFE0)==0))
                    {
                        pEvsFmtp->nModeSetList = 0x07; // mode-set = 0,1,2;
                        IMS_TRACE_D("MakeNegotiatedProfile() - add EVS mode-set", 0, 0, 0);
                    }
                }

                pEVS->pFmtp = (void*)pEvsFmtp;
                pNegotiatedProfile->lstPayload.Append(pEVS);
                lstNegotiatedPayloads.Append(pEVS);

                if (pDestProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list
                    pDestProfile->nNegotiatedPayloadIndex = i;
                    // set nego payload index at src profile
                    pSrcProfile->nNegotiatedPayloadIndex = nSrcPayloadIndex;

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                        pSrcProfile->nNegotiatedPayloadIndex != -1)
                    {
                        AudioProfile::Payload* pTempNegoSrcPayload =
                                pSrcProfile->lstPayload.GetAt(pSrcProfile->nNegotiatedPayloadIndex);
                        pTempNegoSrcPayload->objRtpMap.nPayloadNum =
                                pDestPayload->objRtpMap.nPayloadNum;
                    }
                }
                if (pNegotiatedProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile
                    pNegotiatedProfile->nNegotiatedPayloadIndex =
                            pNegotiatedProfile->lstPayload.GetSize() -1;
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

            for (IMS_UINT32 j = 0; j < templstNegotiatedPayloads.GetSize(); j ++)
            {
                pNegotiatedPayload = templstNegotiatedPayloads.GetAt(j);
                if (pNegotiatedPayload->objRtpMap.nSamplingRate ==
                        pDestPayload->objRtpMap.nSamplingRate)
                {
                    AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                    pTelephoneEvent->SetRtpMap(&pDestPayload->objRtpMap);
                    AudioProfile::TelephoneEventFmtp* pTelephoneFmtp =
                            new AudioProfile::TelephoneEventFmtp(
                            (AudioProfile::TelephoneEventFmtp*)pDestPayload->pFmtp);
                    pTelephoneEvent->pFmtp = (void*)pTelephoneFmtp;
                    pNegotiatedProfile->lstPayload.Append(pTelephoneEvent);
                    bProperNegotiatedTe = IMS_TRUE;
                    break;
                }
            }
        }
        else if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU") ||
                pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMA"))
        {
            if ((lstNegotiatedPayloads.GetSize() == 0 ||
                    pConfig->GetSdpAnswerFullcapa() == IMS_TRUE) &&
                    FindPcmInProfile(pSrcProfile, pDestPayload) == IMS_TRUE)
            {
                AudioProfile::Payload* pPCM = new AudioProfile::Payload();
                pPCM->SetRtpMap(&pDestPayload->objRtpMap);
                pNegotiatedProfile->lstPayload.Append(pPCM);
                lstNegotiatedPayloads.Append(pPCM);

                if (pDestProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list
                    pDestProfile->nNegotiatedPayloadIndex = i;
                    pSrcProfile->nNegotiatedPayloadIndex = FindPayloadIndexFromProfile(
                            pDestPayload->objRtpMap.strPayloadType, pSrcProfile, pDestPayload,
                            bIsOfferReceived);
                }

                if (pNegotiatedProfile->nNegotiatedPayloadIndex == -1)
                {
                    // Set the index of negotiated payload from the list at NegotiatedProfile
                    pNegotiatedProfile->nNegotiatedPayloadIndex =
                            pNegotiatedProfile->lstPayload.GetSize() -1;
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
            pDestProfile->nNegotiatedPayloadIndex,0, 0);

    // accept 8K DTMF when AMR-WB calling if ther are no proper DTMF payload
    if (bProperNegotiatedTe == IMS_FALSE && pNegotiatedProfile->lstPayload.GetSize() > 0 &&
            lstNegotiatedPayloads.GetSize() > 0)
    {
        for (IMS_UINT32 i=0; i<pDestProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* pDestPayload = pDestProfile->lstPayload.GetAt(i);
            if (pDestPayload == IMS_NULL)
            {
                continue;
            }
            if (pDestPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
            {
                // 20130425 - for acceptable 8K DTMF when AMR-WB calling
                pNegotiatedPayload = lstNegotiatedPayloads.GetAt(0);
                if (pNegotiatedPayload->objRtpMap.nSamplingRate >
                        pDestPayload->objRtpMap.nSamplingRate)
                {
                    IMS_TRACE_D("MakeNegotiatedProfile() - Accept sampling rate[%d]->[%d]",
                        pNegotiatedPayload->objRtpMap.nSamplingRate,
                        pDestPayload->objRtpMap.nSamplingRate, 0);
                    AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                    pTelephoneEvent->SetRtpMap(&pDestPayload->objRtpMap);
                    AudioProfile::TelephoneEventFmtp* pTelephoneFmtp =
                            new AudioProfile::TelephoneEventFmtp(
                            (AudioProfile::TelephoneEventFmtp*)pDestPayload->pFmtp);
                    pTelephoneEvent->pFmtp = (void*)pTelephoneFmtp;
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
        // Step 5. Setting bandwidth AS/RS/RR
        IMS_SINT32 nAsValueOfNegoticatedCodec = 0;
        AUDIO_CODEC nCurrCodec = AUDIO_CODEC_NONE;
        IMS_SINT32 nModeSet;

        // find largest AS value..
        if (pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
                pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pNegotiatedPayload->pFmtp;
            if (pNegotiatedPayload->objRtpMap.nSamplingRate == 8000)
            {
                nCurrCodec = AUDIO_CODEC_AMR;
                nModeSet = AudioProfileConfigurer::GetLargestModesetInFmtp(
                        "AMR", pNegotiatedPayload);
            }
            else
            {
                nCurrCodec = AUDIO_CODEC_AMRWB;
                nModeSet = AudioProfileConfigurer::GetLargestModesetInFmtp(
                        "AMR-WB", pNegotiatedPayload);
            }

            nAsValueOfNegoticatedCodec =
                    AudioProfileConfigurer::ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->nOctetAlign,
                    pNegotiatedProfile->objIpAddr.IsIPv6Address(), nModeSet);
        }
        else if (pNegotiatedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
        {
            AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pNegotiatedPayload->pFmtp;
            nCurrCodec = AUDIO_CODEC_EVS;
            nModeSet = AudioProfileConfigurer::GetLargestModesetInFmtp("EVS", pNegotiatedPayload);
            nAsValueOfNegoticatedCodec =
                    AudioProfileConfigurer::ConvertToBandwidthAS(nCurrCodec,
                    pNegotiatedProfile->objIpAddr.IsIPv6Address(), pEvsFmtp->nEvsModeSwitch,
                    nModeSet);
        }
        else
        {
            nCurrCodec = AUDIO_CODEC_NONE;
        }

        // Step 6. Setting direction
        pNegotiatedProfile->eDirection = UpdateDirectionToMine(pDestProfile->eDirection,
                pSrcProfile->eDirection, bIsOfferReceived, pConfig->GetSdpDirLooseCheck());

        if (pNegotiatedProfile->eDirection == MEDIA_DIRECTION_INVALID)
        {
            IMS_TRACE_E(0,"MakeNegotiatedProfile() - invalid direction.", 0, 0, 0);
            return IMS_FALSE;
        }

        // Step 6.1 Cehck Hold Direction
        pNegotiatedProfile->bIsHold = AudioProfileConfigurer::CheckHoldDirection(pConfig,
                pNegotiatedProfile->eDirection, pSrcProfile);

        // if the case using different interval in live and hold, set here.
        if (pNegotiatedProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE &&
                pConfig->GetRtcpLiveInterval() > 0)
        {
            pNegotiatedProfile->nRtcpInterval = pConfig->GetRtcpLiveInterval();
        }
        else
        {
            pNegotiatedProfile->nRtcpInterval = pConfig->GetRtcpInterval();
        }

        AudioProfileConfigurer::MakeNegotiatedBandwidth(pConfig, pSrcProfile, pDestProfile,
                bIsOfferReceived, nAsValueOfNegoticatedCodec, pNegotiatedProfile);

        // step 6.5 RTCP-XR
        if (pSrcProfile->bSupportRtcpXr == IMS_TRUE &&
                pNegotiatedProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE)
        {
            pNegotiatedProfile->bSupportRtcpXr = IMS_TRUE;
            pNegotiatedProfile->objRtcpXrAttr = pSrcProfile->objRtcpXrAttr;
        }

        IMS_TRACE_D("MakeNegotiatedProfile()-nRtcpInterval[%d], RTCP-XR support[%d]",
                pNegotiatedProfile->nRtcpInterval, pNegotiatedProfile->bSupportRtcpXr, 0);

        if (pConfig->GetRearrangeModeSetByAs() == IMS_TRUE)
        {
            RearrangeModeSetByAs(pNegotiatedPayload, pNegotiatedProfile->objIpAddr.IsIPv6Address(),
                    pNegotiatedProfile->nBandwidthAs);
        }
        else
        {
            IMS_TRACE_D("MakeNegotiatedProfile() - RearrangeModeSetByAs is off", 0, 0, 0);
        }

        // Step 7. Setting ptime & maxptime
        // [RFC3264] http://www.ietf.org/rfc/rfc3264.txt
        // The answerer MAY include a non-zero ptime attribute for any media stream;
        // this indicates the packetization interval that the answerer would like to receive.
        // There is no requirement that the packetization interval be the same in each direction
        // for a particular stream.

        if (pSrcProfile->nPtime < 20)
        {
            pNegotiatedProfile->nPtime = 20;
        }
        else
        {
            pNegotiatedProfile->nPtime = pSrcProfile->nPtime;
        }

        if (pSrcProfile->nMaxPtime < 20)
        {
            pNegotiatedProfile->nMaxPtime = 240;
        }
        else
        {
            pNegotiatedProfile->nMaxPtime = pSrcProfile->nMaxPtime;
        }

        // Step 8. Candidate Priority
        pNegotiatedProfile->nCandidatePriority = pSrcProfile->nCandidatePriority;
        return IMS_TRUE;
    }
    else
    {
        return IMS_FALSE;
    }
}

PROTECTED
IMS_BOOL AudioNego::GetFmtpFromString(IN AString strFmtp, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMSList<AString> objSplitColon = strFmtp.Split(';');

    for (IMS_UINT32 i = 0; i < objSplitColon.GetSize(); i++)
    {
        if (objSplitColon.GetAt(i).GetLength() == 0)
        {
            continue;
        }
        IMSList<AString> objSplitEqual = objSplitColon.GetAt(i).Split('=');

        if (objSplitEqual.GetSize() < 2)
        {
            const AString &strTmp = objSplitColon.GetAt(i);
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
            pFmtp->nDtx = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
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
            IMSList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            IMSList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBr = 0;
                IMS_UINT32 nLastBr = 0;
                for (IMS_UINT32 j = 0; j < EVS_BR_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(EVS_BR[j]) == IMS_TRUE)
                    {
                        nFirstBr = j;
                    }
                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(EVS_BR[j]) == IMS_TRUE)
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
            else    // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < EVS_BR_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(EVS_BR[k]) == IMS_TRUE)
                        {
                            pFmtp->nBrList = (pFmtp->nBrList | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("bw") == IMS_TRUE)
        {
            IMSList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            IMSList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBw = 0;
                IMS_UINT32 nLastBw = 0;

                for (IMS_UINT32 j = 0; j < EVS_BW_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(EVS_BW[j]) == IMS_TRUE)
                    {
                        nFirstBw = j;
                    }

                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(EVS_BW[j]) == IMS_TRUE)
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
            else    // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < EVS_BW_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(EVS_BW[k]) == IMS_TRUE)
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
            //pFmtp->nChAwRecv = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->nReceivedChAwRecv = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->nChAwRecv = pFmtp->nReceivedChAwRecv;
            pFmtp->bShowChannelAwMode = IMS_TRUE;
/*
            if ((pFmtp->nChAwRecv == 0) || (pFmtp->nChAwRecv == 2) || (pFmtp->nChAwRecv == 3)
                || (pFmtp->nChAwRecv == 5) || (pFmtp->nChAwRecv == 7) || (pFmtp->nChAwRecv == -1)) {
                pFmtp->bShowChannelAwMode = IMS_TRUE;
            } else {
                pFmtp->nChAwRecv = 0;
                pFmtp->bShowChannelAwMode = IMS_FALSE;
            }
*/
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-set") == IMS_TRUE)
        {
            IMSList<AString> objSplitComma= objSplitEqual.GetAt(1).Split(',');
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                IMS_UINT32 nModeSet = (IMS_UINT32)objSplitComma.GetAt(j).ToInt32();
                pFmtp->nModeSetList = (pFmtp->nModeSetList | (1 << nModeSet));
            }
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
            IMSList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            IMSList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBr = 0;
                IMS_UINT32 nLastBr = 0;
                for (IMS_UINT32 j = 0; j < EVS_BR_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(EVS_BR[j]) == IMS_TRUE)
                    {
                        nFirstBr = j;
                    }

                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(EVS_BR[j]) == IMS_TRUE)
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
            else    // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < EVS_BR_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(EVS_BR[k]) == IMS_TRUE)
                        {
                            pFmtp->nBrRecv = (pFmtp->nBrRecv | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("br-recv") == IMS_TRUE)
        {
            IMSList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            IMSList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBr = 0;
                IMS_UINT32 nLastBr = 0;

                for (IMS_UINT32 j = 0; j < EVS_BR_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(EVS_BR[j]) == IMS_TRUE)
                    {
                        nFirstBr = j;
                    }
                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(EVS_BR[j]) == IMS_TRUE)
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
            else    // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < EVS_BR_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(EVS_BR[k]) == IMS_TRUE)
                        {
                            pFmtp->nBrSend = (pFmtp->nBrSend | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("bw-send") == IMS_TRUE)
        {
            IMSList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            IMSList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBw = 0;
                IMS_UINT32 nLastBw = 0;

                for (IMS_UINT32 j = 0; j < EVS_BW_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(EVS_BW[j]) == IMS_TRUE)
                    {
                        nFirstBw = j;
                    }

                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(EVS_BW[j]) == IMS_TRUE)
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
            else    // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < EVS_BW_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(EVS_BW[k]) == IMS_TRUE)
                        {
                            pFmtp->nBwRecv = (pFmtp->nBwRecv | (1 << k));
                        }
                    }
                }
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("bw-recv") == IMS_TRUE)
        {
            IMSList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            IMSList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
            if (objSplitHyphen.GetSize() == 2)
            {
                IMS_UINT32 nFirstBw = 0;
                IMS_UINT32 nLastBw = 0;

                for (IMS_UINT32 j = 0; j < EVS_BW_CNT; j++)
                {
                    if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(EVS_BW[j]) == IMS_TRUE)
                    {
                        nFirstBw = j;
                    }

                    if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(EVS_BW[j]) == IMS_TRUE)
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
            else    // comma case
            {
                for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
                {
                    for (IMS_UINT32 k = 0; k < EVS_BW_CNT; k++)
                    {
                        if (objSplitComma.GetAt(j).EqualsIgnoreCase(EVS_BW[k]) == IMS_TRUE)
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
    IMS_TRACE_D("GetFmtpFromString() - EVS : nMaxRed[%d], nReceivedChAwRecv[%d]",
            pFmtp->nMaxRed, pFmtp->nReceivedChAwRecv, 0);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AudioNego::FindEvsInProfile(IN AudioProfile* pSrcProfile,
        IN AudioProfile::Payload* pDstPayload, IN IMS_BOOL bIsOfferReceived,
        OUT NegoListSet* pBandwidthNegoList, OUT NegoListSet* pBitrateNegoList,
        OUT NegoListSet* pModeSetNegoList)
{
    if (pSrcProfile == IMS_NULL || pDstPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 legacyCheck=0; legacyCheck<EVS_NEGO_RETRY_COUNT; legacyCheck++)
    {
        for (IMS_UINT32 i=0; i<pSrcProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* comparedPayload = pSrcProfile->lstPayload.GetAt(i);
            if (comparedPayload == IMS_NULL)
            {
                continue;
            }

            if (comparedPayload->objRtpMap.strPayloadType.Equals("EVS"))
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
                            pBandwidthNegoList, pBitrateNegoList, pModeSetNegoList) == IMS_FALSE)
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
                        ((pBandwidthNegoList->nNegoList & 0x06) != 0 &&
                        (pBitrateNegoList->nNegoList & 0x10) != 0))
                {
                    if (bIsOfferReceived != IMS_TRUE)
                    {
                        pReceivedFmtp->nChAwRecv = pCompareFmtp->nChAwRecv;
                        pReceivedFmtp->bShowChannelAwMode = pCompareFmtp->bShowChannelAwMode;
                    }
                }

                IMS_TRACE_D("FindEvsInProfile() Found EVS at[%d], pBandwidthNegoList[0x%04x], \
                        pBitrateNegoList[0x%04x]",
                        i, pBandwidthNegoList->nNegoList, pBitrateNegoList->nNegoList);
                IMS_TRACE_D("FindEvsInProfile() EVS ModeSwitch[%d], pModeSetNegoList[0x%04x], \
                        nSendCmr[%d]", pReceivedFmtp->nEvsModeSwitch, pModeSetNegoList->nNegoList,
                        pCompareFmtp->bSendCmr);
                IMS_TRACE_D("FindEvsInProfile() EVS ChAwMode[%d], opposite ChAwMode[0x%04x], \
                        legacyCheck[%d]", pReceivedFmtp->nChAwRecv,
                        pReceivedFmtp->nReceivedChAwRecv, legacyCheck);

                return IMS_TRUE;
            }
        }
    }
    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioNego::GetFmtpFromString(IN AString strFmtp, OUT AudioProfile::AmrFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMSList<AString> objSplitColon = strFmtp.Split(';');

    for (IMS_UINT32 i = 0; i < objSplitColon.GetSize(); i++)
    {
        if (objSplitColon.GetAt(i).GetLength() == 0)
        {
            continue;
        }

        //IMS_TRACE_I("GetFmtpFromString() - objSplitColon[%d] =[%s]",
        //      i, objSplitColon.GetAt(i).GetStr(), 0);
        IMSList<AString> objSplitEqual = objSplitColon.GetAt(i).Split('=');

        if (objSplitEqual.GetSize() < 2)
        {
            const AString &strTmp = objSplitColon.GetAt(i);
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
            IMSList<AString> objSplitComma= objSplitEqual.GetAt(1).Split(',');

            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                IMS_UINT32 nModeSet = (IMS_UINT32)objSplitComma.GetAt(j).ToInt32();
                pFmtp->nModeSetList = (pFmtp->nModeSetList | (1 << nModeSet));
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("octet-align") == IMS_TRUE)
        {
            pFmtp->nOctetAlign = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShow_OctetAlign = IMS_TRUE;
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
        else if (objSplitEqual.GetAt(0).Equals("robust-sorting") == IMS_TRUE)
        {
            pFmtp->nRobustSorting = (IMS_SINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShow_RobustSorting = IMS_TRUE;
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

PROTECTED
IMS_BOOL AudioNego::FindAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
        IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pnNegoModeSetList,
        OUT IMS_UINT32* pnNegoDefaultRtpModeSet)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }
    IMS_UINT32 nTempNegoModeSetList = 0;
    IMS_UINT32 nTempDefaultModeSetList = 0;
    IMS_BOOL bModeSetFound = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* comparedPayload = pProfile->lstPayload.GetAt(i);
        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->objRtpMap.strPayloadType.Equals("AMR") ||
                comparedPayload->objRtpMap.strPayloadType.Equals("AMR-WB"))
        {
            AudioProfile::AmrFmtp* pCompareFmtp = (AudioProfile::AmrFmtp*)comparedPayload->pFmtp;
            AudioProfile::AmrFmtp* pReceivedFmtp = (AudioProfile::AmrFmtp*)pPayload->pFmtp;

            if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
            {
                continue;
            }
            if (comparedPayload->objRtpMap.strPayloadType.Equals(
                    pPayload->objRtpMap.strPayloadType) == IMS_FALSE)
            {
                continue;
            }
            if (comparedPayload->objRtpMap.nSamplingRate != pPayload->objRtpMap.nSamplingRate)
            {
                continue;
            }

            // TODO - 20220415 - Need to implement this requirement later
            /*if (IMS_OPERATOR(SKT, GetSlotId()) && bIsOfferReceived == IMS_TRUE)
            {
                IMS_TRACE_D("FindAmrInProfile - skt offer received - ignore AMR octet align case",
                        0, 0, 0);
            }
            else
            {
                if (pCompareFmtp->nOctetAlign != pReceivedFmtp->nOctetAlign )
                {
                    continue;
                }
            }*/
            if (pCompareFmtp->nOctetAlign != pReceivedFmtp->nOctetAlign )
            {
                continue;
            }

            // 20160517 Fix for AMR Open Offer
            // In case of MO, mode-set from MT could be mismatched
            // => Keep negotiated mode-set and try to find exact one

            IMS_SINT32 nCompareResult = CompareModeSet(pCompareFmtp, pReceivedFmtp,
                    bIsOfferReceived, pnNegoModeSetList, pnNegoDefaultRtpModeSet);
            if (nCompareResult == -1)
            {
                continue;   // mismatched
            }
            else if (nCompareResult == 0)          // similarly matched
            {
                if (bModeSetFound == IMS_FALSE)
                {
                    nTempNegoModeSetList = *pnNegoModeSetList;
                    nTempDefaultModeSetList = *pnNegoDefaultRtpModeSet;
                    bModeSetFound = IMS_TRUE;
                    IMS_TRACE_I("FindAmrInProfile() Local/Peer is not exactly matched\
                            [0x%04x][0x%04x] =>[0x%04x]. Try next", pCompareFmtp->nModeSetList,
                            pReceivedFmtp->nModeSetList, *pnNegoModeSetList);
                }
                continue;
            }
            else                                    // exactly matched
            {
                IMS_TRACE_D("FindAmrInProfile() Found AMR at[%d], Codec[%s], nOctetAlign[%d]",
                        i, comparedPayload->objRtpMap.strPayloadType.GetStr(),
                        pCompareFmtp->nOctetAlign);
                IMS_TRACE_D("FindAmrInProfile() Local/Peer is exactly matched[0x%04x][0x%04x] \
                        =>[0x%04x]", pCompareFmtp->nModeSetList, pReceivedFmtp->nModeSetList,
                        *pnNegoModeSetList);

                return IMS_TRUE;
            }
        }
    }

    // 20160517 Fix for AMR Open Offer
    if (bModeSetFound == IMS_TRUE)
    {
        *pnNegoModeSetList = nTempNegoModeSetList;
        *pnNegoDefaultRtpModeSet = nTempDefaultModeSetList;

        IMS_TRACE_D("FindAmrInProfile() Found Similar AMR with \
                nModeSetList[0x%04x], nDefaultModeSetList[0x%04x]",
                *pnNegoModeSetList, *pnNegoDefaultRtpModeSet, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
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

        if (comparedPayload->objRtpMap.strPayloadType.Equals("PCMU") ||
                comparedPayload->objRtpMap.strPayloadType.Equals("PCMA"))
        {
            if (comparedPayload->objRtpMap.strPayloadType.Equals(
                    pPayload->objRtpMap.strPayloadType) == IMS_FALSE)
            {
                continue;
            }
            if (comparedPayload->objRtpMap.nSamplingRate != pPayload->objRtpMap.nSamplingRate)
            {
                continue;
            }

            IMS_TRACE_D("FindPcmInProfile() Found G.711 at[%d], Codec[%s], nSamplingRate[%d]",
                    i, comparedPayload->objRtpMap.strPayloadType.GetStr(),
                    pPayload->objRtpMap.nSamplingRate);

            return IMS_TRUE;
        }
    }

    //IMS_TRACE_D("FindPcmInProfile() Not Found G.711[%s], nSamplingRate[%d]",
        //pPayload->objRtpMap.strPayloadType.GetStr(), pPayload->objRtpMap.nSamplingRate, 0);

    return IMS_FALSE;
}

PROTECTED
IMS_SINT32 AudioNego::CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
        IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
        OUT IMS_UINT32* nNegoModeSet, OUT IMS_UINT32* nNegoDefaultRtpModeSet)
{
    IMS_TRACE_I("CompareModeSet() - Src modeSet[0x%04x] Dest modeSet[0x%04x], bIsOfferReceived[%d]",
            pSrcFmtp->nModeSetList, pDestFmtp->nModeSetList, bIsOfferReceived);

    IMS_SINT32 nResult = 1;    // -1 : no matched, 0 : similar, 1 : exactly matched

    AudioConfiguration* pConfig = GetConfig();
    if (pConfig == IMS_NULL)
    {
        return -1;
    }

    if (bIsOfferReceived == IMS_TRUE)   // MT Case
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
            *nNegoModeSet = 0; // MO, MT both has no mode-set
            *nNegoDefaultRtpModeSet = pSrcFmtp->nDefaultRtpModeSet;
        }
    }
    else                                // MO Case
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
                IMS_TRACE_E(0,"CompareModeSet() - ModeSet Not Matched...", 0, 0, 0);
                return -1;
            }
        }
        else    // one of two has no modeset
        {
            *nNegoModeSet = pSrcFmtp->nModeSetList | pDestFmtp->nModeSetList;
        }
    }

    if (pConfig->GetDmOperationPreferredMode() == IMS_TRUE)
    {
        *nNegoDefaultRtpModeSet = pSrcFmtp->nDefaultRtpModeSet;
        IMS_TRACE_D("CompareModeSet() DM Operation Preferred Mode, Use srcFmtp's default mode set",
                0, 0, 0);
    }

    IMS_TRACE_I("CompareModeSet() - Result[%d] : Negotiated modeSet[0x%04x], \
        nNegoDefaultRtpModeSet[0x%04x]", nResult, *nNegoModeSet, *nNegoDefaultRtpModeSet);

    return nResult;
}

PROTECTED
IMS_BOOL AudioNego::CompareEvsBwBrMode(IN AudioProfile::EvsFmtp* pSrcFmtp,
        IN AudioProfile::EvsFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
        OUT NegoListSet* nNegoBwList, OUT NegoListSet* nNegoBrList, OUT NegoListSet* nNegoModeList)
{
    IMS_TRACE_D("CompareEvsBwBrMode() - Src Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pSrcFmtp->nBwList, pSrcFmtp->nBrList, pSrcFmtp->nModeSetList);
    IMS_TRACE_D("CompareEvsBwBrMode() - Dest Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pDestFmtp->nBwList, pDestFmtp->nBrList, pDestFmtp->nModeSetList);

    AudioConfiguration* pConfig = GetConfig();
    if (pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // check EVS ModeSwitch
    if (pDestFmtp->nEvsModeSwitch == 1)   // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pSrcFmtp->nModeSetList == 0) && (pDestFmtp->nModeSetList == 0))
        {
            nNegoModeList->nNegoList = 0;
            nNegoModeList->nDefaultNegoList = pSrcFmtp->nDefaultRtpModeSet;
        }
        else if ((pSrcFmtp->nModeSetList != 0) && (pDestFmtp->nModeSetList != 0))
        {
            nNegoModeList->nNegoList = pSrcFmtp->nModeSetList & pDestFmtp->nModeSetList;

            if (nNegoModeList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrMode() - AMR IO Mode - ModeSet Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            nNegoModeList->nNegoList = pSrcFmtp->nModeSetList | pDestFmtp->nModeSetList;
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        nNegoBwList->nNegoList = pDestFmtp->nBwList;
        nNegoBrList->nNegoList = pDestFmtp->nBrList;

        if (pConfig->GetDmOperationPreferredMode() == IMS_TRUE)
        {
            nNegoModeList->nDefaultNegoList = pSrcFmtp->nDefaultRtpModeSet;
            IMS_TRACE_D("CompareEvsBwBrMode() DM Operation Preferred Mode, \
                    Use srcFmtp's default mode set", 0, 0, 0);
        }
    }
    else    // Primary Mode
    {
        // Set Bandwidth/Bitrate list.
        // 01. check Bandwidth
        if ((pSrcFmtp->nBwList == 0) && (pDestFmtp->nBwList == 0))
        {
            nNegoBwList->nNegoList = 0;
            nNegoBwList->nDefaultNegoList = pSrcFmtp->nDefaultBandwidthList;
        }
        else if ((pSrcFmtp->nBwList != 0) && (pDestFmtp->nBwList != 0))
        {
            // IR92 release15 EVS Answer Case.
            if (bIsOfferReceived == IMS_TRUE)
            {
                // check received EVS SWB only case (category B0, B1, B2 case)
                if (pDestFmtp->nBwList == 0x04)
                {
                    if (pDestFmtp->nBrList == 0x10) // B0 received case.
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B0 Type Nego",
                                0, 0, 0);
                        nNegoBwList->nNegoList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                    }
                    else if (pSrcFmtp->nBwList != 0x04)
                    { // own EVS category is A. Do not negotiate with received category B.
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Not Config B Type Nego",
                                0, 0, 0);
                        return IMS_FALSE;
                    }
                    else
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B Type Nego",
                                0, 0, 0);
                        nNegoBwList->nNegoList = pDestFmtp->nBwList;
                    }
                }
                else    //received EVS category A case
                {
                    // TODO - 20220415 - Need to implement this requirement later
                    /*// except for VZW operator. (only supported category B0 case.)
                    if (IMS_OPERATOR(VZW, GetSlotId()) == IMS_TRUE)
                    {
                        nNegoBwList->nNegoList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
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
                            nNegoBwList->nNegoList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                        }
                    }*/
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
                        nNegoBwList->nNegoList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                    }
                }
            }
            else
            {
                // TODO - 20220415 - Need to implement this requirement later
                /*// IR92 release15 EVS Answer Received Case.
                if (IMS_OPERATOR(VZW, GetSlotId()) == IMS_TRUE)
                {
                    nNegoBwList->nNegoList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                }
                else
                {
                    if (pSrcFmtp->nBwList == 0x04 && pDestFmtp->nBwList != 0x04)
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - check next payload", 0, 0, 0);
                        return IMS_FALSE;
                    }
                    nNegoBwList->nNegoList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
                }*/
                if (pSrcFmtp->nBwList == 0x04 && pDestFmtp->nBwList != 0x04)
                {
                    IMS_TRACE_D("CompareEvsBwBrMode() - check next payload", 0, 0, 0);
                    return IMS_FALSE;
                }
                nNegoBwList->nNegoList = pSrcFmtp->nBwList & pDestFmtp->nBwList;
            }

            if (nNegoBwList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Bandwidth Not Matched...",
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

            nNegoBwList->nNegoList = pSrcFmtp->nBwList & nPeerBWList;
            if (nNegoBwList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            nNegoBwList->nNegoList = pSrcFmtp->nBwList | pDestFmtp->nBwList;
        }

        // 02. check Bitrate
        nNegoBwList->nDefaultNegoList = pSrcFmtp->nDefaultBandwidthList;
        nNegoBrList->nDefaultNegoList = pSrcFmtp->nDefaultBitrateList;

        if ((pSrcFmtp->nBrList == 0) && (pDestFmtp->nBrList == 0))
        {
            nNegoBrList->nNegoList = 0;
            nNegoBrList->nDefaultNegoList = pSrcFmtp->nDefaultBitrateList;
        }
        else if ((pSrcFmtp->nBrList != 0) && (pDestFmtp->nBrList != 0))
        {
            // IR92 release15 EVS Answer Case.
            if (bIsOfferReceived == IMS_TRUE)
            {
                nNegoBrList->nNegoList = pSrcFmtp->nBrList & pDestFmtp->nBrList;
                if ((nNegoBwList->nNegoList == 0x04) && (nNegoBrList->nNegoList == 0x10))
                { // 13.2kbsp swb only case
                    IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B0,B1 Type Nego",
                            0, 0, 0);
                    nNegoBrList->nNegoList = (pDestFmtp->nBrList)&0x1f; // negotiated ~13.2kbps
                }
            }
            else
            {
                nNegoBrList->nNegoList = pSrcFmtp->nBrList & pDestFmtp->nBrList;
            }

            if (nNegoBrList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Bitrate Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->nBrList != 0) &&
                (pDestFmtp->nBrRecv != 0 ||(pDestFmtp->nBrSend != 0)))
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

            nNegoBrList->nNegoList = pSrcFmtp->nBrList & nPeerBRList;

            if (nNegoBrList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Bitrate Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }

        }
        else
        {
            nNegoBrList->nNegoList = pSrcFmtp->nBrList | pDestFmtp->nBrList;
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pDestFmtp->nModeSetList != 0)
        {
            nNegoModeList->nNegoList = pDestFmtp->nModeSetList;
        }

        if (pConfig->GetDmOperationPreferredMode() == IMS_TRUE)
        {
            nNegoBwList->nDefaultNegoList = pSrcFmtp->nDefaultBandwidthList;
            nNegoBrList->nDefaultNegoList = pSrcFmtp->nDefaultBitrateList;
            IMS_TRACE_D("CompareEvsBwBrMode() DM Operation Preferred Mode, \
                    Use srcFmtp's default bandwidth and default bitrate", 0, 0, 0);
        }
    }
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AudioNego::CompareEvsBwBrModeLegacy(IN AudioProfile::EvsFmtp* pSrcFmtp,
        IN AudioProfile::EvsFmtp* pDestFmtp, OUT NegoListSet* nNegoBwList,
        OUT NegoListSet* nNegoBrList, OUT NegoListSet* nNegoModeList)
{
    IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Src BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pSrcFmtp->nBwList, pSrcFmtp->nBrList, pSrcFmtp->nModeSetList);
    IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Dest BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pDestFmtp->nBwList, pDestFmtp->nBrList, pDestFmtp->nModeSetList);

    AudioConfiguration* pConfig = GetConfig();
    if (pConfig == IMS_NULL) \
    {
        return IMS_FALSE;
    }

    // check EVS ModeSwitch
    if (pDestFmtp->nEvsModeSwitch == 1)   // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pSrcFmtp->nModeSetList == 0) && (pDestFmtp->nModeSetList == 0))
        {
            nNegoModeList->nNegoList = 0;
            nNegoModeList->nDefaultNegoList = pSrcFmtp->nDefaultRtpModeSet;
        }
        else if ((pSrcFmtp->nModeSetList != 0) && (pDestFmtp->nModeSetList != 0))
        {
            nNegoModeList->nNegoList = pSrcFmtp->nModeSetList & pDestFmtp->nModeSetList;

            if (nNegoModeList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - AMR IO Mode - ModeSet Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            nNegoModeList->nNegoList = pSrcFmtp->nModeSetList | pDestFmtp->nModeSetList;
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        {
            nNegoBwList->nNegoList = pDestFmtp->nBwList;
            nNegoBrList->nNegoList = pDestFmtp->nBrList;
        }

        if (pConfig->GetDmOperationPreferredMode() == IMS_TRUE)
        {
            nNegoModeList->nDefaultNegoList = pSrcFmtp->nDefaultRtpModeSet;
            IMS_TRACE_D("CompareEvsBwBrModeLegacy() DM Operation Preferred Mode, \
                    Use srcFmtp's default mode set", 0, 0, 0);
        }
    }
    else    // Primary Mode
    {
        // Set Bandwidth/Bitrate list.
        // 01. check Bandwidth
        if ((pSrcFmtp->nBwList == 0) && (pDestFmtp->nBwList == 0))
        {
            nNegoBwList->nNegoList = 0;
            nNegoBwList->nDefaultNegoList = pSrcFmtp->nDefaultBandwidthList;
        }
        else if ((pSrcFmtp->nBwList != 0) && (pDestFmtp->nBwList != 0))
        {
            nNegoBwList->nNegoList = pSrcFmtp->nBwList & pDestFmtp->nBwList;

            if (nNegoBwList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->nBwList != 0) &&
                ((pDestFmtp->nBwRecv != 0) ||(pDestFmtp->nBwSend != 0)))
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

            nNegoBwList->nNegoList = pSrcFmtp->nBwList & nPeerBWList;
            if (nNegoBwList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            nNegoBwList->nNegoList = pSrcFmtp->nBwList | pDestFmtp->nBwList;
        }

        // 02. check Bitrate
        nNegoBwList->nDefaultNegoList = pSrcFmtp->nDefaultBandwidthList;
        nNegoBrList->nDefaultNegoList = pSrcFmtp->nDefaultBitrateList;

        if ((pSrcFmtp->nBrList == 0) && (pDestFmtp->nBrList == 0))
        {
            nNegoBrList->nNegoList = 0;
            nNegoBrList->nDefaultNegoList = pSrcFmtp->nDefaultBitrateList;
        }
        else if ((pSrcFmtp->nBrList != 0) && (pDestFmtp->nBrList != 0))
        {
            nNegoBrList->nNegoList = pSrcFmtp->nBrList & pDestFmtp->nBrList;

            if (nNegoBrList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bitrate Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->nBrList != 0) &&
                (pDestFmtp->nBrRecv != 0 ||(pDestFmtp->nBrSend != 0)))
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

            nNegoBrList->nNegoList = pSrcFmtp->nBrList & nPeerBRList;

            if (nNegoBrList->nNegoList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bitrate Not Matched..."
                        , 0, 0, 0);
                return IMS_FALSE;
            }

        }
        else
        {
            nNegoBrList->nNegoList = pSrcFmtp->nBrList | pDestFmtp->nBrList;
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pDestFmtp->nModeSetList != 0)
        {
            nNegoModeList->nNegoList = pDestFmtp->nModeSetList;
        }

        if (pConfig->GetDmOperationPreferredMode() == IMS_TRUE)
        {
            nNegoBwList->nDefaultNegoList = pSrcFmtp->nDefaultBandwidthList;
            nNegoBrList->nDefaultNegoList = pSrcFmtp->nDefaultBitrateList;
            IMS_TRACE_D("CompareEvsBwBrModeLegacy() DM Operation Preferred Mode, \
                    Use srcFmtp's default bandwidth and default bitrate", 0, 0, 0);
        }
    }
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AudioNego::FindTelephoneEventInProfile(IN AudioProfile* pProfile,
        IN AudioProfile::Payload* pPayload)
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

        if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
        {
            if ((comparedPayload->objRtpMap.strPayloadType.Equals(
                    pPayload->objRtpMap.strPayloadType)) &&
                    (comparedPayload->objRtpMap.nSamplingRate == pPayload->objRtpMap.nSamplingRate))
            {
                IMS_TRACE_D("FindTelephoneEventInProfile() Found Telephone-event at[%d], \
                        PayloadNum[%d], SamplingRate[%d]", i, comparedPayload->objRtpMap.nPayloadNum
                        , comparedPayload->objRtpMap.nSamplingRate);

                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED IMS_SINT32 AudioNego::FindPayloadIndexFromProfile(IN AString strCodecName,
        IN AudioProfile* pSrcProfile, IN AudioProfile::Payload* pDstPayload,
        IN IMS_BOOL bIsOfferReceived)
{
    if (pSrcProfile == IMS_NULL || pDstPayload == IMS_NULL)
    {
        return -1;
    }

    IMS_SINT32 nTempIndex = -1;

    for (IMS_UINT32 legacyCheck=0; legacyCheck<EVS_NEGO_RETRY_COUNT; legacyCheck++)
    {
        for (IMS_UINT32 i=0; i<pSrcProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* comparedPayload = pSrcProfile->lstPayload.GetAt(i);
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
                if (comparedPayload->objRtpMap.strPayloadType.Equals("AMR") ||
                        comparedPayload->objRtpMap.strPayloadType.Equals("AMR-WB"))
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

                    if (comparedPayload->objRtpMap.strPayloadType.Equals(
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
                    if (pCompareFmtp->nOctetAlign != pReceivedFmtp->nOctetAlign )
                    {
                        continue;
                    }

                    // 20160517 Fix for AMR Open Offer
                    // In case of MO, mode-set from MT could be mismatched
                    // => Keep negotiated mode-set and try to find exact one

                    IMS_SINT32 nCompareResult = CompareModeSet(pCompareFmtp, pReceivedFmtp,
                            bIsOfferReceived, &pnNegoModeSetList, &pnNegoDefaultRtpModeSet);
                    if (nCompareResult == -1)
                    {
                        continue;   // mismatched
                    }
                    else if (nCompareResult == 0)         // similarly matched
                    {
                        if (nTempIndex == -1)
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
                    else                                    // exactly matched
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
                if (comparedPayload->objRtpMap.strPayloadType.Equals("EVS"))
                {
                    NegoListSet BandwidthNegoList;
                    NegoListSet BitrateNegoList;
                    NegoListSet ModeSetNegoList;
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
                                &BandwidthNegoList, &BitrateNegoList, &ModeSetNegoList) ==
                                IMS_FALSE)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        // legacy EVS BR/BW check
                        if (CompareEvsBwBrModeLegacy(pCompareFmtp, pReceivedFmtp,
                                &BandwidthNegoList, &BitrateNegoList, &ModeSetNegoList) ==
                                IMS_FALSE)
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
             else if (strCodecName.EqualsIgnoreCase("PCMU") ||
                    strCodecName.EqualsIgnoreCase("PCMA"))
             {
                 if (legacyCheck >= 1)
                 {
                     continue;
                 }
                 if (comparedPayload->objRtpMap.strPayloadType.Equals(
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

PROTECTED
void AudioNego::RearrangeModeSetByAs(OUT AudioProfile::Payload* pPayload,
        IN IMS_BOOL bIpV6, IN IMS_SINT32 nAs)
{
    if (pPayload == IMS_NULL || nAs <= 0)
    {
        return;
    }

    AUDIO_CODEC eCodec = AUDIO_CODEC_NONE;
    if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
    {
        eCodec = AUDIO_CODEC_EVS;
    }
    else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
    {
        eCodec = AUDIO_CODEC_AMRWB;
    }
    else     // AMR
    {
        eCodec = AUDIO_CODEC_AMR;
    }

    IMS_SINT32 nLargestModeSet = AudioProfileConfigurer::GetLargestModesetInFmtp(
            pPayload->objRtpMap.strPayloadType, pPayload);
    IMS_SINT32 nAsFromModeset = 0;

    if (eCodec == AUDIO_CODEC_EVS)
    {
        // Does not need Rearrange Mode set at EVS Codec.
    }
    else
    {   // AMR / AMR-WB case
        AudioProfile::AmrFmtp* pNegotiatedFmtp = (AudioProfile::AmrFmtp*)pPayload->pFmtp;
        if (pNegotiatedFmtp == IMS_NULL)
        {
            return;
        }

        nAsFromModeset = AudioProfileConfigurer::ConvertToBandwidthAS(eCodec,
                pNegotiatedFmtp->nOctetAlign, bIpV6, nLargestModeSet);

        if (nAs < nAsFromModeset)
        {
            IMS_TRACE_D("RearrangeModeSetByAs() - nAs[%d], bIpV6[%d], nAsFromModeset[%d]",
                    nAs, bIpV6, nAsFromModeset);

            // change mode-set according to AS
            IMS_SINT32 nPossibleModeSet = AudioProfileConfigurer::ConvertToModeSet(eCodec,
                    pNegotiatedFmtp->nOctetAlign, bIpV6, nAs);

            IMS_UINT32 nPossibleModeSetList = 0;
            IMS_UINT32 nNegotiatedModeSetList = 0;

            for (IMS_SINT32 i = 0; i <= nPossibleModeSet; i ++)
            {
                nPossibleModeSetList = nPossibleModeSetList | (1 << i);
            }

            if (pNegotiatedFmtp->nModeSetList == 0)
            {
                pNegotiatedFmtp->nModeSetList = nPossibleModeSetList;
            }
            else
            {
                nNegotiatedModeSetList = pNegotiatedFmtp->nModeSetList & nPossibleModeSetList;
                if (nNegotiatedModeSetList != 0)
                {
                    pNegotiatedFmtp->nModeSetList = nNegotiatedModeSetList;
                }
            }
        }
    }
}

PROTECTED
MEDIA_DIRECTION AudioNego::UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDir,
        IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase, IN IMS_SINT32 nDirLooseCheck)
{
    if (ePeerDir < MEDIA_DIRECTION_INACTIVE || ePeerDir > MEDIA_DIRECTION_SEND_RECEIVE)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_D("UpdateDirectionToMine() ePeerDir[%d], eSrcDir[%d], bIsMtCase[%d]",
            ePeerDir, eSrcDir, bIsMtCase);

    MEDIA_DIRECTION eNegotiatedDir = MEDIA_DIRECTION_INVALID;

    switch (ePeerDir)
    {
        case MEDIA_DIRECTION_INACTIVE :
        case MEDIA_DIRECTION_SEND_RECEIVE :
            eNegotiatedDir = ePeerDir;
            break;
        case MEDIA_DIRECTION_RECEIVE :
            eNegotiatedDir = MEDIA_DIRECTION_SEND;
            break;
        case MEDIA_DIRECTION_SEND :
            eNegotiatedDir = MEDIA_DIRECTION_RECEIVE;
            break;
        default :
            return MEDIA_DIRECTION_INVALID;
    }

    if (bIsMtCase == IMS_FALSE)
    {
        IMS_TRACE_D("UpdateDirectionToMine() - bIsDirLooseCheck[%d]",
                nDirLooseCheck, 0, 0);

        if (nDirLooseCheck != 1)    /* 1: loosely dir check, other case: strict dir check */
        {
            // direction check strictly
            if (eSrcDir == MEDIA_DIRECTION_SEND &&
                    (ePeerDir == MEDIA_DIRECTION_SEND || ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE))
            {
                return MEDIA_DIRECTION_INVALID;
            }
            else if (eSrcDir == MEDIA_DIRECTION_RECEIVE &&
                    (ePeerDir == MEDIA_DIRECTION_RECEIVE ||
                    ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE))
            {
                return MEDIA_DIRECTION_INVALID;
            }
            else if (eSrcDir == MEDIA_DIRECTION_INACTIVE && (ePeerDir != MEDIA_DIRECTION_INACTIVE))
            {
                return MEDIA_DIRECTION_INVALID;
            }
        }
        else
        {
            // direction check loosely
            if (eSrcDir == MEDIA_DIRECTION_SEND &&
                    (ePeerDir == MEDIA_DIRECTION_SEND || ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE))
            {
                return eSrcDir;
            }
            else if (eSrcDir == MEDIA_DIRECTION_RECEIVE &&
                    (ePeerDir == MEDIA_DIRECTION_RECEIVE ||
                    ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE))
            {
                return eSrcDir;
            }
            else if (eSrcDir == MEDIA_DIRECTION_INACTIVE && (ePeerDir != MEDIA_DIRECTION_INACTIVE))
            {
                return eSrcDir;
            }
        }
    }

    return eNegotiatedDir;
}

PROTECTED
AString AudioNego::MakeCryptoAttributeFromSrtpProfile(IN AudioProfile* pProfile)
{
    IMS_SINT32 nCryptoCnt = 0;
    AString strCrypto ="";
    AString strMasterKey;

    IMS_TRACE_I("MakeCryptoAttributeFromSrtpProfile() enter",0, 0, 0);
/*
    for (IMS_UINT32 i = 0 ; i < pProfile->objCapaNego.mapAttributeCapa.GetSize(); i++) {
        AString strAttribute = pProfile->objCapaNego.mapAttributeCapa.GetValueAt(i);
        if (strAttribute != IMS_NULL && strAttribute.Contains("crypto") == IMS_TRUE) {
            IMSList<AString> strTemp = strAttribute.Split(' ');
            nCryptoCnt = strTemp.GetAt(0).Split(':').GetAt(1).ToInt32();
            break;
        }
    }
*/
    // Todo Multiple Crypto Suite
    nCryptoCnt = 1;

    switch (pProfile->eSrtpCryptoType)
    {
        case MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_80:
            strMasterKey = AString((IMS_CHAR*)pProfile->szKey,30).ToBase64();
            strCrypto.Sprintf("%d %s inline:%s|2^%d|1:4", nCryptoCnt, "AES_CM_128_HMAC_SHA1_80",
                    strMasterKey.GetStr(), pProfile->nMasterKeyLifeTime);
            break;
        case MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_32:
            strMasterKey = AString((IMS_CHAR*)pProfile->szKey,30).ToBase64();
            strCrypto.Sprintf("%d %s inline:%s|2^%d|1:4", nCryptoCnt, "AES_CM_128_HMAC_SHA1_32",
                    strMasterKey.GetStr(), pProfile->nMasterKeyLifeTime);
            break;
        case MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_80:
            strMasterKey = AString((IMS_CHAR*)pProfile->szKey,32).ToBase64();
            strCrypto.Sprintf("%d %s inline:%s|2^%d|1:4", nCryptoCnt, "AES_CM_256_HMAC_SHA1_80",
                    strMasterKey.GetStr(), pProfile->nMasterKeyLifeTime);
            break;
        case MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_32:
            strMasterKey = AString((IMS_CHAR*)pProfile->szKey,32).ToBase64();
            strCrypto.Sprintf("%d %s inline:%s|2^%d|1:4", nCryptoCnt, "AES_CM_256_HMAC_SHA1_32",
                    strMasterKey.GetStr(), pProfile->nMasterKeyLifeTime);
            break;
        case MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_80:
            strCrypto.Sprintf("%d %s inline:%s|2^%d|1:4", nCryptoCnt, "NULL_HMAC_SHA1_80",
                    strMasterKey.GetStr(), pProfile->nMasterKeyLifeTime);
            break;
        case MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_32:
            strCrypto.Sprintf("%d %s inline:%s|2^%d|1:4", nCryptoCnt, "NULL_HMAC_SHA1_32",
                    strMasterKey.GetStr(), pProfile->nMasterKeyLifeTime);
            break;
        default:
            break;
    }

    IMS_TRACE_I("MakeCryptoAttributeFromSrtpProfile() end - crypto[%s]",strCrypto.GetStr(),0, 0);

    return strCrypto;
}

PROTECTED
IMS_BOOL AudioNego::MakeSrtpProfileFromCapaNego(IN_OUT AudioProfile* pProfile)
{
    IMS_TRACE_I("MakeSrtpProfileFromCapaNego() enter",0, 0, 0);

    if (pProfile == IMS_NULL) return IMS_FALSE;

    AString strTcap ="";
    AString strAcap ="";
    AString strSAVP ="RTP/SAVP";
    AString strSAVPF ="RTP/SAVPF";
    AString strCrypto ="crypto";
    IMS_BOOL bRet = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pProfile->objCapaNego.mapTransportCapa.GetSize(); i++)
    {
        strTcap = pProfile->objCapaNego.mapTransportCapa.GetValueAt(i);
        if (strTcap.Contains(strSAVP) == IMS_TRUE || strTcap.Contains(strSAVPF) == IMS_TRUE)
        {
            IMS_TRACE_I("CheckSRTPFromProfile() find SAVP or SAVPF from profile..", 0, 0, 0);
            bRet = IMS_TRUE;
            break;
        }
    }

    if (bRet == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    //Parsing Crypto Attribute
    for (IMS_UINT32 i = 0; i < pProfile->objCapaNego.mapAttributeCapa.GetSize(); i++)
    {
        strAcap = pProfile->objCapaNego.mapAttributeCapa.GetValueAt(i);

        if (strAcap == IMS_NULL) \
        {
            continue;
        }

        if (strAcap.Contains(strCrypto) == IMS_TRUE)
        {
            bRet = MakeSrtpProfileFromCryptoAttr(pProfile, strAcap);
            break;
        }
    }

    IMS_TRACE_I("MakeSrtpProfileFromCapaNego() end - result[%d]", bRet, 0, 0);
    return bRet;
}

PROTECTED
IMS_BOOL AudioNego::MakeSrtpProfileFromCryptoAttr(OUT AudioProfile* pProfile, IN AString CryptoAttr)
{
    IMS_BOOL bRet = IMS_TRUE;
    IMSList<AString> lstSplitSpace = CryptoAttr.Split(' ');
    IMS_SINT32 nMasterKeyLen = 0;

    //crypto:1 AES_CM_128_HMAC_SHA1_80 inline:WALKJLFKSJLASLDKJAwSLKD|2^20|1:4
    AString strCryptoType = lstSplitSpace.GetAt(1);
    AString strInline = lstSplitSpace.GetAt(2);

    // Set SRTP Profile

    if (strCryptoType.EqualsIgnoreCase("AES_CM_128_HMAC_SHA1_80") == IMS_TRUE)
    {
        pProfile->eSrtpCryptoType= MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_80;
        nMasterKeyLen = 30;
    }
    else if (strCryptoType.EqualsIgnoreCase("AES_CM_128_HMAC_SHA1_32") == IMS_TRUE)
    {
        pProfile->eSrtpCryptoType = MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_32;
        nMasterKeyLen = 30;
    }
    else if (strCryptoType.EqualsIgnoreCase("AES_CM_256_HMAC_SHA1_80") == IMS_TRUE)
    {
        pProfile->eSrtpCryptoType = MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_80;
        nMasterKeyLen = 32;
    }
    else if (strCryptoType.EqualsIgnoreCase("AES_CM_256_HMAC_SHA1_32") == IMS_TRUE)
    {
        pProfile->eSrtpCryptoType = MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_32;
        nMasterKeyLen = 32;
    }
    else if (strCryptoType.EqualsIgnoreCase("NULL_HMAC_SHA1_80") == IMS_TRUE)
    {
        pProfile->eSrtpCryptoType = MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_80;
    }
    else if (strCryptoType.EqualsIgnoreCase("NULL_HMAC_SHA1_32") == IMS_TRUE)
    {
        pProfile->eSrtpCryptoType = MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_32;
    }
    else
    {
        IMS_TRACE_I("MakeSrtpProfileFromCryptoAttr() SRTP Profile is not supported", 0, 0, 0);
        bRet = IMS_FALSE;
    }

// Set SRTP Master Key  and Master Key Life Time

    //inline:WALKJLFKSJLASLDKJAwSLKD|2^20|1:4
    IMSList<AString> lstSubSplitSpace;
    AString strMasterKey, strLifeTime;

    lstSubSplitSpace = strInline.Split(':');
    lstSubSplitSpace = ((AString)lstSubSplitSpace.GetAt(1)).Split('|');
    if (lstSubSplitSpace.GetSize() >= 1)
    {
        strMasterKey = lstSubSplitSpace.GetAt(0);
    }
    if (lstSubSplitSpace.GetSize() >= 2)
    {
        strLifeTime = lstSubSplitSpace.GetAt(1);
    }

    IMS_MEM_Memset(pProfile->szKey,0,sizeof(pProfile->szKey));
    IMS_MEM_Memcpy(pProfile->szKey,AString::FromBase64(strMasterKey).GetStr(),nMasterKeyLen);

    AString temp ="";

    //print master key and master salt key
    for (int i =0 ; i < MEDIA_MAX_KEY_LEN; i++)
    {
        AString temp2 = "";
        temp2.Sprintf(" %02x", pProfile->szKey[i]);
        temp.Append(temp2);
    }
    IMS_TRACE_I("MakeSrtpProfileFromCryptoAttr() szkey : %s", temp.GetStr(), 0, 0);

    //master key life time setting
    if (strLifeTime.GetLength() > 0)
    {
        lstSubSplitSpace = strLifeTime.Split('^');
        pProfile->nMasterKeyLifeTime = lstSubSplitSpace.GetAt(1).ToInt32(IMS_NULL,10);
    }
    else
    {
        pProfile->nMasterKeyLifeTime = 48;
    }

    return bRet;
}

PROTECTED
IMS_BOOL AudioNego::MakeCapaNegoProfileFromSdp(IN IMediaDescriptor* pDescriptor,
        OUT AudioProfile::CapaNego* pObjCapaNego)
{
    if (pDescriptor == IMS_NULL || pObjCapaNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nTcapInitNum = 0, nAcapNum = 0;
    AString strTcap ="";
    AString strAcap ="";

    IMSList<AString> lstTcapAttr = pDescriptor->GetAttributes(SdpAttribute::TCAP);
    IMSList<AString> lstAcapAttr = pDescriptor->GetAttributes(SdpAttribute::ACAP);
    IMSList<AString> lstAcfgAttr = pDescriptor->GetAttributes(SdpAttribute::ACFG);

    if (lstAcfgAttr.GetSize() > 0)
    {
        pObjCapaNego->strNegotiatedAcfg = lstAcfgAttr.GetAt(0);
        IMS_TRACE_I("GetCapaNegoValueFromSDP() - ACFG[%s]",
                pObjCapaNego->strNegotiatedAcfg.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    // Get Potential configuration list (pcfg) -"'prio #' SP"t=Tcap #' SP 'a=Acap #'" pair
    pObjCapaNego->lstPotentialConfig = pDescriptor->GetAttributes(SdpAttribute::PCFG);

    IMS_TRACE_I("GetCapaNegoValueFromSDP() - PcfgSize[%d], TcapSize[%d], AcapSize[%d]",
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

        IMSList<AString> lstSplitSpace = strTcapline.Split(' ');

        // save Tcap String to CapaNego Obj
        for (IMS_UINT32 j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            if (j == 0)
            {
                nTcapInitNum = lstSplitSpace.GetAt(j).ToInt32();
                IMS_TRACE_I("GetCapaNegoValueFromSDP() - nTcapInitNum[%d]", nTcapInitNum, 0, 0);
            } else {
                // mapped - key : 'number' value:'Tcap'
                strTcap.Sprintf("%s", lstSplitSpace.GetAt(j).GetStr());
                pObjCapaNego->mapTransportCapa.Add(nTcapInitNum,strTcap);
                nTcapInitNum++;
                strTcap ="";
            }
        }
    }

    // Get attribute capability(ACAP) list -"'number' SP 'Acap'" pair
    for (IMS_UINT32 i = 0; i < lstAcapAttr.GetSize(); i++)
    {
        strAcap ="";
        nAcapNum = 0;
        AString strAcapline = lstAcapAttr.GetAt(i);
        if (strAcapline.GetLength() == 0) \
        {
            continue;
        }

        IMSList<AString> lstSplitSpace = strAcapline.Split(' ');

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
                    strAcap.Append(""+lstSplitSpace.GetAt(j));
                }
            }
        }
        // save Acap String...
        if (strAcap.GetLength() != 0)
        {
            IMS_TRACE_I("GetCapaNegoValueFromSDP() - add map[%d - %s]",
                    nAcapNum, strAcap.GetStr(),0);
            pObjCapaNego->mapAttributeCapa.Add(nAcapNum,strAcap);
        }
    }
    if (lstAcapAttr.GetSize() > 0)
    {
        pObjCapaNego->bIsAttCapaInPcfg = IMS_TRUE;
    }

    if (pObjCapaNego->mapTransportCapa.GetSize() == 0)
    {
        IMS_TRACE_I("GetCapaNegoValueFromSDP() - Therer are no Tcap value in SDP", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pObjCapaNego->lstPotentialConfig.GetSize() == 0)
    {
        IMS_TRACE_I("GetCapaNegoValueFromSDP() - Therer are no PCFG value in SDP", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL AudioNego::MakeNegotiatedCapaNegoProfile(IN AudioProfile::CapaNego* pSrcCapaNego,
        IN AudioProfile::CapaNego* pDestCapaNego, OUT AudioProfile::CapaNego* pNegotiatedCapaNego)
{
    IMS_TRACE_I("MakeNegotiatedCapaNego() enter", 0, 0, 0);

    if (pSrcCapaNego == IMS_NULL || pDestCapaNego == IMS_NULL  || pNegotiatedCapaNego == IMS_NULL)
    {
        IMS_TRACE_E(0,"AudioNego::MakeNegotiatedCapaNego() invalid argument, %" PFLS_x" %" PFLS_x,
                pSrcCapaNego, pDestCapaNego, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bRet = IMS_FALSE;
    IMS_BOOL bAttributeCheckable = IMS_FALSE;
    IMS_BOOL bPCFGSupportable = IMS_FALSE;

    // pNegotiatedCapaNego->lstPotentialConfig.GetSize();
    IMSMap<IMS_SINT32, AString> mapSrcTCap = pSrcCapaNego->mapTransportCapa;
    IMSMap<IMS_SINT32, AString> mapSrcACap = pSrcCapaNego->mapAttributeCapa;

    IMSList<AString> lstDstPCFG = pDestCapaNego->lstPotentialConfig;
    IMSMap<IMS_SINT32, AString> mapDstTCap = pDestCapaNego->mapTransportCapa;
    IMSMap<IMS_SINT32, AString> mapDstACap = pDestCapaNego->mapAttributeCapa;

    if (pDestCapaNego->strNegotiatedAcfg.GetLength() > 0) {
        IMS_TRACE_I("VideoNego::MakeNegotiatedCapaNego() ACFG - %s",
                pDestCapaNego->strNegotiatedAcfg.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    // parse pcfg
    for (IMS_UINT32 i = 0; i < lstDstPCFG.GetSize(); i++)
    {
        AString strPcfgline = lstDstPCFG.GetAt(i);      // get"# t=# a=#,#,#,# ..."
        if (strPcfgline.GetLength() == 0)
        {
            continue;
        }

        IMSList<AString> lstSplitSpace = lstDstPCFG.GetAt(i).Split(' ');

        for (IMS_UINT32 j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            bAttributeCheckable = IMS_FALSE;
            if (j == 1)    // t=#
            {
                AString strPcfg_Transport = lstSplitSpace.GetAt(j);
                if (strPcfg_Transport.GetLength() == 0)
                {
                    continue;
                }

                IMSList<AString> lstSplitEquals = strPcfg_Transport.Split('=');
                if (lstSplitEquals.GetSize() == 0)
                {
                    continue;
                }

                if (lstSplitEquals.GetAt(0).Equals('t') == IMS_TRUE && lstSplitEquals.GetSize() > 1)
                {
                    // 1. compare transport capa
                    AString tmp = mapDstTCap.GetValue(lstSplitEquals.GetAt(1).ToInt32());

                    for (IMS_UINT32 k = 0; k < mapSrcTCap.GetSize(); k++)
                    {
                        if (tmp.Equals(mapSrcTCap.GetValueAt(k)) == IMS_TRUE)
                        {
                            bAttributeCheckable = IMS_TRUE;
                            bPCFGSupportable    = IMS_TRUE;

                            // set Negotiated Transport Capa Nego Value...
                            pNegotiatedCapaNego->mapTransportCapa.Add(
                                    lstSplitEquals.GetAt(1).ToInt32(), tmp);
                            break;
                        }
                    }

                    // 2. if there are no matched transport capa, then next pcfg check...
                    // -----it's first for_loop break case..
                    if (bAttributeCheckable == IMS_FALSE)
                    {
                        IMS_TRACE_I("MakeNegotiatedCapaNego() does not match transport capa \
                                - PCFG #[%d]", i, 0, 0);
                        break;
                    }
                    // 3. if there are matched transport capa, check attribute capa
                    // -----it's not first for_loop break case..
                }
            }
            else if (j == 2)   // a=#,#,#,#...
            {
                // if attribute pcfg is exist in SDP,
                // then bPCFGSupportable reset to IMS_FALSE for attribute capa nego..
                bPCFGSupportable    = IMS_FALSE;

                AString strPcfg_Attribute = lstSplitSpace.GetAt(j);
                if (strPcfg_Attribute.GetLength() == 0)
                {
                    continue;
                }

                IMSList<AString> lstSplitEquals = strPcfg_Attribute.Split('=');
                if (lstSplitEquals.GetSize() == 0)
                {
                    continue;
                }

                if (lstSplitEquals.GetAt(0).Equals('a') == IMS_TRUE && lstSplitEquals.GetSize() > 1)
                {
                    IMS_UINT32 cnt = 0;
                    // 1. compare Attribute capa
                    AString tmp = lstSplitEquals.GetAt(1);        // tmp ="1,2,3,4"

                    // attribute comma parsing..
                    IMSList<AString> lstSplitComma = tmp.Split(',');
                    IMS_TRACE_I("MakeNegotiatedCapaNego() attribute size[%d]",
                            lstSplitComma.GetSize(), 0, 0);

                    if (lstSplitComma.GetSize() == 0)
                    {
                        continue;
                    }

                    // 2. check attribute capa negotiation
                    for (IMS_UINT32 k = 0; k < lstSplitComma.GetSize(); k++)
                    {
                        AString strDestAttributeCapa =
                                mapDstACap.GetValue(lstSplitComma.GetAt(k).ToInt32());
                        IMS_TRACE_I("MakeNegotiatedCapaNego() strDestAttributeCapa[%s]",
                                strDestAttributeCapa.GetStr(), 0, 0);

                        for (IMS_UINT32 l = 0; l < mapSrcACap.GetSize(); l++)
                        {
                            IMS_TRACE_I("MakeNegotiatedCapaNego() SrcAttributeCapa[%s]",
                                    mapSrcACap.GetValueAt(l).GetStr(), 0, 0);
                            if (strDestAttributeCapa.Equals(mapSrcACap.GetValueAt(l)) == IMS_TRUE)
                            {
                                cnt++;
                                // set Negotiated Attribute Capa Nego value...
                                pNegotiatedCapaNego->mapAttributeCapa.Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);

                                IMS_TRACE_I("MakeNegotiatedCapaNego() - \
                                        strDestAttributeCapa.Equals CNT[%d]", cnt, 0, 0);
                                break;
                            }
                            else if (strDestAttributeCapa.Contains("crypto") == IMS_TRUE)
                            {
                                //crypto attribute negotiate only srtp profile type
                                IMSList<AString> lstSrcCryptoAttribute =
                                        mapSrcACap.GetValueAt(l).Split(' ');
                                IMSList<AString> lstDestCryptoAttribute =
                                        strDestAttributeCapa.Split(' ');
                                if (lstDestCryptoAttribute.GetAt(1).Equals(
                                        lstSrcCryptoAttribute.GetAt(1)) == IMS_TRUE)
                                {
                                    cnt++;
                                    pNegotiatedCapaNego->mapAttributeCapa.Add(
                                            lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);

                                    IMS_TRACE_I("MakeNegotiatedCapaNego() - \
                                            strDestAttributeCapa.Equals CNT[%d]", cnt, 0, 0);
                                    break;
                                }
                            }
                        }
                    }

                    // 3. if ue support pcfg about transport capa,
                    // bPCFGSupportable variable set to True..
                    if (cnt == lstSplitComma.GetSize())
                    {
                        IMS_TRACE_I("MakeNegotiatedCapaNego() - capa nego success..", 0, 0, 0);
                        bPCFGSupportable = IMS_TRUE;
                        break;
                    }
                }
            }
        }

        // check capa nego success
        if (bPCFGSupportable == IMS_TRUE)
        {
            pNegotiatedCapaNego->strNegotiatedAcfg.Sprintf("%s",strPcfgline.GetStr());
            IMS_TRACE_I("MakeNegotiatedCapaNego() UE support capa nego- ACFG[%s]",
                    strPcfgline.GetStr(), 0, 0);
            // strNegotiatedAcfg value available, if capa nego success.
            bRet = IMS_TRUE;
            break;
        }
        else    // capa nego dose not succsee case//
        {
            // clear saved negotiatedCapaNego imfo.
            IMS_TRACE_I("MakeNegotiatedCapaNego() capa nego does not success pcfg[%d]", i, 0, 0);
            pNegotiatedCapaNego->mapTransportCapa.Clear();
            pNegotiatedCapaNego->mapAttributeCapa.Clear();
        }
    }

    return bRet;
}

PUBLIC
AudioConfiguration* AudioNego::GetConfig()
{
    //IMS_TRACE_D("GetConfig() eSessionType[%d]", m_eSessionType, 0, 0);

    if (m_pMediaEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0,"GetConfig - There is no MediaEnvironment", 0, 0, 0);
        return IMS_NULL;
    }

    AudioConfiguration* pAudioConfig = MediaSessionConfigFactory::GetInstance()->
            FindMediaSessionConfig(GetSlotId(), m_pMediaEnvironment->eServiceType)->
            GetAudioConfiguration(m_eSessionType);
    if (pAudioConfig == IMS_NULL)
    {
        IMS_TRACE_E(0,"GetConfig - There is no audio config", 0, 0, 0);
        return IMS_NULL;
    }
    return pAudioConfig;
}
