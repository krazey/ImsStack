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
#include "config/MediaConfigUtil.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaSessionConfigFactory.h"
#include "text/TextNego.h"
#include "text/TextProfileGenerator.h"
#include "text/TextSdpParser.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextNego::TextNego(IMS_SINT32 nSlotId) :
        BaseNego(nSlotId, MEDIA_TYPE_TEXT),
        m_pSdpParser(std::make_unique<TextSdpParser>())
{
    IMS_TRACE_I("+TextNego() - slot[%d]", nSlotId, 0, 0);

    m_pSdpGenerator = std::make_shared<TextSdpGenerator>();
    m_pProfileNegotiator = std::make_shared<TextProfileNegotiator>();
    m_pProfileGenerator = std::make_shared<TextProfileGenerator>();
}

PUBLIC
TextNego::TextNego(IN const TextNego& objTextNego) :
        BaseNego(objTextNego.GetSlotId())
{
    Copy(&objTextNego);
}

PUBLIC
TextNego& TextNego::operator=(IN const TextNego& obj)
{
    if (this != &obj)
    {
        Copy(&obj);
    }
    return (*this);
}

PUBLIC VIRTUAL TextNego::~TextNego()
{
    IMS_TRACE_I("~TextNego()", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL TextNego::IsMediaCodecFromSdpSupported(
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
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT, m_pBaseProfile);

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(&objOaModel)) !=
            IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (std::static_pointer_cast<TextProfileNegotiator>(m_pProfileNegotiator)
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

PUBLIC
TEXT_CODEC TextNego::GetNegotiatedCodec(void)
{
    MediaBaseProfile::BasePayload* pPayload = GetNegotiatedPayload();

    if (pPayload == IMS_NULL)
    {
        return TEXT_CODEC_NONE;
    }

    IMS_TRACE_D("GetNegotiatedCodec() - Negotiated Payload Type is [%s]",
            pPayload->GetRtpMap().GetPayloadType().GetStr(), 0, 0);

    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"))
    {
        return TEXT_CODEC_T140;
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
    {
        return TEXT_CODEC_T140_RED;
    }

    return TEXT_CODEC_NONE;
}

PUBLIC TextConfiguration* TextNego::ConfigCasting(IN MediaConfiguration* pConfig)
{
    return (pConfig != IMS_NULL) ? static_cast<TextConfiguration*>(pConfig) : IMS_NULL;
}

PUBLIC TextProfile* TextNego::ProfileCasting(IN MediaBaseProfile* pProfile)
{
    return (pProfile != IMS_NULL) ? static_cast<TextProfile*>(pProfile) : IMS_NULL;
}

PUBLIC TextProfile::Payload* TextNego::PayloadCasting(IN MediaBaseProfile::BasePayload* pPayload)
{
    return (pPayload != IMS_NULL) ? static_cast<TextProfile::Payload*>(pPayload) : IMS_NULL;
}

PROTECTED TextProfile* TextNego::GetLocalProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetLocalProfile(pOaModel));
}

PROTECTED TextProfile* TextNego::GetPeerProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetPeerProfile(pOaModel));
}

PROTECTED TextProfile* TextNego::GetNegotiatedProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetNegotiatedProfile(pOaModel));
}

PROTECTED IMS_BOOL TextNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_pSdpGenerator == IMS_NULL)
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
        if (pNewOaModel->pPeerProfile->GetPayloadList().GetSize() > 0)
        {
            IMS_TRACE_D("FormAnswer() - use peer profile", 0, 0, 0);
            *pNewOaModel->pNegotiatedProfile = *GetPeerProfile(pNewOaModel);
        }
        else
        {
            // in case of no payload can be answer if there is no payload from dest profile
            // it is a very exceptional case - received offer should have payloads
            // use previous payload contained profile
            if (pNewOaModel->pLocalProfile->GetPayloadList().GetSize() == 0)
            {
                OaModel* pPrevOaModel = IMS_NULL;
                pPrevOaModel = GetNegotiatedOaModel();  // get negotiated OA model which is previous
                                                        // nego result

                if (pPrevOaModel != NULL &&
                        pPrevOaModel->pNegotiatedProfile->GetPayloadList().GetSize() > 0)
                {
                    IMS_TRACE_D("FormAnswer() use previous nego payloads", 0, 0, 0);
                    *pNewOaModel->pNegotiatedProfile = *GetNegotiatedProfile(pPrevOaModel);
                }
                else
                {
                    IMS_TRACE_D("FormAnswer() use src payloads", 0, 0, 0);
                    *pNewOaModel->pNegotiatedProfile = *GetLocalProfile(pNewOaModel);
                }
            }
            else
            {
                IMS_TRACE_D("FormAnswer() use src payloads", 0, 0, 0);
                *pNewOaModel->pNegotiatedProfile = *GetLocalProfile(pNewOaModel);
            }
        }

        pNewOaModel->pNegotiatedProfile->SetIpAddress(pSessionDescriptor->GetLocalAddress());
        pNewOaModel->pNegotiatedProfile->SetBandwidthAs(0);
        pNewOaModel->pNegotiatedProfile->SetBandwidthRs(0);
        pNewOaModel->pNegotiatedProfile->SetBandwidthRr(0);
        pNewOaModel->pNegotiatedProfile->SetDataPort(0);
        pNewOaModel->pNegotiatedProfile->SetControlPort(0);
        pNewOaModel->pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    // Modify a direction
    if (eDir > MEDIA_DIRECTION_INVALID && eDir <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        IMS_TRACE_D("FormAnswer() - set direction[%d]", eDir, 0, 0);
        pNewOaModel->pNegotiatedProfile->SetDirection(eDir);
    }

    // Make the SDP from profile
    return std::static_pointer_cast<TextSdpGenerator>(m_pSdpGenerator)
            ->Generate(pSessionDescriptor, pDescriptor, GetNegotiatedProfile(pNewOaModel));
}

