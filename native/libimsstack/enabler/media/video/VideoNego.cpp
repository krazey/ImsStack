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
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpRtcpFeedback.h"

#include "video/VideoNego.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "MediaResourceMngr.h"
#include "MediaManager.h"

// == DEFINES =========================================================
__IMS_TRACE_TAG_USER_DECL__("MED.VN");

// == Constructor, Destructor, Operator Overloading ========================================
PUBLIC VideoNego::VideoNego(IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_lstOaModel(IMSList<OaModel*>()),
        m_objBaseProfile(VideoProfile()),
        m_pMediaEnvironment(IMS_NULL),
        m_eSessionType(MEDIA_TYPE_INVALID),
        m_bNegotiatedCvoResult(IMS_FALSE)
{
    IMS_TRACE_I("+VideoNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC VideoNego::~VideoNego()
{
    IMS_TRACE_I("~VideoNego()", 0, 0, 0);
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

GLOBAL PUBLIC VideoNego* VideoNego::Create(
        IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType)
{
    (void)eServiceType;
    return new VideoNego(nSlotId);
}

PUBLIC
void VideoNego::Copy(IN VideoNego* pVideoNego)
{
    if (pVideoNego == IMS_NULL)
    {
        return;
    }
    IMS_TRACE_I("Copy() - list size[%d]", pVideoNego->m_lstOaModel.GetSize(), 0, 0);

    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());
    if (pMediaManager == IMS_NULL)
    {
        return;
    }
    MediaResourceMngr* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr != IMS_NULL)
    {
        // To release previous used port
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
        }
    }

    m_objBaseProfile.Copy(&pVideoNego->m_objBaseProfile);

    if (pResourceMngr != IMS_NULL)
    {
        // To add port (it would be duplicated)
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->AcquireRtpPort(m_objBaseProfile.nDataPort, m_objBaseProfile.nDataPort);
        }
    }

    m_bNegotiatedCvoResult = pVideoNego->m_bNegotiatedCvoResult;
    m_pMediaEnvironment = pVideoNego->m_pMediaEnvironment;

    IMS_TRACE_I("Copy() - pVideoNego->m_lstOaModel.GetSize()[%d]",
            pVideoNego->m_lstOaModel.GetSize(), 0, 0);

    if (pVideoNego->m_lstOaModel.GetSize() < 1)
    {
        return;
    }

    OaModel* pNewOaModel = new OaModel();
    OaModel* pOldOaModel = pVideoNego->m_lstOaModel.GetAt(0);
    pNewOaModel->pSrcProfile = new VideoProfile(pOldOaModel->pSrcProfile);
    m_lstOaModel.Append(pNewOaModel);
    IMS_TRACE_I("Copy() - m_lstOaModel.GetSize()[%d]", m_lstOaModel.GetSize(), 0, 0);

    return;
}

// == PUBLIC METHOD ==============================================================
PUBLIC VIRTUAL void VideoNego::CreateProfiles(IN MediaEnvironment* pEnvironment)
{
    if (pEnvironment == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("CreateProfiles() svc[%d]", (IMS_SINT32)pEnvironment->eServiceType, 0, 0);

    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    GetSlotId(), pEnvironment->eServiceType);

    if (pMediaSessionConfig == IMS_NULL)
    {
        return;
    }

    VideoProfile* pProfile;
    VideoConfiguration* pConfig = pMediaSessionConfig->GetVideoConfiguration();

    if (pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfiles() pConfig is NULL", 0, 0, 0);
    }
    pProfile = &m_objBaseProfile;

    VideoProfileConfigurer::CreateVideoProfile(pProfile, pEnvironment, pConfig, GetSlotId());
}

PUBLIC VIRTUAL void VideoNego::DestroyProfiles()
{
    while (m_objBaseProfile.lstPayload.GetSize() > 0)
    {
        VideoProfile::Payload* pPayload = m_objBaseProfile.lstPayload.GetAt(0);
        if (pPayload != IMS_NULL)
        {
            delete pPayload;
        }

        m_objBaseProfile.lstPayload.RemoveAt(0);
    }

    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());
    if (pMediaManager != IMS_NULL)
    {
        MediaResourceMngr* pResourceMngr = pMediaManager->GetResourceManager();

        if (pResourceMngr != IMS_NULL)
        {
            if (m_objBaseProfile.nDataPort != 0)
            {
                pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
            }
        }
    }
}

PUBLIC VIRTUAL void VideoNego::SetMediaEnvironment(IN MediaEnvironment* pMediaEnvironment)
{
    m_pMediaEnvironment = pMediaEnvironment;
}

PUBLIC VIRTUAL void VideoNego::SetSessionType(IN MEDIA_CONTENT_TYPE eSessionType)
{
    m_eSessionType = eSessionType;
}

// -- Negotiation APIs -------------------------------------------------------------------------
PUBLIC VIRTUAL IMS_BOOL VideoNego::FormSDP(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
        IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eDir)
{
    IMS_TRACE_D("FormSDP() eNegoState[%d], eType[%d], eDir[%d]", eNegoState, eType, eDir);
    switch (eNegoState)
    {
        case STATE_IDLE:
            return FormOffer(pSessionDescriptor, pDescriptor, eType, eDir);
        case STATE_OFFER_RECEIVED:
            return FormAnswer(pSessionDescriptor, pDescriptor, eType, eDir);
        case STATE_NEGOTIATED:
            return FormReOffer(pSessionDescriptor, pDescriptor, eType, eDir);
        default:
            IMS_TRACE_E(0, "FormSDP fail eNegoState[%d]", eNegoState, 0, 0);
            return IMS_FALSE;
    }
}

PROTECTED VIRTUAL IMS_BOOL VideoNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eDir)
{
    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_VIDEO) != IMS_TRUE ||
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

    pNewOaModel->pSrcProfile = new VideoProfile(&m_objBaseProfile);

    // Step 2. Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID && eDir < MEDIA_DIRECTION_ACCORDING_TO_NEGO)
    {
        IMS_TRACE_I("FormOffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pSrcProfile->eDirection = eDir;
    }
    else if (eDir == MEDIA_DIRECTION_INVALID)
    {
        pNewOaModel->pSrcProfile->nDataPort = 0;
        pNewOaModel->pSrcProfile->nControlPort = 0;
    }

    // Step 3. Modify a RS/RR by conditions (for RTCP enable/disable)
    VideoProfileConfigurer::SetVideoRsRr(pNewOaModel->pSrcProfile, GetConfig());

    m_lstOaModel.Append(pNewOaModel);

    // Step 4. Make the SDP from profile
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pSrcProfile);
}

PROTECTED VIRTUAL IMS_BOOL VideoNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eDir)
{
    IMS_TRACE_D("FormAnswer() enter. eType[%d], eDir[%d]", eType, eDir, 0);

    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_lstOaModel.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    // Do not return even if direction is MEDIA_DIRECTION_INVALID
    if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_VIDEO) && eDir == MEDIA_DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    // Step 1. Getting OaModel from list
    OaModel* pNewOaModel = GetNegotiatedOaModel();
    if (pNewOaModel == IMS_NULL || pNewOaModel->IsAllProfileExist() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    // Step 2. Modify a RTP/RTCP port to ZERO if video is not supported
    if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_VIDEO) != IMS_TRUE)
    {
        pNewOaModel->pNegotiatedProfile->nDataPort = 0;
        pNewOaModel->pNegotiatedProfile->nControlPort = 0;
    }

    // Step 3. Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID && eDir < MEDIA_DIRECTION_ACCORDING_TO_NEGO)
    {
        IMS_TRACE_D("FormAnswer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pNegotiatedProfile->eDirection = eDir;
    }
    else
    {
        pNewOaModel->pNegotiatedProfile->eDirection =
                UpdateDirectionToMine(pNewOaModel->pDestProfile->eDirection,
                        pNewOaModel->pSrcProfile->eDirection, IMS_FALSE);

        IMS_TRACE_D("FormAnswer() Enforced Set to direction[%d] made from peer direction[%d]",
                pNewOaModel->pNegotiatedProfile->eDirection, pNewOaModel->pDestProfile->eDirection,
                0);
    }

    // Step 4. Make the SDP from profile
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pNegotiatedProfile);
}

PROTECTED VIRTUAL IMS_BOOL VideoNego::FormReOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType, IN MEDIA_DIRECTION eDir)
{
    IMS_TRACE_D("FormReOffer() enter - eType[%d], eDir[%d], GetNegotiatedDirection[%d]", eType,
            eDir, GetNegotiatedDirection());

    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Do not return if direction is MEDIA_DIRECTION_INVALID
    if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_VIDEO) && eDir == MEDIA_DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);

    // Step 1. Make new Offer/Answer model, and copy source profile from previous negotiated profile
    OaModel* pNewOaModel = new OaModel();
    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_lstOaModel.GetSize() == 0)
    {
        pNewOaModel->pSrcProfile = new VideoProfile(&m_objBaseProfile);
    }
    else
    {
        OaModel* pPrevOaModel = GetNegotiatedOaModel();

        if (pPrevOaModel == IMS_NULL)
        {
            if (pNewOaModel != IMS_NULL)
            {
                delete pNewOaModel;
            }
            return IMS_FALSE;
        }

        if (pPrevOaModel->pNegotiatedProfile != IMS_NULL &&
                pPrevOaModel->pNegotiatedProfile->nDataPort == 0 &&
                MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_VIDEO) != IMS_TRUE)
        {
            pNewOaModel->pSrcProfile = new VideoProfile(pPrevOaModel->pNegotiatedProfile);
        }
        else
        {
            pNewOaModel->pSrcProfile = new VideoProfile(&m_objBaseProfile);
        }
    }

    // Step 2. Modify a RTP/RTCP port if video is not supported
    if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_VIDEO) != IMS_TRUE)
    {
        pNewOaModel->pSrcProfile->nDataPort = 0;
        pNewOaModel->pSrcProfile->nControlPort = 0;
    }
    else
    {
        pNewOaModel->pSrcProfile->nDataPort = m_objBaseProfile.nDataPort;
        pNewOaModel->pSrcProfile->nControlPort = m_objBaseProfile.nControlPort;
    }

    // Step 3. Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID && eDir < MEDIA_DIRECTION_ACCORDING_TO_NEGO)
    {
        IMS_TRACE_I("FormReOffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pSrcProfile->eDirection = eDir;
    }
    else if (eDir == MEDIA_DIRECTION_INVALID)
    {
        pNewOaModel->pSrcProfile->nDataPort = 0;
        pNewOaModel->pSrcProfile->nControlPort = 0;
    }

    // Step 4. Modify a RS/RR by conditions (for RTCP enable/disable)
    VideoProfileConfigurer::SetVideoRsRr(pNewOaModel->pSrcProfile, GetConfig());

    m_lstOaModel.Append(pNewOaModel);

    // Step 5. Make the SDP from profile
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pSrcProfile);
}

PUBLIC VIRTUAL IMS_BOOL VideoNego::NegotiateSDP(NEGO_STATE eNegoState, IN IMS_BOOL bForking,
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
        case STATE_IDLE:
            *eDir = NegotiateOffer(pSessionDescriptor, pDescriptor);
            break;
        case STATE_OFFER_SENT:
            *eDir = NegotiateAnswer(pSessionDescriptor, pDescriptor);
            break;
        case STATE_NEGOTIATED:
            *eDir = (bForking == IMS_TRUE) ? NegotiateReanswer(pSessionDescriptor, pDescriptor)
                                           : NegotiateOffer(pSessionDescriptor, pDescriptor);
            break;
        default:
            break;
    }

    return (*eDir != MEDIA_DIRECTION_INVALID) ? IMS_TRUE : IMS_FALSE;
}

