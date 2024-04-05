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
#include "video/VideoNegoAvc.h"
#include "video/VideoNegoHevc.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "MediaResourceManager.h"
#include "MediaManager.h"

__IMS_TRACE_TAG_USER_DECL__("MED.VN");

PUBLIC VideoNego::VideoNego(IN const IMS_SINT32 nSlotId) :
        BaseNego(nSlotId),
        m_listOaModel(ImsList<OaModel*>()),
        m_objBaseProfile(VideoProfile()),
        m_pConfig(IMS_NULL),
        m_bNegotiatedCvoResult(IMS_FALSE)
{
    IMS_TRACE_I("+VideoNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC
VideoNego::VideoNego(IN const VideoNego& obj) :
        BaseNego(obj.GetSlotId())
{
    Copy(&obj);
}

PUBLIC
VideoNego& VideoNego::operator=(IN const VideoNego& obj)
{
    if (this != &obj)
    {
        Copy(&obj);
    }
    return (*this);
}

PUBLIC VideoNego::~VideoNego()
{
    IMS_TRACE_I("~VideoNego()", 0, 0, 0);
    VideoNego::DestroyProfiles();

    while (m_listOaModel.GetSize() > 0)
    {
        OaModel* pOaModel = m_listOaModel.GetAt(0);

        if (pOaModel != IMS_NULL)
        {
            delete pOaModel;
        }
        m_listOaModel.RemoveAt(0);
    }
}

PUBLIC VIRTUAL void VideoNego::CreateProfiles(
        IN MediaEnvironment* pEnvironment, IN VideoConfiguration* pConfig)
{
    if (pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfiles() - invalid configuration", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("CreateProfiles()", 0, 0, 0);

    m_pEnvironment = pEnvironment;
    m_pConfig = pConfig;

    VideoProfile* pProfile = VideoProfileUtil::CreateProfile(pEnvironment, m_pConfig, GetSlotId());

    if (pProfile != IMS_NULL)
    {
        m_objBaseProfile = *pProfile;
        delete pProfile;
    }
}

PUBLIC
void VideoNego::DestroyProfiles()
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
        MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

        if (pResourceMngr != IMS_NULL)
        {
            if (m_objBaseProfile.nDataPort != 0)
            {
                pResourceMngr->ReleaseRtpPort(m_objBaseProfile.nDataPort);
            }
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL VideoNego::FormSdp(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
        IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable, IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormSdp() - NegoState[%d], lstOaModel size[%d]", eNegoState,
            m_listOaModel.GetSize(), 0);
    IMS_TRACE_I("FormSdp() - eDirection[%d], bDisable[%d]", eDirection, bDisable, 0);
    IMS_TRACE_D("FormSdp() - EnforceReofferMode[%d]", bEnforceReofferMode, 0, 0);

    switch (eNegoState)
    {
        case STATE_IDLE:
            return FormOffer(pSessionDescriptor, pDescriptor, eDirection, bDisable);
        case STATE_OFFER_RECEIVED:
            return FormAnswer(pSessionDescriptor, pDescriptor, eDirection, bDisable);
        case STATE_NEGOTIATED:
            return FormReoffer(
                    pSessionDescriptor, pDescriptor, eDirection, bDisable, bEnforceReofferMode);
        default:
            return IMS_FALSE;
    }
}

PUBLIC VIRTUAL IMS_BOOL VideoNego::IsMediaCodecFromSdpSupported(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_TYPE_INVALID;
    }

    IMS_TRACE_I("IsMediaCodecFromSdpSupported()", 0, 0, 0);

    OaModel objOaModel;
    objOaModel.pLocalProfile = new VideoProfile(m_objBaseProfile);

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = new VideoProfile();

    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, objOaModel.pPeerProfile) != IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile = new VideoProfile();

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

PUBLIC VIRTUAL void VideoNego::NegotiateSdp(NEGO_STATE eNegoState,
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

PUBLIC VIRTUAL void VideoNego::FinalizeSdp(
        IN ISessionDescriptor* pSessionDescriptor, IN NEGO_STATE eNegoState)
{
    IMS_BOOL bFoundOaModel = IMS_FALSE;

    // reset confirmed Session check variable
    for (IMS_UINT32 i = 0; i < m_listOaModel.GetSize(); i++)
    {
        OaModel* pCheckedOaModel = m_listOaModel.GetAt(i);

        if (pCheckedOaModel != IMS_NULL)
        {
            pCheckedOaModel->bConfirmedSession = IMS_FALSE;
        }
    }

    // check latest OA model
    OaModel* pLatestOaModel = IMS_NULL;

    if (m_listOaModel.GetSize() > 0)
    {
        pLatestOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);
    }

    if (pLatestOaModel != IMS_NULL)
    {
        if ((pLatestOaModel->IsAllProfileExist() &&
                    (eNegoState == STATE_IDLE || eNegoState == STATE_NEGOTIATED)) == IMS_FALSE)
        {
            IMS_TRACE_I("FinalizeSdp() - Incomplete OaModel[%d]. Delete profile",
                    m_listOaModel.GetSize() - 1, 0, 0);
            delete pLatestOaModel;
            m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        }
    }

    for (IMS_UINT32 i = 0; i < m_listOaModel.GetSize(); i++)
    {
        // get OaModel
        OaModel* pTempOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1 - i);

        // find matched SessionDescriptor key
        if (pTempOaModel != IMS_NULL)
        {
            if (pTempOaModel->nSessionDescriptorKey ==
                    reinterpret_cast<IMS_SINTP>(pSessionDescriptor))
            {
                pTempOaModel->bConfirmedSession = IMS_TRUE;
                bFoundOaModel = IMS_TRUE;
                IMS_TRACE_D("FinalizeSdp() - find comfirmed Session OaModel [%d]",
                        m_listOaModel.GetSize() - i, 0, 0);
                break;
            }
        }
    }

    // SessionDescriptor key mismatch case handling, not select OaModel
    if (bFoundOaModel != IMS_TRUE && m_listOaModel.GetSize() > 0)
    {
        IMS_TRACE_D("FinalizeSdp() - not found comfirmed Session OaModel", 0, 0, 0);
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
const IpAddress& VideoNego::GetNegotiatedRemoteAddress()
{
    VideoProfile* pProfile = GetNegotiatedPeerProfile();

    if (pProfile != IMS_NULL)
    {
        return pProfile->objIpAddress;
    }

    return IpAddress::NONE;
}

PUBLIC
IMS_SINT32 VideoNego::GetRemotePort()
{
    VideoProfile* pProfile = GetNegotiatedPeerProfile();

    if (pProfile != IMS_NULL)
    {
        return pProfile->nDataPort;
    }

    return MEDIA_PORT_INVALID;
}

PUBLIC
VideoProfile* VideoNego::GetNegotiatedLocalProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pLocalProfile;
    }

    return IMS_NULL;
}

PUBLIC
VideoProfile* VideoNego::GetNegotiatedNegoProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pNegotiatedProfile;
    }

    return IMS_NULL;
}

PUBLIC
VideoProfile* VideoNego::GetNegotiatedPeerProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pPeerProfile;
    }

    return IMS_NULL;
}

