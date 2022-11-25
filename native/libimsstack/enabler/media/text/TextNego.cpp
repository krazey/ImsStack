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
#include "text/TextNego.h"
#include "config/MediaSessionConfigFactory.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaConfigUtil.h"
#include "MediaResourceManager.h"
#include "MediaManager.h"

__IMS_TRACE_TAG_USER_DECL__("MED.TN");

static const IMS_UINT32 MAX_TEXT_OAMODEL_SIZE = 6;

PUBLIC TextNego::TextNego(IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_listOaModel(IMSList<OaModel*>()),
        m_objBaseProfile(TextProfile()),
        m_pEnvironment(IMS_NULL),
        m_pConfig(IMS_NULL)
{
    IMS_TRACE_I("+TextNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC
TextNego::TextNego(IN const TextNego& objTextNego) :
        ImsSlot(objTextNego.GetSlotId())
{
    copy(&objTextNego);
}

PUBLIC
TextNego& TextNego::operator=(IN const TextNego& obj)
{
    copy(&obj);
    return *this;
}

PUBLIC VIRTUAL TextNego::~TextNego()
{
    IMS_TRACE_I("~TextNego()", 0, 0, 0);

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

    IMS_TRACE_I("~TextNego() - listOaModel size[%d]", m_listOaModel.GetSize(), 0, 0);

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

PUBLIC VIRTUAL void TextNego::CreateProfiles(
        IN MediaEnvironment* pEnvironment, IN TextConfiguration* pConfig)
{
    if (pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfiles() - invalid configuration", 0, 0, 0);
        return;
    }

    m_pEnvironment = pEnvironment;
    m_pConfig = pConfig;

    IMS_TRACE_I("CreateProfiles()", 0, 0, 0);
    TextProfile* pProfile = TextProfileUtil::CreateProfile(pEnvironment, m_pConfig, GetSlotId());

    if (pProfile != IMS_NULL)
    {
        m_objBaseProfile = *pProfile;
        delete pProfile;
    }
}

PUBLIC VIRTUAL IMS_BOOL TextNego::FormSDP(IN NEGO_STATE eNegoState,
        IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
        IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable, IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_D("FormSDP() - eDir[%d], bDisable[%d] EnforceReofferMode[%d]", eDir, bDisable,
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

PUBLIC VIRTUAL void TextNego::NegotiateSDP(IN NEGO_STATE eNegoState,
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

PUBLIC VIRTUAL void TextNego::FinalizeSDP(
        IN ISessionDescriptor* pSessionDescriptor, IN NEGO_STATE eNegoState)
{
    IMS_BOOL bFoundOaModel = IMS_FALSE;
    IMS_UINT32 nOaModelSize = m_listOaModel.GetSize();

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
        if (!(pLatestOaModel->IsAllProfileExist() &&
                    (eNegoState == STATE_IDLE || eNegoState == STATE_NEGOTIATED)))
        {
            IMS_TRACE_I("FinalizeSDP() - There is incomplete OaModel[%d]. Delete profile",
                    m_listOaModel.GetSize() - 1, 0, 0);
            delete pLatestOaModel;
            m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        }
    }

    for (IMS_UINT32 i = 0; i < nOaModelSize; i++)
    {
        // get OaModel
        OaModel* pTempOaModel = m_listOaModel.GetAt(nOaModelSize - 1 - i);

        // find matched SessionDescriptor key
        if (pTempOaModel != IMS_NULL)
        {
            if (pTempOaModel->nSessionDescriptorKey ==
                    reinterpret_cast<IMS_SINTP>(pSessionDescriptor))
            {
                pTempOaModel->bConfirmedSession = IMS_TRUE;
                bFoundOaModel = IMS_TRUE;
                IMS_TRACE_D("FinalizeSDP() - find comfirmed Session OaModel[%d]",
                        m_listOaModel.GetSize() - i, 0, 0);
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
        if ((nOaModelSize - i - 1) < MAX_TEXT_OAMODEL_SIZE)
        {
            IMS_TRACE_D("FinalizeSDP() - nOaModelSize is under 6", 0, 0, 0);
            break;
        }

        OaModel* pDeleteCheckOaModel = m_listOaModel.GetAt(0);

        if (pDeleteCheckOaModel != IMS_NULL)
        {
            IMS_TRACE_D("FinalizeSDP() - remove old OaModel", 0, 0, 0);
            if (pDeleteCheckOaModel->nSessionDescriptorKey ==
                            reinterpret_cast<IMS_SINTP>(pSessionDescriptor) &&
                    pDeleteCheckOaModel->bConfirmedSession == IMS_TRUE)
            {
                break;
            }

            IMS_TRACE_D("FinalizeSDP() - Delete the oldest[%" PFLS_x "] OaModel",
                    pDeleteCheckOaModel, 0, 0);
            delete pDeleteCheckOaModel;
            m_listOaModel.RemoveAt(0);
        }
    }
}

PUBLIC IMS_BOOL TextNego::SetPort(IN IMS_UINT32 nPort)
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

    IMS_TRACE_I("SetPort() - changed Data Port[%d]->[%d]", m_objBaseProfile.nDataPort, nPort, 0);

    if (nPort != 0)
    {
        m_objBaseProfile.nDataPort = pResourceMngr->AcquireRtpPort(nPort, nPort);
        m_objBaseProfile.nControlPort = m_objBaseProfile.nDataPort + 1;
    }
    else
    {
        m_objBaseProfile.nDataPort = 0;
        m_objBaseProfile.nControlPort = 0;
        IMS_TRACE_I("SetPort() - data port is 0", 0, 0, 0);
    }

    return IMS_TRUE;
}

PUBLIC
const IPAddress& TextNego::GetNegotiatedRemoteAddress()
{
    TextProfile* pProfile = GetNegotiatedPeerProfile();

    if (pProfile != IMS_NULL)
    {
        return pProfile->objIpAddress;
    }

    return IPAddress::NONE;
}

PUBLIC
IMS_UINT32 TextNego::GetNegotiatedRemotePort()
{
    TextProfile* pProfile = GetNegotiatedPeerProfile();

    if (pProfile != IMS_NULL)
    {
        return pProfile->nDataPort;
    }

    return 0;
}

PUBLIC
TextProfile* TextNego::GetNegotiatedLocalProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pLocalProfile;
    }

    return IMS_NULL;
}

PUBLIC
TextProfile* TextNego::GetNegotiatedNegoProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pNegotiatedProfile;
    }

    return IMS_NULL;
}

PUBLIC
TextProfile* TextNego::GetNegotiatedPeerProfile()
{
    OaModel* pOaModel = GetNegotiatedOaModel();

    if (pOaModel != IMS_NULL)
    {
        return pOaModel->pPeerProfile;
    }

    return IMS_NULL;
}

PUBLIC
MEDIA_DIRECTION TextNego::GetNegotiatedDirection(void)
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
TEXT_CODEC TextNego::GetNegotiatedCodec(void)
{
    if (m_listOaModel.GetSize() > 0)
    {
        OaModel* pLatestOaModel = IMS_NULL;
        pLatestOaModel = GetNegotiatedOaModel();

        if (pLatestOaModel == IMS_NULL)
        {
            return TEXT_CODEC_NONE;
        }

        if (pLatestOaModel->IsAllProfileExist() == IMS_FALSE)
        {
            return TEXT_CODEC_NONE;
        }

        if (pLatestOaModel->pNegotiatedProfile->nDataPort == 0)
        {
            return TEXT_CODEC_NONE;
        }

        if (pLatestOaModel->pNegotiatedProfile->lstPayload.GetSize() == 0)
        {
            return TEXT_CODEC_NONE;
        }

        TextProfile::Payload* pPayload = pLatestOaModel->pNegotiatedProfile->lstPayload.GetAt(0);

        if (pPayload == IMS_NULL)
        {
            return TEXT_CODEC_NONE;
        }

        IMS_TRACE_I("GetNegotiatedCodec() - Negotiated Payload Type is [%s]",
                pPayload->objRtpMap.strPayloadType.GetStr(), 0, 0);

        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("t140"))
        {
            return TEXT_CODEC_T140;
        }
        else if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("red"))
        {
            return TEXT_CODEC_T140_RED;
        }
    }

    return TEXT_CODEC_NOT_USED;
}

PUBLIC
IMS_SINT32 TextNego::GetNegotiatedRtpPort(void)
{
    const IMS_SINT32 PORT_NONE = -1;

    if (m_listOaModel.GetSize() > 0)
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
IMS_SINT32 TextNego::GetMediaBandwidth(void)
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
void TextNego::copy(IN const TextNego* pTextNego)
{
    if (pTextNego == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("copy() - list size[%d]", pTextNego->m_listOaModel.GetSize(), 0, 0);

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

    m_objBaseProfile = pTextNego->m_objBaseProfile;

    if (pResourceMngr != IMS_NULL)
    {
        // To add port (it would be duplicated)
        if (m_objBaseProfile.nDataPort != 0)
        {
            pResourceMngr->AcquireRtpPort(m_objBaseProfile.nDataPort, m_objBaseProfile.nDataPort);
        }
    }

    m_pEnvironment = pTextNego->m_pEnvironment;

    if (pTextNego->m_listOaModel.GetSize() < 1)
    {
        return;
    }

    OaModel* pNewOaModel = new OaModel();
    OaModel* pOldOaModel = pTextNego->m_listOaModel.GetAt(0);
    pNewOaModel->pLocalProfile = new TextProfile(*pOldOaModel->pLocalProfile);
    m_listOaModel.Append(pNewOaModel);
    this->m_pConfig = pTextNego->m_pConfig;

    IMS_TRACE_I("copy() - OA model list size[%d]", m_listOaModel.GetSize(), 0, 0);
}

PRIVATE IMS_BOOL TextNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable)
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

    IMS_TRACE_D("FormOffer() - eDir[%d] - bDisable[%d]", eDir, bDisable, 0);

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile = new TextProfile(m_objBaseProfile);

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID && eDir <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        IMS_TRACE_I("FormOffer() - set direction[%d]", eDir, 0, 0);
        pNewOaModel->pLocalProfile->eDirection = eDir;
    }

    if (bDisable == IMS_TRUE)
    {
        IMS_TRACE_I("FormOffer() - disable", 0, 0, 0);
        pNewOaModel->pLocalProfile->nDataPort = 0;
        pNewOaModel->pLocalProfile->nControlPort = 0;
    }

    pNewOaModel->pLocalProfile->bISOfferCase = IMS_TRUE;
    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    return MakeSDPFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pLocalProfile);
}