PROTECTED VIRTUAL MEDIA_DIRECTION VideoNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Step 0. Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateOffer() Entered", 0, 0, 0);

    // Step 1. Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pSrcProfile = new VideoProfile(&m_objBaseProfile);

    // Step 2. Make a destination profile from SDP
    pNewOaModel->pDestProfile = new VideoProfile();
    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pDestProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 3. Make a negotiated profile from Src&Dest profile
    pNewOaModel->pNegotiatedProfile = new VideoProfile();
    if (MakeNegotiatedProfile(pNewOaModel->pSrcProfile, pNewOaModel->pDestProfile, IMS_TRUE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateOffer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_lstOaModel.Append(pNewOaModel);

    // Step 4. Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PROTECTED VIRTUAL MEDIA_DIRECTION VideoNego::NegotiateAnswer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
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

    IMS_TRACE_I("NegotiateAnswer() Entered", 0, 0, 0);

    // Step 1. Get the latest OAmodel from list
    OaModel* pNewOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize() - 1);
    if (pNewOaModel == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 2. Make a destination profile from SDP
    pNewOaModel->pDestProfile = new VideoProfile();
    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pDestProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_lstOaModel.RemoveAt(m_lstOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 3. Make a negotiated profile from Src&Dest profile
    pNewOaModel->pNegotiatedProfile = new VideoProfile();
    if (MakeNegotiatedProfile(pNewOaModel->pSrcProfile, pNewOaModel->pDestProfile, IMS_FALSE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_lstOaModel.RemoveAt(m_lstOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateAnswer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);

    // Step 4. Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PROTECTED VIRTUAL MEDIA_DIRECTION VideoNego::NegotiateReanswer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
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
    pNewOaModel->pSrcProfile = new VideoProfile(&m_objBaseProfile);

    // Step 2. Make a destination profile from SDP
    pNewOaModel->pDestProfile = new VideoProfile();
    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pDestProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Step 3. Make a negotiated profile from Src&Dest profile
    pNewOaModel->pNegotiatedProfile = new VideoProfile();
    if (MakeNegotiatedProfile(pNewOaModel->pSrcProfile, pNewOaModel->pDestProfile, IMS_FALSE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateReanswer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_lstOaModel.Append(pNewOaModel);

    // Step 4. Return the direction of negotiated profile
    return pNewOaModel->pDestProfile->eDirection;
}

PUBLIC VIRTUAL void VideoNego::FinalizeSDP(
        IN IMS_SINTP nInputSessionDesciptorKey, IN NEGO_STATE eNegoState)
{
    IMS_BOOL bFoundOaModel = IMS_FALSE;
    IMS_UINT32 nOaModelSize = m_lstOaModel.GetSize();

    // reset confirmed Session check variable
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
        pLatestOaModel = m_lstOaModel.GetAt(nOaModelSize - 1);
    }

    if (pLatestOaModel != IMS_NULL)
    {
        if ((pLatestOaModel->IsAllProfileExist() &&
                    (eNegoState == STATE_IDLE || eNegoState == STATE_NEGOTIATED)) == IMS_FALSE)
        {
            IMS_TRACE_I("FinalizeSDP() - Incomplete OaModel[%d]. Delete profile", nOaModelSize - 1,
                    0, 0);
            delete pLatestOaModel;
            m_lstOaModel.RemoveAt(--nOaModelSize);
        }
    }

    for (IMS_UINT32 i = 0; i < nOaModelSize; i++)
    {
        // get OaModel
        OaModel* pTempOaModel = m_lstOaModel.GetAt(nOaModelSize - 1 - i);

        // find matched SessionDescriptor key
        if (pTempOaModel != IMS_NULL)
        {
            if (pTempOaModel->nSessionDescriptorKey == nInputSessionDesciptorKey)
            {
                pTempOaModel->bConfirmedSession = IMS_TRUE;
                bFoundOaModel = IMS_TRUE;
                IMS_TRACE_D("FinalizeSDP() - find comfirmed Session OaModel [%d]",
                        m_lstOaModel.GetSize() - i, 0, 0);
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
        if ((nOaModelSize - i - 1) < 6)
        {
            IMS_TRACE_D("FinalizeSDP() - nOaModelSize is under 6", 0, 0, 0);
            break;
        }

        OaModel* pDeleteCheckOaModel = m_lstOaModel.GetAt(0);

        if (pDeleteCheckOaModel != IMS_NULL)
        {
            IMS_TRACE_D("FinalizeSDP() - remove old OaModel", 0, 0, 0);
            if (pDeleteCheckOaModel->nSessionDescriptorKey == nInputSessionDesciptorKey &&
                    pDeleteCheckOaModel->bConfirmedSession == IMS_TRUE)
            {
                break;
            }

            IMS_TRACE_D("FinalizeSDP() - Delete the oldest[%" PFLS_x "] OaModel",
                    pDeleteCheckOaModel, 0, 0);
            delete pDeleteCheckOaModel;
            m_lstOaModel.RemoveAt(0);
        }
    }
}

PUBLIC
IMS_BOOL VideoNego::SetPort(IN IMS_UINT32 nPort)
{
    MediaManager* pMediaManager = MediaManager::GetInstance(GetSlotId());
    if (pMediaManager == IMS_NULL)
    {
        return IMS_FALSE;
    }

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

    IMS_TRACE_I(
            "SetPort() - Video Changed Data Port[%d]->[%d]", m_objBaseProfile.nDataPort, nPort, 0);

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

        IMS_TRACE_I("SetPort() - Video Data Port is 0!!!", 0, 0, 0);
    }

    return IMS_TRUE;
}

PUBLIC
IPAddress VideoNego::GetNegotiatedRemoteAddr()
{
    VideoProfile* pProfile = GetNegotiatedDestProfile();

    if (pProfile != IMS_NULL)
    {
        return pProfile->objIpAddr;
    }

    return IPAddress("");
}

PUBLIC
IMS_UINT32 VideoNego::GetNegotiatedRemotePort()
{
    VideoProfile* pProfile = GetNegotiatedDestProfile();

    if (pProfile != IMS_NULL)
    {
        return pProfile->nDataPort;
    }

    return 0;
}

PUBLIC
IMS_BOOL VideoNego::GetNegotiatedCvoResult()
{
    IMS_TRACE_I(
            "GetNegotiatedCvoResult() - m_bNegotiatedCvoResult [%d]", m_bNegotiatedCvoResult, 0, 0);
    return m_bNegotiatedCvoResult;
}

PUBLIC
IMS_BOOL VideoNego::GetNegotiatedProfileSet(OUT VideoProfile*& pSrcProfile,
        OUT VideoProfile*& pDestProfile, OUT VideoProfile*& pNegotiatedProfile)
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
VideoProfile* VideoNego::GetNegotiatedDestProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pDestProfile;
    }

    return IMS_NULL;
}

PUBLIC
VideoNego::OaModel* VideoNego::GetNegotiatedOaModel(IN IMS_BOOL bCheckConfirmed)
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
MEDIA_DIRECTION VideoNego::GetNegotiatedDirection()
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
VIDEO_RESOLUTION VideoNego::GetNegotiatedResolution(IN IMS_BOOL bCheckConfirmed)
{
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel(bCheckConfirmed);
        if (pLatestOaModel == IMS_NULL || pLatestOaModel->IsAllProfileExist() == IMS_FALSE ||
                pLatestOaModel->pNegotiatedProfile->nDataPort == 0 ||
                pLatestOaModel->pNegotiatedProfile->lstPayload.GetSize() == 0)
        {
            return VIDEO_RESOLUTION_INVALID;
        }

        VideoProfile::Payload* pPayload = pLatestOaModel->pNegotiatedProfile->lstPayload.GetAt(0);
        if (pPayload == IMS_NULL)
        {
            return VIDEO_RESOLUTION_INVALID;
        }

        if (pPayload->objRtpMap.strPayloadType == "H264")
        {
            VideoProfile::AvcFmtp* pFmtp = (VideoProfile::AvcFmtp*)pPayload->pFmtp;
            if (pFmtp != IMS_NULL)
            {
                return pFmtp->eResolution;
            }
        }
        else if (pPayload->objRtpMap.strPayloadType == "H265")
        {
            VideoProfile::HevcFmtp* pFmtp = (VideoProfile::HevcFmtp*)pPayload->pFmtp;
            if (pFmtp != IMS_NULL)
            {
                return pFmtp->eResolution;
            }
        }
    }

    return VIDEO_RESOLUTION_NOT_USED;
}

PUBLIC
IMS_SINT32 VideoNego::GetNegotiatedRtpPort()
{
    CONST IMS_SINT32 PORT_NONE = -1;
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = GetNegotiatedOaModel();
        if (pLatestOaModel == IMS_NULL || pLatestOaModel->IsAllProfileExist() == IMS_FALSE)
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
IMS_SINT32 VideoNego::GetMediaBandwidth()
{
    if (m_lstOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = m_lstOaModel.GetAt(m_lstOaModel.GetSize() - 1);
        if (pLatestOaModel == IMS_NULL || pLatestOaModel->pSrcProfile == IMS_NULL)
        {
            return -1;
        }

        // returned negotiated bandwidth.
        if (pLatestOaModel->pNegotiatedProfile != IMS_NULL)
        {
            return (IMS_SINT32)pLatestOaModel->pNegotiatedProfile->nBandwidthAs;
        }

        return (IMS_SINT32)pLatestOaModel->pSrcProfile->nBandwidthAs;
    }

    return -1;
}

PUBLIC
IMS_BOOL VideoNego::GetWidthHeightFromResolutionId(
        IN VIDEO_RESOLUTION eResolutionId, OUT IMS_UINT32* pnWidth, OUT IMS_UINT32* pnHeight)
{
    IMS_TRACE_D("GetWidthHeightFromResolutionId() resolution ID [%d]", eResolutionId, 0, 0);
    switch (eResolutionId)
    {
        case VIDEO_RESOLUTION_QCIF_LS:
            *pnWidth = 176;
            *pnHeight = 144;
            break;
        case VIDEO_RESOLUTION_QCIF_PR:
            *pnWidth = 144;
            *pnHeight = 176;
            break;
        case VIDEO_RESOLUTION_QVGA_LS:
            *pnWidth = 320;
            *pnHeight = 240;
            break;
        case VIDEO_RESOLUTION_QVGA_PR:
            *pnWidth = 240;
            *pnHeight = 320;
            break;
        case VIDEO_RESOLUTION_VGA_LS:
            *pnWidth = 640;
            *pnHeight = 480;
            break;
        case VIDEO_RESOLUTION_VGA_PR:
            *pnWidth = 480;
            *pnHeight = 640;
            break;
        case VIDEO_RESOLUTION_CIF_LS:
            *pnWidth = 352;
            *pnHeight = 288;
            break;
        case VIDEO_RESOLUTION_CIF_PR:
            *pnWidth = 288;
            *pnHeight = 352;
            break;
        case VIDEO_RESOLUTION_SIF_PR:
            *pnWidth = 240;
            *pnHeight = 352;
            break;
        case VIDEO_RESOLUTION_SIF_LS:
            *pnWidth = 352;
            *pnHeight = 240;
            break;
        case VIDEO_RESOLUTION_SQCIF_LS:
            *pnWidth = 128;
            *pnHeight = 96;
            break;
        case VIDEO_RESOLUTION_SQCIF_PR:
            *pnWidth = 96;
            *pnHeight = 128;
            break;
        case VIDEO_RESOLUTION_HD_LS:
            *pnWidth = 1280;
            *pnHeight = 720;
            break;
        case VIDEO_RESOLUTION_HD_PR:
            *pnWidth = 720;
            *pnHeight = 1280;
            break;
        case VIDEO_RESOLUTION_FHD_LS:
            *pnWidth = 1920;
            *pnHeight = 1080;
            break;
        case VIDEO_RESOLUTION_FHD_PR:
            *pnWidth = 1080;
            *pnHeight = 1920;
            break;
        default:
            *pnWidth = 176;
            *pnHeight = 144;
            IMS_TRACE_E(0, "GetWidthHeightFromResolutionId() INVALID resolution ID", 0, 0, 0);

            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
VIDEO_RESOLUTION VideoNego::GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel)
{
    IMS_TRACE_D("GetAvcMaxResolutionFromLevel() Enter Level[%d]", nLevel, 0, 0);

    if (nLevel > 31)
    {
        nLevel = 31;
    }

    // basically return portrait mode and framerate is 15fps
    switch (nLevel)
    {
        case 31:
            return VIDEO_RESOLUTION_HD_PR;
        case 30:
        case 22:
            return VIDEO_RESOLUTION_VGA_PR;
        case 21:
        case 20:
        case 14:
        case 13:
        case 12:
            return VIDEO_RESOLUTION_CIF_PR;
        case 11:
        case 10:
            return VIDEO_RESOLUTION_QCIF_PR;
        default:
            return VIDEO_RESOLUTION_VGA_PR;
    }
}

// == PROTECTED METHOD ==========================================================
PROTECTED
IMS_BOOL VideoNego::MakeSdpFromProfile(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("MakeSdpFromProfile() - PayloadSize[%d], AS[%d]", pProfile->lstPayload.GetSize(),
            pProfile->nBandwidthAs, 0);

    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    IMSList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // Step 0. make "c" line of media level if IP does not matched
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->objIpAddr))
    {
        IMS_TRACE_D("MakeSdpFromProfile() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->objIpAddr.ToCharString(), 0);

        pDescriptor->SetConnectionAddress(pProfile->objIpAddr.ToString());
    }

    // Step 1. make "m" line
    // ------ "m=video xxxx RTP/AVP aaa bbb ccc ddd"
    AStringArray objVideoFormat;
    AString strPayloadNum;

    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        VideoProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->objRtpMap.nPayloadNum);
        objVideoFormat.AddElement(strPayloadNum);
    }

    // Step 1-1. make SDPCapNeg attributes for initial SDP if AVPF is supported
    if (pProfile->strTransportType.Equals("RTP/AVPF"))
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_VIDEO, pProfile->nDataPort,
                SdpMedia::TRANSPORT_RTP_AVPF, objVideoFormat);
    }
    else
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_VIDEO, pProfile->nDataPort,
                SdpMedia::TRANSPORT_RTP_AVP, objVideoFormat);
    }

    // Step 1-2. Previously check all payload for RTCP-FB wildcard(*) attributes
    IMS_BOOL bTrrSupportedAll = IMS_TRUE;
    IMS_BOOL bNackSupportedAll = IMS_TRUE;
    IMS_BOOL bTmmbrSupportedAll = IMS_TRUE;
    IMS_BOOL bPliSupportedAll = IMS_TRUE;
    IMS_BOOL bFirSupportedAll = IMS_TRUE;

    if (pProfile->bSupportAvpf == IMS_TRUE)
    {
        for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
        {
            VideoProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);
            if (pPayload != IMS_NULL)
            {
                if (pPayload->objRtcpFbAttr.bTrrSupported == IMS_FALSE)
                {
                    bTrrSupportedAll = IMS_FALSE;
                }
                if (pPayload->objRtcpFbAttr.bNackSupported == IMS_FALSE)
                {
                    bNackSupportedAll = IMS_FALSE;
                }
                if (pPayload->objRtcpFbAttr.bTmmbrSupported == IMS_FALSE)
                {
                    bTmmbrSupportedAll = IMS_FALSE;
                }
                if (pPayload->objRtcpFbAttr.bPliSupported == IMS_FALSE)
                {
                    bPliSupportedAll = IMS_FALSE;
                }
                if (pPayload->objRtcpFbAttr.bFirSupported == IMS_FALSE)
                {
                    bFirSupportedAll = IMS_FALSE;
                }
            }
        }
    }
    else
    {
        bTrrSupportedAll = IMS_FALSE;
        bNackSupportedAll = IMS_FALSE;
        bTmmbrSupportedAll = IMS_FALSE;
        bPliSupportedAll = IMS_FALSE;
        bFirSupportedAll = IMS_FALSE;
    }

    // Step 2. make bandwidth
    // ------ "b=AS:xx"
    // ------ "b=AS:xx"
    // ------ "b=AS:xx"
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
    // ------ "a=rtpmap:104 H264/16000/1"
    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AString strRtpmap, strFmtp;
        AString strResolutionAttr;
        VIDEO_RESOLUTION eResolution;

        VideoProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // Step 4-1. make "rtpmap"
        strRtpmap.Sprintf("%d %s/%d", pPayload->objRtpMap.nPayloadNum,
                pPayload->objRtpMap.strPayloadType.GetStr(), pPayload->objRtpMap.nSamplingRate);

        if (pPayload->objRtpMap.nChannel != 0)
        {
            AString strChannel;
            strChannel.Sprintf("/%d", pPayload->objRtpMap.nChannel);
            strRtpmap.Append(strChannel);
        }

        IMS_TRACE_I("MakeSdpFromProfile() - Payload[%d], strRtpmap[%s]", i, strRtpmap.GetStr(), 0);

        // Step 4-2. make "fmtp"
        // ------ "a=fmtp:104 profile-level-id=42C016; packetization-mode=1;
        // ----------  sprop-parameter-sets=Z0LAFukDwKMg,aM4G4g=="
        SdpAvCodec* pFormat = new SdpAvCodec();
        if (pPayload->objRtpMap.strPayloadType.Equals("H264"))
        {
            VideoProfile::AvcFmtp* pAvcFmtp = (VideoProfile::AvcFmtp*)pPayload->pFmtp;

            if (pAvcFmtp == IMS_NULL)
            {
                delete pFormat;
                continue;
            }

            if (pAvcFmtp->bShow_ProfileLevelId)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }
                AString strTemp;
                strTemp.Sprintf("profile-level-id=%s", pAvcFmtp->strProfileLevelId.GetStr());
                strFmtp.Append(strTemp);
            }

            if (pAvcFmtp->bShow_PacketizationMode)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }
                AString strTemp;
                strTemp.Sprintf("packetization-mode=%d", pAvcFmtp->nPacketizationMode);
                strFmtp.Append(strTemp);
            }

            if (pAvcFmtp->bShow_SpropParam)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append(";");
                }
                AString strTemp;
                strTemp.Sprintf("sprop-parameter-sets=%s", pAvcFmtp->strSpropParam.GetStr());
                strFmtp.Append(strTemp);
            }

            eResolution = pAvcFmtp->eResolution;
        }
        else if (pPayload->objRtpMap.strPayloadType.Equals("H265"))
        {
            VideoProfile::HevcFmtp* pHevcFmtp = (VideoProfile::HevcFmtp*)pPayload->pFmtp;

            if (pHevcFmtp == IMS_NULL)
            {
                delete pFormat;
                continue;
            }

            if (pHevcFmtp->bShow_Profile)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append("; ");
                }
                AString strTemp;
                strTemp.Sprintf("profile-id=%d", pHevcFmtp->nProfile);
                strFmtp.Append(strTemp);
            }

            if (pHevcFmtp->bShow_Level)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append("; ");
                }
                AString strTemp;
                strTemp.Sprintf("level-id=%d", pHevcFmtp->nLevel);
                strFmtp.Append(strTemp);
            }
            /*
                        if (pHevcFmtp->bShow_PacketizationMode)
                        {
                            if (strFmtp.GetLength() > 0)
                            {
                                strFmtp.Append("; ");
                            }
                            AString strTemp;
                            strTemp.Sprintf("packetization-mode=%d", pHevcFmtp->nPacketizationMode);
                            strFmtp.Append(strTemp);
                        }
            */
            if (pHevcFmtp->bShow_SpropParam)
            {
                IMSList<AString> objSplitComma = pHevcFmtp->strSpropParam.Split(',');

                if (objSplitComma.GetSize() == 3)
                {
                    if (strFmtp.GetLength() > 0)
                    {
                        strFmtp.Append("; ");
                    }

                    pHevcFmtp->strVps = objSplitComma.GetAt(0);
                    pHevcFmtp->strSps = objSplitComma.GetAt(1);
                    pHevcFmtp->strPps = objSplitComma.GetAt(2);

                    if (pHevcFmtp->strVps.GetLength() > 0 || pHevcFmtp->strSps.GetLength() > 0 ||
                            pHevcFmtp->strPps.GetLength() > 0)
                    {
                        AString strTemp;

                        strTemp.Sprintf("sprop-vps=%s", pHevcFmtp->strVps.GetStr());
                        strFmtp.Append(strTemp);
                        strFmtp.Append("; ");

                        strTemp.Sprintf("sprop-sps=%s", pHevcFmtp->strSps.GetStr());
                        strFmtp.Append(strTemp);
                        strFmtp.Append("; ");

                        strTemp.Sprintf("sprop-pps=%s", pHevcFmtp->strPps.GetStr());
                        strFmtp.Append(strTemp);
                    }
                }
            }

            eResolution = pHevcFmtp->eResolution;
        }

        else
        {
            delete pFormat;
            continue;
        }

        if (strFmtp.GetLength() == 0)
        {
            strFmtp = AString::ConstNull();
        }

        AString strCompletedFmtp = AString::ConstNull();
        if (!strFmtp.IsNULL())
        {
            strCompletedFmtp.Sprintf("%d ", pPayload->objRtpMap.nPayloadNum);
            strCompletedFmtp.Append(strFmtp);
        }
        if (pFormat == IMS_NULL)
        {
            continue;
        }
        if (pFormat->SetParameters(strRtpmap, strCompletedFmtp) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "MakeSdpFromProfile() SetParameters() Fail. strRtpmap[%s], strFmtp[%s]",
                    strRtpmap.GetStr(), strCompletedFmtp.GetStr(), 0);
        }

        // Step 4-3. make "image attribute"
        if (pPayload->bIncludeImageAttr == IMS_TRUE)
        {
            if (MakeImageAttributeLine(
                        pPayload->objRtpMap.nPayloadNum, eResolution, strResolutionAttr))
            {
                pDescriptor->AddAttribute(SdpAttribute::IMAGEATTR, strResolutionAttr);
            }
        }

        // Step 4-3. make "framesize"
        if (pPayload->bIncludeFrameSize == IMS_TRUE)
        {
            if (MakeFrameSizeLine(pPayload->objRtpMap.nPayloadNum, eResolution, strResolutionAttr))
            {
                pDescriptor->AddAttribute(SdpAttribute::FRAMESIZE, strResolutionAttr);
            }
        }

        // Step 4-4. make "rtcp-fb"
        if ((pProfile->bSupportAvpf == IMS_TRUE) &&
                ((pProfile->bSupportCapaNegoForAvpf == IMS_FALSE) ||
                        (pProfile->bSupportCapaNegoForAvpf == IMS_TRUE &&
                                pProfile->objCapaNego.bIsAttCapaInPcfg == IMS_FALSE)))
        {
            IMS_SINT32 nPayloadNumForRtcpFb = -1;

            // TRR-INT
            if (bTrrSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bTrrSupportedAll == IMS_FALSE &&
                    pPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->objRtpMap.nPayloadNum;
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                AString strTemp = "";
                SdpRtcpFeedback* pTrr_IntAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);

                pTrr_IntAttr->SetType("trr-int");

                strTemp.Sprintf("%d", pPayload->objRtcpFbAttr.nTrrInt);
                pTrr_IntAttr->SetParameter(strTemp);

                pFormat->AddExtraParameter(pTrr_IntAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // NACK
            if (bNackSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bNackSupportedAll == IMS_FALSE &&
                    pPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->objRtpMap.nPayloadNum;
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pNackAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pNackAttr->SetType("nack");
                pFormat->AddExtraParameter(pNackAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // PLI
            if (bPliSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bPliSupportedAll == IMS_FALSE &&
                    pPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->objRtpMap.nPayloadNum;
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pPliAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pPliAttr->SetType("nack");
                pPliAttr->SetParameter("pli");
                pFormat->AddExtraParameter(pPliAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // FIR
            if (bFirSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bFirSupportedAll == IMS_FALSE &&
                    pPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->objRtpMap.nPayloadNum;
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pFIRAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pFIRAttr->SetType("ccm");
                pFIRAttr->SetParameter("fir");
                pFormat->AddExtraParameter(pFIRAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // TMMBR
            if (bTmmbrSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bTmmbrSupportedAll == IMS_FALSE &&
                    pPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->objRtpMap.nPayloadNum;
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pTmmbrAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pTmmbrAttr->SetType("ccm");
                pTmmbrAttr->SetParameter("tmmbr");
                pFormat->AddExtraParameter(pTmmbrAttr);
            }
        }

        pDescriptor->SetMediaFormat(pFormat);

        delete pFormat;
    }

    // Step 4. make direction
    pDescriptor->SetDirection(pProfile->eDirection);

    // Step 5. make framerate
    pDescriptor->AddAttributeInt(SdpAttribute::FRAMERATE, pProfile->nFrameRate);

    // Step 6. make CVO
    if (pProfile->nCvoId > 0)
    {
        AString strCvoAttribute;
        strCvoAttribute.Sprintf("%d urn:3gpp:video-orientation", pProfile->nCvoId);
        pDescriptor->AddAttribute(SdpAttribute::ATTRIBUTE_OTHER, strCvoAttribute, "extmap");
    }

    // Step 8. make Capa Nego Attribute
    if (pProfile->bSupportCapaNegoForAvpf == IMS_TRUE)
    {
        // add "ACFG" if it's a initial answer
        if (pProfile->objCapaNego.strNegotiatedAcfg.GetLength() > 0)
        {
            AString strAcfg;
            IMS_TRACE_D("MakeSdpFromProfile() - strNegotiatedAcfg [%s]",
                    pProfile->objCapaNego.strNegotiatedAcfg.GetStr(), 0, 0);
            strAcfg.Sprintf("%s", pProfile->objCapaNego.strNegotiatedAcfg.GetStr());
            pDescriptor->AddAttribute(SdpAttribute::ACFG, strAcfg);
        }

        IMS_TRACE_D("MakeSdpFromProfile() bSupportAvpf[%d], strTransportType[%s]",
                pProfile->bSupportAvpf, pProfile->strTransportType.GetStr(), 0);

        if (pProfile->bSupportAvpf == IMS_TRUE &&
                pProfile->strTransportType.Contains("AVPF") == IMS_FALSE)
        {
            // make tcap, acap, pcfg for capa nego offer...
            IMS_UINT32 i = 0;
            // AString strTcap = "1 RTP/AVPF";             // only support avpf profile
            AString strTcap = "";
            AString strAcap = "";
            AString strPcfg = "";

            IMS_TRACE_I("MakeSdpFromProfile() - Entered, PcfgSize[%d], TcapSize[%d], AcapSize[%d]",
                    pProfile->objCapaNego.lstPotentialConfig.GetSize(),
                    pProfile->objCapaNego.mapTransportCapa.GetSize(),
                    pProfile->objCapaNego.mapAttributeCapa.GetSize());

            for (i = 0; i < pProfile->objCapaNego.mapTransportCapa.GetSize(); i++)
            {
                strTcap = "";
                strTcap.Sprintf("%d %s", i + 1,
                        pProfile->objCapaNego.mapTransportCapa.GetValueAt(i).GetStr());
                pDescriptor->AddAttribute(SdpAttribute::TCAP, strTcap);
            }

            if (pProfile->objCapaNego.bIsAttCapaInPcfg == IMS_TRUE)
            {
                for (i = 0; i < pProfile->objCapaNego.mapAttributeCapa.GetSize(); i++)
                {
                    strAcap = "";
                    strAcap.Sprintf("%d %s", i + 1,
                            pProfile->objCapaNego.mapAttributeCapa.GetValueAt(i).GetStr());
                    pDescriptor->AddAttribute(SdpAttribute::ACAP, strAcap);
                    IMS_TRACE_I("MakeSdpFromProfile() - Add strAcap : %s", strAcap.GetStr(), 0, 0);
                }
            }

            for (i = 0; i < pProfile->objCapaNego.lstPotentialConfig.GetSize(); i++)
            {
                strPcfg = "";
                strPcfg.Sprintf(
                        "%d %s", i + 1, pProfile->objCapaNego.lstPotentialConfig.GetAt(i).GetStr());
                pDescriptor->AddAttribute(SdpAttribute::PCFG, strPcfg);
            }
        }
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL VideoNego::MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
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

    IMS_TRACE_I("MakeProfileFromSdp() - AS[%d], RS[%d], RR[%d]", pProfile->nBandwidthAs,
            pProfile->nBandwidthRs, pProfile->nBandwidthRr);

    // Setting transport type (for supporting AVPF)
    SdpMedia* pSDPMedia = (SdpMedia*)pDescriptor->GetMediaDescriptionEx();
    IMS_TRACE_I("MakeProfileFromSdp() - pSDPMedia[%d]", pSDPMedia, 0, 0);

    if (pSDPMedia != IMS_NULL)
    {
        pProfile->strTransportType = pSDPMedia->GetTransportProtocolEx();

        if (pProfile->strTransportType.Equals("RTP/AVP") == IMS_TRUE)
        {
            pProfile->bSupportAvpf = IMS_FALSE;
            pProfile->bSupportCapaNegoForAvpf = IMS_FALSE;
        }
        else if (pProfile->strTransportType.Equals("RTP/AVPF") == IMS_TRUE)
        {
            pProfile->bSupportAvpf = IMS_TRUE;
            pProfile->bSupportCapaNegoForAvpf = IMS_TRUE;
        }
    }

    // read CapaNego profile From SDP
    if (MakeCapaNegoProfileFromSdp(pDescriptor, &(pProfile->objCapaNego)) == IMS_TRUE)
    {
        // Get Capa nego value from the incoming SDP
        if (CheckAvpfFromProfile(pProfile) == IMS_TRUE)
        {
            pProfile->bSupportCapaNegoForAvpf = IMS_TRUE;
        }
    }

    // payload
    IMSList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    // Read ImageAttr list From SDP
    IMSList<AString> objImageAttributes = pDescriptor->GetAttributes(SdpAttribute::IMAGEATTR);

    // Read FrameSize list From SDP
    IMSList<AString> objFrameSizes = pDescriptor->GetAttributes(SdpAttribute::FRAMESIZE);
    IMS_TRACE_I("MakeProfileFromSdp() - lstMediaFormat.GetSize(%d), objImageAttributes.GetSize(%d),\
            objFrameSizes.GetSize(%d)",
            lstMediaFormat.GetSize(), objImageAttributes.GetSize(), objFrameSizes.GetSize());

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));
        if (pSdpCodec == IMS_NULL)
        {
            return IMS_FALSE;
        }
        AString strCodecName = pSdpCodec->GetName();

        IMS_TRACE_I("MakeProfileFromSdp() - At(%d), GetPayloadType(%d), GetClockRate(%d)", i,
                pSdpCodec->GetPayloadType(), pSdpCodec->GetClockRate());

        // Create RTP MAP
        VideoProfile::Payload* pPayload = new VideoProfile::Payload();
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        pPayload->SetRtpMap(
                pSdpCodec->GetPayloadType(), strCodecName, pSdpCodec->GetClockRate(), 0);

        // Get Image Attribute from the incoming SDP
        AString strImageAttrFromSdp = AString::ConstNull();
        if (objImageAttributes.GetSize() > i)
        {
            IMS_UINT32 nIndex = 0;
            if (GetCorrectImageIndex(pSdpCodec->GetPayloadType(), objImageAttributes, &nIndex))
            {
                strImageAttrFromSdp = objImageAttributes.GetAt(nIndex);
                pPayload->bIncludeImageAttr = IMS_TRUE;
            }
        }

        // Get Image FrameSize from the incoming SDP
        AString strFrameSizeFromSdp = AString::ConstNull();
        if (objFrameSizes.GetSize() > i)
        {
            IMS_UINT32 nIndex = 0;
            if (GetCorrectImageIndex(pSdpCodec->GetPayloadType(), objFrameSizes, &nIndex))
            {
                strFrameSizeFromSdp = objFrameSizes.GetAt(nIndex);
                pPayload->bIncludeFrameSize = IMS_TRUE;
            }
        }

        if (strCodecName.Equals("H264"))
        {
            // Create AMR fmtp
            VideoProfile::AvcFmtp* pAvcFmtp = new VideoProfile::AvcFmtp();
            GetFmtpFromString(pSdpCodec->GetFormatSpecificParameter(), pAvcFmtp);

            pPayload->pFmtp = pAvcFmtp;

            // Create Resolution from SDP -- true: image attr, false: spropParam
            pAvcFmtp->eResolution = GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttrFromSdp,
                    strFrameSizeFromSdp, pAvcFmtp->strSpropParam);
            // Create AVPF attributes
            if ((pProfile->bSupportAvpf == IMS_TRUE) || (pProfile->bSupportCapaNegoForAvpf))
            {
                if (GetAvpfFromAttributes((SdpMediaFormat*)pSdpCodec, &pProfile->objCapaNego,
                            &pPayload->objRtcpFbAttr) == IMS_FALSE)
                {
                    GetAvpfFromAttributes_EX(
                            pDescriptor, &pProfile->objCapaNego, &pPayload->objRtcpFbAttr);
                }
            }
        }
        else if (strCodecName.Equals("H265"))
        {
            // Create AMR fmtp
            VideoProfile::HevcFmtp* pHevcFmtp = new VideoProfile::HevcFmtp();
            GetFmtpFromString(pSdpCodec->GetFormatSpecificParameter(), pHevcFmtp);

            pPayload->pFmtp = pHevcFmtp;

            // Create Resolution from SDP -- true: image attr, false: spropParam
            pHevcFmtp->eResolution = GetResolutionFromSdp(
                    VIDEO_CODEC_HEVC, strImageAttrFromSdp, strFrameSizeFromSdp, pHevcFmtp->strSps);

            // Create AVPF attributes
            if (pProfile->bSupportAvpf == IMS_TRUE)
            {
                if (GetAvpfFromAttributes((SdpMediaFormat*)pSdpCodec, &pProfile->objCapaNego,
                            &pPayload->objRtcpFbAttr) == IMS_FALSE)
                {
                    GetAvpfFromAttributes_EX(
                            pDescriptor, &pProfile->objCapaNego, &pPayload->objRtcpFbAttr);
                }
            }
        }
        else
        {
            IMS_TRACE_E(0, "MakeProfileFromSdp() - NOT SUPPORTED video codec[%s]",
                    strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        pProfile->lstPayload.Append(pPayload);
    }

    // direction
    pProfile->eDirection = (MEDIA_DIRECTION)pDescriptor->GetDirection();

    if (pProfile->eDirection == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_D("MakeProfileFromSdp() - Video Media level Direction does not exist..", 0, 0, 0);
        // check session level attribute Direction
        pProfile->eDirection = (MEDIA_DIRECTION)pSessionDescriptor->GetDirection();
        if (pProfile->eDirection == MEDIA_DIRECTION_INVALID)
        {
            pProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
        }
    }

    // framerate
    pProfile->nFrameRate = pDescriptor->GetAttributeInt(SdpAttribute::FRAMERATE);

    // find CVO
    IMSList<AString> objAttributes =
            pDescriptor->GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, "extmap");
    for (IMS_UINT32 nIndex = 0; nIndex < objAttributes.GetSize(); nIndex++)
    {
        AString strExtmap = objAttributes.GetAt(nIndex);

        if (strExtmap.Contains("urn:3gpp:video-orientation") == IMS_TRUE)
        {
            AString strCVOTrim = strExtmap.Trim();
            IMSList<AString> strSplitSpace = strCVOTrim.Split(' ');

            if (strSplitSpace.GetAt(0).GetLength() > 0)
            {
                if (strSplitSpace.GetAt(0).Contains("/"))
                {
                    IMSList<AString> strSplitSlash = strSplitSpace.GetAt(0).Split('/');

                    if (strSplitSlash.GetSize() > 0 && strSplitSlash.GetAt(0).GetLength() > 0)
                    {
                        pProfile->nCvoId = strSplitSlash.GetAt(0).ToInt32();
                    }
                }
                else
                {
                    pProfile->nCvoId = strSplitSpace.GetAt(0).ToInt32();
                }

                IMS_TRACE_D("MakeProfileFromSdp() - CVO found. ID[%d]", pProfile->nCvoId, 0, 0);
            }
        }
    }

    m_bNegotiatedCvoResult = (pProfile->nCvoId > 0) ? IMS_TRUE : IMS_FALSE;

    IMS_TRACE_I("MakeProfileFromSdp() - Ended[%d]", pProfile->lstPayload.GetSize(), 0, 0);
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL VideoNego::MakeNegotiatedPayload(IN VideoProfile::Payload* pSrcPayload,
        IN VideoProfile::Payload* pDstPayload, IN VideoConfiguration* pConfig,
        OUT VideoProfile::Payload* pNegoPayload)
{
    (void)pConfig;
    if (pSrcPayload == IMS_NULL || pDstPayload == IMS_NULL || pNegoPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pNegoPayload->SetRtpMap(&pDstPayload->objRtpMap);

    pNegoPayload->bIncludeFrameSize = pSrcPayload->bIncludeFrameSize;
    pNegoPayload->bIncludeImageAttr = pSrcPayload->bIncludeImageAttr;

    if (pDstPayload->objRtpMap.strPayloadType.Equals("H264"))
    {
        VideoProfile::AvcFmtp* pNegoAvc_Fmtp =
                new VideoProfile::AvcFmtp((VideoProfile::AvcFmtp*)pSrcPayload->pFmtp);

        pNegoPayload->pFmtp = (void*)pNegoAvc_Fmtp;
    }
    else if (pDstPayload->objRtpMap.strPayloadType.Equals("H265"))
    {
        VideoProfile::HevcFmtp* pNegoHevc_Fmtp =
                new VideoProfile::HevcFmtp((VideoProfile::HevcFmtp*)pSrcPayload->pFmtp);

        pNegoPayload->pFmtp = (void*)pNegoHevc_Fmtp;
    }
    else
    {
        IMS_TRACE_E(0, "MakeNegotiatedPayload() cannot make negotiated Payload", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

VIRTUAL IMS_BOOL VideoNego::MakeNegotiatedProfile(IN VideoProfile* pSrcProfile,
        IN VideoProfile* pDestProfile, IN IMS_BOOL bIsOfferReceived,
        OUT VideoProfile* pNegotiatedProfile)
{
    IMS_BOOL ret = IMS_FALSE;

    if (pSrcProfile == IMS_NULL || pDestProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedProfile() invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    VideoConfiguration* pConfig = GetConfig();
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

        IMS_TRACE_D("MakeNegotiatedProfile() - ZERO Port. DO NOT Use the video[%d][%d],\
                But nego is successful",
                pNegotiatedProfile->nDataPort, pDestProfile->nDataPort, 0);
        return IMS_TRUE;
    }

    // Step 3. Setting profile type
    if (pSrcProfile->bSupportAvpf == IMS_TRUE && pDestProfile->bSupportAvpf == IMS_TRUE)
    {
        pNegotiatedProfile->bSupportAvpf = IMS_TRUE;
    }

    pNegotiatedProfile->bSupportCapaNegoForAvpf = pDestProfile->bSupportCapaNegoForAvpf;

    IMS_TRACE_I("MakeNegotiatedProfile() - SupportCapaNegoForAVPF[%d]]",
            pNegotiatedProfile->bSupportCapaNegoForAvpf, 0, 0);

    // Capability Negotiation for AVPF, SRTP
    if (pNegotiatedProfile->bSupportCapaNegoForAvpf == IMS_TRUE)
    {
        if (MakeNegotiatedCapaNegoProfile(&(pSrcProfile->objCapaNego), &(pDestProfile->objCapaNego),
                    &(pNegotiatedProfile->objCapaNego)) != IMS_TRUE)
        {
            // Capa Nego Fail, return to original transport protocol.
            IMS_TRACE_D("MakeNegotiatedProfile() - Capability Negotiation Fail Case", 0, 0, 0);
        }
        else
        {
            // Check Negotiated Transport Type
            IMS_BOOL bNegotiatedAVPF = IMS_FALSE;

            for (IMS_UINT32 i = 0; i < pNegotiatedProfile->objCapaNego.mapTransportCapa.GetSize();
                    i++)
            {
                AString strAttribute =
                        pNegotiatedProfile->objCapaNego.mapTransportCapa.GetValueAt(i);

                if (strAttribute != IMS_NULL && strAttribute.Contains("AVPF") == IMS_TRUE)
                {
                    bNegotiatedAVPF = IMS_TRUE;
                }
            }

            if (pNegotiatedProfile->bSupportCapaNegoForAvpf == IMS_TRUE)
            {
                pNegotiatedProfile->bSupportAvpf = bNegotiatedAVPF;
            }
        }

        pNegotiatedProfile->objCapaNego.mapTransportCapa.Clear();
        pNegotiatedProfile->objCapaNego.mapAttributeCapa.Clear();
    }

    pNegotiatedProfile->strTransportType =
            (pNegotiatedProfile->bSupportAvpf == IMS_TRUE) ? "RTP/AVPF" : "RTP/AVP";

    IMS_TRACE_D(
            "MakeNegotiatedProfile() - AVPF enable[%d]", pNegotiatedProfile->bSupportAvpf, 0, 0);

    // Step 4. Compare each payload based destination's profile
    VideoProfile::Payload* pNegotiatedPayload = IMS_NULL;
    IMS_SINT32 nNegotiatedMaxFrameRate = 0;
    IMS_SINT32 nNegotiatedMaxAs = 0;

    VideoProfile::Payload* pSrcPayload = IMS_NULL;
    VideoProfile::Payload* pDstPayload = IMS_NULL;
    VideoProfile::Payload* pTmpPayload = IMS_NULL;
    VideoProfile::Payload* pMatchedDstPayload = IMS_NULL;

    for (IMS_UINT32 nDstIndex = 0; nDstIndex < pDestProfile->lstPayload.GetSize(); nDstIndex++)
    {
        pDstPayload = pDestProfile->lstPayload.GetAt(nDstIndex);

        if (pDstPayload == IMS_NULL)
        {
            continue;
        }

        if (pDstPayload->objRtpMap.strPayloadType.Equals("H264"))
        {
            // start source profile loop
            for (IMS_UINT32 nSrcIndex = 0; nSrcIndex < pSrcProfile->lstPayload.GetSize();
                    nSrcIndex++)
            {
                pSrcPayload = pSrcProfile->lstPayload.GetAt(nSrcIndex);
                if (pSrcPayload == IMS_NULL)
                {
                    continue;
                }

                // find matched payload - H264 find options
                if (pSrcPayload->objRtpMap.strPayloadType.Equals("H264"))
                {
                    // FMTP compare
                    VideoProfile::AvcFmtp* pSrcFmtp = (VideoProfile::AvcFmtp*)pSrcPayload->pFmtp;
                    VideoProfile::AvcFmtp* pDstFmtp = (VideoProfile::AvcFmtp*)pDstPayload->pFmtp;

                    if (pSrcFmtp == IMS_NULL || pDstFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() profileLevelID[%s]<->profileLevelID[%s]",
                            pSrcFmtp->strProfileLevelId.GetStr(),
                            pDstFmtp->strProfileLevelId.GetStr(), 0);

                    IMS_TRACE_D("MakeNegotiatedProfile() Level[%d]<->Level[%d]", pSrcFmtp->nLevel,
                            pDstFmtp->nLevel, 0);
                    IMS_TRACE_D("MakeNegotiatedProfile() Profile[%d]<->Profile[%d]",
                            pSrcFmtp->nProfile, pDstFmtp->nProfile, 0);

                    // same level is adapt first, reject higher level
                    if (pSrcFmtp->nLevel < pDstFmtp->nLevel)
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile( NOT MATCHED AVC Level[%d]<->[%d]",
                                pSrcFmtp->nLevel, pDstFmtp->nLevel, 0);

                        if (pTmpPayload == IMS_NULL)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() Accept Highest Temp Src \
                                    profileLevelID[%s]",
                                    pSrcFmtp->strProfileLevelId.GetStr(), 0, 0);
                            pTmpPayload = pSrcPayload;
                            pMatchedDstPayload = pDstPayload;
                        }

                        continue;
                    }
                    else
                    {
                        if (pSrcFmtp->nLevel != pDstFmtp->nLevel)
                        {
                            // if find matching level fmtp, skip unmatched level payload
                            VideoProfile::Payload* pPotentialPayload = IMS_NULL;
                            IMS_BOOL bFoundPayload = IMS_FALSE;

                            for (IMS_UINT32 nIndex = nSrcIndex;
                                    nIndex < pSrcProfile->lstPayload.GetSize(); nIndex++)
                            {
                                pPotentialPayload = pSrcProfile->lstPayload.GetAt(nIndex);

                                if (pPotentialPayload->objRtpMap.strPayloadType.Equals("H264"))
                                {
                                    VideoProfile::AvcFmtp* pPotentialFmtp =
                                            (VideoProfile::AvcFmtp*)pPotentialPayload->pFmtp;

                                    // check level and payload
                                    if (pPotentialFmtp->nLevel == pDstFmtp->nLevel &&
                                            pPotentialFmtp->eResolution == pDstFmtp->eResolution)
                                    {
                                        bFoundPayload = IMS_TRUE;
                                        break;
                                    }
                                }
                            }

                            if (bFoundPayload == IMS_TRUE)
                            {
                                continue;
                            }
                        }
                    }

                    if (pDstFmtp->eResolution == VIDEO_RESOLUTION_NOT_USED)
                    {
                        VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

                        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                                eTempResolution != VIDEO_RESOLUTION_INVALID)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Prev. Negotiated Resolution[%d]",
                                    pDstFmtp->eResolution, eTempResolution, 0);
                            pDstFmtp->eResolution = eTempResolution;
                        }
                        else
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Src Resolution[%d]",
                                    pDstFmtp->eResolution, pDstFmtp->eResolution, 0);

                            pDstFmtp->eResolution = pSrcFmtp->eResolution;
                        }
                    }

                    if (pSrcFmtp->eResolution != pDstFmtp->eResolution)
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() NOT MATCHED Avc Resolution[%d]<->[%d]",
                                pSrcFmtp->eResolution, pDstFmtp->eResolution, 0);

                        if (pSrcFmtp->nLevel >= pDstFmtp->nLevel)
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep profileLevelID[%s]",
                                        pSrcFmtp->strProfileLevelId.GetStr(), 0, 0);
                                pTmpPayload = pSrcPayload;
                                pMatchedDstPayload = pDstPayload;
                            }
                        }
                        else
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep dynamic resolution \
                                        profileLevelID[%s]",
                                        pSrcFmtp->strProfileLevelId.GetStr(), 0, 0);
                                pTmpPayload = pSrcPayload;
                                pMatchedDstPayload = pDstPayload;
                            }
                        }
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - Matched payload found, \
                            Profile[%d], Level[%d], Resolution[%d]",
                            pSrcFmtp->nProfile, pSrcFmtp->nLevel, pSrcFmtp->eResolution);

                    // make nego payload
                    VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

                    if (MakeNegotiatedPayload(pSrcPayload, pDstPayload, pConfig, pNegoPayload) ==
                            IMS_FALSE)
                    {
                        IMS_TRACE_E(
                                0, "MakeNegotiatedProfile() - Cannot Make Nego payload", 0, 0, 0);
                        continue;
                    }

                    // Make a RTCP-FB negotiation result
                    if (pNegotiatedProfile->bSupportAvpf == IMS_TRUE)
                    {
                        if (pSrcPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.nTrrInt =
                                    pDstPayload->objRtcpFbAttr.nTrrInt;
                            pNegoPayload->objRtcpFbAttr.bTrrSupported = IMS_TRUE;
                        }
                        if (pSrcPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                        }
                        if (pSrcPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                            pNegoPayload->objRtcpFbAttr.nTmmbrDownInterval =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrDownInterval;
                            pNegoPayload->objRtcpFbAttr.nTmmbrUpInterval =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrUpInterval;
                            pNegoPayload->objRtcpFbAttr.nTmmbrLossThreshold =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrLossThreshold;
                            pNegoPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio;
                            pNegoPayload->objRtcpFbAttr.nTmmbrBitrateLevel =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrBitrateLevel;
                            pNegoPayload->objRtcpFbAttr.nTmmbrUpLevel =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrUpLevel;
                        }
                        if (pSrcPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                        }
                        if (pSrcPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                        }

                        IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                                bNACK[%d], bTMMBR[%d], bPLI[%d]",
                                pNegoPayload->objRtcpFbAttr.bNackSupported,
                                pNegoPayload->objRtcpFbAttr.bTmmbrSupported,
                                pNegoPayload->objRtcpFbAttr.bPliSupported);
                        IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                                bFIR[%d], bTRR_Int[%d], nTrr-int[%d]",
                                pNegoPayload->objRtcpFbAttr.bFirSupported,
                                pNegoPayload->objRtcpFbAttr.bTrrSupported,
                                pNegoPayload->objRtcpFbAttr.nTrrInt);
                    }

                    VideoProfile::AvcFmtp* fmtp = (VideoProfile::AvcFmtp*)pNegoPayload->pFmtp;

                    if (fmtp == IMS_NULL)
                    {
                        break;
                    }

                    pNegotiatedProfile->lstPayload.Append(pNegoPayload);

                    if (pDestProfile->nNegotiatedPayloadIndex == -1)
                    {
                        pDestProfile->nNegotiatedPayloadIndex = nDstIndex;
                        pSrcProfile->nNegotiatedPayloadIndex = nSrcIndex;

                        // MT case : change src PT# to dest PT#
                        if (bIsOfferReceived == IMS_TRUE &&
                                pSrcProfile->nNegotiatedPayloadIndex != -1)
                        {
                            VideoProfile::Payload* pTempNegoSrcPayload =
                                    pSrcProfile->lstPayload.GetAt(
                                            pSrcProfile->nNegotiatedPayloadIndex);
                            pTempNegoSrcPayload->objRtpMap.nPayloadNum =
                                    pDstPayload->objRtpMap.nPayloadNum;
                        }
                    }

                    if (fmtp->nFrameRate > nNegotiatedMaxFrameRate)
                    {
                        nNegotiatedMaxFrameRate = fmtp->nFrameRate;
                    }
                    if (fmtp->nAs > nNegotiatedMaxAs)
                    {
                        nNegotiatedMaxAs = fmtp->nAs;
                    }

                    break;
                }
            }
        }
        else if (pDstPayload->objRtpMap.strPayloadType.Equals("H265"))
        {
            // start source profile loop
            for (IMS_UINT32 nSrcIndex = 0; nSrcIndex < pSrcProfile->lstPayload.GetSize();
                    nSrcIndex++)
            {
                pSrcPayload = pSrcProfile->lstPayload.GetAt(nSrcIndex);
                if (pSrcPayload == IMS_NULL)
                {
                    continue;
                }

                // find matched payload - H265 find options
                if (pSrcPayload->objRtpMap.strPayloadType.Equals("H265"))
                {
                    // FMTP compare
                    VideoProfile::HevcFmtp* pSrcFmtp = (VideoProfile::HevcFmtp*)pSrcPayload->pFmtp;
                    VideoProfile::HevcFmtp* pDstFmtp = (VideoProfile::HevcFmtp*)pDstPayload->pFmtp;
                    if (pSrcFmtp == IMS_NULL || pDstFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - profileId[%d]<->profileId[%d]",
                            pSrcFmtp->nProfile, pDstFmtp->nProfile, 0);

                    // same level is adapt first, reject higher level
                    if (pSrcFmtp->nLevel < pDstFmtp->nLevel)
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() - NOT MATCHED HEVC Level[%d]<->[%d]",
                                pSrcFmtp->nLevel, pDstFmtp->nLevel, 0);

                        if (pTmpPayload == IMS_NULL)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Accept Highest Temp Src \
                                    profileID[%d]",
                                    pSrcFmtp->nProfile, 0, 0);
                            pTmpPayload = pSrcPayload;
                            pMatchedDstPayload = pDstPayload;
                        }
                        continue;
                    }

                    if (pDstFmtp->eResolution == VIDEO_RESOLUTION_NOT_USED)
                    {
                        VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

                        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                                eTempResolution != VIDEO_RESOLUTION_INVALID)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Prev. Negotiated Resolution[%d]",
                                    pDstFmtp->eResolution, eTempResolution, 0);
                            pDstFmtp->eResolution = eTempResolution;
                        }
                        else
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Src Resolution[%d]",
                                    pDstFmtp->eResolution, pDstFmtp->eResolution, 0);
                            pDstFmtp->eResolution = pSrcFmtp->eResolution;
                        }
                    }

                    if (pSrcFmtp->eResolution != pDstFmtp->eResolution)
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() - NOT MATCHED HEVC Resolution\
                                [%d]<->[%d]",
                                pSrcFmtp->eResolution, pDstFmtp->eResolution, 0);

                        if (pSrcFmtp->nLevel >= pDstFmtp->nLevel)
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep profile[%d]",
                                        pSrcFmtp->nProfile, 0, 0);
                                pTmpPayload = pSrcPayload;
                                pMatchedDstPayload = pDstPayload;
                            }
                        }
                        else
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep dynamic resolution \
                                        profileID[%d]",
                                        pSrcFmtp->nProfile, 0, 0);
                                pTmpPayload = pSrcPayload;
                                pMatchedDstPayload = pDstPayload;
                            }
                        }
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - Matched payload found, \
                            Profile[%d], Level[%d], Resolution[%d]",
                            pSrcFmtp->nProfile, pSrcFmtp->nLevel, pSrcFmtp->eResolution);

                    // make nego payload
                    VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

                    if (MakeNegotiatedPayload(pSrcPayload, pDstPayload, pConfig, pNegoPayload) ==
                            IMS_FALSE)
                    {
                        IMS_TRACE_E(0, "MakeNegotiatedProfile() Cannot Make Nego payload", 0, 0, 0);
                        continue;
                    }

                    // Make a RTCP-FB negotiation result
                    if (pNegotiatedProfile->bSupportAvpf == IMS_TRUE)
                    {
                        if (pSrcPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                        }
                        if (pSrcPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                            pNegoPayload->objRtcpFbAttr.nTmmbrDownInterval =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrDownInterval;
                            pNegoPayload->objRtcpFbAttr.nTmmbrUpInterval =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrUpInterval;
                            pNegoPayload->objRtcpFbAttr.nTmmbrLossThreshold =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrLossThreshold;
                            pNegoPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio;
                            pNegoPayload->objRtcpFbAttr.nTmmbrBitrateLevel =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrBitrateLevel;
                            pNegoPayload->objRtcpFbAttr.nTmmbrUpLevel =
                                    pSrcPayload->objRtcpFbAttr.nTmmbrUpLevel;
                        }
                        if (pSrcPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                        }
                        if (pSrcPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE &&
                                pDstPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                        }

                        IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                                NACK[%d], TMMBR[%d], PLI[%d]",
                                pNegoPayload->objRtcpFbAttr.bNackSupported,
                                pNegoPayload->objRtcpFbAttr.bTmmbrSupported,
                                pNegoPayload->objRtcpFbAttr.bPliSupported);
                        IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. FIR[%d]",
                                pNegoPayload->objRtcpFbAttr.bFirSupported, 0, 0);
                    }

                    VideoProfile::HevcFmtp* fmtp = (VideoProfile::HevcFmtp*)pNegoPayload->pFmtp;

                    if (fmtp == IMS_NULL)
                    {
                        break;
                    }

                    pNegotiatedProfile->lstPayload.Append(pNegoPayload);

                    if (pDestProfile->nNegotiatedPayloadIndex == -1)
                    {
                        pDestProfile->nNegotiatedPayloadIndex = nDstIndex;
                        pSrcProfile->nNegotiatedPayloadIndex = nSrcIndex;

                        // MT case : change src PT# to dest PT#
                        if (bIsOfferReceived == IMS_TRUE &&
                                pSrcProfile->nNegotiatedPayloadIndex != -1)
                        {
                            VideoProfile::Payload* pTempNegoSrcPayload =
                                    pSrcProfile->lstPayload.GetAt(
                                            pSrcProfile->nNegotiatedPayloadIndex);
                            pTempNegoSrcPayload->objRtpMap.nPayloadNum =
                                    pDstPayload->objRtpMap.nPayloadNum;
                        }
                    }

                    if (fmtp->nFrameRate > nNegotiatedMaxFrameRate)
                    {
                        nNegotiatedMaxFrameRate = fmtp->nFrameRate;
                    }
                    if (fmtp->nAs > nNegotiatedMaxAs)
                    {
                        nNegotiatedMaxAs = fmtp->nAs;
                    }

                    break;
                }
            }
        }
        else
        {
            IMS_TRACE_D("MakeNegotiatedProfile() UNSUPPORTED codec[%s]",
                    pDstPayload->objRtpMap.strPayloadType.GetStr(), 0, 0);
        }
    }

    if (pNegotiatedProfile->lstPayload.GetSize() > 0)
    {
        pNegotiatedPayload = pNegotiatedProfile->lstPayload.GetAt(0);
    }
    else  // negotiated payload is not exist, use temporary payload
    {
        if (pTmpPayload != IMS_NULL)
        {
            VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();
            if (MakeNegotiatedPayload(pTmpPayload, pMatchedDstPayload, pConfig, pNegoPayload) ==
                    IMS_FALSE)
            {
                IMS_TRACE_E(0, "MakeNegotiatedProfile() - Cannot Make Nego payload", 0, 0, 0);
                return IMS_FALSE;
            }

            if (pMatchedDstPayload->objRtpMap.strPayloadType.Equals("H264"))
            {
                VideoProfile::AvcFmtp* pAvcFmtp = (VideoProfile::AvcFmtp*)pNegoPayload->pFmtp;

                if (pAvcFmtp == IMS_NULL)
                {
                    return IMS_FALSE;
                }

                VIDEO_RESOLUTION nProperResol = GetAvcMaxResolutionFromLevel(pAvcFmtp->nLevel);

                // if the set resolution is invalid or too big with level,
                // decide resolution via payload pre-set and negotatied level
                IMS_BOOL bFoundResol = IMS_FALSE;

                // first decide with source profile payload
                for (IMS_UINT32 nSrcIndex = 0; nSrcIndex < pSrcProfile->lstPayload.GetSize();
                        nSrcIndex++)
                {
                    VideoProfile::Payload* pSrcPayload = pSrcProfile->lstPayload.GetAt(nSrcIndex);
                    VideoProfile::AvcFmtp* pTempSrcFmtp =
                            reinterpret_cast<VideoProfile::AvcFmtp*>(pSrcPayload->pFmtp);

                    if (pTempSrcFmtp->nLevel <= pAvcFmtp->nLevel)
                    {
                        pAvcFmtp->eResolution = pTempSrcFmtp->eResolution;
                        bFoundResol = IMS_TRUE;
                        break;
                    }
                }

                // decide by level
                if (bFoundResol == IMS_FALSE)
                {
                    pAvcFmtp->eResolution = nProperResol;
                }

                IMS_TRACE_D("MakeNegotiatedProfile() Answer Highest payload[%s], nProperResol[%d], \
                        set Resol[%d]",
                        pAvcFmtp->strProfileLevelId.GetStr(), nProperResol, pAvcFmtp->eResolution);

                if (pNegotiatedProfile->bSupportAvpf == IMS_TRUE)
                {
                    if (pSrcPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.nTrrInt = pDstPayload->objRtcpFbAttr.nTrrInt;
                        pNegoPayload->objRtcpFbAttr.bTrrSupported = IMS_TRUE;
                    }
                    if (pSrcPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                    }
                    if (pSrcPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                        pNegoPayload->objRtcpFbAttr.nTmmbrDownInterval =
                                pSrcPayload->objRtcpFbAttr.nTmmbrDownInterval;
                        pNegoPayload->objRtcpFbAttr.nTmmbrUpInterval =
                                pSrcPayload->objRtcpFbAttr.nTmmbrUpInterval;
                        pNegoPayload->objRtcpFbAttr.nTmmbrLossThreshold =
                                pSrcPayload->objRtcpFbAttr.nTmmbrLossThreshold;
                        pNegoPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio =
                                pSrcPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio;
                        pNegoPayload->objRtcpFbAttr.nTmmbrBitrateLevel =
                                pSrcPayload->objRtcpFbAttr.nTmmbrBitrateLevel;
                        pNegoPayload->objRtcpFbAttr.nTmmbrUpLevel =
                                pSrcPayload->objRtcpFbAttr.nTmmbrUpLevel;
                    }
                    if (pSrcPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                    }
                    if (pSrcPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                            bNACK[%d], bTMMBR[%d], bPLI[%d]",
                            pNegoPayload->objRtcpFbAttr.bNackSupported,
                            pNegoPayload->objRtcpFbAttr.bTmmbrSupported,
                            pNegoPayload->objRtcpFbAttr.bPliSupported);
                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                            bFIR[%d], bTRR_Int[%d], nTrr-int[%d]",
                            pNegoPayload->objRtcpFbAttr.bFirSupported,
                            pNegoPayload->objRtcpFbAttr.bTrrSupported,
                            pNegoPayload->objRtcpFbAttr.nTrrInt);
                }

                if (pDestProfile->nNegotiatedPayloadIndex == -1)
                {
                    pDestProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pDestProfile, pMatchedDstPayload);
                    pSrcProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pSrcProfile, pTmpPayload);

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE && pSrcProfile->nNegotiatedPayloadIndex != -1)
                    {
                        VideoProfile::Payload* pTempNegoSrcPayload =
                                pSrcProfile->lstPayload.GetAt(pSrcProfile->nNegotiatedPayloadIndex);
                        pTempNegoSrcPayload->objRtpMap.nPayloadNum =
                                pDstPayload->objRtpMap.nPayloadNum;
                    }
                }

                pNegotiatedProfile->lstPayload.Append(pNegoPayload);
                pNegotiatedPayload = pNegoPayload;

                if (pAvcFmtp->nFrameRate > nNegotiatedMaxFrameRate)
                {
                    nNegotiatedMaxFrameRate = pAvcFmtp->nFrameRate;
                }
                if (pAvcFmtp->nAs > nNegotiatedMaxAs)
                {
                    nNegotiatedMaxAs = pAvcFmtp->nAs;
                }
            }
            else if (pMatchedDstPayload->objRtpMap.strPayloadType.Equals("H265"))
            {
                // Make a RTCP-FB negotiation result
                if (pNegotiatedProfile->bSupportAvpf == IMS_TRUE)
                {
                    if (pSrcPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                    }
                    if (pSrcPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                        pNegoPayload->objRtcpFbAttr.nTmmbrDownInterval =
                                pSrcPayload->objRtcpFbAttr.nTmmbrDownInterval;
                        pNegoPayload->objRtcpFbAttr.nTmmbrUpInterval =
                                pSrcPayload->objRtcpFbAttr.nTmmbrUpInterval;
                        pNegoPayload->objRtcpFbAttr.nTmmbrLossThreshold =
                                pSrcPayload->objRtcpFbAttr.nTmmbrLossThreshold;
                        pNegoPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio =
                                pSrcPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio;
                        pNegoPayload->objRtcpFbAttr.nTmmbrBitrateLevel =
                                pSrcPayload->objRtcpFbAttr.nTmmbrBitrateLevel;
                        pNegoPayload->objRtcpFbAttr.nTmmbrUpLevel =
                                pSrcPayload->objRtcpFbAttr.nTmmbrUpLevel;
                    }
                    if (pSrcPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                    }
                    if (pSrcPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE &&
                            pDstPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                            NACK[%d], TMMBR[%d], PLI[%d]",
                            pNegoPayload->objRtcpFbAttr.bNackSupported,
                            pNegoPayload->objRtcpFbAttr.bTmmbrSupported,
                            pNegoPayload->objRtcpFbAttr.bPliSupported);
                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. FIR[%d]",
                            pNegoPayload->objRtcpFbAttr.bFirSupported, 0, 0);
                }

                VideoProfile::HevcFmtp* fmtp =
                        reinterpret_cast<VideoProfile::HevcFmtp*>(pNegoPayload->pFmtp);
                if (fmtp == IMS_NULL)
                {
                    return IMS_FALSE;
                }

                VideoProfile::HevcFmtp* pTempSrcFmtp =
                        reinterpret_cast<VideoProfile::HevcFmtp*>(pMatchedDstPayload->pFmtp);
                fmtp->eResolution = pTempSrcFmtp->eResolution;

                if (pDestProfile->nNegotiatedPayloadIndex == -1)
                {
                    pDestProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pDestProfile, pMatchedDstPayload);
                    pSrcProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pSrcProfile, pTmpPayload);

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE && pSrcProfile->nNegotiatedPayloadIndex != -1)
                    {
                        VideoProfile::Payload* pTempNegoSrcPayload =
                                pSrcProfile->lstPayload.GetAt(pSrcProfile->nNegotiatedPayloadIndex);
                        pTempNegoSrcPayload->objRtpMap.nPayloadNum =
                                pDstPayload->objRtpMap.nPayloadNum;
                    }
                }

                pNegotiatedProfile->lstPayload.Append(pNegoPayload);
                pNegotiatedPayload = pNegoPayload;

                if (fmtp->nFrameRate > nNegotiatedMaxFrameRate)
                {
                    nNegotiatedMaxFrameRate = fmtp->nFrameRate;
                }
                if (fmtp->nAs > nNegotiatedMaxAs)
                {
                    nNegotiatedMaxAs = fmtp->nAs;
                }
            }
        }
    }

    if (pNegotiatedPayload != IMS_NULL)
    {
        if (pNegotiatedProfile->nDataPort == 0 || pDestProfile->nDataPort == 0 ||
                pNegotiatedProfile->lstPayload.GetSize() == 0)
        {
            pNegotiatedProfile->eDirection = MEDIA_DIRECTION_INVALID;
        }
        else
        {
            pNegotiatedProfile->eDirection = UpdateDirectionToMine(
                    pDestProfile->eDirection, pSrcProfile->eDirection, bIsOfferReceived);
        }

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

        // Step 6. Setting bandwidth AS/RS/RR
        VideoProfileConfigurer::MakeNegotiatedBandwidth(pConfig, pSrcProfile, pDestProfile,
                bIsOfferReceived, nNegotiatedMaxAs, pNegotiatedProfile);

        // Step 7. Setting framerate
        pNegotiatedProfile->nFrameRate = nNegotiatedMaxFrameRate;

        // Step 8. Candidate Priority (no need in video)

        // Step 9. CVO mode
        if (pSrcProfile->nCvoId > 0 && pDestProfile->nCvoId > 0)
        {
            pNegotiatedProfile->nCvoId = pDestProfile->nCvoId;
        }
        else
        {
            if (pNegotiatedProfile->nDataPort == 0)
            {
                pNegotiatedProfile->nCvoId = pSrcProfile->nCvoId;
            }
            else
            {
                pNegotiatedProfile->nCvoId = 0;
            }
        }

        IMS_TRACE_D("MakeNegotiatedProfile() nCvoId[%d]", pNegotiatedProfile->nCvoId, 0, 0);

        ret = IMS_TRUE;
    }
    else
    {
        if (pSrcProfile->lstPayload.GetSize() > 0)
        {
            IMS_TRACE_D("MakeNegotiatedProfile() There's no negotiated payload. \
                    copy SrcProfile and make port 0 ",
                    0, 0, 0);

            pNegotiatedProfile->Copy(pSrcProfile);
            pNegotiatedProfile->nDataPort = 0;
            pNegotiatedProfile->eDirection = MEDIA_DIRECTION_INVALID;
            ret = IMS_TRUE;
        }
        else
        {
            IMS_TRACE_E(0, "There's no Payload in Src Profile", 0, 0, 0);
        }
    }

    IMS_TRACE_D("MakeNegotiatedProfile() Ended - Negotiated srcIndex[%d], destIndex[%d]",
            pSrcProfile->nNegotiatedPayloadIndex, pDestProfile->nNegotiatedPayloadIndex, 0);

    return ret;
}

