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
#include "ServiceTrace.h"

#include "MediaProfileFactory.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaSessionConfigFactory.h"
#include "text/TextNego.h"
#include "text/TextProfileGenerator.h"
#include "text/TextSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextNego::TextNego(IMS_SINT32 nSlotId) :
        BaseNego(nSlotId, MEDIA_TYPE_TEXT),
        m_pSdpParser(std::make_shared<TextSdpParser>()),
        m_pProfileNegotiator(std::make_shared<TextProfileNegotiator>())
{
    IMS_TRACE_I("+TextNego(): slot[%d]", nSlotId, 0, 0);
    m_pSdpGenerator = std::make_shared<TextSdpGenerator>();
    m_pProfileGenerator = std::make_shared<TextProfileGenerator>();
}

PUBLIC
TextNego::TextNego(IN const TextNego& obj) :
        BaseNego(obj),
        m_pSdpParser(std::make_shared<TextSdpParser>()),
        m_pProfileNegotiator(std::make_shared<TextProfileNegotiator>())
{
    IMS_TRACE_I("+TextNego(): slot[%d]", GetSlotId(), 0, 0);

    m_pSdpGenerator = std::make_shared<TextSdpGenerator>();
    m_pProfileGenerator = std::make_shared<TextProfileGenerator>();
    Copy(&obj);
}

PUBLIC
TextNego& TextNego::operator=(IN const TextNego& obj)
{
    if (this != &obj)
    {
        BaseNego::operator=(obj);
        m_pSdpParser = std::make_shared<TextSdpParser>();
        m_pSdpGenerator = std::make_shared<TextSdpGenerator>();
        m_pProfileNegotiator = std::make_shared<TextProfileNegotiator>();
        m_pProfileGenerator = std::make_shared<TextProfileGenerator>();
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
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): invalid arguments", 0, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    OaModel objOaModel;
    objOaModel.pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile);

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(&objOaModel)))
    {
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): failed to parse SDP", 0, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(&objOaModel), GetPeerProfile(&objOaModel),
                IMS_TRUE, GetNegotiatedProfile(&objOaModel), m_pConfig))
    {
        IMS_TRACE_E(0, "IsMediaCodecFromSdpSupported(): failed to negotiate SDP", 0, 0, 0);
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

    IMS_TRACE_D("GetNegotiatedCodec(): Negotiated Payload Type is [%s]",
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

PROTECTED
IMS_BOOL TextNego::FormOffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    if (CheckArgument(pSessionDescriptor, pDescriptor, eDirection) && m_pSdpGenerator)
    {
        // Make the SDP from profile
        return m_pSdpGenerator->Generate(pSessionDescriptor, pDescriptor,
                GetLocalProfile(CreateOaModel(eDirection, bDisable)));
    }

    return IMS_FALSE;
}

PROTECTED IMS_BOOL TextNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_pSdpGenerator == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormAnswer(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (eDirection == MEDIA_DIRECTION_INVALID && !bDisable)
    {
        IMS_TRACE_E(0, "FormAnswer(): invalid direction", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_listOaModel.IsEmpty())
    {
        IMS_TRACE_E(0, "FormAnswer(): empty OA model list", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("FormAnswer(): direction[%d], disable[%d]", eDirection, bDisable, 0);

    // Getting OaModel from list
    OaModel* pNewOaModel = GetNegotiatedOaModel();

    if (pNewOaModel == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormAnswer(): invalid OA model", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!pNewOaModel->IsAllProfileExist())
    {
        IMS_TRACE_E(0, "FormAnswer(): invalid OA model", 0, 0, 0);
        return IMS_FALSE;
    }

    // Modify a Rtp/RTCP port if text is not supported
    if (bDisable)
    {
        // copy the dest profile as nego profile
        if (pNewOaModel->pPeerProfile->GetPayloadList().GetSize() > 0)
        {
            IMS_TRACE_D("FormAnswer(): use peer profile", 0, 0, 0);
            *pNewOaModel->pNegotiatedProfile = *GetPeerProfile(pNewOaModel);
        }
        else
        {
            // If the peer profile lacks payloads, try using previously negotiated ones. (Unusual
            // case.)
            if (pNewOaModel->pLocalProfile->GetPayloadList().GetSize() == 0)
            {
                OaModel* pPrevOaModel = IMS_NULL;
                // get negotiated OA model which is previou nego result
                pPrevOaModel = GetNegotiatedOaModel();

                if (pPrevOaModel != IMS_NULL &&
                        pPrevOaModel->pNegotiatedProfile->GetPayloadList().GetSize() > 0)
                {
                    IMS_TRACE_D("FormAnswer(): use previous nego payloads", 0, 0, 0);
                    *pNewOaModel->pNegotiatedProfile = *GetNegotiatedProfile(pPrevOaModel);
                }
                else
                {
                    IMS_TRACE_D("FormAnswer(): use local payloads", 0, 0, 0);
                    *pNewOaModel->pNegotiatedProfile = *GetLocalProfile(pNewOaModel);
                }
            }
            else
            {
                IMS_TRACE_D("FormAnswer(): use local payloads", 0, 0, 0);
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
    if (eDirection > MEDIA_DIRECTION_INVALID && eDirection <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        pNewOaModel->pNegotiatedProfile->SetDirection(eDirection);
    }

    // Make the SDP from profile
    return m_pSdpGenerator->Generate(
            pSessionDescriptor, pDescriptor, GetNegotiatedProfile(pNewOaModel));
}

PROTECTED
IMS_BOOL TextNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pSdpGenerator == IMS_NULL)
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

    // Make new Offer/Answer model, and copy source profile from previous negotiated profile
    OaModel* pNewOaModel = new OaModel();
    IMS_BOOL bIsFullCapability = IMS_FALSE;

    if (m_listOaModel.IsEmpty())
    {
        pNewOaModel->pLocalProfile =
                MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile);
        bIsFullCapability = IMS_TRUE;
    }
    else
    {
        OaModel* pPrevOaModel = GetNegotiatedOaModel();

        if (pPrevOaModel == IMS_NULL)
        {
            IMS_TRACE_E(0, "FormReoffer(): invalid OA model", 0, 0, 0);
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
                bEnforceReofferMode)
        {
            pNewOaModel->pLocalProfile =
                    MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile);
            bIsFullCapability = IMS_TRUE;
        }
        else
        {
            if (pMediaSessionConfig->IsSdpReofferFullCapability())
            {
                if (m_pBaseProfile->GetPayloadList().GetSize() > 0)
                {
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            m_eType, m_pBaseProfile);
                    bIsFullCapability = IMS_TRUE;
                }
                else
                {
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            m_eType, GetNegotiatedProfile(pPrevOaModel));
                }
            }
            else
            {
                pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                        m_eType, GetNegotiatedProfile(pPrevOaModel));
            }
        }
    }

    if (pNewOaModel->pLocalProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Modify a direction by Enabler
    if (eDirection > MEDIA_DIRECTION_INVALID && eDirection <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        pNewOaModel->pLocalProfile->SetDirection(eDirection);
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
                pNewOaModel->pLocalProfile->SetBandwidthAs(m_pBaseProfile->GetBandwidthAs());
            }

            pNewOaModel->pLocalProfile->SetBandwidthRs(m_pConfig->GetRsBandwidthBps());
            pNewOaModel->pLocalProfile->SetBandwidthRr(m_pConfig->GetRrBandwidthBps());
        }
    }

    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    return m_pSdpGenerator->Generate(pSessionDescriptor, pDescriptor, GetLocalProfile(pNewOaModel));
}

