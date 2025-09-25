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

#include "video/VideoNego.h"
#include "ISessionDescriptor.h"
#include "MediaEnvironment.h"
#include "MediaProfileFactory.h"
#include "MediaProfileUtil.h"
#include "ServiceTrace.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaSessionConfigFactory.h"
#include "video/VideoProfileGenerator.h"
#include "video/VideoSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC VideoNego::VideoNego(IN const IMS_SINT32 nSlotId) :
        BaseNego(nSlotId, MEDIA_TYPE_VIDEO),
        m_pSdpParser(std::make_shared<VideoSdpParser>()),
        m_pProfileNegotiator(std::make_shared<VideoProfileNegotiator>())
{
    IMS_TRACE_I("+VideoNego(): slot[%d]", nSlotId, 0, 0);

    m_pSdpGenerator = std::make_shared<VideoSdpGenerator>();
    m_pProfileGenerator = std::make_shared<VideoProfileGenerator>();
}

PUBLIC
VideoNego::VideoNego(IN const VideoNego& obj) :
        BaseNego(obj),
        m_pSdpParser(std::make_shared<VideoSdpParser>()),
        m_pProfileNegotiator(std::make_shared<VideoProfileNegotiator>())
{
    IMS_TRACE_I("+VideoNego(): slot[%d]", GetSlotId(), 0, 0);

    m_pSdpGenerator = std::make_shared<VideoSdpGenerator>();
    m_pProfileGenerator = std::make_shared<VideoProfileGenerator>();
    Copy(&obj);
}

PUBLIC
VideoNego& VideoNego::operator=(IN const VideoNego& obj)
{
    if (this != &obj)
    {
        BaseNego::operator=(obj);
        m_pSdpParser = std::make_shared<VideoSdpParser>();
        m_pSdpGenerator = std::make_shared<VideoSdpGenerator>();
        m_pProfileNegotiator = std::make_shared<VideoProfileNegotiator>();
        m_pProfileGenerator = std::make_shared<VideoProfileGenerator>();
        Copy(&obj);
    }

    return (*this);
}

PUBLIC VIRTUAL VideoNego::~VideoNego()
{
    IMS_TRACE_I("~VideoNego()", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL VideoNego::IsMediaCodecFromSdpSupported(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    OaModel objOaModel;
    objOaModel.pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile.get());

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(objOaModel)))
    {
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): failed to parse SDP", 0, 0, 0);
        return IMS_FALSE;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(objOaModel), GetPeerProfile(objOaModel),
                IMS_TRUE, GetNegotiatedProfile(objOaModel), m_pConfig))
    {
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): failed to negotiate SDP", 0, 0, 0);
        return IMS_FALSE;
    }

    return (objOaModel.pNegotiatedProfile != IMS_NULL &&
                   objOaModel.pNegotiatedProfile->GetPayloadList().GetSize() > 0 &&
                   objOaModel.pNegotiatedProfile->GetDataPort() != 0)
            ? IMS_TRUE
            : IMS_FALSE;
}

PUBLIC VIDEO_RESOLUTION VideoNego::GetNegotiatedResolution()
{
    MediaBaseProfile::BasePayload* pPayload = GetNegotiatedPayload();

    if (m_pProfileNegotiator != IMS_NULL && pPayload != IMS_NULL)
    {
        return m_pProfileNegotiator->GetNegotiatedResolution(pPayload);
    }

    return VIDEO_RESOLUTION_INVALID;
}

PROTECTED VideoProfile* VideoNego::GetLocalProfile(IN const OaModel& objOaModel)
{
    return static_cast<VideoProfile*>(BaseNego::GetLocalProfile(objOaModel));
}

PROTECTED VideoProfile* VideoNego::GetPeerProfile(IN const OaModel& objOaModel)
{
    return static_cast<VideoProfile*>(BaseNego::GetPeerProfile(objOaModel));
}

PROTECTED VideoProfile* VideoNego::GetNegotiatedProfile(IN const OaModel& objOaModel)
{
    return static_cast<VideoProfile*>(BaseNego::GetNegotiatedProfile(objOaModel));
}

PROTECTED
IMS_BOOL VideoNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    if (CheckArgument(pSessionDescriptor, pDescriptor, eDirection) && m_pSdpGenerator)
    {
        // Make the SDP from profile
        return m_pSdpGenerator->Generate(pSessionDescriptor, pDescriptor,
                GetLocalProfile(*CreateOaModel(eDirection, bDisable)));
    }

    return IMS_FALSE;
}