PROTECTED
IMS_BOOL VideoNego::GetFmtpFromString(IN AString strFmtp, OUT VideoProfile::AvcFmtp* pFmtp)
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
            const AString& strTmp = objSplitColon.GetAt(i);
            IMS_TRACE_D("GetFmtpFromString() - Invalid AVC fmtp parameter(%s) at index(%d)",
                    strTmp.GetStr(), i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (objSplitEqual.GetAt(0).Equals("profile-level-id") == IMS_TRUE)
        {
            pFmtp->strProfileLevelId = objSplitEqual.GetAt(1);
            pFmtp->nProfile = VideoProfileConfigurer::GetAvcProfileFromProfileLevelId(
                    pFmtp->strProfileLevelId);
            pFmtp->nLevel =
                    VideoProfileConfigurer::GetAvcLevelFromProfileLevelId(pFmtp->strProfileLevelId);
            pFmtp->bShow_ProfileLevelId = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("packetization-mode") == IMS_TRUE)
        {
            pFmtp->nPacketizationMode = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShow_PacketizationMode = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("sprop-parameter-sets") == IMS_TRUE)
        {
            IMS_SINT32 nIndexOf1stEqual = objSplitColon.GetAt(i).GetIndexOf("=");
            AString strRealSpropParam = objSplitColon.GetAt(i).GetSubStr(nIndexOf1stEqual + 1);

            IMSList<AString> objSplitComma = strRealSpropParam.Split(',');
            if (objSplitComma.GetSize() < 2)
            {
                IMS_TRACE_E(
                        0, "GetFmtpFromString() - objSplitComma's size less than 2 !!!", 0, 0, 0);
                continue;
            }
            else if ((objSplitComma.GetAt(0).GetLength() % 4 != 0) &&
                    (objSplitComma.GetAt(1).GetLength() % 4 != 0))
            {
                IMS_TRACE_E(0, "GetFmtpFromString() - Sprop Length Error - SPS[%d], PPS[%d]",
                        objSplitComma.GetAt(0).GetLength(), objSplitComma.GetAt(1).GetLength(), 0);
                continue;
            }

            pFmtp->strSpropParam = strRealSpropParam;
            pFmtp->bShow_SpropParam = IMS_TRUE;
        }
        else
        {
            IMS_TRACE_D("GetFmtpFromString() - Not Parsable [%s]", objSplitEqual.GetAt(0).GetStr(),
                    0, 0);
        }
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL VideoNego::GetFmtpFromString(IN AString strFmtp, OUT VideoProfile::HevcFmtp* pFmtp)
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
            const AString& strTmp = objSplitColon.GetAt(i);

            IMS_TRACE_D("GetFmtpFromString() - Invalid HEVC fmtp parameter(%s) at index(%d)",
                    strTmp.GetStr(), i, 0);

            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (objSplitEqual.GetAt(0).Equals("profile-id") == IMS_TRUE)
        {
            pFmtp->nProfile = (VIDEO_PROFILE_HEVC)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShow_Profile = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("level-id") == IMS_TRUE)
        {
            pFmtp->nLevel = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShow_Level = IMS_TRUE;
        }
        else if (objSplitEqual.GetAt(0).Equals("sprop-vps") == IMS_TRUE)
        {
            pFmtp->strVps = objSplitEqual.GetAt(1);
        }
        else if (objSplitEqual.GetAt(0).Equals("sprop-sps") == IMS_TRUE)
        {
            pFmtp->strSps = objSplitEqual.GetAt(1);
        }
        else if (objSplitEqual.GetAt(0).Equals("sprop-pps") == IMS_TRUE)
        {
            pFmtp->strPps = objSplitEqual.GetAt(1);
        }
        else if (objSplitEqual.GetAt(0).Equals("packetization-mode") == IMS_TRUE)
        {
            pFmtp->nPacketizationMode = (IMS_UINT32)objSplitEqual.GetAt(1).ToInt32();
            pFmtp->bShow_PacketizationMode = IMS_TRUE;
        }
        else
        {
            IMS_TRACE_D("GetFmtpFromString() - Not Parsable [%s]", objSplitEqual.GetAt(0).GetStr(),
                    0, 0);
        }
    }

    if (!pFmtp->strVps.IsNULL() && !pFmtp->strSps.IsNULL() && !pFmtp->strPps.IsNULL())
    {
        AString strTemp;
        strTemp.Append(pFmtp->strVps);
        strTemp.Append(",");
        strTemp.Append(pFmtp->strSps);
        strTemp.Append(",");
        strTemp.Append(pFmtp->strPps);

        pFmtp->strSpropParam = strTemp;
        pFmtp->bShow_SpropParam = IMS_TRUE;
    }

    return IMS_TRUE;
}