PUBLIC
MEDIA_DIRECTION VideoNego::GetNegotiatedDirection()
{
    if (m_listOaModel.GetSize() > 0)
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
VIDEO_RESOLUTION VideoNego::GetNegotiatedResolution()
{
    if (m_listOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel();

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

        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
        {
            VideoProfile::AvcFmtp* pFmtp = (VideoProfile::AvcFmtp*)pPayload->pFmtp;
            if (pFmtp != IMS_NULL)
            {
                return pFmtp->eResolution;
            }
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
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
    if (m_listOaModel.GetSize() > 0)
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
    if (m_listOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

        if (pLatestOaModel == IMS_NULL || pLatestOaModel->pLocalProfile == IMS_NULL)
        {
            return -1;
        }

        // returned negotiated bandwidth.
        if (pLatestOaModel->pNegotiatedProfile != IMS_NULL)
        {
            return (IMS_SINT32)pLatestOaModel->pNegotiatedProfile->nBandwidthAs;
        }

        return (IMS_SINT32)pLatestOaModel->pLocalProfile->nBandwidthAs;
    }

    return -1;
}

PRIVATE
void VideoNego::Copy(IN const VideoNego* pVideoNego)
{
    if (pVideoNego == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("Copy() - listOaModel size[%d]", pVideoNego->m_listOaModel.GetSize(), 0, 0);

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

    m_objBaseProfile = pVideoNego->m_objBaseProfile;

    if (pResourceMngr != IMS_NULL)
    {
        // To add port (it would be duplicated)
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->AcquireRtpPort(m_objBaseProfile.nDataPort, m_objBaseProfile.nDataPort);
        }
    }

    m_bNegotiatedCvoResult = pVideoNego->m_bNegotiatedCvoResult;
    m_pEnvironment = pVideoNego->m_pEnvironment;

    if (pVideoNego->m_listOaModel.GetSize() < 1)
    {
        return;
    }

    OaModel* pNewOaModel = new OaModel();
    OaModel* pOldOaModel = pVideoNego->m_listOaModel.GetAt(0);
    pNewOaModel->pLocalProfile = new VideoProfile(*pOldOaModel->pLocalProfile);
    m_listOaModel.Append(pNewOaModel);

    IMS_TRACE_I("Copy() - listOaModel size[%d]", m_listOaModel.GetSize(), 0, 0);
    return;
}

PRIVATE IMS_BOOL VideoNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            eDirection == MEDIA_DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("FormOffer() - eDirection[%d] - bDisable[%d]", eDirection, bDisable, 0);

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();

    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pNewOaModel->pLocalProfile = new VideoProfile(m_objBaseProfile);

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDirection))
    {
        IMS_TRACE_I("FormOffer() Enforced Set to direction[%d]", eDirection, 0, 0);
        pNewOaModel->pLocalProfile->eDirection = eDirection;
    }

    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pLocalProfile->nDataPort = 0;
        pNewOaModel->pLocalProfile->nControlPort = 0;
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    VideoProfileUtil::SetVideoRsRr(pNewOaModel->pLocalProfile, m_pConfig);
    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pLocalProfile);
}

PRIVATE IMS_BOOL VideoNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_listOaModel.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    if (eDirection == MEDIA_DIRECTION_INVALID && bDisable != IMS_TRUE)
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

    IMS_TRACE_D("FormAnswer() - eDirection[%d] - bDisable[%d]", eDirection, bDisable, 0);

    // Modify a RTP/RTCP port to ZERO if video is not supported
    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pNegotiatedProfile->nDataPort = 0;
        pNewOaModel->pNegotiatedProfile->nControlPort = 0;
    }

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDirection))
    {
        IMS_TRACE_D("FormAnswer() Enforced Set to direction[%d]", eDirection, 0, 0);
        pNewOaModel->pNegotiatedProfile->eDirection = eDirection;
    }
    else
    {
        pNewOaModel->pNegotiatedProfile->eDirection =
                UpdateDirectionToMine(pNewOaModel->pPeerProfile->eDirection,
                        pNewOaModel->pLocalProfile->eDirection, IMS_FALSE);

        IMS_TRACE_D("FormAnswer() Enforced Set to direction[%d] made from peer direction[%d]",
                pNewOaModel->pNegotiatedProfile->eDirection, pNewOaModel->pPeerProfile->eDirection,
                0);
    }

    // Make the SDP from profile
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pNegotiatedProfile);
}

PRIVATE IMS_BOOL VideoNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormReoffer() - pDescriptor[%" PFLS_x "], eDirection[%d], OaModel Size[%d]",
            pDescriptor, eDirection, m_listOaModel.GetSize());
    IMS_TRACE_I("FormReoffer() - bDisable[%d] EnforceReofferMode[%d]", bDisable,
            bEnforceReofferMode, 0);

    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (eDirection == MEDIA_DIRECTION_INVALID && bDisable != IMS_TRUE)
    {
        IMS_TRACE_E(0, "FormReoffer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);

    // Make new Offer/Answer model, and copy source profile from previous negotiated profile
    OaModel* pNewOaModel = new OaModel();

    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }

    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    GetSlotId(), m_pEnvironment->eServiceType);

    if (m_listOaModel.GetSize() == 0)
    {
        pNewOaModel->pLocalProfile = new VideoProfile(m_objBaseProfile);
    }
    else
    {
        OaModel* pPrevOaModel = GetNegotiatedOaModel();

        if (pPrevOaModel == IMS_NULL)
        {
            delete pNewOaModel;
            return IMS_FALSE;
        }

        if (pPrevOaModel->pNegotiatedProfile != IMS_NULL &&
                pPrevOaModel->pNegotiatedProfile->nDataPort == 0 && bDisable == IMS_TRUE)
        {
            if (pMediaSessionConfig->IsSdpReofferFullCapability() == IMS_TRUE)
            {
                IMS_TRACE_D("VideoNego::FormReOffer() - Try to Full Capability", 0, 0, 0);
                pNewOaModel->pLocalProfile = new VideoProfile(m_objBaseProfile);
            }
            else
            {
                IMS_TRACE_D("VideoNego::FormReOffer() - Try to Negotiated Capability", 0, 0, 0);
                pNewOaModel->pLocalProfile = new VideoProfile(*pPrevOaModel->pNegotiatedProfile);
            }
        }
        else
        {
            // pNewOaModel->pLocalProfile = new VideoProfile(m_objBaseProfile);  //org
            if (bEnforceReofferMode == IMS_TRUE)
            {
                pNewOaModel->pLocalProfile = new VideoProfile(m_objBaseProfile);
            }
            else
            {
                if (pMediaSessionConfig->IsSdpReofferFullCapability() == IMS_TRUE)
                {
                    IMS_TRACE_D("VideoNego::FormReOffer() - Try to Full Capability", 0, 0, 0);
                    pNewOaModel->pLocalProfile = new VideoProfile(m_objBaseProfile);
                }
                else
                {
                    if (pPrevOaModel->pNegotiatedProfile != IMS_NULL)
                    {
                        IMS_TRACE_D(
                                "VideoNego::FormReOffer() - Try to Negotiated Capability", 0, 0, 0);
                        pNewOaModel->pLocalProfile =
                                new VideoProfile(*pPrevOaModel->pNegotiatedProfile);
                    }
                }
            }
        }
    }

    if (pNewOaModel->pLocalProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "VideoNego::Create Local Profile failed", 0, 0, 0);
        delete pNewOaModel;
        return IMS_FALSE;
    }

    // Modify a RTP/RTCP port if video is not supported
    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pLocalProfile->nDataPort = 0;
        pNewOaModel->pLocalProfile->nControlPort = 0;
    }
    else
    {
        pNewOaModel->pLocalProfile->nDataPort = m_objBaseProfile.nDataPort;
        pNewOaModel->pLocalProfile->nControlPort = m_objBaseProfile.nControlPort;
    }

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDirection))
    {
        IMS_TRACE_I("FormReoffer() Enforced Set to direction[%d]", eDirection, 0, 0);
        pNewOaModel->pLocalProfile->eDirection = eDirection;
    }
    else
    {
        pNewOaModel->pLocalProfile->nDataPort = 0;
        pNewOaModel->pLocalProfile->nControlPort = 0;
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    VideoProfileUtil::SetVideoRsRr(pNewOaModel->pLocalProfile, m_pConfig);
    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pLocalProfile);
}