PROTECTED IMS_BOOL VideoNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_pSdpGenerator == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormAnswer(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_listOaModel.IsEmpty())
    {
        IMS_TRACE_E(0, "FormAnswer(): empty OA model list", 0, 0, 0);
        return IMS_FALSE;
    }

    if (eDirection == MEDIA_DIRECTION_INVALID && !bDisable)
    {
        IMS_TRACE_E(0, "FormAnswer(): invalid direction", 0, 0, 0);
        return IMS_FALSE;
    }

    // Getting OaModel from list
    std::shared_ptr<OaModel> pNewOaModel = GetNegotiatedOaModel();

    if (pNewOaModel == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormAnswer(): invalid OA model", 0, 0, 0);
        return IMS_FALSE;
    }

    // Modify a RTP/RTCP port to ZERO if video is not supported
    if (bDisable)
    {
        pNewOaModel->pNegotiatedProfile->SetDataPort(0);
        pNewOaModel->pNegotiatedProfile->SetControlPort(0);
    }

    IMS_TRACE_D("FormAnswer(): direction[%d], disable[%d], port[%d]", eDirection, bDisable,
            pNewOaModel->pNegotiatedProfile->GetDataPort());

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDirection))
    {
        pNewOaModel->pNegotiatedProfile->SetDirection(eDirection);
    }
    else
    {
        MEDIA_DIRECTION eTempDirection = m_pProfileNegotiator->UpdateDirectionToMine(
                pNewOaModel->pPeerProfile->GetDirection(),
                pNewOaModel->pLocalProfile->GetDirection(), IMS_FALSE);
        pNewOaModel->pNegotiatedProfile->SetDirection(eTempDirection);
    }

    // Make the SDP from profile
    return m_pSdpGenerator->Generate(
            pSessionDescriptor, pDescriptor, GetNegotiatedProfile(*pNewOaModel));
}

PROTECTED IMS_BOOL VideoNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL || m_pSdpGenerator == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormReoffer(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (eDirection == MEDIA_DIRECTION_INVALID && !bDisable)
    {
        IMS_TRACE_E(0, "FormReoffer(): invalid direction", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("FormReoffer(): direction[%d], OA model[%d], reOffer[%d]", eDirection,
            m_listOaModel.GetSize(), bEnforceReofferMode);

    // Remove session level direction
    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);

    // Make new Offer/Answer model, and copy source profile from previous negotiated profile
    std::shared_ptr<OaModel> pNewOaModel = std::make_shared<OaModel>();

    const MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    GetSlotId(), m_pEnvironment->eServiceType);

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
            IMS_TRACE_E(0, "FormReoffer(): invalid state", 0, 0, 0);
            return IMS_FALSE;
        }

        if (pPrevOaModel->pNegotiatedProfile != IMS_NULL &&
                pPrevOaModel->pNegotiatedProfile->GetDataPort() == 0 && !bDisable)  // upgrade case
        {
            pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                    m_eType, m_pBaseProfile.get());
        }
        else
        {
            if (bEnforceReofferMode || pMediaSessionConfig->IsSdpReofferFullCapability())
            {
                pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                        m_eType, m_pBaseProfile.get());
            }
            else
            {
                if (pPrevOaModel->pNegotiatedProfile != IMS_NULL)
                {
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            m_eType, GetNegotiatedProfile(*pPrevOaModel));
                }
                else
                {
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            m_eType, m_pBaseProfile.get());
                }
            }
        }
    }

    if (pNewOaModel->pLocalProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormReoffer(): invalid local profile", 0, 0, 0);
        return IMS_FALSE;
    }

    // Modify a RTP/RTCP port if video is not supported
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

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDirection))
    {
        pNewOaModel->pLocalProfile->SetDirection(eDirection);
    }
    else
    {
        pNewOaModel->pLocalProfile->SetDataPort(0);
        pNewOaModel->pLocalProfile->SetControlPort(0);
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    /* bDirHold is not proper for video stream */
    MediaProfileUtil::SetRtcpRsRr(GetLocalProfile(*pNewOaModel), m_pConfig, IMS_FALSE);
    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    return m_pSdpGenerator->Generate(
            pSessionDescriptor, pDescriptor, GetLocalProfile(*pNewOaModel));
}

PROTECTED MEDIA_DIRECTION VideoNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateOffer(): invalid arguments", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

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

    pNewOaModel->pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(*pNewOaModel),
                GetPeerProfile(*pNewOaModel), IMS_TRUE, GetNegotiatedProfile(*pNewOaModel),
                m_pConfig))
    {
        IMS_TRACE_E(0, "NegotiateOffer(): failed to negotiate SDP", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    m_listOaModel.Append(pNewOaModel);
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}

PROTECTED MEDIA_DIRECTION VideoNego::NegotiateAnswer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): invalid arguments", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    if (m_listOaModel.IsEmpty())
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): empty OA model list", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // Get the latest OA model from list
    std::shared_ptr<OaModel> pNewOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

    if (pNewOaModel == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): invalid OA model", 0, 0, 0);
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

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(*pNewOaModel),
                GetPeerProfile(*pNewOaModel), IMS_FALSE, GetNegotiatedProfile(*pNewOaModel),
                m_pConfig))
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): failed to negotiate SDP", 0, 0, 0);
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    return pNewOaModel->pNegotiatedProfile->GetDirection();
}