PROTECTED
VideoProfile::Payload* VideoNego::FindPayloadInProfile(
        IN VideoProfile* pProfile, IN VideoProfile::Payload* pTargetPayload)
{
    if (pProfile == IMS_NULL || pTargetPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    VideoProfile::Payload* pOriginPayload = IMS_NULL;  // payload from profile which whil be checked
    VideoProfile::Payload* pTempPayload = IMS_NULL;    // To keep secondary payload

    VideoConfiguration* pConfig = GetConfig();
    if (pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        pOriginPayload = pProfile->lstPayload.GetAt(i);
        if (pOriginPayload == IMS_NULL)
        {
            continue;
        }

        if ((pOriginPayload->objRtpMap.strPayloadType.Equals(
                    pTargetPayload->objRtpMap.strPayloadType)) &&
                (pOriginPayload->objRtpMap.nSamplingRate ==
                        pTargetPayload->objRtpMap.nSamplingRate))
        {
            if (pOriginPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
            {
                VideoProfile::AvcFmtp* pOriginFmtp = (VideoProfile::AvcFmtp*)pOriginPayload->pFmtp;
                VideoProfile::AvcFmtp* pReceivedFmtp =
                        (VideoProfile::AvcFmtp*)pTargetPayload->pFmtp;
                if (pOriginFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                // same level is adapt first
                IMS_TRACE_D("FindPayloadInProfile() - profileLevelID[%s]<->profileLevelID[%s]",
                        pOriginFmtp->strProfileLevelId.GetStr(),
                        pReceivedFmtp->strProfileLevelId.GetStr(), 0);

                if (pOriginFmtp->nLevel < pReceivedFmtp->nLevel)
                {
                    IMS_TRACE_D("FindPayloadInProfile() - NOT MATCHED AVC Level[%d]<->[%d]",
                            pOriginFmtp->nLevel, pReceivedFmtp->nLevel, 0);

                    if (pTempPayload == IMS_NULL)
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Priority profileLevelID[%d]",
                                pOriginFmtp->strProfileLevelId.GetStr(), 0, 0);
                        pTempPayload = pOriginPayload;
                    }
                    continue;
                }

                if (pReceivedFmtp->eResolution == VIDEO_RESOLUTION_NOT_USED)
                {
                    VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution(IMS_TRUE);

                    if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                            eTempResolution != VIDEO_RESOLUTION_INVALID)
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Far Resolution is not specified[%d]\
                                -> Temp use Prev. Negotiated Resolution[%d]",
                                pReceivedFmtp->eResolution, eTempResolution, 0);

                        pReceivedFmtp->eResolution = eTempResolution;
                    }
                    else
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Far Resolution is not specified[%d]\
                                -> Temp use Src Resolution[%d]",
                                pReceivedFmtp->eResolution, pOriginFmtp->eResolution, 0);

                        pReceivedFmtp->eResolution = pOriginFmtp->eResolution;
                    }
                }

                if (pOriginFmtp->eResolution != pReceivedFmtp->eResolution)
                {
                    // Keep 1st payload(resolution mismatched) to be used
                    // when no strictly matched resolution is found
                    pTempPayload = pOriginPayload;

                    IMS_TRACE_D("FindPayloadInProfile() - NOT MATCHED AVC Resolution[%d]<->[%d]",
                            pOriginFmtp->eResolution, pReceivedFmtp->eResolution, 0);
                    continue;
                }
                else if (pOriginFmtp->nLevel != pReceivedFmtp->nLevel)
                {
                    pTempPayload = pOriginPayload;
                    continue;
                }

                IMS_TRACE_D("FindPayloadInProfile() Found, Profile[%d], Level[%d], Resolution[%d]",
                        pOriginFmtp->nProfile, pOriginFmtp->nLevel, pOriginFmtp->eResolution);

                return pOriginPayload;
            }
            else if (pOriginPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
            {
                VideoProfile::HevcFmtp* pOriginFmtp =
                        (VideoProfile::HevcFmtp*)pOriginPayload->pFmtp;
                VideoProfile::HevcFmtp* pReceivedFmtp =
                        (VideoProfile::HevcFmtp*)pTargetPayload->pFmtp;
                if (pOriginFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                // same level is adapt first
                IMS_TRACE_D("FindPayloadInProfile() - profileID[%d] <-> profileID[%d]",
                        pOriginFmtp->nProfile, pReceivedFmtp->nProfile, 0);

                if (pOriginFmtp->nLevel < pReceivedFmtp->nLevel)
                {
                    IMS_TRACE_D("FindPayloadInProfile() - NOT MATCHED HEVC Level [%d]<->[%d]",
                            pOriginFmtp->nLevel, pReceivedFmtp->nLevel, 0);
                }

                if (pReceivedFmtp->eResolution == VIDEO_RESOLUTION_NOT_USED)
                {
                    VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution(IMS_TRUE);

                    if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                            eTempResolution != VIDEO_RESOLUTION_INVALID)
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Far Resolution is not specified[%d]\
                                -> Temp use Prev. Negotiated Resolution[%d]",
                                pReceivedFmtp->eResolution, eTempResolution, 0);

                        pReceivedFmtp->eResolution = eTempResolution;
                    }
                    else
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Far Resolution is not specified[%d]\
                                -> Temp use Src Resolution[%d]",
                                pReceivedFmtp->eResolution, pOriginFmtp->eResolution, 0);

                        pReceivedFmtp->eResolution = pOriginFmtp->eResolution;
                    }
                }

                if (pOriginFmtp->eResolution != pReceivedFmtp->eResolution)
                {
                    // Keep 1st payload(resolution mismatched) to be used
                    // when no strictly matched resolution is found
                    pTempPayload = pOriginPayload;

                    IMS_TRACE_D("FindPayloadInProfile() - NOT MATCHED HEVC Resolution [%d]<->[%d]",
                            pOriginFmtp->eResolution, pReceivedFmtp->eResolution, 0);
                    continue;
                }
                else if (pOriginFmtp->nLevel != pReceivedFmtp->nLevel)
                {
                    pTempPayload = pOriginPayload;
                    continue;
                }

                IMS_TRACE_D("FindPayloadInProfile() Found, Profile[%d], Level[%d], Resolution[%d]",
                        pOriginFmtp->nProfile, pOriginFmtp->nLevel, pOriginFmtp->eResolution);

                return pOriginPayload;
            }
        }
    }

    // When there's no perfectly matched payload, use secondary (only resolution is mismatched)
    if (pTempPayload != IMS_NULL && pTempPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
    {
        VideoProfile::AvcFmtp* pOriginFmtp = (VideoProfile::AvcFmtp*)pTempPayload->pFmtp;
        VideoProfile::AvcFmtp* pReceivedFmtp = (VideoProfile::AvcFmtp*)pTargetPayload->pFmtp;
        if (pOriginFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
        {
            return IMS_NULL;
        }

        if (pOriginFmtp->eResolution != pReceivedFmtp->eResolution)
        {
            IMS_TRACE_D("FindPayloadInProfile() - Accept mismatched Resolution[%d]<->[%d]",
                    pOriginFmtp->eResolution, pReceivedFmtp->eResolution, 0);

            if (pOriginFmtp->nLevel >= pReceivedFmtp->nLevel)
            {
                pOriginFmtp->eResolution = pReceivedFmtp->eResolution;
            }
        }
        else if (pOriginFmtp->nLevel != pReceivedFmtp->nLevel)
        {
            IMS_TRACE_D("FindPayloadInProfile() - Accept lower Level[%d]<->[%d]",
                    pOriginFmtp->nLevel, pReceivedFmtp->nLevel, 0);
        }

        return pTempPayload;
    }
    if (pTempPayload != IMS_NULL && pTempPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
    {
        VideoProfile::HevcFmtp* pOriginFmtp = (VideoProfile::HevcFmtp*)pTempPayload->pFmtp;
        VideoProfile::HevcFmtp* pReceivedFmtp = (VideoProfile::HevcFmtp*)pTargetPayload->pFmtp;
        if (pOriginFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
            return IMS_NULL;

        if (pOriginFmtp->eResolution != pReceivedFmtp->eResolution)
        {
            IMS_TRACE_D("FindPayloadInProfile() - Accept mismatched Resolution [%d]<->[%d]",
                    pOriginFmtp->eResolution, pReceivedFmtp->eResolution, 0);

            if (pOriginFmtp->nLevel >= pReceivedFmtp->nLevel)
            {
                pOriginFmtp->eResolution = pReceivedFmtp->eResolution;
            }
        }
        else if (pOriginFmtp->nLevel != pReceivedFmtp->nLevel)
        {
            IMS_TRACE_D("FindPayloadInProfile() - Accept lower Level[%d]<->[%d]",
                    pOriginFmtp->nLevel, pReceivedFmtp->nLevel, 0);
        }

        return pTempPayload;
    }

    IMS_TRACE_E(0, "FindPayloadInProfile() - No matched payload Found", 0, 0, 0);
    return IMS_NULL;
}

PROTECTED
IMS_SINT32 VideoNego::FindPayloadIndexFromProfile(
        IN VideoProfile* pProfile, IN VideoProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindPayloadInProfile() - Null Input", 0, 0, 0);
        return -1;
    }

    // find the index of negotiated payload
    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        VideoProfile::Payload* comparedPayload = pProfile->lstPayload.GetAt(i);
        if (comparedPayload == IMS_NULL)
            continue;

        if (comparedPayload == pPayload)
        {
            IMS_TRACE_D("FindPayloadIndexFromProfile() - FindIndex[%d]", i, 0, 0);
            return i;
        }
    }

    return -1;
}

PROTECTED
MEDIA_DIRECTION VideoNego::UpdateDirectionToMine(
        IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase)
{
    IMS_TRACE_D("UpdateDirectionToMine() Entered. ePeerDir[%d], eSrcDir[%d], bIsMtCase[%d]",
            ePeerDir, eSrcDir, bIsMtCase);
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
        if (eSrcDir == MEDIA_DIRECTION_SEND &&
                (ePeerDir == MEDIA_DIRECTION_SEND || ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE))
        {
            return MEDIA_DIRECTION_INVALID;
        }
        else if (eSrcDir == MEDIA_DIRECTION_RECEIVE &&
                (ePeerDir == MEDIA_DIRECTION_RECEIVE || ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE))
        {
            return MEDIA_DIRECTION_INVALID;
        }
        else if (eSrcDir == MEDIA_DIRECTION_INACTIVE && (ePeerDir != MEDIA_DIRECTION_INACTIVE))
        {
            return MEDIA_DIRECTION_INVALID;
        }
    }
    return eNegotiatedDir;
}