PRIVATE IMS_BOOL TextNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (eDir == MEDIA_DIRECTION_INVALID && bDisable != IMS_TRUE)
    {
        IMS_TRACE_E(0, "FormAnswer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_listOaModel.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("FormAnswer() - eDir[%d] - bDisable[%d]", eDir, bDisable, 0);

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

    // Modify a Rtp/RTCP port if text is not supported
    if (bDisable == IMS_TRUE)
    {
        IMS_TRACE_D("FormAnswer() - text session is not supported", 0, 0, 0);

        // copy the dest profile as nego profile
        if (pNewOaModel->pPeerProfile->lstPayload.GetSize() > 0)
        {
            IMS_TRACE_D("FormAnswer() - use peer profile", 0, 0, 0);
            *pNewOaModel->pNegotiatedProfile = *pNewOaModel->pPeerProfile;
        }
        else
        {
            // in case of no payload can be answer if there is no payload from dest profile
            // it is a very exceptional case - received offer should have payloads
            // use previous payload contained profile
            if (pNewOaModel->pLocalProfile->lstPayload.GetSize() == 0)
            {
                OaModel* pPrevOaModel = IMS_NULL;
                pPrevOaModel = GetNegotiatedOaModel();  // get negotiated OA model which is previous
                                                        // nego result

                if (pPrevOaModel != NULL &&
                        pPrevOaModel->pNegotiatedProfile->lstPayload.GetSize() > 0)
                {
                    IMS_TRACE_D("FormAnswer() use previous nego payloads", 0, 0, 0);
                    *pNewOaModel->pNegotiatedProfile = *pPrevOaModel->pNegotiatedProfile;
                }
                else
                {
                    IMS_TRACE_D("FormAnswer() use src payloads", 0, 0, 0);
                    *pNewOaModel->pNegotiatedProfile = *pNewOaModel->pLocalProfile;
                }
            }
            else
            {
                IMS_TRACE_D("FormAnswer() use src payloads", 0, 0, 0);
                *pNewOaModel->pNegotiatedProfile = *pNewOaModel->pLocalProfile;
            }
        }

        pNewOaModel->pNegotiatedProfile->objIpAddress = pSessionDescriptor->GetLocalAddress();
        pNewOaModel->pNegotiatedProfile->nBandwidthAs = 0;
        pNewOaModel->pNegotiatedProfile->nBandwidthRs = 0;
        pNewOaModel->pNegotiatedProfile->nBandwidthRr = 0;
        pNewOaModel->pNegotiatedProfile->nDataPort = 0;
        pNewOaModel->pNegotiatedProfile->nControlPort = 0;
        pNewOaModel->pNegotiatedProfile->eDirection = MEDIA_DIRECTION_INVALID;
    }

    // Modify a direction
    if (eDir > MEDIA_DIRECTION_INVALID && eDir <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        IMS_TRACE_D("FormAnswer() - set direction[%d]", eDir, 0, 0);
        pNewOaModel->pNegotiatedProfile->eDirection = eDir;
    }

    pNewOaModel->pLocalProfile->bISOfferCase = IMS_FALSE;

    // Make the SDP from profile
    return MakeSDPFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pNegotiatedProfile);
}