PRIVATE IMS_SINT32 VideoNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateOffer()", 0, 0, 0);

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile = new VideoProfile(m_objBaseProfile);

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = new VideoProfile();

    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pPeerProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile = new VideoProfile();

    if (MakeNegotiatedProfile(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_TRUE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateOffer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_listOaModel.Append(pNewOaModel);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PRIVATE IMS_SINT32 VideoNego::NegotiateAnswer(
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

    IMS_TRACE_I("NegotiateAnswer()", 0, 0, 0);

    // Get the latest OAmodel from list
    OaModel* pNewOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);
    if (pNewOaModel == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = new VideoProfile();
    if (MakeProfileFromSdp(pSessionDescriptor, pDescriptor, pNewOaModel->pPeerProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile = new VideoProfile();
    if (MakeNegotiatedProfile(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_FALSE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateAnswer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PRIVATE
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
    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // make "c" line of media level if IP does not matched
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->objIpAddress))
    {
        IMS_TRACE_D("MakeSdpFromProfile() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->objIpAddress.ToCharString(), 0);

        pDescriptor->SetConnectionAddress(pProfile->objIpAddress.ToString());
    }

    // make "m" line
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

    // make SDPCapNeg attributes for initial SDP if AVPF is supported
    if (pProfile->strTransportType.EqualsIgnoreCase("RTP/AVPF"))
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_VIDEO, pProfile->nDataPort,
                SdpMedia::TRANSPORT_RTP_AVPF, objVideoFormat);
    }
    else
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_VIDEO, pProfile->nDataPort,
                SdpMedia::TRANSPORT_RTP_AVP, objVideoFormat);
    }

    // Previously check all payload for RTCP-FB wildcard(*) attributes
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

    // make bandwidth
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

    // make each payload
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

        // make "rtpmap"
        strRtpmap.Sprintf("%d %s/%d", pPayload->objRtpMap.nPayloadNum,
                pPayload->objRtpMap.strPayloadType.GetStr(), pPayload->objRtpMap.nSamplingRate);

        if (pPayload->objRtpMap.nChannel > 0)
        {
            AString strChannel;
            strChannel.Sprintf("/%d", pPayload->objRtpMap.nChannel);
            strRtpmap.Append(strChannel);
        }

        IMS_TRACE_I("MakeSdpFromProfile() - Payload[%d], strRtpmap[%s]", i, strRtpmap.GetStr(), 0);

        // make "fmtp"
        // ------ "a=fmtp:104 profile-level-id=42C016; packetization-mode=1;
        // ----------  sprop-parameter-sets=Z0LAFukDwKMg,aM4G4g=="
        SdpAvCodec* pFormat = new SdpAvCodec();

        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
        {
            VideoProfile::AvcFmtp* pAvcFmtp = (VideoProfile::AvcFmtp*)pPayload->pFmtp;

            if (pAvcFmtp == IMS_NULL)
            {
                delete pFormat;
                continue;
            }

            strFmtp = VideoNegoAvc::SetSdpFmtpFromAvcFmtp(pAvcFmtp);

            eResolution = pAvcFmtp->eResolution;
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
        {
            VideoProfile::HevcFmtp* pHevcFmtp = (VideoProfile::HevcFmtp*)pPayload->pFmtp;

            if (pHevcFmtp == IMS_NULL)
            {
                delete pFormat;
                continue;
            }

            strFmtp = VideoNegoHevc::SetSdpFmtpFromHevcFmtp(pHevcFmtp);

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

        // make "image attribute"
        if (pPayload->bIncludeImageAttr == IMS_TRUE)
        {
            if (MakeImageAttributeLine(
                        pPayload->objRtpMap.nPayloadNum, eResolution, strResolutionAttr))
            {
                pDescriptor->AddAttribute(SdpAttribute::IMAGEATTR, strResolutionAttr);
            }
        }

        // make "framesize"
        if (pPayload->bIncludeFrameSize == IMS_TRUE)
        {
            if (MakeFrameSizeLine(pPayload->objRtpMap.nPayloadNum, eResolution, strResolutionAttr))
            {
                pDescriptor->AddAttribute(SdpAttribute::FRAMESIZE, strResolutionAttr);
            }
        }

        // make "rtcp-fb"
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

    // make direction
    pDescriptor->SetDirection(pProfile->eDirection);

    // make framerate
    pDescriptor->AddAttributeInt(SdpAttribute::FRAMERATE, pProfile->nFrameRate);

    // make CVO
    if (pProfile->nCvoId > 0)
    {
        AString strCvoAttribute;
        strCvoAttribute.Sprintf("%d urn:3gpp:video-orientation", pProfile->nCvoId);
        pDescriptor->AddAttribute(SdpAttribute::ATTRIBUTE_OTHER, strCvoAttribute, "extmap");
    }

    // make Capa Nego Attribute
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

PRIVATE
IMS_BOOL VideoNego::MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
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

    // Setting transport type (for supporting AVPF)
    SdpMedia* pSDPMedia = const_cast<SdpMedia*>(pDescriptor->GetMediaDescriptionEx());

    if (pSDPMedia != IMS_NULL)
    {
        pProfile->strTransportType = pSDPMedia->GetTransportProtocolEx();

        if (pProfile->strTransportType.EqualsIgnoreCase("RTP/AVP") == IMS_TRUE)
        {
            pProfile->bSupportAvpf = IMS_FALSE;
            pProfile->bSupportCapaNegoForAvpf = IMS_FALSE;
        }
        else if (pProfile->strTransportType.EqualsIgnoreCase("RTP/AVPF") == IMS_TRUE)
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

    IMS_TRACE_I("MakeProfileFromSdp() - bSupportAvpf[%d], bSupportCapaNegoForAvpf[%d]",
            pProfile->bSupportAvpf, pProfile->bSupportCapaNegoForAvpf, 0);

    // payload
    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    // Read ImageAttr list From SDP
    ImsList<AString> objImageAttributes = pDescriptor->GetAttributes(SdpAttribute::IMAGEATTR);

    // Read FrameSize list From SDP
    ImsList<AString> objFrameSizes = pDescriptor->GetAttributes(SdpAttribute::FRAMESIZE);

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

        if (strCodecName.EqualsIgnoreCase("H264"))
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
                if (GetAvpfFromAttributes(pSdpCodec, &pProfile->objCapaNego,
                            &pPayload->objRtcpFbAttr) == IMS_FALSE)
                {
                    GetAvpfFromAttributes_EX(&pProfile->objCapaNego, &pPayload->objRtcpFbAttr);
                }
            }
        }
        else if (strCodecName.EqualsIgnoreCase("H265"))
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
                if (GetAvpfFromAttributes(pSdpCodec, &pProfile->objCapaNego,
                            &pPayload->objRtcpFbAttr) == IMS_FALSE)
                {
                    GetAvpfFromAttributes_EX(&pProfile->objCapaNego, &pPayload->objRtcpFbAttr);
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
    ImsList<AString> objAttributes =
            pDescriptor->GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, "extmap");
    for (IMS_UINT32 nIndex = 0; nIndex < objAttributes.GetSize(); nIndex++)
    {
        AString strExtmap = objAttributes.GetAt(nIndex);

        if (strExtmap.Contains("urn:3gpp:video-orientation") == IMS_TRUE)
        {
            AString strCVOTrim = strExtmap.Trim();
            ImsList<AString> strSplitSpace = strCVOTrim.Split(' ');

            if (strSplitSpace.GetAt(0).GetLength() > 0)
            {
                if (strSplitSpace.GetAt(0).Contains("/"))
                {
                    ImsList<AString> strSplitSlash = strSplitSpace.GetAt(0).Split('/');

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

PRIVATE IMS_BOOL VideoNego::MakeNegotiatedPayload(IN VideoProfile::Payload* pLocalPayload,
        IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload)
{
    if (pLocalPayload == IMS_NULL || pPeerPayload == IMS_NULL || pNegoPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    *pNegoPayload = *pLocalPayload;
    pNegoPayload->objRtpMap.nPayloadNum = pPeerPayload->objRtpMap.nPayloadNum;
    pNegoPayload->bIncludeFrameSize = pLocalPayload->bIncludeFrameSize;
    pNegoPayload->bIncludeImageAttr = pLocalPayload->bIncludeImageAttr;

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoNego::MakeNegotiatedProfile(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT VideoProfile* pNegotiatedProfile)
{
    IMS_BOOL ret = IMS_FALSE;

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
        *pNegotiatedProfile = *pLocalProfile;
        pNegotiatedProfile->nDataPort = 0;
        pNegotiatedProfile->nNegotiatedPayloadIndex = -1;

        IMS_TRACE_D("MakeNegotiatedProfile() - ZERO Port. DO NOT Use the video[%d][%d]",
                pNegotiatedProfile->nDataPort, pPeerProfile->nDataPort, 0);
        return IMS_TRUE;
    }

    // Setting profile type
    if (pLocalProfile->bSupportAvpf == IMS_TRUE && pPeerProfile->bSupportAvpf == IMS_TRUE)
    {
        pNegotiatedProfile->bSupportAvpf = IMS_TRUE;
    }

    pNegotiatedProfile->bSupportCapaNegoForAvpf = pPeerProfile->bSupportCapaNegoForAvpf;

    IMS_TRACE_I("MakeNegotiatedProfile() - PeerProfile: CapaNegoForAVPF[%d], Avpf[%d]",
            pNegotiatedProfile->bSupportCapaNegoForAvpf, pNegotiatedProfile->bSupportAvpf, 0);

    // Capability Negotiation for AVPF, SRTP
    if (pNegotiatedProfile->bSupportCapaNegoForAvpf == IMS_TRUE)
    {
        if (MakeNegotiatedCapaNegoProfile(&(pLocalProfile->objCapaNego),
                    &(pPeerProfile->objCapaNego), &(pNegotiatedProfile->objCapaNego)) != IMS_TRUE)
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

    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF enable[%d], strTransportType[%s]",
            pNegotiatedProfile->bSupportAvpf, pNegotiatedProfile->strTransportType.GetStr(), 0);

    // Compare each payload based destination's profile
    VideoProfile::Payload* pNegotiatedPayload = IMS_NULL;
    IMS_SINT32 nNegotiatedMaxFrameRate = 0;
    IMS_SINT32 nNegotiatedMaxAs = 0;

    VideoProfile::Payload* pLocalPayload = IMS_NULL;
    VideoProfile::Payload* pPeerPayload = IMS_NULL;
    VideoProfile::Payload* pTmpPayload = IMS_NULL;
    VideoProfile::Payload* pMatchedPeerPayload = IMS_NULL;

    for (IMS_UINT32 nPeerIndex = 0; nPeerIndex < pPeerProfile->lstPayload.GetSize(); nPeerIndex++)
    {
        if (pNegotiatedProfile->lstPayload.GetSize() > 0)
        {
            break;
        }

        pPeerPayload = pPeerProfile->lstPayload.GetAt(nPeerIndex);

        if (pPeerPayload == IMS_NULL)
        {
            continue;
        }

        if (pPeerPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
        {
            // start source profile loop
            for (IMS_UINT32 nLocalIndex = 0; nLocalIndex < pLocalProfile->lstPayload.GetSize();
                    nLocalIndex++)
            {
                pLocalPayload = pLocalProfile->lstPayload.GetAt(nLocalIndex);

                if (pLocalPayload == IMS_NULL)
                {
                    continue;
                }

                // find matched payload - H264 find options
                if (pLocalPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
                {
                    // FMTP compare
                    VideoProfile::AvcFmtp* pLocalFmtp =
                            (VideoProfile::AvcFmtp*)pLocalPayload->pFmtp;
                    VideoProfile::AvcFmtp* pPeerFmtp = (VideoProfile::AvcFmtp*)pPeerPayload->pFmtp;

                    if (pLocalFmtp == IMS_NULL || pPeerFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() profileLevelID[%s]<->profileLevelID[%s]",
                            pLocalFmtp->strProfileLevelId.GetStr(),
                            pPeerFmtp->strProfileLevelId.GetStr(), 0);

                    IMS_TRACE_D("MakeNegotiatedProfile() Level[%d]<->Level[%d]", pLocalFmtp->nLevel,
                            pPeerFmtp->nLevel, 0);
                    IMS_TRACE_D("MakeNegotiatedProfile() Profile[%d]<->Profile[%d]",
                            pLocalFmtp->nProfile, pPeerFmtp->nProfile, 0);

                    // same level is adapt first, reject higher level
                    if (pLocalFmtp->nLevel < pPeerFmtp->nLevel)
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile( NOT MATCHED AVC Level[%d]<->[%d]",
                                pLocalFmtp->nLevel, pPeerFmtp->nLevel, 0);

                        if (pTmpPayload == IMS_NULL)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() Accept Highest Temp Src \
                                    profileLevelID[%s]",
                                    pLocalFmtp->strProfileLevelId.GetStr(), 0, 0);
                            pTmpPayload = pLocalPayload;
                            pMatchedPeerPayload = pPeerPayload;
                        }

                        continue;
                    }
                    else
                    {
                        if (pLocalFmtp->nLevel != pPeerFmtp->nLevel)
                        {
                            IMS_BOOL bFoundPayload = IMS_FALSE;

                            for (IMS_UINT32 nIndex = nLocalIndex;
                                    nIndex < pLocalProfile->lstPayload.GetSize(); nIndex++)
                            {
                                // if find matching level fmtp, skip unmatched level payload
                                VideoProfile::Payload* pPotentialPayload =
                                        pLocalProfile->lstPayload.GetAt(nIndex);

                                if (pPotentialPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                                            "H264"))
                                {
                                    VideoProfile::AvcFmtp* pPotentialFmtp =
                                            (VideoProfile::AvcFmtp*)pPotentialPayload->pFmtp;

                                    // check level and payload
                                    if (pPotentialFmtp->nLevel == pPeerFmtp->nLevel &&
                                            pPotentialFmtp->eResolution == pPeerFmtp->eResolution)
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

                    if (pPeerFmtp->eResolution == VIDEO_RESOLUTION_NOT_USED)
                    {
                        VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

                        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                                eTempResolution != VIDEO_RESOLUTION_INVALID)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Prev. Negotiated Resolution[%d]",
                                    pPeerFmtp->eResolution, eTempResolution, 0);
                            pPeerFmtp->eResolution = eTempResolution;
                        }
                        else
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Src Resolution[%d]",
                                    pPeerFmtp->eResolution, pPeerFmtp->eResolution, 0);

                            pPeerFmtp->eResolution = pLocalFmtp->eResolution;
                        }
                    }

                    if (pLocalFmtp->eResolution != pPeerFmtp->eResolution)
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() NOT MATCHED Avc Resolution[%d]<->[%d]",
                                pLocalFmtp->eResolution, pPeerFmtp->eResolution, 0);

                        if (pLocalFmtp->nLevel >= pPeerFmtp->nLevel)
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep profileLevelID[%s]",
                                        pLocalFmtp->strProfileLevelId.GetStr(), 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
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
                                        pLocalFmtp->strProfileLevelId.GetStr(), 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
                            }
                        }
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - Matched payload found, \
                            Profile[%d], Level[%d], Resolution[%d]",
                            pLocalFmtp->nProfile, pLocalFmtp->nLevel, pLocalFmtp->eResolution);

                    // make nego payload
                    VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

                    if (MakeNegotiatedPayload(pLocalPayload, pPeerPayload, pNegoPayload) ==
                            IMS_FALSE)
                    {
                        IMS_TRACE_E(
                                0, "MakeNegotiatedProfile() - Cannot Make Nego payload", 0, 0, 0);
                        continue;
                    }

                    // Make a RTCP-FB negotiation result
                    if (pNegotiatedProfile->bSupportAvpf == IMS_TRUE)
                    {
                        if (pLocalPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.nTrrInt =
                                    pPeerPayload->objRtcpFbAttr.nTrrInt;
                            pNegoPayload->objRtcpFbAttr.bTrrSupported = IMS_TRUE;
                        }

                        if (pLocalPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                        }

                        if (pLocalPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                        }

                        if (pLocalPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                        }

                        if (pLocalPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
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

                    if (pPeerProfile->nNegotiatedPayloadIndex == -1)
                    {
                        pPeerProfile->nNegotiatedPayloadIndex = nPeerIndex;
                        pLocalProfile->nNegotiatedPayloadIndex = nLocalIndex;

                        // MT case : change src PT# to dest PT#
                        if (bIsOfferReceived == IMS_TRUE &&
                                pLocalProfile->nNegotiatedPayloadIndex != -1)
                        {
                            VideoProfile::Payload* pTempNegoLocalPayload =
                                    pLocalProfile->lstPayload.GetAt(
                                            pLocalProfile->nNegotiatedPayloadIndex);
                            pTempNegoLocalPayload->objRtpMap.nPayloadNum =
                                    pPeerPayload->objRtpMap.nPayloadNum;
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
        else if (pPeerPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
        {
            // start source profile loop
            for (IMS_UINT32 nLocalIndex = 0; nLocalIndex < pLocalProfile->lstPayload.GetSize();
                    nLocalIndex++)
            {
                pLocalPayload = pLocalProfile->lstPayload.GetAt(nLocalIndex);
                if (pLocalPayload == IMS_NULL)
                {
                    continue;
                }

                // find matched payload - H265 find options
                if (pLocalPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
                {
                    // FMTP compare
                    VideoProfile::HevcFmtp* pLocalFmtp =
                            (VideoProfile::HevcFmtp*)pLocalPayload->pFmtp;
                    VideoProfile::HevcFmtp* pPeerFmtp =
                            (VideoProfile::HevcFmtp*)pPeerPayload->pFmtp;
                    if (pLocalFmtp == IMS_NULL || pPeerFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - profileId[%d]<->profileId[%d]",
                            pLocalFmtp->nProfile, pPeerFmtp->nProfile, 0);

                    // same level is adapt first, reject higher level
                    if (pLocalFmtp->nLevel < pPeerFmtp->nLevel)
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() - NOT MATCHED HEVC Level[%d]<->[%d]",
                                pLocalFmtp->nLevel, pPeerFmtp->nLevel, 0);

                        if (pTmpPayload == IMS_NULL)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Accept Highest Temp Src \
                                    profileID[%d]",
                                    pLocalFmtp->nProfile, 0, 0);
                            pTmpPayload = pLocalPayload;
                            pMatchedPeerPayload = pPeerPayload;
                        }
                        continue;
                    }

                    if (pPeerFmtp->eResolution == VIDEO_RESOLUTION_NOT_USED)
                    {
                        VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

                        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                                eTempResolution != VIDEO_RESOLUTION_INVALID)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Prev. Negotiated Resolution[%d]",
                                    pPeerFmtp->eResolution, eTempResolution, 0);
                            pPeerFmtp->eResolution = eTempResolution;
                        }
                        else
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Src Resolution[%d]",
                                    pPeerFmtp->eResolution, pPeerFmtp->eResolution, 0);
                            pPeerFmtp->eResolution = pLocalFmtp->eResolution;
                        }
                    }

                    if (pLocalFmtp->eResolution != pPeerFmtp->eResolution)
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() - NOT MATCHED HEVC Resolution\
                                [%d]<->[%d]",
                                pLocalFmtp->eResolution, pPeerFmtp->eResolution, 0);

                        if (pLocalFmtp->nLevel >= pPeerFmtp->nLevel)
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep profile[%d]",
                                        pLocalFmtp->nProfile, 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
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
                                        pLocalFmtp->nProfile, 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
                            }
                        }
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - Matched payload found, \
                            Profile[%d], Level[%d], Resolution[%d]",
                            pLocalFmtp->nProfile, pLocalFmtp->nLevel, pLocalFmtp->eResolution);

                    // make nego payload
                    VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

                    if (MakeNegotiatedPayload(pLocalPayload, pPeerPayload, pNegoPayload) ==
                            IMS_FALSE)
                    {
                        IMS_TRACE_E(0, "MakeNegotiatedProfile() Cannot Make Nego payload", 0, 0, 0);
                        continue;
                    }

                    // Make a RTCP-FB negotiation result
                    if (pNegotiatedProfile->bSupportAvpf == IMS_TRUE)
                    {
                        if (pLocalPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                        }
                        if (pLocalPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                        }
                        if (pLocalPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
                        {
                            pNegoPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                        }
                        if (pLocalPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE &&
                                pPeerPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
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

                    if (pPeerProfile->nNegotiatedPayloadIndex == -1)
                    {
                        pPeerProfile->nNegotiatedPayloadIndex = nPeerIndex;
                        pLocalProfile->nNegotiatedPayloadIndex = nLocalIndex;

                        // MT case : change src PT# to dest PT#
                        if (bIsOfferReceived == IMS_TRUE &&
                                pLocalProfile->nNegotiatedPayloadIndex != -1)
                        {
                            VideoProfile::Payload* pTempNegoLocalPayload =
                                    pLocalProfile->lstPayload.GetAt(
                                            pLocalProfile->nNegotiatedPayloadIndex);
                            pTempNegoLocalPayload->objRtpMap.nPayloadNum =
                                    pPeerPayload->objRtpMap.nPayloadNum;
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
                    pPeerPayload->objRtpMap.strPayloadType.GetStr(), 0, 0);
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

            if (MakeNegotiatedPayload(pTmpPayload, pMatchedPeerPayload, pNegoPayload) == IMS_FALSE)
            {
                IMS_TRACE_E(0, "MakeNegotiatedProfile() - Cannot Make Nego payload", 0, 0, 0);
                return IMS_FALSE;
            }

            if (pMatchedPeerPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
            {
                VideoProfile::AvcFmtp* pAvcFmtp = (VideoProfile::AvcFmtp*)pNegoPayload->pFmtp;

                if (pAvcFmtp == IMS_NULL)
                {
                    return IMS_FALSE;
                }

                VIDEO_RESOLUTION nProperResol = GetAvcMaxResolutionFromLevel(pAvcFmtp->nLevel);

                // if the set resolution is invalid or too big with level, decide resolution via
                // payload pre-set and negotatied level
                IMS_BOOL bFoundResol = IMS_FALSE;

                // first decide with source profile payload
                for (IMS_UINT32 nLocalIndex = 0; nLocalIndex < pLocalProfile->lstPayload.GetSize();
                        nLocalIndex++)
                {
                    VideoProfile::Payload* pPayload = pLocalProfile->lstPayload.GetAt(nLocalIndex);
                    VideoProfile::AvcFmtp* pTempLocalFmtp =
                            reinterpret_cast<VideoProfile::AvcFmtp*>(pPayload->pFmtp);

                    if (pTempLocalFmtp->nLevel <= pAvcFmtp->nLevel)
                    {
                        pAvcFmtp->eResolution = pTempLocalFmtp->eResolution;
                        bFoundResol = IMS_TRUE;
                        break;
                    }
                }

                // decide by level
                if (bFoundResol == IMS_FALSE)
                {
                    pAvcFmtp->eResolution = nProperResol;
                }

                IMS_TRACE_D("MakeNegotiatedProfile() set profile[%s] set Resol[%d]",
                        pAvcFmtp->strProfileLevelId.GetStr(), pAvcFmtp->eResolution, 0);

                if (pNegotiatedProfile->bSupportAvpf == IMS_TRUE)
                {
                    if (pLocalPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bTrrSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.nTrrInt = pPeerPayload->objRtcpFbAttr.nTrrInt;
                        pNegoPayload->objRtcpFbAttr.bTrrSupported = IMS_TRUE;
                    }
                    if (pLocalPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                    }
                    if (pLocalPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                    }
                    if (pLocalPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                    }
                    if (pLocalPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF bNACK[%d], bTMMBR[%d], bPLI[%d]",
                            pNegoPayload->objRtcpFbAttr.bNackSupported,
                            pNegoPayload->objRtcpFbAttr.bTmmbrSupported,
                            pNegoPayload->objRtcpFbAttr.bPliSupported);
                    IMS_TRACE_D(
                            "MakeNegotiatedProfile() - AVPF bFIR[%d], bTRR_Int[%d], nTrr-int[%d]",
                            pNegoPayload->objRtcpFbAttr.bFirSupported,
                            pNegoPayload->objRtcpFbAttr.bTrrSupported,
                            pNegoPayload->objRtcpFbAttr.nTrrInt);
                }

                if (pPeerProfile->nNegotiatedPayloadIndex == -1)
                {
                    pPeerProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pPeerProfile, pMatchedPeerPayload);
                    pLocalProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pLocalProfile, pTmpPayload);

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                            pLocalProfile->nNegotiatedPayloadIndex != -1)
                    {
                        VideoProfile::Payload* pTempNegoLocalPayload =
                                pLocalProfile->lstPayload.GetAt(
                                        pLocalProfile->nNegotiatedPayloadIndex);
                        pTempNegoLocalPayload->objRtpMap.nPayloadNum =
                                pPeerPayload->objRtpMap.nPayloadNum;
                    }
                }

                pNegotiatedProfile->lstPayload.Append(pNegoPayload);
                pNegotiatedPayload = pNegoPayload;
                pNegotiatedProfile->nNegotiatedPayloadIndex = 0;

                if (pAvcFmtp->nFrameRate > nNegotiatedMaxFrameRate)
                {
                    nNegotiatedMaxFrameRate = pAvcFmtp->nFrameRate;
                }
                if (pAvcFmtp->nAs > nNegotiatedMaxAs)
                {
                    nNegotiatedMaxAs = pAvcFmtp->nAs;
                }
            }
            else if (pMatchedPeerPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
            {
                // Make a RTCP-FB negotiation result
                if (pNegotiatedProfile->bSupportAvpf == IMS_TRUE)
                {
                    if (pLocalPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bNackSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                    }
                    if (pLocalPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                    }
                    if (pLocalPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bPliSupported == IMS_TRUE)
                    {
                        pNegoPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                    }
                    if (pLocalPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE &&
                            pPeerPayload->objRtcpFbAttr.bFirSupported == IMS_TRUE)
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

                VideoProfile::HevcFmtp* pTempLocalFmtp =
                        reinterpret_cast<VideoProfile::HevcFmtp*>(pMatchedPeerPayload->pFmtp);
                fmtp->eResolution = pTempLocalFmtp->eResolution;

                if (pPeerProfile->nNegotiatedPayloadIndex == -1)
                {
                    pPeerProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pPeerProfile, pMatchedPeerPayload);
                    pLocalProfile->nNegotiatedPayloadIndex =
                            FindPayloadIndexFromProfile(pLocalProfile, pTmpPayload);

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                            pLocalProfile->nNegotiatedPayloadIndex != -1)
                    {
                        VideoProfile::Payload* pTempNegoLocalPayload =
                                pLocalProfile->lstPayload.GetAt(
                                        pLocalProfile->nNegotiatedPayloadIndex);
                        pTempNegoLocalPayload->objRtpMap.nPayloadNum =
                                pPeerPayload->objRtpMap.nPayloadNum;
                    }
                }

                pNegotiatedProfile->lstPayload.Append(pNegoPayload);
                pNegotiatedPayload = pNegoPayload;
                pNegotiatedProfile->nNegotiatedPayloadIndex = 0;

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
        if (pNegotiatedProfile->nDataPort == 0 || pPeerProfile->nDataPort == 0 ||
                pNegotiatedProfile->lstPayload.GetSize() == 0)
        {
            pNegotiatedProfile->eDirection = MEDIA_DIRECTION_INVALID;
        }
        else
        {
            pNegotiatedProfile->eDirection = UpdateDirectionToMine(
                    pPeerProfile->eDirection, pLocalProfile->eDirection, bIsOfferReceived);
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

        // Setting bandwidth AS/RS/RR
        VideoProfileUtil::MakeNegotiatedBandwidth(m_pConfig, pLocalProfile, pPeerProfile,
                bIsOfferReceived, nNegotiatedMaxAs, pNegotiatedProfile);

        // Setting framerate
        pNegotiatedProfile->nFrameRate = nNegotiatedMaxFrameRate;

        // Candidate Priority (no need in video)

        // CVO mode
        if (pLocalProfile->nCvoId > 0 && pPeerProfile->nCvoId > 0)
        {
            pNegotiatedProfile->nCvoId = pPeerProfile->nCvoId;
        }
        else
        {
            if (pNegotiatedProfile->nDataPort == 0)
            {
                pNegotiatedProfile->nCvoId = pLocalProfile->nCvoId;
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
        if (pLocalProfile->lstPayload.GetSize() > 0)
        {
            IMS_TRACE_D("MakeNegotiatedProfile() There's no negotiated payload. \
                    copy LocalProfile and make port 0 ",
                    0, 0, 0);

            *pNegotiatedProfile = *pLocalProfile;
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
            pLocalProfile->nNegotiatedPayloadIndex, pPeerProfile->nNegotiatedPayloadIndex, 0);

    return ret;
}

PRIVATE
IMS_BOOL VideoNego::GetFmtpFromString(IN const AString& strFmtp, OUT VideoProfile::AvcFmtp* pFmtp)
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
            pFmtp->nProfile =
                    VideoProfileUtil::GetAvcProfileFromProfileLevelId(pFmtp->strProfileLevelId);
            pFmtp->nLevel =
                    VideoProfileUtil::GetAvcLevelFromProfileLevelId(pFmtp->strProfileLevelId);
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

            ImsList<AString> objSplitComma = strRealSpropParam.Split(',');
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

PRIVATE
IMS_BOOL VideoNego::GetFmtpFromString(IN const AString& strFmtp, OUT VideoProfile::HevcFmtp* pFmtp)
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

PRIVATE
VideoProfile::Payload* VideoNego::FindPayloadInProfile(
        IN VideoProfile* pProfile, IN VideoProfile::Payload* pTargetPayload)
{
    if (pProfile == IMS_NULL || pTargetPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    VideoProfile::Payload* pTempPayload = IMS_NULL;  // To keep secondary payload

    if (m_pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        VideoProfile::Payload* pOriginPayload = pProfile->lstPayload.GetAt(i);
        if (pOriginPayload == IMS_NULL)
        {
            continue;
        }

        if ((pOriginPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
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
                    VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

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
                    VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

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

PRIVATE
IMS_SINT32 VideoNego::FindPayloadIndexFromProfile(
        IN VideoProfile* pProfile, IN const VideoProfile::Payload* pPayload)
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

PRIVATE
MEDIA_DIRECTION VideoNego::UpdateDirectionToMine(
        IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase)
{
    IMS_TRACE_D("UpdateDirectionToMine() - ePeerDir[%d], eSrcDir[%d], bIsMtCase[%d]", ePeerDir,
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

PRIVATE IMS_BOOL VideoNego::GetCorrectImageIndex(
        IN IMS_SINT32 nPayloadTypeNum, IN ImsList<AString> objAttributes, OUT IMS_UINT32* nIndex)
{
    for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); i++)
    {
        ImsList<AString> objTokens = objAttributes.GetAt(i).Split(TextParser::CHAR_SP);

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

PRIVATE VIDEO_RESOLUTION VideoNego::GetResolutionFromSdp(IN VIDEO_CODEC codecType,
        IN const AString& strImageAttrFromSdp, IN const AString& strFrameSizeFromSdp,
        IN const AString& strSpropParam, IN IMS_SINT32 nQcif)
{
    IMS_UINT32 nWidth, nHeight;

    // Get nWidth, nHeight From Image Attribute
    if (strImageAttrFromSdp.GetLength() != 0 &&
            (GetWidthHeightFromSdp_ImageAttr(strImageAttrFromSdp, &nWidth, &nHeight) != IMS_FALSE))
    {
        return GetResolutionFromWidthHeight(nWidth, nHeight);
    }

    /** TODO_MEDIA video sprop */
    (void)codecType;
    (void)strSpropParam;
    // // - Get nWidth, nHeight From SpropParam
    // if (strSpropParam.GetLength() != 0 &&
    //        (GetWidthHeightFromSdp_SpropParam(
    //                 codecType, strSpropParam.GetStr(), &nWidth, &nHeight) != IMS_FALSE))
    // {
    //    return GetResolutionFromWidthHeight(nWidth, nHeight);
    // }

    // Get nWidth, nHeight From Framesize
    if (strFrameSizeFromSdp.GetLength() != 0 &&
            (GetWidthHeightFromSdp_FrameSize(strFrameSizeFromSdp, &nWidth, &nHeight) != IMS_FALSE))
    {
        return GetResolutionFromWidthHeight(nWidth, nHeight);
    }

    // Check if nQcif exist
    else if (nQcif != -1)
    {
        return VIDEO_RESOLUTION_QCIF_LS;
    }
    else
    {
        IMS_TRACE_E(0, "GetResolutionFromSdp() - No preferred resolution from SDP...", 0, 0, 0);
        return VIDEO_RESOLUTION_NOT_USED;
    }
}

PRIVATE IMS_BOOL VideoNego::GetWidthHeightFromSdp_ImageAttr(IN const AString& strImageAttrFromSdp,
        OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    IMS_UINT32 nImagePayloadNum = 0;  // Payload Number in Image Attr
    IMS_UINT32 nDirection = 1;        // Direction : send or recv
    IMS_UINT32 nImageValueIndex = 2;  // Image value : width and height
    ImsList<AString> objTokens;
    ImsList<AString> strTempValue;
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
                ImsList<AString> strSendValue = strTempValue.GetAt(i).Split(TextParser::CHAR_EQUAL);

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
                /* TODO: need to add
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
                */
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
                ImsList<AString> strRecvValue = strTempValue.GetAt(i).Split(TextParser::CHAR_EQUAL);

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
                /* TODO: Need to add
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
                */
            }
        }

        IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() nImagePayloadNum[%d] ImageWidth[%d], "
                    "ImageHeight[%d]",
                nImagePayloadNum, (*nImageWidth), (*nImageHeight));
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/** TODO_MEDIA video sprop
PRIVATE IMS_BOOL VideoNego::GetWidthHeightFromSdp_SpropParam(IN VIDEO_CODEC codecType,
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

PRIVATE IMS_BOOL VideoNego::GetWidthHeightFromSdp_FrameSize(
        IN AString strFrameSizeFromSdp, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    IMS_UINT32 nFrameSizePayloadNum = 0;  // Payload Number in Image Attr

    ImsList<AString> objTokens = strFrameSizeFromSdp.Split(TextParser::CHAR_SP);

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
        ImsList<AString> strFrameSizeValue = objTokens.GetAt(1).Split(TextParser::CHAR_HYPHEN);

        *nImageWidth = strFrameSizeValue.GetAt(0).ToInt32();   // Width
        *nImageHeight = strFrameSizeValue.GetAt(1).ToInt32();  // Height
        IMS_TRACE_D("GetWidthHeightFromSdp_FrameSize() nFrameSizePayloadNum[%d] ImageWidth[%d], "
                    "ImageHeight[%d]",
                nFrameSizePayloadNum, *nImageWidth, *nImageHeight);

        return IMS_TRUE;
    }
}

PRIVATE VIDEO_RESOLUTION VideoNego::GetResolutionFromWidthHeight(
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

PRIVATE IMS_BOOL VideoNego::MakeImageAttributeLine(
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

PRIVATE IMS_BOOL VideoNego::MakeFrameSizeLine(
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

PRIVATE IMS_BOOL VideoNego::GetAvpfFromAttributes(IN SdpMediaFormat* pMediaFormat,
        IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr)
{
    if (pMediaFormat == IMS_NULL || pRtcpFbAttr == IMS_NULL || pCapaNego == IMS_NULL)
        return IMS_FALSE;

    ImsList<SdpMediaFormatParameter*> lstRTCPFeedback = pMediaFormat->GetExtraParameters();

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
            SdpRtcpFeedback* pRtcpParam = static_cast<SdpRtcpFeedback*>(pMediaParam);
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
                ImsList<AString> strTemp =
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

PRIVATE IMS_BOOL VideoNego::GetAvpfFromAttributes_EX(
        IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr)
{
    if (pRtcpFbAttr == IMS_NULL || pCapaNego == IMS_NULL)
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
                ImsList<AString> strTemp =
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

PRIVATE IMS_BOOL VideoNego::MakeCapaNegoProfileFromSdp(
        IN IMediaDescriptor* pDescriptor, OUT VideoProfile::CapaNego* pObjCapaNego)
{
    IMS_TRACE_I("MakeCapaNegoProfileFromSdp()", 0, 0, 0);

    if (pDescriptor == IMS_NULL || pObjCapaNego == IMS_NULL)
        return IMS_FALSE;

    ImsList<AString> lstTCAPAttr = pDescriptor->GetAttributes(SdpAttribute::TCAP);
    ImsList<AString> lstACAPAttr = pDescriptor->GetAttributes(SdpAttribute::ACAP);
    ImsList<AString> lstACFGAttr = pDescriptor->GetAttributes(SdpAttribute::ACFG);

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
    for (IMS_UINT32 i = 0; i < lstTCAPAttr.GetSize(); i++)
    {
        AString strTCAPline = lstTCAPAttr.GetAt(i);
        if (strTCAPline.GetLength() == 0)
            continue;

        ImsList<AString> lstSplitSpace = strTCAPline.Split(' ');
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

    // Get attribute capability(ACAP) list - "'number' SP 'Acap'" pair
    for (IMS_UINT32 i = 0; i < lstACAPAttr.GetSize(); i++)
    {
        AString strAcap = "";
        IMS_SINT32 nAcapNum = 0;
        AString strACAPline = lstACAPAttr.GetAt(i);
        if (strACAPline.GetLength() == 0)
            continue;

        ImsList<AString> lstSplitSpace = strACAPline.Split(' ');

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

PRIVATE IMS_BOOL VideoNego::MakeNegotiatedCapaNegoProfile(IN VideoProfile::CapaNego* pLocalCapaNego,
        IN VideoProfile::CapaNego* pPeerCapaNego, OUT VideoProfile::CapaNego* pNegotiatedCapaNego)
{
    if (pLocalCapaNego == IMS_NULL || pPeerCapaNego == IMS_NULL || pNegotiatedCapaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedCapaNego() invalid argument, %" PFLS_x " %" PFLS_x,
                pLocalCapaNego, pPeerCapaNego, 0);
        return IMS_FALSE;
    }

    IMS_BOOL ret = IMS_FALSE;
    IMS_UINT32 i = 0, j = 0, k = 0, l = 0;
    IMS_BOOL bAttributeCheckable = IMS_FALSE;
    IMS_BOOL bPCFGSupportable = IMS_FALSE;

    ImsMap<IMS_SINT32, AString> mapLocalTCap = pLocalCapaNego->mapTransportCapa;
    ImsMap<IMS_SINT32, AString> mapLocalACap = pLocalCapaNego->mapAttributeCapa;

    ImsList<AString> lstDstPCFG = pPeerCapaNego->lstPotentialConfig;
    ImsMap<IMS_SINT32, AString> mapPeerTCap = pPeerCapaNego->mapTransportCapa;
    ImsMap<IMS_SINT32, AString> mapPeerACap = pPeerCapaNego->mapAttributeCapa;

    if (pPeerCapaNego->strNegotiatedAcfg.GetLength() > 0)
    {
        if (mapPeerTCap.IsEmpty() == IMS_TRUE)
        {
            pNegotiatedCapaNego->mapTransportCapa = mapLocalTCap;
        }

        IMS_TRACE_I("MakeNegotiatedCapaNego() ACFG - %s", pPeerCapaNego->strNegotiatedAcfg.GetStr(),
                0, 0);
        return IMS_TRUE;
    }

    // parse pcfg
    for (i = 0; i < lstDstPCFG.GetSize(); i++)
    {
        AString strPCFGline = lstDstPCFG.GetAt(i);  // get "# t=# a=#,#,#,# ..."
        if (strPCFGline.GetLength() == 0)
            continue;

        ImsList<AString> lstSplitSpace = lstDstPCFG.GetAt(i).Split(' ');

        for (j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            bAttributeCheckable = IMS_FALSE;
            if (j == 1)  // t=#
            {
                AString strPCFG_Transport = lstSplitSpace.GetAt(j);
                if (strPCFG_Transport.GetLength() == 0)
                    continue;

                ImsList<AString> lstSplitEquals = strPCFG_Transport.Split('=');
                if (lstSplitEquals.GetSize() == 0)
                    continue;

                if (lstSplitEquals.GetAt(0).Equals('t') == IMS_TRUE && lstSplitEquals.GetSize() > 1)
                {
                    // compare transport capa
                    AString strTmp = mapPeerTCap.GetValue(lstSplitEquals.GetAt(1).ToInt32());

                    for (k = 0; k < mapLocalTCap.GetSize(); k++)
                    {
                        if (strTmp.Equals(mapLocalTCap.GetValueAt(k)) == IMS_TRUE)
                        {
                            bAttributeCheckable = IMS_TRUE;
                            bPCFGSupportable = IMS_TRUE;

                            // set Negotiated Transport Capa Nego Value...
                            pNegotiatedCapaNego->mapTransportCapa.Add(
                                    lstSplitEquals.GetAt(1).ToInt32(), strTmp);
                            break;
                        }
                    }

                    // if there are no matched transport capa, then next pcfg check...
                    // -----it's first for_loop break case..
                    if (bAttributeCheckable == IMS_FALSE)
                    {
                        IMS_TRACE_I("MakeNegotiatedCapaNego() does not match transport capa - PCFG "
                                    "#[%d]",
                                i, 0, 0);
                        break;
                    }
                    // if there are matched transport capa, check attribute capa
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

                ImsList<AString> lstSplitEquals = strPCFG_Attribute.Split('=');
                if (lstSplitEquals.GetSize() == 0)
                    continue;

                if (lstSplitEquals.GetAt(0).Equals('a') == IMS_TRUE && lstSplitEquals.GetSize() > 1)
                {
                    IMS_UINT32 cnt = 0;
                    // compare Attribute capa
                    AString strTmp = lstSplitEquals.GetAt(1);  // tmp = "1,2,3,4"

                    // attribute comma parsing..
                    ImsList<AString> lstSplitComma = strTmp.Split(',');
                    IMS_TRACE_I("MakeNegotiatedCapaNego() attribute size[%d]",
                            lstSplitComma.GetSize(), 0, 0);

                    if (lstSplitComma.GetSize() == 0)
                        continue;

                    // check attribute capa negotiation
                    for (k = 0; k < lstSplitComma.GetSize(); k++)
                    {
                        AString strDestAttributeCapa =
                                mapPeerACap.GetValue(lstSplitComma.GetAt(k).ToInt32());
                        IMS_TRACE_I("MakeNegotiatedCapaNego() strDestAttributeCapa [%s]",
                                strDestAttributeCapa.GetStr(), 0, 0);

                        if (strDestAttributeCapa.Contains("trr-int") == IMS_TRUE ||
                                strDestAttributeCapa.Contains("nack") == IMS_TRUE)
                        {
                            cnt++;
                            pNegotiatedCapaNego->mapAttributeCapa.Add(
                                    lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                        }
                        else if (strDestAttributeCapa.Contains("ccm") == IMS_TRUE)
                        {
                            if (strDestAttributeCapa.Contains("fir") == IMS_TRUE ||
                                    strDestAttributeCapa.Contains("tmmbr") == IMS_TRUE)
                            {
                                cnt++;
                                pNegotiatedCapaNego->mapAttributeCapa.Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                            }
                        }
                        else if (strDestAttributeCapa.Contains("crypto") == IMS_TRUE)
                        {
                            // crypto attribute negotiate only srtp profile type
                            ImsList<AString> lstSrcCryptoAttribute =
                                    mapLocalACap.GetValueAt(l).Split(' ');
                            ImsList<AString> lstDestCryptoAttribute =
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

                    // if ue support pcfg about transport capa, bPCFGSupportable variable set to
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

PRIVATE IMS_BOOL VideoNego::CheckAvpfFromProfile(IN VideoProfile* pProfile)
{
    IMS_TRACE_I("CheckAvpfFromProfile()", 0, 0, 0);

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

PRIVATE VideoNego::OaModel* VideoNego::GetNegotiatedOaModel()
{
    IMS_UINT32 nOaModelCount = m_listOaModel.GetSize();
    IMS_UINT32 nTempOaModelCount = nOaModelCount;

    while (nTempOaModelCount > 0)
    {
        OaModel* pLatestOaModel = m_listOaModel.GetAt(nTempOaModelCount - 1);

        if (pLatestOaModel != IMS_NULL)
        {
            if (pLatestOaModel->IsAllProfileExist() == IMS_TRUE)
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

PRIVATE IMS_BOOL VideoNego::GetWidthHeightFromResolutionId(
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

PRIVATE
VIDEO_RESOLUTION VideoNego::GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel)
{
    IMS_TRACE_D("GetAvcMaxResolutionFromLevel() - Level[%d]", nLevel, 0, 0);

    if (nLevel > 31)
    {
        nLevel = 31;
    }

    // default resoltuion is portrait
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