PROTECTED
IMS_BOOL TextNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormReoffer() - pDescriptor[%" PFLS_x "], eDir[%d], OaModel Size(%d)", pDescriptor,
            eDir, m_listOaModel.GetSize());

    IMS_TRACE_D("TextNego - FormReoffer() - eDir[%d] bDisable[%d] EnforceReofferMode[%d]", eDir,
            bDisable, bEnforceReofferMode);

    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pSdpGenerator == IMS_NULL)
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
    IMS_BOOL bIsFullCapability = IMS_FALSE;

    if (m_listOaModel.GetSize() == 0)
    {
        pNewOaModel->pLocalProfile =
                MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT, m_pBaseProfile);
        bIsFullCapability = IMS_TRUE;
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

        if (pPrevOaModel->pNegotiatedProfile->GetPayloadList().GetSize() == 0 ||
                bEnforceReofferMode == IMS_TRUE)
        {
            pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                    MEDIA_TYPE_TEXT, m_pBaseProfile);
            bIsFullCapability = IMS_TRUE;
            IMS_TRACE_I("TextNego::FormReOffer() - Fullcapability - enforce reoffer[%d]",
                    bEnforceReofferMode, 0, 0);
        }
        else
        {
            if (pMediaSessionConfig->IsSdpReofferFullCapability() == IMS_TRUE)
            {
                if (m_pBaseProfile->GetPayloadList().GetSize() > 0)
                {
                    IMS_TRACE_I("FormReoffer() - Fullcapability", 0, 0, 0);
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            MEDIA_TYPE_TEXT, m_pBaseProfile);
                    bIsFullCapability = IMS_TRUE;
                }
                else
                {
                    // this case is only for reoffer but no src profile payload existed
                    IMS_TRACE_I("FormReoffer() - src profile is empty, use nego profile", 0, 0, 0);
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            MEDIA_TYPE_TEXT, GetNegotiatedProfile(pPrevOaModel));
                }
            }
            else
            {
                IMS_TRACE_I("FormReoffer() - use nego profile", 0, 0, 0);
                pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                        MEDIA_TYPE_TEXT, GetNegotiatedProfile(pPrevOaModel));
            }
        }
    }

    if (pNewOaModel->pLocalProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Modify a direction by Enabler
    if (eDir > MEDIA_DIRECTION_INVALID && eDir <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        IMS_TRACE_I("FormReoffer() Enforced Set to direction[%d]", eDir, 0, 0);
        pNewOaModel->pLocalProfile->SetDirection(eDir);
    }

    // Modify a Rtp/RTCP port if text is not supported
    // TODO : change "bDisable" naming to be more clear
    if (bDisable)
    {
        pNewOaModel->pLocalProfile->SetDataPort(0);
        pNewOaModel->pLocalProfile->SetControlPort(0);
        pNewOaModel->pLocalProfile->SetBandwidthAs(0);
        pNewOaModel->pLocalProfile->SetBandwidthRs(0);
        pNewOaModel->pLocalProfile->SetBandwidthRr(0);
    }
    else
    {
        pNewOaModel->pLocalProfile->SetDataPort(m_pBaseProfile->GetDataPort());
        pNewOaModel->pLocalProfile->SetControlPort(m_pBaseProfile->GetControlPort());

        if (bIsFullCapability)
        {
            // set default AS value when srcProfile AS value is 0 in ReOffer case
            if (pNewOaModel->pLocalProfile->GetBandwidthAs() <= 0)
            {
                IMS_TRACE_I("FormReoffer() LocalProfile AS value is 0, change to default AS value",
                        0, 0, 0);
                pNewOaModel->pLocalProfile->SetBandwidthAs(m_pBaseProfile->GetBandwidthAs());
            }

            pNewOaModel->pLocalProfile->SetBandwidthRs(m_pConfig->GetRsBandwidthBps());
            pNewOaModel->pLocalProfile->SetBandwidthRr(m_pConfig->GetRrBandwidthBps());
        }
    }

    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    return std::static_pointer_cast<TextSdpGenerator>(m_pSdpGenerator)
            ->Generate(pSessionDescriptor, pDescriptor, GetLocalProfile(pNewOaModel));
}

PROTECTED MEDIA_DIRECTION TextNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT, m_pBaseProfile);

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) !=
            IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateOffer() - Parse failed", 0, 0, 0);
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (std::static_pointer_cast<TextProfileNegotiator>(m_pProfileNegotiator)
                    ->Negotiate(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel), IMS_TRUE,
                            GetNegotiatedProfile(pNewOaModel), m_pConfig) != IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateOffer() - Negotiate failed", 0, 0, 0);
    }

    // add sessionDesciptorkey key in NewOaModel
    IMS_TRACE_D("NegotiateOffer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_listOaModel.Append(pNewOaModel);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}

PROTECTED MEDIA_DIRECTION TextNego::NegotiateAnswer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
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
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) !=
            IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateAnswer() - Parse failed", 0, 0, 0);
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (std::static_pointer_cast<TextProfileNegotiator>(m_pProfileNegotiator)
                    ->Negotiate(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel),
                            IMS_FALSE, GetNegotiatedProfile(pNewOaModel), m_pConfig) != IMS_TRUE)
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
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}