PRIVATE
IMS_BOOL TextNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormReoffer() - pDescriptor[%" PFLS_x "], eDir[%d], OaModel Size(%d)", pDescriptor,
            eDir, m_listOaModel.GetSize());

    IMS_TRACE_D("TextNego - FormReoffer() - eDir[%d] bDisable[%d] EnforceReofferMode[%d]", eDir,
            bDisable, bEnforceReofferMode);

    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (eDir == MEDIA_DIRECTION_INVALID && bDisable != IMS_TRUE)
    {
        IMS_TRACE_E(0, "FormReoffer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    // Step 1. Make new Offer/Answer model, and copy source profile from previous negotiated profile
    OaModel* pNewOaModel = new OaModel();

    if (m_listOaModel.GetSize() == 0)
    {
        pNewOaModel->pLocalProfile = new TextProfile(m_objBaseProfile);
    }
    else
    {
        OaModel* pPrevOaModel = GetNegotiatedOaModel();

        if (pPrevOaModel == IMS_NULL)
        {
            delete pNewOaModel;
            return IMS_FALSE;
        }

        MediaSessionConfig* pMediaSessionConfig =
                MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                        GetSlotId(), m_pEnvironment->eServiceType);

        if (pMediaSessionConfig == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pPrevOaModel->pNegotiatedProfile->lstPayload.GetSize() == 0 ||
                bEnforceReofferMode == IMS_TRUE)
        {
            pNewOaModel->pLocalProfile = new TextProfile(m_objBaseProfile);
            IMS_TRACE_I("TextNego::FormReOffer() - Fullcapa - enforce reoffer[%d]",
                    bEnforceReofferMode, 0, 0);
        }
        else
        {
            if (pMediaSessionConfig->IsSdpReofferFullCapability() == IMS_TRUE)
            {
                if (m_objBaseProfile.lstPayload.GetSize() > 0)
                {
                    IMS_TRACE_I("FormReoffer() - Fullcapa", 0, 0, 0);
                    pNewOaModel->pLocalProfile = new TextProfile(m_objBaseProfile);
                }
                else
                {
                    // this case is only for reoffer but no src profile payload existed
                    IMS_TRACE_I("FormReoffer() - src profile is empty, use nego profile", 0, 0, 0);
                    pNewOaModel->pLocalProfile = new TextProfile(*pPrevOaModel->pNegotiatedProfile);
                }
            }
            else
            {
                IMS_TRACE_I("FormReoffer() - src profile is empty, use nego profile", 0, 0, 0);
                pNewOaModel->pLocalProfile = new TextProfile(*pPrevOaModel->pNegotiatedProfile);
            }
        }
    }

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID && eDir <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        IMS_TRACE_I("FormReoffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pLocalProfile->eDirection = eDir;
    }

    // Modify a Rtp/RTCP port if text is not supported
    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pLocalProfile->nDataPort = 0;
        pNewOaModel->pLocalProfile->nControlPort = 0;
        pNewOaModel->pLocalProfile->nBandwidthAs = 0;
        pNewOaModel->pLocalProfile->nBandwidthRs = 0;
        pNewOaModel->pLocalProfile->nBandwidthRr = 0;
    }
    else
    {
        pNewOaModel->pLocalProfile->nDataPort = m_objBaseProfile.nDataPort;
        pNewOaModel->pLocalProfile->nControlPort = m_objBaseProfile.nControlPort;

        // set default AS value when srcProfile AS value is 0 in ReOffer case
        if (pNewOaModel->pLocalProfile->nBandwidthAs <= 0)
        {
            IMS_TRACE_I("FormReoffer() LocalProfile AS value is 0.. so change to default AS value",
                    0, 0, 0);
            pNewOaModel->pLocalProfile->nBandwidthAs = m_objBaseProfile.nBandwidthAs;
        }

        pNewOaModel->pLocalProfile->nBandwidthRs = m_pConfig->GetRsBandwidthBps();
        pNewOaModel->pLocalProfile->nBandwidthRr = m_pConfig->GetRrBandwidthBps();
    }

    pNewOaModel->pLocalProfile->bISOfferCase = IMS_TRUE;
    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    return MakeSDPFromProfile(pSessionDescriptor, pDescriptor, pNewOaModel->pLocalProfile);
}

