/**
 * Copyright (C) 2024 The Android Open Source Project
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

#include "BaseNego.h"
#include "MediaNegoUtil.h"
#include "MediaProfileFactory.h"
#include "MediaProfileGenerator.h"
#include "MediaProfileUtil.h"

#include "audio/AudioSdpGenerator.h"
#include "text/TextSdpGenerator.h"
#include "video/VideoSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC BaseNego::BaseNego(IN const IMS_SINT32 nSlotId, IN const MEDIA_CONTENT_TYPE eType) :
        ImsSlot(nSlotId),
        m_eType(eType),
        m_pBaseProfile(new MediaBaseProfile()),
        m_listOaModel(ImsList<OaModel*>()),
        m_pConfig(IMS_NULL),
        m_pEnvironment(IMS_NULL),
        m_pSdpGenerator(IMS_NULL),
        m_pProfileNegotiator(IMS_NULL),
        m_pProfileGenerator(IMS_NULL)
{
    IMS_TRACE_I("+BaseNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC VIRTUAL BaseNego::~BaseNego()
{
    IMS_TRACE_I("~BaseNego()", 0, 0, 0);

    if (m_pBaseProfile != IMS_NULL)
    {
        MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
        m_pBaseProfile->DeletePayloads();
    }

    delete m_pBaseProfile;
    m_pBaseProfile = IMS_NULL;

    DestroyListOaModel();
}

PUBLIC VIRTUAL void BaseNego::CreateProfiles(
        IN MediaEnvironment* pEnvironment, IN MediaConfiguration* pConfig)
{
    if (pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfiles() - invalid configuration", 0, 0, 0);
        return;
    }

    if (m_pBaseProfile != IMS_NULL && m_pBaseProfile->GetDataPort() != 0)
    {
        MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
    }
    delete m_pBaseProfile;

    IMS_TRACE_I("CreateProfiles()", 0, 0, 0);

    m_pEnvironment = pEnvironment;
    m_pConfig = pConfig;

    if (m_pProfileGenerator != IMS_NULL)
    {
        m_pBaseProfile = m_pProfileGenerator->Generate(pEnvironment, m_pConfig, GetSlotId());
    }
}

PUBLIC VIRTUAL IMS_BOOL BaseNego::FormSdp(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
        IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable, IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormSdp() - State[%d], OaModel size[%d]", eNegoState, m_listOaModel.GetSize(), 0);
    IMS_TRACE_D("FormSdp() - eDir[%d], bDisable[%d] EnforceReofferMode[%d]", eDir, bDisable,
            bEnforceReofferMode);

    switch (eNegoState)
    {
        case STATE_IDLE:
            return FormOffer(pSessionDescriptor, pDescriptor, eDir, bDisable);
        case STATE_OFFER_RECEIVED:
            return FormAnswer(pSessionDescriptor, pDescriptor, eDir, bDisable);
        case STATE_NEGOTIATED:
            return FormReoffer(
                    pSessionDescriptor, pDescriptor, eDir, bDisable, bEnforceReofferMode);
        default:
            return IMS_FALSE;
    }
}

PUBLIC VIRTUAL void BaseNego::NegotiateSdp(IN NEGO_STATE eNegoState,
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

PUBLIC VIRTUAL void BaseNego::FinalizeSdp(
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

PUBLIC const IpAddress& BaseNego::GetNegotiatedRemoteAddress()
{
    MediaBaseProfile* pProfile = GetNegotiatedPeerProfile();

    return (pProfile != IMS_NULL) ? pProfile->GetIpAddress() : IpAddress::NONE;
}

PUBLIC VIRTUAL IMS_SINT32 BaseNego::GetRemotePort()
{
    MediaBaseProfile* pProfile = GetNegotiatedPeerProfile();

    return (pProfile != IMS_NULL) ? pProfile->GetDataPort() : MEDIA_PORT_INVALID;
}

PUBLIC VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedLocalProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return GetLocalProfile(pOaModel);
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedNegoProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return GetNegotiatedProfile(pOaModel);
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedPeerProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return GetPeerProfile(pOaModel);
    }

    return IMS_NULL;
}

PUBLIC
MEDIA_DIRECTION BaseNego::GetNegotiatedDirection()
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
            return pLatestOaModel->pNegotiatedProfile->GetDirection();
        }
    }
    return MEDIA_DIRECTION_INVALID;
}

PUBLIC IMS_SINT32 BaseNego::GetNegotiatedRtpPort()
{
    const IMS_SINT32 PORT_NONE = -1;

    if (m_listOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel();

        if (pLatestOaModel == IMS_NULL || pLatestOaModel->IsAllProfileExist() == IMS_FALSE)
        {
            return PORT_NONE;
        }

        IMS_TRACE_I("GetNegotiatedRtpPort() - Previous negotiated port[%d] found",
                pLatestOaModel->pNegotiatedProfile->GetDataPort(), 0, 0);

        return (IMS_SINT32)pLatestOaModel->pNegotiatedProfile->GetDataPort();
    }

    return PORT_NONE;
}

PUBLIC IMS_SINT32 BaseNego::GetNegotiatedBandwidth()
{
    if (m_listOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

        if (pLatestOaModel == IMS_NULL || GetLocalProfile(pLatestOaModel) == IMS_NULL)
        {
            return -1;
        }

        return (GetNegotiatedProfile(pLatestOaModel) != IMS_NULL)
                ? (IMS_SINT32)GetNegotiatedProfile(pLatestOaModel)->GetBandwidthAs()
                : (IMS_SINT32)GetLocalProfile(pLatestOaModel)->GetBandwidthAs();
    }

    return -1;
}

PUBLIC MediaBaseProfile::BasePayload* BaseNego::GetNegotiatedPayload()
{
    if (m_listOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = GetNegotiatedOaModel();

        if (pLatestOaModel == IMS_NULL || pLatestOaModel->IsAllProfileExist() == IMS_FALSE)
        {
            return IMS_NULL;
        }

        MediaBaseProfile* pProfile = GetNegotiatedProfile(pLatestOaModel);

        if (pProfile == IMS_NULL)
        {
            return IMS_NULL;
        }
        if (pProfile->GetPayloadList().GetSize() == 0)
        {
            if (pProfile->GetDataPort() == 0)
            {
                IMS_TRACE_D("GetNegotiatedPayload() - empty Payload list, zero port", 0, 0, 0);
                pProfile->CopyPayloads(GetLocalProfile(pLatestOaModel)->GetPayloadList());
            }
            else
            {
                return IMS_NULL;
            }
        }

        return (pProfile->GetNegotiatedPayloadIndex() > 0)
                ? pProfile->GetPayloadAt(pProfile->GetNegotiatedPayloadIndex())
                : pProfile->GetPayloadAt(0);
    }

    return IMS_NULL;
}

PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetLocalProfile(IN OaModel* pOaModel)
{
    return (pOaModel != IMS_NULL) ? pOaModel->pLocalProfile : IMS_NULL;
}
PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetPeerProfile(IN OaModel* pOaModel)
{
    return (pOaModel != IMS_NULL) ? pOaModel->pPeerProfile : IMS_NULL;
}
PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedProfile(IN OaModel* pOaModel)
{
    return (pOaModel != IMS_NULL) ? pOaModel->pNegotiatedProfile : IMS_NULL;
}

PROTECTED
BaseNego::OaModel* BaseNego::GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed)
{
    IMS_UINT32 nTempOaModelCount = m_listOaModel.GetSize();
    IMS_TRACE_I("GetNegotiatedOaModel()", 0, 0, 0);
    while (nTempOaModelCount > 0)
    {
        OaModel* pLatestOaModel = m_listOaModel.GetAt(nTempOaModelCount - 1);

        if (pLatestOaModel != IMS_NULL)
        {
            if ((pLatestOaModel->IsAllProfileExist() == IMS_TRUE && bCheckConfirmed == IMS_FALSE) ||
                    (pLatestOaModel->bConfirmedSession == IMS_TRUE && bCheckConfirmed == IMS_TRUE))
            {
                return pLatestOaModel;
            }

            IMS_TRACE_I("GetNegotiatedOaModel() - [%d/%d]th is not perfect. Try next",
                    nTempOaModelCount, m_listOaModel.GetSize(), 0);
        }
        nTempOaModelCount--;
    }

    return IMS_NULL;
}

PUBLIC IMS_BOOL BaseNego::SetPort(IN IMS_UINT32 nPort)
{
    if (m_pBaseProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());

    IMS_TRACE_I("SetPort() - Changed Data Port[%d]->[%d]", m_pBaseProfile->GetDataPort(), nPort, 0);

    if (nPort != 0)
    {
        m_pBaseProfile->SetDataPort(MediaNegoUtil::AcquireRtpPort(GetSlotId(), nPort));
        m_pBaseProfile->SetControlPort(m_pBaseProfile->GetDataPort() + 1);
    }
    else
    {
        m_pBaseProfile->SetDataPort(0);
        m_pBaseProfile->SetControlPort(0);

        IMS_TRACE_I("SetPort() - Data Port is 0!!!", 0, 0, 0);
    }

    return IMS_TRUE;
}

PROTECTED void BaseNego::DestroyListOaModel()
{
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

PROTECTED
void BaseNego::Copy(IN const BaseNego* pNego)
{
    if (pNego == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("Copy()", 0, 0, 0);

    if (m_pBaseProfile != IMS_NULL)
    {
        MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
        delete m_pBaseProfile;
    }

    m_pBaseProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, pNego->m_pBaseProfile);

    if (m_pBaseProfile != IMS_NULL && m_pBaseProfile->GetDataPort() != 0)
    {
        MediaNegoUtil::AcquireRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
    }

    m_pEnvironment = pNego->m_pEnvironment;
    m_pConfig = pNego->m_pConfig;

    OaModel* pNewOaModel = new OaModel();
    if (pNewOaModel != IMS_NULL)
    {
        pNewOaModel->pLocalProfile =
                MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile);
        m_listOaModel.Append(pNewOaModel);
    }
    IMS_TRACE_I("Copy() - OA model list size[%d]", m_listOaModel.GetSize(), 0, 0);
}

PROTECTED
IMS_BOOL BaseNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pSdpGenerator == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (eDir == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_E(0, "FormOffer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("FormOffer() - media type[%d], eDir[%d], bDisable[%d]", m_eType, eDir, bDisable);

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();

    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pNewOaModel->pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile);

    if (pNewOaModel->pLocalProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDir))
    {
        IMS_TRACE_I("FormOffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pLocalProfile->SetDirection(eDir);
    }

    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pLocalProfile->SetDataPort(0);
        pNewOaModel->pLocalProfile->SetControlPort(0);
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    if (m_eType == MEDIA_TYPE_AUDIO || m_eType == MEDIA_TYPE_VIDEO)
    {
        MediaProfileUtil::SetRtcpRsRr(GetLocalProfile(pNewOaModel), m_pConfig);
    }
    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile

    IMS_BOOL bSdpMade = m_pSdpGenerator->Generate(
            pSessionDescriptor, pDescriptor, GetLocalProfile(pNewOaModel));

    // Delete Session Level Direction Attribute
    if (m_eType == MEDIA_TYPE_AUDIO)
    {
        pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    return bSdpMade;
}