PROTECTED IMS_BOOL VideoNego::GetCorrectImageIndex(
        IN IMS_SINT32 nPayloadTypeNum, IN IMSList<AString> objAttributes, OUT IMS_UINT32* nIndex)
{
    for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); i++)
    {
        IMSList<AString> objTokens = objAttributes.GetAt(i).Split(TextParser::CHAR_SP);

        if (objTokens.GetSize() < 2)
        {
            return IMS_FALSE;
        }

        // add a case of wild-card
        if (objTokens.GetAt(0).Equals("*"))
        {
            IMS_TRACE_D("GetCorrectImageIndex()-wild-card - nIndex[%d], nPayloadNum[%d]", *nIndex,
                    nPayloadTypeNum, 0);
            return IMS_TRUE;
        }

        if (nPayloadTypeNum == objTokens.GetAt(0).ToInt32())
        {
            *nIndex = i;
            IMS_TRACE_D("GetCorrectImageIndex()-nIndex[%d], nPayloadNum[%d]", *nIndex,
                    nPayloadTypeNum, 0);

            return IMS_TRUE;
        }
    }

    IMS_TRACE_E(0, "GetCorrectImageIndex()-No matched", 0, 0, 0);

    return IMS_FALSE;
}

PROTECTED VIDEO_RESOLUTION VideoNego::GetResolutionFromSdp(IN VIDEO_CODEC codecType,
        IN AString strImageAttrFromSdp, IN AString strFrameSizeFromSdp, IN AString strSpropParam,
        IN IMS_SINT32 nQcif)
{
    IMS_UINT32 nWidth, nHeight;

    // 1. - Get nWidth, nHeight From Image Attribute
    if (!strImageAttrFromSdp.IsEmpty() && !strImageAttrFromSdp.IsNULL() &&
            (GetWidthHeightFromSdp_ImageAttr(strImageAttrFromSdp, &nWidth, &nHeight) != IMS_FALSE))
    {
        return GetResolutionFromWidthHeight(nWidth, nHeight);
    }

    // TODO_MEDIA video sprop
    (void)codecType;
    (void)strSpropParam;
    // // 2. - Get nWidth, nHeight From SpropParam
    // if (!strSpropParam.IsEmpty() && !strSpropParam.IsNULL() &&
    //         (GetWidthHeightFromSdp_SpropParam(
    //                  codecType, strSpropParam.GetStr(), &nWidth, &nHeight) != IMS_FALSE))
    // {
    //     return GetResolutionFromWidthHeight(nWidth, nHeight);
    // }

    // 3. - Get nWidth, nHeight From Framesize
    if (!strFrameSizeFromSdp.IsEmpty() && !strFrameSizeFromSdp.IsNULL() &&
            (GetWidthHeightFromSdp_FrameSize(strFrameSizeFromSdp, &nWidth, &nHeight) != IMS_FALSE))
    {
        return GetResolutionFromWidthHeight(nWidth, nHeight);
    }

    // 4. - Check if nQcif exist
    else if (nQcif != -1)
    {
        return VIDEO_RESOLUTION_QCIF_LS;
    }
    // 5. Return Default Resolution
    else
    {
        IMS_TRACE_E(0, "GetResolutionFromSdp() - No preferred resolution from SDP...", 0, 0, 0);
        return VIDEO_RESOLUTION_NOT_USED;
    }
}