PRIVATE IMS_SINT32 TextNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateOffer() - pSessionDescriptor or pDescriptor is NULL", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile = new TextProfile(m_objBaseProfile);

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = new TextProfile();

    if (MakeProfileFromSDP(pSessionDescriptor, pDescriptor, pNewOaModel->pPeerProfile) != IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateOffer() - MakeProfileFromSDP failed", 0, 0, 0);
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile = new TextProfile();

    if (MakeNegotiatedProfile(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_TRUE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateOffer() - MakeNegotiatedProfile failed", 0, 0, 0);
    }

    // add sessionDesciptorkey key in NewOaModel
    IMS_TRACE_D("NegotiateOffer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_listOaModel.Append(pNewOaModel);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PRIVATE IMS_SINT32 TextNego::NegotiateAnswer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAnswer() - pSessionDescriptor or pDescriptor is NULL", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    if (m_listOaModel.GetSize() < 1)
    {
        IMS_TRACE_E(0, "NegotiateAnswer() - Failed. m_listOaModel.GetSize() [%d]",
                m_listOaModel.GetSize(), 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // Get the latest OAmodel from list
    OaModel* pNewOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

    if (pNewOaModel == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAnswer() - Failed. pNewOaModel is NULL ", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = new TextProfile();

    if (MakeProfileFromSDP(pSessionDescriptor, pDescriptor, pNewOaModel->pPeerProfile) != IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateAnswer() - MakeProfileFromSDP failed", 0, 0, 0);
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile = new TextProfile();

    if (MakeNegotiatedProfile(pNewOaModel->pLocalProfile, pNewOaModel->pPeerProfile, IMS_FALSE,
                pNewOaModel->pNegotiatedProfile) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Add session key in NewOaModel
    IMS_TRACE_D("NegotiateAnswer - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->eDirection;
}

PRIVATE
IMS_BOOL TextNego::MakeSDPFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN TextProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // clean attr & bandwidth line
    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    IMSList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // Make "c" & "o" line of session level if IP does not matched
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->objIpAddress))
    {
        IMS_TRACE_D("MakeSDPFromProfile() - Ip is not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->objIpAddress.ToCharString(), 0);

        pSessionDescriptor->SetConnectionAddress(pProfile->objIpAddress.ToString());
        pSessionDescriptor->SetOriginAddress(pProfile->objIpAddress.ToString());
    }

    // Check and delete "red" type which contains invalid sub payload type
    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        TextProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        if (pPayload->objRtpMap.strPayloadType.Equals("red"))
        {
            TextProfile::RedFmtp* pRedFmtp =
                    reinterpret_cast<TextProfile::RedFmtp*>(pPayload->pFmtp);

            if (pRedFmtp == IMS_NULL)
            {
                continue;
            }

            IMS_TRACE_I("MakeSDPFromProfile() - fmtp, nRedundancy [%d], nRedPayload[%d]",
                    pRedFmtp->nRedLevel, pRedFmtp->nRedPayload, 0);

            IMS_BOOL bRedSubPTExist = IMS_FALSE;

            for (IMS_UINT32 j = 0; j < pProfile->lstPayload.GetSize(); j++)
            {
                TextProfile::Payload* pTempPayload = pProfile->lstPayload.GetAt(j);

                if (pTempPayload == IMS_NULL)
                {
                    continue;
                }

                IMS_TRACE_I("MakeSDPFromProfile() - RedSubPT, PT[%d] of PL(%d) / nRedPayload "
                            "[%d]",
                        pTempPayload->objRtpMap.nPayloadNum, j, pRedFmtp->nRedPayload);

                if (pTempPayload->objRtpMap.nPayloadNum == (IMS_UINT32)pRedFmtp->nRedPayload)
                {
                    bRedSubPTExist = IMS_TRUE;
                }
            }

            if (bRedSubPTExist == IMS_FALSE)
            {
                IMS_TRACE_E(0,
                        "MakeSDPFromProfile() - SubPayloadtype for Redundancy isn't exist. skip "
                        "Payload, Payload[%s], PT[%d]",
                        pPayload->objRtpMap.strPayloadType.GetStr(),
                        pPayload->objRtpMap.nPayloadNum, 0);
                pProfile->lstPayload.RemoveAt(i);
                delete pPayload;
            }
        }
    }

    IMS_TRACE_I("MakeSDPFromProfile() - After Check Validity, PayloadSize[%d]",
            pProfile->lstPayload.GetSize(), 0, 0);

    // Make "m" line
    // ------ "m=text xxxx Rtp/AVP 100 98"
    AStringArray objTextFormat;
    AString strPayloadNum;

    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        TextProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->objRtpMap.nPayloadNum);
        objTextFormat.AddElement(strPayloadNum);
    }

    pDescriptor->SetMediaDescription(
            SdpMedia::TYPE_TEXT, pProfile->nDataPort, SdpMedia::TRANSPORT_RTP_AVP, objTextFormat);

    // Make bandwidth
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

    // Make each payload
    // ------ "a=rtpmap:98 t140/1000"
    // ------ "a=rtpmap:112 red/1000
    // ------ "a=fmtp:112 111/111/111"
    // ------ "a=rtpmap:111 t140/1000
    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        AString strRtpmap, strFmtp;
        TextProfile::Payload* pPayload = pProfile->lstPayload.GetAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // make "rtpmap"
        strPayloadNum.Sprintf("%d", pPayload->objRtpMap.nPayloadNum);
        strRtpmap.Sprintf("%s/%d", pPayload->objRtpMap.strPayloadType.GetStr(),
                pPayload->objRtpMap.nSamplingRate);

        // make "fmtp"
        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("red"))
        {
            TextProfile::RedFmtp* pRedFmtp = (TextProfile::RedFmtp*)pPayload->pFmtp;

            if (pRedFmtp == IMS_NULL)
            {
                continue;
            }

            IMS_UINT32 nCount = pRedFmtp->nRedLevel;
            AString TempSubPT;
            TempSubPT.Sprintf("%d", pRedFmtp->nRedPayload);

            while (nCount-- > 0)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append("/");
                }

                strFmtp.Append(TempSubPT);
            }

            IMS_TRACE_I("MakeSDPFromProfile() - Add fmtp, nRedundancy[%d], nRedPayload[%d], "
                        "Fmtp[%s]",
                    pRedFmtp->nRedLevel, pRedFmtp->nRedPayload, strFmtp.GetStr());
        }

        if (strFmtp.GetLength() == 0)
        {
            strFmtp = AString::ConstNull();
        }

        pDescriptor->SetMediaFormat(SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpmap, strFmtp);
    }

    // Make direction
    pDescriptor->SetDirection(pProfile->eDirection);
    IMS_TRACE_I("MakeSDPFromProfile() - payloadSize[%d]", pProfile->lstPayload.GetSize(), 0, 0);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL TextNego::MakeProfileFromSDP(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeProfileFromSDP() - pDescriptor or pProfile is NULL", 0, 0, 0);
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

    IMS_TRACE_I("MakeProfileFromSDP() - AS[%d], RS[%d], RR[%d]", pProfile->nBandwidthAs,
            pProfile->nBandwidthRs, pProfile->nBandwidthRr);

    // payload
    IMSList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSDPCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));

        if (pSDPCodec == IMS_NULL)
        {
            IMS_TRACE_E(0, "MakeProfileFromSDP() - pSDPCodec is NULL", 0, 0, 0);
            return IMS_FALSE;
        }
        AString strCodecName = pSDPCodec->GetName();

        IMS_TRACE_I("MakeProfileFromSDP() - At(%d), GetPayloadType(%d), GetClockRate(%d)", i,
                pSDPCodec->GetPayloadType(), pSDPCodec->GetClockRate());

        TextProfile::Payload* pPayload = new TextProfile::Payload();

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        pPayload->SetRtpMap(pSDPCodec->GetPayloadType(), strCodecName, pSDPCodec->GetClockRate());
        // check fmtp of t140 redundancy
        if (strCodecName.EqualsIgnoreCase("red"))
        {
            TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp();

            if (GetFmtpFromString(pSDPCodec->GetFormatSpecificParameter(), pRedFmtp) == IMS_FALSE)
            {
                IMS_TRACE_E(0, "MakeProfileFromSDP() - Cannot make fmtp for 'red'", 0, 0, 0);
                delete pPayload;
                delete pRedFmtp;
                continue;
            }

            IMS_BOOL bRedSubPTExist = IMS_FALSE;

            for (IMS_UINT32 j = 0; j < lstMediaFormat.GetSize(); j++)
            {
                pSDPCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(j));
                if (pSDPCodec == IMS_NULL)
                    continue;

                IMS_TRACE_I("MakeSDPFromProfile() - Check RedSubPT, PT[%d] of PL(%d) / nRedPayload "
                            "[%d]",
                        pSDPCodec->GetPayloadType(), j, pRedFmtp->nRedPayload);
                if (pSDPCodec->GetPayloadType() == pRedFmtp->nRedPayload)
                {
                    bRedSubPTExist = IMS_TRUE;
                }
            }

            if (bRedSubPTExist == IMS_FALSE)
            {
                IMS_TRACE_E(0, "MakeProfileFromSDP() - No matched rtpmap for subtype of 'red'", 0,
                        0, 0);
                delete pPayload;
                delete pRedFmtp;
                continue;
            }

            IMS_TRACE_I(
                    "MakeProfileFromSDP() - Redundancy presented [%d]", pRedFmtp->nRedLevel, 0, 0);
            pPayload->pFmtp = pRedFmtp;
        }
        else if (!strCodecName.EqualsIgnoreCase("t140"))
        {
            IMS_TRACE_E(
                    0, "MakeProfileFromSDP() - Invalid codec [%s]", strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        pProfile->lstPayload.Append(pPayload);
    }

    // direction
    pProfile->eDirection = (MEDIA_DIRECTION)pDescriptor->GetDirection();
    if (pProfile->eDirection == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_D("MakeProfileFromSDP() - Text Media level Direction does not exist..", 0, 0, 0);
        // check session level attribute Direction
        pProfile->eDirection = (MEDIA_DIRECTION)pSessionDescriptor->GetDirection();
        if (pProfile->eDirection == MEDIA_DIRECTION_INVALID)
            pProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
    }

    IMS_TRACE_I("MakeProfileFromSDP() - Ended[%d]", 0, 0, 0);

    return IMS_TRUE;
}