PROTECTED MEDIA_DIRECTION TextNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL ||
            m_pProfileNegotiator == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateOffer(): invalid arguments", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(m_eType, m_pBaseProfile);

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)))
    {
        IMS_TRACE_E(0, "NegotiateOffer(): failed to parse SDP", 0, 0, 0);
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel),
                IMS_TRUE, GetNegotiatedProfile(pNewOaModel), m_pConfig))
    {
        IMS_TRACE_E(0, "NegotiateOffer(): failed to negotiate SDP", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_listOaModel.Append(pNewOaModel);
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}

PROTECTED MEDIA_DIRECTION TextNego::NegotiateAnswer(
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

    // Get the latest OAmodel from list
    OaModel* pNewOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);

    if (pNewOaModel == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): invalid OA model", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    if (pNewOaModel->pPeerProfile != IMS_NULL)
    {
        delete pNewOaModel->pPeerProfile;
    }

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pSdpParser->Parse(pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)))
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): failed to parse SDP", 0, 0, 0);
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    if (pNewOaModel->pNegotiatedProfile != IMS_NULL)
    {
        delete pNewOaModel->pNegotiatedProfile;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);

    if (!m_pProfileNegotiator->Negotiate(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel),
                IMS_FALSE, GetNegotiatedProfile(pNewOaModel), m_pConfig))
    {
        IMS_TRACE_E(0, "NegotiateAnswer(): failed to negotiate SDP", 0, 0, 0);
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}