PROTECTED IMS_BOOL VideoNego::GetWidthHeightFromSdp_ImageAttr(
        IN AString strImageAttrFromSdp, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    IMS_UINT32 nImagePayloadNum = 0;  // Payload Number in Image Attr
    IMS_UINT32 nDirection = 1;        // Direction : send or recv
    IMS_UINT32 nImageValueIndex = 2;  // Image value : width and height
    IMSList<AString> objTokens;
    IMSList<AString> strTempValue;
    AString nRealValueString = AString::ConstNull();
    // Check SPACE is ...
    if (strImageAttrFromSdp.Contains(TextParser::CHAR_SP) == IMS_FALSE)
    {
        IMS_TRACE_E(0,
                "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have CHAR_SP, ...", 0,
                0, 0);
        return IMS_FALSE;
    }
    else
    {
        objTokens = strImageAttrFromSdp.Split(TextParser::CHAR_SP);

        if (objTokens.GetSize() < 3)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - the size of Peer ImageAttr is too small "
                    "to parse it ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
    }

    // 1st step : Take "Payload Number" out of Image Attribute.
    nImagePayloadNum = objTokens.GetAt(0).ToInt32();

    // 2nd step : Take "Image Size Values" out of Image Size according the Send/Recv direction.
    if (objTokens.GetAt(nDirection).EqualsIgnoreCase("send") == IMS_TRUE)
    {
        if (objTokens.GetAt(nImageValueIndex).Equals("*") == IMS_TRUE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - PeerResolution-Wildcard, Not Support", 0,
                    0, 0);
            return IMS_FALSE;
        }
        // remove spaces in Bracket, [ ]
        for (IMS_UINT32 i = nImageValueIndex; i < objTokens.GetSize(); i++)
        {
            if (objTokens.GetAt(i).EqualsIgnoreCase("recv") == IMS_TRUE)
            {
                // IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() Removed all spaces in send
                // direction Bracket", 0, 0, 0);
                break;
            }
            nRealValueString.Append(objTokens.GetAt(i));
        }

        // Check LSBRACKET is ...
        if ((nRealValueString == AString::ConstNull()) ||
                (nRealValueString.Contains(TextParser::CHAR_LSBRACKET) == IMS_FALSE))
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                    "CHAR_LSBRACKET, [ ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = nRealValueString.Split(TextParser::CHAR_LSBRACKET);
        }

        // Check RSBRACKET is ...
        if (strTempValue.GetAt(1).Contains(TextParser::CHAR_RSBRACKET) == IMS_FALSE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                    "CHAR_RSBRACKET, ] ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(1).Split(TextParser::CHAR_RSBRACKET);
        }

        // Check COMMA is ...
        if (strTempValue.GetAt(0).Contains(TextParser::CHAR_COMMA) == IMS_FALSE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have a CHAR_COMMA, "
                    ", ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(0).Split(TextParser::CHAR_COMMA);
        }

        for (IMS_UINT32 i = 0; i < strTempValue.GetSize(); i++)
        {
            // Check EQUAL is ...
            if (strTempValue.GetAt(i).Contains(TextParser::CHAR_EQUAL) == IMS_FALSE)
            {
                IMS_TRACE_E(0,
                        "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have a "
                        "CHAR_EQUAL, = ...",
                        0, 0, 0);
                return IMS_FALSE;
            }
            else
            {
                IMSList<AString> strSendValue = strTempValue.GetAt(i).Split(TextParser::CHAR_EQUAL);

                if (strSendValue.GetAt(0).Equals("x") == IMS_TRUE)
                {
                    (*nImageWidth) =
                            strSendValue.GetAt(1).ToInt32();  // Image Width for Send Direction
                }
                else if (strSendValue.GetAt(0).Equals("y") == IMS_TRUE)
                {
                    (*nImageHeight) =
                            strSendValue.GetAt(1).ToInt32();  // Image Height for Send Direction
                }
                else if (strSendValue.GetAt(0).Equals("sar") == IMS_TRUE)
                {
                    // dImageOptSAR = (IMS_DOUBLE)strSendValue.GetAt(1).GetStr();  // Sample Aspect
                    // Ratio for Send Direction
                }
                else if (strSendValue.GetAt(0).Equals("par") == IMS_TRUE)
                {
                    // dImageOptPAR = strSendValue.GetAt(1).ToDouble();  // Picture Aspect Ratio for
                    // Send Direction
                }
                else if (strSendValue.GetAt(0).Equals("q") == IMS_TRUE)
                {
                    // dImageOptQ = strSendValue.GetAt(1).ToDouble();    // Higher Preference for
                    // Send Direction
                }
            }
        }

        IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() nImagePayloadNum[%d] ImageWidth[%d], "
                    "ImageHeight[%d]",
                nImagePayloadNum, (*nImageWidth), (*nImageHeight));

        return IMS_TRUE;
    }
    else if (objTokens.GetAt(nDirection).Equals("recv") == IMS_TRUE)
    {
        if (objTokens.GetAt(nImageValueIndex).Equals("*") == IMS_TRUE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - PeerResolution-Wildcard, Not Support", 0,
                    0, 0);
            return IMS_FALSE;
        }
        // remove spaces in Bracket, [ ]
        for (IMS_UINT32 i = nImageValueIndex; i < objTokens.GetSize(); i++)
        {
            if (objTokens.GetAt(i).Equals("send") == IMS_TRUE)
            {
                // IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() Removed all spaces in recv
                // direction Bracket", 0, 0, 0);
                break;
            }

            nRealValueString.Append(objTokens.GetAt(i));
        }

        // Check LSBRACKET is ...
        if ((nRealValueString == AString::ConstNull()) ||
                (nRealValueString.Contains(TextParser::CHAR_LSBRACKET) == IMS_FALSE))
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                    "CHAR_LSBRACKET, [ ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = nRealValueString.Split(TextParser::CHAR_LSBRACKET);
        }

        // Check RSBRACKET is ...
        if (strTempValue.GetAt(1).Contains(TextParser::CHAR_RSBRACKET) == IMS_FALSE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                    "CHAR_RSBRACKET, ] ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(1).Split(TextParser::CHAR_RSBRACKET);
        }

        // Check COMMA is ...
        if (strTempValue.GetAt(0).Contains(TextParser::CHAR_COMMA) == IMS_FALSE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have CHAR_COMMA, , "
                    "...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(0).Split(TextParser::CHAR_COMMA);
        }

        for (IMS_UINT32 i = 0; i < strTempValue.GetSize(); i++)
        {
            if (strTempValue.GetAt(i).Contains(TextParser::CHAR_EQUAL) == IMS_FALSE)
            {
                IMS_TRACE_E(0,
                        "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                        "CHAR_EQUAL, = ...",
                        0, 0, 0);
                return IMS_FALSE;
            }
            else
            {
                IMSList<AString> strRecvValue = strTempValue.GetAt(i).Split(TextParser::CHAR_EQUAL);

                if (strRecvValue.GetAt(0).Equals("x") == IMS_TRUE)
                {
                    (*nImageWidth) =
                            strRecvValue.GetAt(1).ToInt32();  // Image Width for Recv Direction
                }
                else if (strRecvValue.GetAt(0).Equals("y") == IMS_TRUE)
                {
                    (*nImageHeight) =
                            strRecvValue.GetAt(1).ToInt32();  // Image Height for Recv Direction
                }
                else if (strRecvValue.GetAt(0).Equals("sar") == IMS_TRUE)
                {
                    // dImageOptSAR = strRecvValue.GetAt(1).ToDouble();  // Sample Aspect Ratio for
                    // Recv Direction
                }
                else if (strRecvValue.GetAt(0).Equals("par") == IMS_TRUE)
                {
                    // dImageOptPAR = strRecvValue.GetAt(1).ToDouble();  // Picture Aspect Ratio for
                    // Recv Direction
                }
                else if (strRecvValue.GetAt(0).Equals("q") == IMS_TRUE)
                {
                    // dImageOptQ = strRecvValue.GetAt(1).ToDouble();    // Higher Preference for
                    // Recv Direction
                }
            }
        }

        IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() nImagePayloadNum[%d] ImageWidth[%d], "
                    "ImageHeight[%d]",
                nImagePayloadNum, (*nImageWidth), (*nImageHeight));
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/* // TODO_MEDIA video sprop
PROTECTED IMS_BOOL VideoNego::GetWidthHeightFromSdp_SpropParam(IN VIDEO_CODEC codecType,
        IN IMS_CHAR* szSprop, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    tMMPFGetInfoParam getInfo;
    tMMPFGetInfoResult InfoResult;
    eMMPFResult eResult = MMPF_RESULT_ERR_UNKNOWN;

    if (codecType == VIDEO_CODEC_AVC)
    {
        getInfo.type = MMPF_INFO_PARSE_SPROPPARAM;
    }
    else if (codecType == VIDEO_CODEC_HEVC)
    {
        getInfo.type = MMPF_INFO_PARSE_SPROPPARAM_H265;
    }
    else
    {
        return IMS_FALSE;
    }

    IMS_StrCpy(getInfo.szSpropParam, MMPF_MAX_CONFIG_LEN, szSprop);

    // eResult = MMPFSession::getInfo(MMPF_INTERFACEID_AUTO, &getInfo, &InfoResult);

    if (eResult == MMPF_RESULT_OK)
    {
        *nImageWidth = InfoResult.tCodecAttribute.nWidth;
        *nImageHeight = InfoResult.tCodecAttribute.nHeight;

        IMS_TRACE_D("GetWidthHeightFromSdp_SpropParam() - Parsing Success. nWidth[%d], nHeight[%d]",
                *nImageWidth, *nImageHeight, 0);

        return IMS_TRUE;
    }
    else
    {
        IMS_TRACE_E(0, "GetWidthHeightFromSdp_SpropParam() INVALID szSprop[%s]", szSprop, 0, 0);
        return IMS_FALSE;
    }
}
*/