IMS_BOOL TextNego::MakeNegotiatedProfile(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT TextProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedProfile() invalid argument, %" PFLS_x " %" PFLS_x,
                pLocalProfile, pPeerProfile, 0);
        return IMS_FALSE;
    }

    // Setting IP of mine
    pNegotiatedProfile->objIpAddress = pLocalProfile->objIpAddress;

    IMS_TRACE_D("MakeNegotiatedProfile() - local address[%s] PeerPayloadSize[%d]",
            pLocalProfile->objIpAddress.ToCharString(), pPeerProfile->lstPayload.GetSize(), 0);

    // Setting Rtp/RTCP port of mine
    pNegotiatedProfile->nDataPort = pLocalProfile->nDataPort;
    pNegotiatedProfile->nControlPort = pLocalProfile->nControlPort;

    IPAddress objLocalIPAddr = pLocalProfile->objIpAddress;

    if (pNegotiatedProfile->nDataPort == 0 || pPeerProfile->nDataPort == 0)
    {
        *pNegotiatedProfile = *pLocalProfile;

        // copy the dest profile as nego profile
        if (pPeerProfile->lstPayload.GetSize() > 0)
        {
            *pNegotiatedProfile = *pPeerProfile;
        }
        else
        {
            *pNegotiatedProfile = *pLocalProfile;
        }

        pNegotiatedProfile->objIpAddress = objLocalIPAddr;
        pNegotiatedProfile->nDataPort = 0;

        IMS_TRACE_D("MakeNegotiatedProfile() - ZERO Port. DO NOT Use the text[%d][%d],\
                But nego is successful",
                pNegotiatedProfile->nDataPort, pPeerProfile->nDataPort, 0);
        return IMS_TRUE;
    }

    if (m_pConfig == IMS_NULL)
    {
        IMS_TRACE_D("MakeNegotiatedProfile() - no config, return true to reject", 0, 0, 0);
        return IMS_TRUE;
    }

    // Compare each payload based destination's profile
    IMSList<TextProfile::Payload*> listNegotiatedPayloads;

    for (IMS_UINT32 i = 0; i < pPeerProfile->lstPayload.GetSize(); i++)
    {
        TextProfile::Payload* pPayload = pPeerProfile->lstPayload.GetAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("t140") ||
                pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("red"))
        {
            if (FindT140InProfile(pLocalProfile, pPayload) == IMS_TRUE)
            {
                TextProfile::Payload* pT140 = new TextProfile::Payload();
                pT140->SetRtpMap(pPayload->objRtpMap);

                if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("red"))
                {
                    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp(
                            *reinterpret_cast<TextProfile::RedFmtp*>(pPayload->pFmtp));
                    pT140->pFmtp = reinterpret_cast<void*>(pRedFmtp);
                }

                pNegotiatedProfile->lstPayload.Append(pT140);
                listNegotiatedPayloads.Append(pT140);
            }
        }
    }

    IMS_BOOL bRet = IMS_FALSE;

    if (listNegotiatedPayloads.GetSize() > 0)
    {
        // Setting direction
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

        pNegotiatedProfile->bIsHold = pNegotiatedProfile->eDirection != MEDIA_DIRECTION_SEND_RECEIVE
                ? IMS_TRUE
                : IMS_FALSE;
        TextProfileUtil::MakeNegotiatedBandwidth(
                m_pConfig, pLocalProfile, pPeerProfile, bIsOfferReceived, -1, pNegotiatedProfile);
        bRet = IMS_TRUE;
    }
    else
    {
        // TODO: need to change this if condition later
        if (pNegotiatedProfile->nDataPort == 0 || pPeerProfile->nDataPort == 0 ||
                pNegotiatedProfile->lstPayload.GetSize() == 0)
        {
            pNegotiatedProfile->eDirection = MEDIA_DIRECTION_INVALID;
            bRet = IMS_TRUE;  // TODO: need to check later
            IMS_TRACE_D("eDirection: %d bRet:%d", pNegotiatedProfile->eDirection, bRet, 0);
        }

        if (pLocalProfile->lstPayload.GetSize() > 0)
        {
            IMS_TRACE_D("MakeNegotiatedProfile() - no negotiated payload. use the LocalProfile and "
                        "make port 0 ",
                    0, 0, 0);
            *pNegotiatedProfile = *pLocalProfile;
            pNegotiatedProfile->nDataPort = 0;
            pNegotiatedProfile->eDirection = MEDIA_DIRECTION_INVALID;
        }
        else
        {
            IMS_TRACE_E(0, "There's no Payload in LocalProfile", 0, 0, 0);
        }
    }

    if (pNegotiatedProfile->nBandwidthRs != 0 || pNegotiatedProfile->nBandwidthRr != 0)
    {
        pNegotiatedProfile->nRtcpInterval = pLocalProfile->nRtcpInterval;
    }

    IMS_TRACE_D("MakeNegotiatedProfile() - negotiated payload size[%d], port[%d], direction[%d], ",
            pNegotiatedProfile->lstPayload.GetSize(), pNegotiatedProfile->nDataPort,
            pNegotiatedProfile->eDirection);
    return bRet;
}

