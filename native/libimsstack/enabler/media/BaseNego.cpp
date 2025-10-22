/*
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

#include "BaseNego.h"
#include "ISessionDescriptor.h"
#include "ImsTypeDef.h"
#include "MediaBaseProfile.h"
#include "MediaDef.h"
#include "MediaEnvironment.h"
#include "MediaNegoUtil.h"
#include "MediaProfileFactory.h"
#include "MediaProfileGenerator.h"
#include "MediaProfileUtil.h"
#include "ServiceTrace.h"
#include "config/MediaConfiguration.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC BaseNego::BaseNego(IN const IMS_SINT32 nSlotId, IN const MEDIA_CONTENT_TYPE eType) :
        ImsSlot(nSlotId),
        m_eType(eType),
        m_pBaseProfile(IMS_NULL),
        m_listOaModel(ImsList<std::shared_ptr<OaModel>>()),
        m_pConfig(IMS_NULL),
        m_pEnvironment(IMS_NULL),
        m_pSdpGenerator(IMS_NULL),
        m_pProfileGenerator(IMS_NULL)
{
    IMS_TRACE_I("+BaseNego() - type[%d], slot[%d]", m_eType, nSlotId, 0);
}

BaseNego::BaseNego(IN const BaseNego& obj) :
        ImsSlot(obj),
        m_pBaseProfile(IMS_NULL),
        m_listOaModel(ImsList<std::shared_ptr<OaModel>>())
{
    *this = obj;
}

BaseNego& BaseNego::operator=(IN const BaseNego& obj)
{
    if (this != &obj)
    {
        ImsSlot::operator=(obj);
        m_eType = obj.m_eType;

        if (m_pBaseProfile)
        {
            MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
            m_pBaseProfile.reset();
        }

        if (obj.m_pBaseProfile)
        {
            m_pBaseProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                    m_eType, obj.m_pBaseProfile.get());
        }

        m_pConfig = obj.m_pConfig;
        m_pEnvironment = obj.m_pEnvironment;
        m_pSdpGenerator = obj.m_pSdpGenerator;
        m_pProfileGenerator = obj.m_pProfileGenerator;
    }

    return *this;
}

PUBLIC VIRTUAL BaseNego::~BaseNego()
{
    IMS_TRACE_I("~BaseNego(): type[%d]", m_eType, 0, 0);

    if (m_pBaseProfile)
    {
        MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
        m_pBaseProfile->DeletePayloads();
    }

    m_pBaseProfile.reset();

    DestroyListOaModel();
}

PUBLIC VIRTUAL void BaseNego::CreateProfiles(
        IN std::shared_ptr<MediaEnvironment> pEnvironment, IN MediaConfiguration* pConfig)
{
    if (pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfiles(): type[%d], invalid arguments", m_eType, 0, 0);
        return;
    }

    if (m_pBaseProfile && m_pBaseProfile->GetDataPort() != 0)
    {
        MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
    }

    m_pBaseProfile.reset();

    m_pEnvironment = pEnvironment;
    m_pConfig = pConfig;

    if (m_pProfileGenerator != IMS_NULL)
    {
        m_pBaseProfile = m_pProfileGenerator->Generate(
                m_pEnvironment->eServiceType, m_pEnvironment->pIService, m_pConfig, GetSlotId());
    }
}

PUBLIC VIRTUAL IMS_BOOL BaseNego::FormSdp(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
        IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable, IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormSdp(): type[%d], state[%d], OA model size[%d]", m_eType, eNegoState,
            m_listOaModel.GetSize());
    IMS_TRACE_D("FormSdp(): direction[%d], disable[%d], mode[%d]", eDirection, bDisable,
            bEnforceReofferMode);

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

PUBLIC VIRTUAL void BaseNego::NegotiateSdp(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
        OUT MEDIA_DIRECTION& eDirection)
{
    IMS_TRACE_I("NegotiateSdp(): type[%d], state[%d], OaModel size[%d]", m_eType, eNegoState,
            m_listOaModel.GetSize());
    eDirection = MEDIA_DIRECTION_INVALID;

    switch (eNegoState)
    {
        case STATE_IDLE:
        case STATE_NEGOTIATED:
            eDirection = NegotiateOffer(pSessionDescriptor, pDescriptor);
            break;
        case STATE_OFFER_SENT:
            eDirection = NegotiateAnswer(pSessionDescriptor, pDescriptor);
            break;
        default:
            break;
    }
}

PUBLIC VIRTUAL void BaseNego::CleanupIncompleteOaModels()
{
    // Clean up incomplete OA model from the end of the list.
    // This can happen if an offer was created but never answered.
    if (!m_listOaModel.IsEmpty())
    {
        std::shared_ptr<OaModel> pLastOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);
        if (pLastOaModel != IMS_NULL && !pLastOaModel->IsAllProfileExist())
        {
            IMS_TRACE_I(
                    "CleanupIncompleteOaModels(): type[%d], removing incomplete OA model index[%d]",
                    m_eType, m_listOaModel.GetSize() - 1, 0);
            m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        }
    }
}

PUBLIC const IpAddress& BaseNego::GetNegotiatedRemoteAddress()
{
    auto pProfile = GetNegotiatedPeerProfile();

    return (pProfile != IMS_NULL) ? pProfile->GetIpAddress() : IpAddress::NONE;
}

PUBLIC VIRTUAL IMS_SINT32 BaseNego::GetRemotePort()
{
    auto pProfile = GetNegotiatedPeerProfile();

    return (pProfile != IMS_NULL) ? pProfile->GetDataPort() : MEDIA_PORT_INVALID;
}

PUBLIC VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedLocalProfile()
{
    auto pOaModel = GetNegotiatedOaModel();

    if (pOaModel)
    {
        return GetLocalProfile(*pOaModel);
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedNegoProfile()
{
    auto pOaModel = GetNegotiatedOaModel();

    if (pOaModel)
    {
        return GetNegotiatedProfile(*pOaModel);
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedPeerProfile()
{
    auto pOaModel = GetNegotiatedOaModel();

    if (pOaModel)
    {
        return GetPeerProfile(*pOaModel);
    }

    return IMS_NULL;
}
PUBLIC
MEDIA_DIRECTION BaseNego::GetNegotiatedDirection()
{
    auto pOaModel = GetNegotiatedOaModel();

    if (pOaModel == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedDirection(): type[%d], invalid OA model", m_eType, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    return pOaModel->pNegotiatedProfile->GetDirection();
}

PUBLIC IMS_SINT32 BaseNego::GetNegotiatedRtpPort()
{
    auto pOaModel = GetNegotiatedOaModel();

    if (pOaModel == IMS_NULL)
    {
        return -1;
    }

    return (IMS_SINT32)pOaModel->pNegotiatedProfile->GetDataPort();
}

PUBLIC IMS_SINT32 BaseNego::GetNegotiatedBandwidth()
{
    if (m_listOaModel.GetSize() > 0)
    {
        std::shared_ptr<OaModel> pLatestOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

        if (pLatestOaModel == IMS_NULL || GetLocalProfile(*pLatestOaModel) == IMS_NULL)
        {
            IMS_TRACE_E(0, "GetNegotiatedBandwidth(): type[%d], invalid OA model", m_eType, 0, 0);
            return -1;
        }

        return (GetNegotiatedProfile(*pLatestOaModel) != IMS_NULL)
                ? (IMS_SINT32)GetNegotiatedProfile(*pLatestOaModel)->GetBandwidthAs()
                : (IMS_SINT32)GetLocalProfile(*pLatestOaModel)->GetBandwidthAs();
    }

    return -1;
}

PUBLIC MediaBaseProfile::BasePayload* BaseNego::GetNegotiatedPayload()
{
    auto pOaModel = GetNegotiatedOaModel();

    if (pOaModel == IMS_NULL)
    {
        IMS_TRACE_I("GetNegotiatedPayload(): type[%d], no negotiated OA model", m_eType, 0, 0);
        return IMS_NULL;
    }

    MediaBaseProfile* pProfile = GetNegotiatedProfile(*pOaModel);

    if (pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedPayload(): type[%d], invalid profile", m_eType, 0, 0);
        return IMS_NULL;
    }

    if (pProfile->GetPayloadList().GetSize() == 0)
    {
        if (pProfile->GetDataPort() == 0)
        {
            pProfile->CopyPayloads(GetLocalProfile(*pOaModel)->GetPayloadList());
        }
        else
        {
            IMS_TRACE_E(0, "GetNegotiatedPayload(): type[%d], empty payloads", m_eType, 0, 0);
            return IMS_NULL;
        }
    }

    return (pProfile->GetNegotiatedPayloadIndex() > 0)
            ? pProfile->GetPayloadAt(pProfile->GetNegotiatedPayloadIndex())
            : pProfile->GetPayloadAt(0);
}

PUBLIC ImsList<std::shared_ptr<BaseNego::OaModel>>& BaseNego::GetOaModelList()
{
    return m_listOaModel;
}

void BaseNego::SetSdpGenerator(std::shared_ptr<MediaSdpGenerator> pSdpGenerator)
{
    m_pSdpGenerator = pSdpGenerator;
}

void BaseNego::SetProfileGenerator(std::shared_ptr<MediaProfileGenerator> pProfileGenerator)
{
    m_pProfileGenerator = pProfileGenerator;
}

PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetLocalProfile(IN const OaModel& objOaModel)
{
    return objOaModel.pLocalProfile.get();
}
PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetPeerProfile(IN const OaModel& objOaModel)
{
    return objOaModel.pPeerProfile.get();
}
PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedProfile(IN const OaModel& objOaModel)
{
    return objOaModel.pNegotiatedProfile.get();
}

PROTECTED
std::shared_ptr<BaseNego::OaModel> BaseNego::GetNegotiatedOaModel()
{
    IMS_UINT32 nTempOaModelCount = m_listOaModel.GetSize();

    while (nTempOaModelCount > 0)
    {
        const auto& pLatestOaModel = m_listOaModel.GetAt(nTempOaModelCount - 1);

        if (pLatestOaModel != IMS_NULL && pLatestOaModel->IsAllProfileExist())
        {
            return pLatestOaModel;
        }

        nTempOaModelCount--;
    }

    return IMS_NULL;
}

PUBLIC IMS_BOOL BaseNego::SetLocalPort(IN IMS_UINT32 nPort)
{
    if (!m_pBaseProfile)
    {
        IMS_TRACE_E(0, "SetLocalPort(): type[%d], invalid profile", m_eType, 0, 0);
        return IMS_FALSE;
    }

    MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());

    if (nPort != 0)
    {
        m_pBaseProfile->SetDataPort(MediaNegoUtil::AcquireRtpPort(GetSlotId(), nPort));
        m_pBaseProfile->SetControlPort(m_pBaseProfile->GetDataPort() + 1);
    }
    else
    {
        m_pBaseProfile->SetDataPort(0);
        m_pBaseProfile->SetControlPort(0);
    }

    return IMS_TRUE;
}

PROTECTED void BaseNego::DestroyListOaModel()
{
    while (m_listOaModel.GetSize() > 0)
    {
        m_listOaModel.RemoveAt(0);
    }
}

PROTECTED
void BaseNego::Copy(IN const BaseNego* pNego)
{
    if (pNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "Copy(): type[%d], invalid parameter", m_eType, 0, 0);
        return;
    }

    IMS_TRACE_I("Copy(): type[%d]", m_eType, 0, 0);

    if (m_pBaseProfile)
    {
        MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
        m_pBaseProfile.reset();
    }

    m_pBaseProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, pNego->GetBaseProfile());

    if (m_pBaseProfile && m_pBaseProfile->GetDataPort() != 0)
    {
        MediaNegoUtil::AcquireRtpPort(GetSlotId(), m_pBaseProfile->GetDataPort());
    }

    std::shared_ptr<OaModel> pNewOaModel = std::make_shared<OaModel>();

    if (pNewOaModel != IMS_NULL)
    {
        pNewOaModel->pLocalProfile =
                MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile.get());
        m_listOaModel.Append(pNewOaModel);
    }

    IMS_TRACE_I("Copy(): type[%d], OA model list size[%d]", m_eType, m_listOaModel.GetSize(), 0);
}

PROTECTED
std::shared_ptr<BaseNego::OaModel> BaseNego::CreateOaModel(
        IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    // Make new Offer/Answer model, and copy source profile
    std::shared_ptr<OaModel> pNewOaModel = std::make_shared<OaModel>();
    pNewOaModel->pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile.get());

    if (!pNewOaModel->pLocalProfile)
    {
        IMS_TRACE_E(0, "CreateOaModel(): type[%d], invalid profile", m_eType, 0, 0);
        return IMS_NULL;
    }

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDirection))
    {
        pNewOaModel->pLocalProfile->SetDirection(eDirection);
    }

    if (bDisable)
    {
        pNewOaModel->pLocalProfile->SetDataPort(0);
        pNewOaModel->pLocalProfile->SetControlPort(0);
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    if (m_eType == MEDIA_TYPE_AUDIO || m_eType == MEDIA_TYPE_VIDEO)
    {
        // bDirHold is not proper for formoffer()
        MediaProfileUtil::SetRtcpRsRr(GetLocalProfile(*pNewOaModel), m_pConfig, IMS_FALSE);
    }

    m_listOaModel.Append(pNewOaModel);
    return pNewOaModel;
}

PROTECTED
IMS_BOOL BaseNego::CheckArgument(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection)
{
    // Handling exception case
    if (!m_pBaseProfile || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        IMS_TRACE_E(0, "CheckArgument(): type[%d], invalid arguments", m_eType, 0, 0);
        return IMS_FALSE;
    }

    if (eDirection == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_E(0, "CheckArgument(): type[%d], invalid direction", m_eType, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