PROTECTED IMS_BOOL VideoNego::GetWidthHeightFromSdp_FrameSize(
        IN AString strFrameSizeFromSdp, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    IMS_UINT32 nFrameSizePayloadNum = 0;  // Payload Number in Image Attr

    IMSList<AString> objTokens = strFrameSizeFromSdp.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 2)
    {
        return IMS_FALSE;
    }

    // 1st step : Take "Payload Number" out of Image Attribute.
    nFrameSizePayloadNum = objTokens.GetAt(0).ToInt32();

    // 2nd step : Take "Frame Size Values" out of Frame Size Attribute.
    // Check CHAR_HYPHEN is ...
    if (objTokens.GetAt(1).Contains(TextParser::CHAR_HYPHEN) == IMS_FALSE)
    {
        IMS_TRACE_E(0,
                "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have CHAR_HYPHEN, "
                "[%s], ...",
                strFrameSizeFromSdp.GetStr(), 0, 0);
        return IMS_FALSE;
    }
    else
    {
        IMSList<AString> strFrameSizeValue = objTokens.GetAt(1).Split(TextParser::CHAR_HYPHEN);

        *nImageWidth = strFrameSizeValue.GetAt(0).ToInt32();   // Width
        *nImageHeight = strFrameSizeValue.GetAt(1).ToInt32();  // Height
        IMS_TRACE_D("GetWidthHeightFromSdp_FrameSize() nFrameSizePayloadNum[%d] ImageWidth[%d], "
                    "ImageHeight[%d]",
                nFrameSizePayloadNum, *nImageWidth, *nImageHeight);

        return IMS_TRUE;
    }
}

PROTECTED VIDEO_RESOLUTION VideoNego::GetResolutionFromWidthHeight(
        IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight)
{
    IMS_TRACE_D("GetResolutionFromWidthHeight() Widht[%d], Height[%d]", nWidth, nHeight, 0);
    if (nWidth == 480 && nHeight == 640)
        return VIDEO_RESOLUTION_VGA_PR;
    else if (nWidth == 640 && nHeight == 480)
        return VIDEO_RESOLUTION_VGA_LS;
    else if (nWidth == 240 && nHeight == 320)
        return VIDEO_RESOLUTION_QVGA_PR;
    else if (nWidth == 320 && nHeight == 240)
        return VIDEO_RESOLUTION_QVGA_LS;
    else if (nWidth == 144 && nHeight == 176)
        return VIDEO_RESOLUTION_QCIF_PR;
    else if (nWidth == 176 && nHeight == 144)
        return VIDEO_RESOLUTION_QCIF_LS;
    else if (nWidth == 352 && nHeight == 288)
        return VIDEO_RESOLUTION_CIF_LS;
    else if (nWidth == 288 && nHeight == 352)
        return VIDEO_RESOLUTION_CIF_PR;
    else if (nWidth == 240 && nHeight == 352)
        return VIDEO_RESOLUTION_SIF_PR;
    else if (nWidth == 352 && nHeight == 240)
        return VIDEO_RESOLUTION_SIF_LS;
    else if (nWidth == 128 && nHeight == 96)
        return VIDEO_RESOLUTION_SQCIF_LS;
    else if (nWidth == 96 && nHeight == 128)
        return VIDEO_RESOLUTION_SQCIF_PR;
    else if (nWidth == 720 && nHeight == 1280)
        return VIDEO_RESOLUTION_HD_PR;
    else if (nWidth == 1280 && nHeight == 720)
        return VIDEO_RESOLUTION_HD_LS;
    else if (nWidth == 1080 && nHeight == 1920)
        return VIDEO_RESOLUTION_FHD_PR;
    else if (nWidth == 1920 && nHeight == 1080)
        return VIDEO_RESOLUTION_FHD_LS;
    else
        IMS_TRACE_E(0, "GetResolutionFromWidthHeight() INVALID Widht[%d], Height[%d]", nWidth,
                nHeight, 0);
    return VIDEO_RESOLUTION_QCIF_PR;
}

PROTECTED IMS_BOOL VideoNego::MakeImageAttributeLine(
        IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId, OUT AString& strImageAttr)
{
    IMS_UINT32 nWidth, nHeight;

    if (GetWidthHeightFromResolutionId(eResolutionId, &nWidth, &nHeight) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    strImageAttr.Sprintf(
            "%d send [x=%d,y=%d] recv [x=%d,y=%d]", nPayloadType, nWidth, nHeight, nWidth, nHeight);

    return IMS_TRUE;
}

PROTECTED IMS_BOOL VideoNego::MakeFrameSizeLine(
        IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId, OUT AString& strFrameSize)
{
    IMS_UINT32 nWidth, nHeight;

    if (GetWidthHeightFromResolutionId(eResolutionId, &nWidth, &nHeight) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    strFrameSize.Sprintf("%d %d-%d", nPayloadType, nWidth, nHeight);

    return IMS_TRUE;
}

//--------------------------------------------------------------------
//    Capa Nego APIs
//--------------------------------------------------------------------

PROTECTED IMS_BOOL VideoNego::GetAvpfFromAttributes(IN SdpMediaFormat* pMediaFormat,
        IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr)
{
    if (pMediaFormat == IMS_NULL || pRtcpFbAttr == IMS_NULL || pCapaNego == IMS_NULL)
        return IMS_FALSE;

    IMSList<SdpMediaFormatParameter*> lstRTCPFeedback = pMediaFormat->GetExtraParameters();

    IMS_TRACE_I("GetAvpfFromAttributes() - Entered. number of RTCP attributes[%d]",
            lstRTCPFeedback.GetSize(), 0, 0);
    if (lstRTCPFeedback.GetSize() == 0)
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < lstRTCPFeedback.GetSize(); i++)
    {
        SdpMediaFormatParameter* pMediaParam = lstRTCPFeedback.GetAt(i);
        if (pMediaParam == IMS_NULL)
            continue;

        if (pMediaParam->GetAttribute() == SdpAttribute::RTCP_FB)
        {
            SdpRtcpFeedback* pRtcpParam = (SdpRtcpFeedback*)pMediaParam;
            if (pRtcpParam->GetType().Equals("trr-int"))
            {
                pRtcpFbAttr->bTrrSupported = IMS_TRUE;
                pRtcpFbAttr->nTrrInt = pRtcpParam->GetParamName().ToInt32();
            }
            else if (pRtcpParam->GetType().Equals("nack"))
            {
                pRtcpFbAttr->bNackSupported = IMS_TRUE;

                if (pRtcpParam->GetParamName().Equals("pli"))
                {
                    pRtcpFbAttr->bPliSupported = IMS_TRUE;
                }
            }
            else if (pRtcpParam->GetType().Equals("ccm"))
            {
                if (pRtcpParam->GetParamName().Equals("fir"))
                {
                    pRtcpFbAttr->bFirSupported = IMS_TRUE;
                }

                if (pRtcpParam->GetParamName().Equals("tmmbr"))
                {
                    pRtcpFbAttr->bTmmbrSupported = IMS_TRUE;
                }
            }

            IMS_TRACE_D("GetAvpfFromAttributes() - pRtcpParam->GetType[%s], GetParamName[%s]",
                    pRtcpParam->GetType().GetStr(), pRtcpParam->GetParamName().GetStr(), 0);
        }
    }

    if (pCapaNego->mapAttributeCapa.GetSize() > 0)
    {
        IMS_TRACE_D("NegotiateVideoMediaLine() - AttributeCapa value exist - Size[%d]",
                pCapaNego->mapAttributeCapa.GetSize(), 0, 0);
        for (IMS_UINT32 i = 0; i < pCapaNego->mapAttributeCapa.GetSize(); i++)
        {
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("trr-int") == IMS_TRUE)
            {
                IMSList<AString> strTemp =
                        pCapaNego->mapAttributeCapa.GetValueAt(i).Split(TextParser::CHAR_SP);

                if (strTemp.GetSize() >= 2)
                {
                    pRtcpFbAttr->nTrrInt = strTemp.GetAt(strTemp.GetSize() - 1).ToInt32();
                    pRtcpFbAttr->bTrrSupported = IMS_TRUE;
                }
            }
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("nack") == IMS_TRUE)
                pRtcpFbAttr->bNackSupported = IMS_TRUE;
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("pli") == IMS_TRUE)
                pRtcpFbAttr->bPliSupported = IMS_TRUE;
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("fir") == IMS_TRUE)
                pRtcpFbAttr->bFirSupported = IMS_TRUE;
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("tmmbr") == IMS_TRUE)
                pRtcpFbAttr->bTmmbrSupported = IMS_TRUE;
        }
    }

    IMS_TRACE_D("NegotiateVideoMediaLine() - support = bNACK[%d], bPLI[%d], bTMMBR[%d]",
            pRtcpFbAttr->bNackSupported, pRtcpFbAttr->bPliSupported, pRtcpFbAttr->bTmmbrSupported);
    IMS_TRACE_D("NegotiateVideoMediaLine() - support = bFIR[%d], bTRR_Int[%d]",
            pRtcpFbAttr->bFirSupported, pRtcpFbAttr->bTrrSupported, 0);

    return IMS_TRUE;
}