PRIVATE IMS_BOOL TextNego::GetFmtpFromString(IN AString strFmtp, OUT TextProfile::RedFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL || strFmtp.IsEmpty() == IMS_TRUE)
        return IMS_FALSE;

    IMSList<AString> strArrTemp = strFmtp.Split('/');
    pFmtp->nRedLevel = strArrTemp.GetSize();

    if (pFmtp->nRedLevel == 0)
    {
        return IMS_FALSE;
    }

    pFmtp->nRedPayload = strArrTemp.GetAt(0).ToInt32();

    for (IMS_SINT32 i = 0; i < pFmtp->nRedLevel - 1; i++)
    {
        if (strArrTemp.GetAt(i).ToInt32() != pFmtp->nRedPayload)
        {
            pFmtp->nRedLevel = -1;
            pFmtp->nRedPayload = -1;
            return IMS_FALSE;
        }
    }

    IMS_TRACE_D("GetFmtpFromString() Ended. nRedLevel[%d], nRedPayload[%d]", pFmtp->nRedLevel,
            pFmtp->nRedPayload, 0);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL TextNego::FindT140InProfile(
        IN TextProfile* pProfile, IN TextProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
    {
        TextProfile::Payload* comparedPayload = pProfile->lstPayload.GetAt(i);

        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("t140"))
        {
            if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                        pPayload->objRtpMap.strPayloadType) &&
                    comparedPayload->objRtpMap.nSamplingRate == pPayload->objRtpMap.nSamplingRate)
            {
                IMS_TRACE_D("FindT140InProfile() - Found T140 at [%d], Codec[%s]", i,
                        comparedPayload->objRtpMap.strPayloadType.GetStr(), 0);

                return IMS_TRUE;
            }
        }
        else if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("red"))
        {
            if (comparedPayload->objRtpMap.strPayloadType.EqualsIgnoreCase(
                        pPayload->objRtpMap.strPayloadType) &&
                    comparedPayload->objRtpMap.nSamplingRate == pPayload->objRtpMap.nSamplingRate)
            {
                TextProfile::RedFmtp* pComparedFmtp = (TextProfile::RedFmtp*)comparedPayload->pFmtp;
                TextProfile::RedFmtp* pReceivedFmtp = (TextProfile::RedFmtp*)pPayload->pFmtp;

                if (pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                if (pReceivedFmtp->nRedLevel > pComparedFmtp->nRedLevel ||
                        pReceivedFmtp->nRedPayload < 0)
                {
                    continue;
                }

                IMS_TRACE_D("FindT140InProfile() - Found RED at [%d], Codec[%s]", i,
                        comparedPayload->objRtpMap.strPayloadType.GetStr(), 0);

                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE MEDIA_DIRECTION TextNego::UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
        IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase)
{
    if (ePeerDirection < MEDIA_DIRECTION_INACTIVE || ePeerDirection > MEDIA_DIRECTION_SEND_RECEIVE)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_D("UpdateDirectionToMine() - Entered. ePeerDirection[%d], eLocalDirection[%d], "
                "bIsMtCase[%d]",
            ePeerDirection, eLocalDirection, bIsMtCase);

    MEDIA_DIRECTION eNegotiatedDir = MEDIA_DIRECTION_INVALID;

    switch (ePeerDirection)
    {
        case MEDIA_DIRECTION_INACTIVE:  // FALL_THROUGH
        case MEDIA_DIRECTION_SEND_RECEIVE:
            eNegotiatedDir = ePeerDirection;
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
        eNegotiatedDir = eLocalDirection;
    }

    return eNegotiatedDir;
}

PRIVATE TextNego::OaModel* TextNego::GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed)
{
    OaModel* pFoundOaModel = IMS_NULL;
    IMS_UINT32 nOaModelCount = m_listOaModel.GetSize();

    // remake function - find negotiated payload is existed OA model
    while (nOaModelCount > 0)
    {
        OaModel* pLatestOaModel = m_listOaModel.GetAt(nOaModelCount - 1);
        if (pLatestOaModel != IMS_NULL)
        {
            if (pLatestOaModel->IsAllProfileExist() == IMS_TRUE)
            {
                // find nago payload is existed
                if (pLatestOaModel->pNegotiatedProfile->lstPayload.GetSize() > 0)
                {
                    if (bCheckConfirmed == IMS_TRUE &&
                            bCheckConfirmed != pLatestOaModel->bConfirmedSession)
                    {
                        nOaModelCount--;
                        continue;
                    }

                    return pLatestOaModel;
                }
                else
                {
                    // in this condition, only return latest OA model without checking confirmed or
                    // not
                    if (pFoundOaModel == IMS_NULL)
                    {
                        pFoundOaModel = pLatestOaModel;
                    }
                }
            }

            IMS_TRACE_I("GetNegotiatedOaModel() - There is not all profiles in [%d/%d]th OaModel. "
                        "Try next",
                    nOaModelCount, m_listOaModel.GetSize(), 0);
        }
        nOaModelCount--;
    }

    return pFoundOaModel;
}