PROTECTED IMS_BOOL VideoNego::GetAvpfFromAttributes_EX(IN IMediaDescriptor* pMediaDescriptor,
        IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr)
{
    if (pMediaDescriptor == IMS_NULL || pRtcpFbAttr == IMS_NULL || pCapaNego == IMS_NULL)
        return IMS_FALSE;

    // check attribute..
    if (pCapaNego->mapAttributeCapa.GetSize() > 0)
    {
        IMS_TRACE_D("GetAvpfFromAttributes_EX() - AttributeCapa value exist - Size[%d]",
                pCapaNego->mapAttributeCapa.GetSize(), 0, 0);
        for (IMS_UINT32 i = 0; i < pCapaNego->mapAttributeCapa.GetSize(); i++)
        {
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("trr-int") == IMS_TRUE)
            {
                IMSList<AString> strTemp =
                        pCapaNego->mapAttributeCapa.GetValueAt(i).Split(TextParser::CHAR_SP);

                if (strTemp.GetSize() >= 2)
                {
                    pRtcpFbAttr->nTrrInt = strTemp.GetAt(strTemp.GetSize() - 1).ToInt32();
                    pRtcpFbAttr->bTrrSupported = IMS_TRUE;
                }
            }
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("nack") == IMS_TRUE)
                pRtcpFbAttr->bNackSupported = IMS_TRUE;
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("pli") == IMS_TRUE)
                pRtcpFbAttr->bPliSupported = IMS_TRUE;
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("fir") == IMS_TRUE)
                pRtcpFbAttr->bFirSupported = IMS_TRUE;
            if (pCapaNego->mapAttributeCapa.GetValueAt(i).Contains("tmmbr") == IMS_TRUE)
                pRtcpFbAttr->bTmmbrSupported = IMS_TRUE;
        }
        IMS_TRACE_D("GetAvpfFromAttributes_EX() - support = bNACK[%d], bPLI[%d], bTMMBR[%d]",
                pRtcpFbAttr->bNackSupported, pRtcpFbAttr->bPliSupported,
                pRtcpFbAttr->bTmmbrSupported);
        IMS_TRACE_D("GetAvpfFromAttributes_EX() - support = bFIR[%d], bTRR_Int[%d], nTTR-Int[%d]",
                pRtcpFbAttr->bFirSupported, pRtcpFbAttr->bTrrSupported, pRtcpFbAttr->nTrrInt);
    }

    return IMS_TRUE;
}

PROTECTED IMS_BOOL VideoNego::MakeCapaNegoProfileFromSdp(
        IN IMediaDescriptor* pDescriptor, OUT VideoProfile::CapaNego* pObjCapaNego)
{
    IMS_TRACE_I("MakeCapaNegoProfileFromSdp() enter", 0, 0, 0);

    if (pDescriptor == IMS_NULL || pObjCapaNego == IMS_NULL)
        return IMS_FALSE;

    IMS_UINT32 i = 0, j = 0;
    IMS_SINT32 nTcapInitNum = 0, nAcapNum = 0;
    AString strTcap = "";
    AString strAcap = "";

    IMSList<AString> lstTCAPAttr = pDescriptor->GetAttributes(SdpAttribute::TCAP);
    IMSList<AString> lstACAPAttr = pDescriptor->GetAttributes(SdpAttribute::ACAP);
    IMSList<AString> lstACFGAttr = pDescriptor->GetAttributes(SdpAttribute::ACFG);

    if (lstACFGAttr.GetSize() > 0)
    {
        pObjCapaNego->strNegotiatedAcfg = lstACFGAttr.GetAt(0);
        IMS_TRACE_I("GetCapaNegoValueFromSdp() - Answer Case, ACFG[%s]",
                pObjCapaNego->strNegotiatedAcfg.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    // Get Potential configuration list (pcfg) - "'prio #' SP "t=Tcap #' SP 'a=Acap #'" pair
    pObjCapaNego->lstPotentialConfig = pDescriptor->GetAttributes(SdpAttribute::PCFG);

    IMS_TRACE_I("GetCapaNegoValueFromSdp() - Entered, PCFGsize[%d], TCAPsize[%d], ACAPsize[%d]",
            pObjCapaNego->lstPotentialConfig.GetSize(), pObjCapaNego->mapTransportCapa.GetSize(),
            pObjCapaNego->mapAttributeCapa.GetSize());

    // Get transport capability(TCAP) list - "'number' SP 'Tcap'" pair
    for (i = 0; i < lstTCAPAttr.GetSize(); i++)
    {
        AString strTCAPline = lstTCAPAttr.GetAt(i);
        if (strTCAPline.GetLength() == 0)
            continue;

        IMSList<AString> lstSplitSpace = strTCAPline.Split(' ');

        // save Tcap String to CapaNego Obj
        for (j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            if (j == 0)
            {
                nTcapInitNum = lstSplitSpace.GetAt(j).ToInt32();
                IMS_TRACE_I("GetCapaNegoValueFromSdp() - nTcapInitNum[%d]", nTcapInitNum, 0, 0);
            }
            else
            {
                // mapped - key : 'number' value:'Tcap'
                strTcap.Sprintf("%s", lstSplitSpace.GetAt(j).GetStr());
                pObjCapaNego->mapTransportCapa.Add(nTcapInitNum, strTcap);
                nTcapInitNum++;
                strTcap = "";
            }
        }
    }

    // Get attribute capability(ACAP) list - "'number' SP 'Acap'" pair
    for (i = 0; i < lstACAPAttr.GetSize(); i++)
    {
        strAcap = "";
        nAcapNum = 0;
        AString strACAPline = lstACAPAttr.GetAt(i);
        if (strACAPline.GetLength() == 0)
            continue;

        IMSList<AString> lstSplitSpace = strACAPline.Split(' ');

        // save Acap String to CapaNego Obj
        for (j = 0; j < lstSplitSpace.GetSize(); j++)
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
                    strAcap.Append(" " + lstSplitSpace.GetAt(j));
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
    if (lstACAPAttr.GetSize() > 0)
        pObjCapaNego->bIsAttCapaInPcfg = IMS_TRUE;

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

VIRTUAL IMS_BOOL VideoNego::MakeNegotiatedCapaNegoProfile(IN VideoProfile::CapaNego* pSrcCapaNego,
        IN VideoProfile::CapaNego* pDestCapaNego, OUT VideoProfile::CapaNego* pNegotiatedCapaNego)
{
    IMS_TRACE_I("MakeNegotiatedCapaNego() enter", 0, 0, 0);

    if (pSrcCapaNego == IMS_NULL || pDestCapaNego == IMS_NULL || pNegotiatedCapaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedCapaNego() invalid argument, %" PFLS_x " %" PFLS_x,
                pSrcCapaNego, pDestCapaNego, 0);
        return IMS_FALSE;
    }

    IMS_BOOL ret = IMS_FALSE;
    IMS_UINT32 i = 0, j = 0, k = 0, l = 0;
    IMS_BOOL bAttributeCheckable = IMS_FALSE;
    IMS_BOOL bPCFGSupportable = IMS_FALSE;

    // pNegotiatedCapaNego->lstPotentialConfig.GetSize();
    IMSMap<IMS_SINT32, AString> mapSrcTCap = pSrcCapaNego->mapTransportCapa;
    IMSMap<IMS_SINT32, AString> mapSrcACap = pSrcCapaNego->mapAttributeCapa;

    IMSList<AString> lstDstPCFG = pDestCapaNego->lstPotentialConfig;
    IMSMap<IMS_SINT32, AString> mapDstTCap = pDestCapaNego->mapTransportCapa;
    IMSMap<IMS_SINT32, AString> mapDstACap = pDestCapaNego->mapAttributeCapa;

    if (pDestCapaNego->strNegotiatedAcfg.GetLength() > 0)
    {
        IMS_TRACE_I("MakeNegotiatedCapaNego() ACFG - %s", pDestCapaNego->strNegotiatedAcfg.GetStr(),
                0, 0);
        return IMS_TRUE;
    }

    // parse pcfg
    for (i = 0; i < lstDstPCFG.GetSize(); i++)
    {
        AString strPCFGline = lstDstPCFG.GetAt(i);  // get "# t=# a=#,#,#,# ..."
        if (strPCFGline.GetLength() == 0)
            continue;

        IMSList<AString> lstSplitSpace = lstDstPCFG.GetAt(i).Split(' ');

        for (j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            bAttributeCheckable = IMS_FALSE;
            if (j == 1)  // t=#
            {
                AString strPCFG_Transport = lstSplitSpace.GetAt(j);
                if (strPCFG_Transport.GetLength() == 0)
                    continue;

                IMSList<AString> lstSplitEquals = strPCFG_Transport.Split('=');
                if (lstSplitEquals.GetSize() == 0)
                    continue;

                if (lstSplitEquals.GetAt(0).Equals('t') == IMS_TRUE && lstSplitEquals.GetSize() > 1)
                {
                    // 1. compare transport capa
                    AString strTmp = mapDstTCap.GetValue(lstSplitEquals.GetAt(1).ToInt32());

                    for (k = 0; k < mapSrcTCap.GetSize(); k++)
                    {
                        if (strTmp.Equals(mapSrcTCap.GetValueAt(k)) == IMS_TRUE)
                        {
                            bAttributeCheckable = IMS_TRUE;
                            bPCFGSupportable = IMS_TRUE;

                            // set Negotiated Transport Capa Nego Value...
                            pNegotiatedCapaNego->mapTransportCapa.Add(
                                    lstSplitEquals.GetAt(1).ToInt32(), strTmp);
                            break;
                        }
                    }

                    // 2. if there are no matched transport capa, then next pcfg check...
                    // -----it's first for_loop break case..
                    if (bAttributeCheckable == IMS_FALSE)
                    {
                        IMS_TRACE_I("MakeNegotiatedCapaNego() does not match transport capa - PCFG "
                                    "#[%d]",
                                i, 0, 0);
                        break;
                    }
                    // 3. if there are matched transport capa, check attribute capa
                    // -----it's not first for_loop break case..
                }
            }
            else if (j == 2)  // a=#,#,#,#...
            {
                // if attribute pcfg is exist in SDP, then bPCFGSupportable reset to IMS_FALSE for
                // attribute capa nego..
                bPCFGSupportable = IMS_FALSE;

                AString strPCFG_Attribute = lstSplitSpace.GetAt(j);
                if (strPCFG_Attribute.GetLength() == 0)
                    continue;

                IMSList<AString> lstSplitEquals = strPCFG_Attribute.Split('=');
                if (lstSplitEquals.GetSize() == 0)
                    continue;

                if (lstSplitEquals.GetAt(0).Equals('a') == IMS_TRUE && lstSplitEquals.GetSize() > 1)
                {
                    IMS_UINT32 cnt = 0;
                    // 1. compare Attribute capa
                    AString strTmp = lstSplitEquals.GetAt(1);  // tmp = "1,2,3,4"

                    // attribute comma parsing..
                    IMSList<AString> lstSplitComma = strTmp.Split(',');
                    IMS_TRACE_I("MakeNegotiatedCapaNego() attribute size[%d]",
                            lstSplitComma.GetSize(), 0, 0);

                    if (lstSplitComma.GetSize() == 0)
                        continue;

                    // 2. check attribute capa negotiation
                    for (k = 0; k < lstSplitComma.GetSize(); k++)
                    {
                        AString strDestAttributeCapa =
                                mapDstACap.GetValue(lstSplitComma.GetAt(k).ToInt32());
                        IMS_TRACE_I("MakeNegotiatedCapaNego() strDestAttributeCapa [%s]",
                                strDestAttributeCapa.GetStr(), 0, 0);

                        if (strDestAttributeCapa.Contains("trr-int") == IMS_TRUE)
                        {
                            cnt++;
                            pNegotiatedCapaNego->mapAttributeCapa.Add(
                                    lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                        }
                        else if (strDestAttributeCapa.Contains("nack") == IMS_TRUE)
                        {
                            if (strDestAttributeCapa.Contains("pli") == IMS_TRUE)
                            {
                                cnt++;
                                pNegotiatedCapaNego->mapAttributeCapa.Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                            }
                            else
                            {
                                cnt++;
                                pNegotiatedCapaNego->mapAttributeCapa.Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                            }
                        }
                        else if (strDestAttributeCapa.Contains("ccm") == IMS_TRUE)
                        {
                            if (strDestAttributeCapa.Contains("fir") == IMS_TRUE)
                            {
                                cnt++;
                                pNegotiatedCapaNego->mapAttributeCapa.Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                            }
                            else if (strDestAttributeCapa.Contains("tmmbr") == IMS_TRUE)
                            {
                                cnt++;
                                pNegotiatedCapaNego->mapAttributeCapa.Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                            }
                        }
                        else if (strDestAttributeCapa.Contains("crypto") == IMS_TRUE)
                        {
                            // crypto attribute negotiate only srtp profile type
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

                                IMS_TRACE_I("MakeNegotiatedCapaNego()  strDestAttributeCapa.Equals "
                                            "CNT[%d]",
                                        cnt, 0, 0);
                                break;
                            }
                        }
                    }

                    // 3. if ue support pcfg about transport capa, bPCFGSupportable variable set to
                    // True..
                    if (cnt == lstSplitComma.GetSize())
                    {
                        IMS_TRACE_I(
                                "MakeNegotiatedCapaNego()  capa nego success.. cnt[%d]", cnt, 0, 0);
                        bPCFGSupportable = IMS_TRUE;
                        break;
                    }
                }
            }
        }

        // check capa nego success
        if (bPCFGSupportable == IMS_TRUE)
        {
            pNegotiatedCapaNego->strNegotiatedAcfg.Sprintf("%s", strPCFGline.GetStr());
            IMS_TRACE_I("MakeNegotiatedCapaNego() UE support capa nego- ACFG [%s]",
                    strPCFGline.GetStr(), 0, 0);
            // strNegotiatedAcfg value available, if capa nego success.
            ret = IMS_TRUE;
            break;
        }
        else  // capa nego dose not succsee case//
        {
            // clear saved negotiatedCapaNego imfo.
            IMS_TRACE_I("MakeNegotiatedCapaNego() capa nego does not success pcfg[%d]", i, 0, 0);
            pNegotiatedCapaNego->mapTransportCapa.Clear();
            pNegotiatedCapaNego->mapAttributeCapa.Clear();
        }
    }

    return ret;
}

PROTECTED IMS_BOOL VideoNego::CheckAvpfFromProfile(IN VideoProfile* pProfile)
{
    IMS_TRACE_I("CheckAvpfFromProfile() enter", 0, 0, 0);

    if (pProfile == IMS_NULL)
        return IMS_FALSE;

    IMS_UINT32 i = 0;
    AString strTcap = "";
    AString strAVPF = "AVPF";

    for (i = 0; i < pProfile->objCapaNego.mapTransportCapa.GetSize(); i++)
    {
        strTcap = pProfile->objCapaNego.mapTransportCapa.GetValueAt(i);
        if (strTcap.Contains(strAVPF) == IMS_TRUE)
        {
            IMS_TRACE_I("CheckAvpfFromProfile() find AVPF from profile.. ", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC VideoConfiguration* VideoNego::GetConfig()
{
    IMS_TRACE_D("GetConfig() eSessionType[%d]", m_eSessionType, 0, 0);

    if (m_pMediaEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetConfig - There is no MediaEnvironment ", 0, 0, 0);
        return IMS_NULL;
    }

    VideoConfiguration* pVideoConfig = IMS_NULL;
    pVideoConfig = MediaSessionConfigFactory::GetInstance()
                           ->FindMediaSessionConfig(GetSlotId(), m_pMediaEnvironment->eServiceType)
                           ->GetVideoConfiguration();
    if (pVideoConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetConfig - There is no video config ", 0, 0, 0);
        return IMS_NULL;
    }
    return pVideoConfig;
}
