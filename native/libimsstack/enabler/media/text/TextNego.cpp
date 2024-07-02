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

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextNego::TextNego(IMS_SINT32 nSlotId) :
        BaseNego(nSlotId, MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextNego() - slot[%d]", nSlotId, 0, 0);
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
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_TYPE_INVALID;
    }

    IMS_TRACE_I("IsMediaCodecFromSdpSupported()", 0, 0, 0);

    OaModel objOaModel;
    objOaModel.pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT, m_pBaseProfile);

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (MakeProfileFromSDP(pSessionDescriptor, pDescriptor, GetPeerProfile(&objOaModel)) !=
            IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

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
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, GetNegotiatedProfile(pNewOaModel));
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
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
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
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, GetLocalProfile(pNewOaModel));
}

PROTECTED MEDIA_DIRECTION TextNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateOffer() - pSessionDescriptor or pDescriptor is NULL", 0, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT, m_pBaseProfile);

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (MakeProfileFromSDP(pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) !=
            IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateOffer() - MakeProfileFromSDP failed", 0, 0, 0);
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (MakeNegotiatedProfile(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel), IMS_TRUE,
                GetNegotiatedProfile(pNewOaModel)) != IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateOffer() - MakeNegotiatedProfile failed", 0, 0, 0);
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
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (MakeProfileFromSDP(pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) !=
            IMS_TRUE)
    {
        IMS_TRACE_E(0, "NegotiateAnswer() - MakeProfileFromSDP failed", 0, 0, 0);
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_TEXT);

    if (MakeNegotiatedProfile(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel), IMS_FALSE,
                GetNegotiatedProfile(pNewOaModel)) != IMS_TRUE)
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

PROTECTED
IMS_BOOL TextNego::MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pBaseProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    TextProfile* pProfile = ProfileCasting(pBaseProfile);

    // clean attr & bandwidth line
    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // Make "c" & "o" line of session level if IP does not matched
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->GetIpAddress()))
    {
        IMS_TRACE_D("MakeSdpFromProfile() - Ip is not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->GetIpAddress().ToCharString(), 0);

        pSessionDescriptor->SetConnectionAddress(pProfile->GetIpAddress().ToString());
        pSessionDescriptor->SetOriginAddress(pProfile->GetIpAddress().ToString());
    }

    // Check and delete "red" type which contains invalid sub payload type
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* pPayload = pProfile->GetPayloadAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            TextProfile::RedFmtp* pRedFmtp =
                    static_cast<TextProfile::RedFmtp*>(pPayload->GetFmtp());

            if (pRedFmtp == IMS_NULL)
            {
                continue;
            }

            IMS_TRACE_I("MakeSdpFromProfile() - fmtp, Redundancy Level [%d], Red Payload[%d]",
                    pRedFmtp->GetRedLevel(), pRedFmtp->GetRedPayload(), 0);

            IMS_BOOL bRedSubPTExist = IMS_FALSE;

            for (IMS_UINT32 j = 0; j < pProfile->GetPayloadList().GetSize(); j++)
            {
                TextProfile::Payload* pTempPayload = pProfile->GetPayloadAt(j);

                if (pTempPayload == IMS_NULL)
                {
                    continue;
                }

                IMS_TRACE_I("MakeSdpFromProfile() - RedSubPT, PT[%d] of PL(%d) / Red Payload "
                            "[%d]",
                        pTempPayload->GetRtpMap().GetPayloadNumber(), j, pRedFmtp->GetRedPayload());

                if (pTempPayload->GetRtpMap().GetPayloadNumber() ==
                        (IMS_UINT32)pRedFmtp->GetRedPayload())
                {
                    bRedSubPTExist = IMS_TRUE;
                }
            }

            if (bRedSubPTExist == IMS_FALSE)
            {
                IMS_TRACE_E(0,
                        "MakeSdpFromProfile() - SubPayloadtype for Redundancy isn't exist. skip "
                        "Payload, Payload[%s], PT[%d]",
                        pPayload->GetRtpMap().GetPayloadType().GetStr(),
                        pPayload->GetRtpMap().GetPayloadNumber(), 0);
                pProfile->GetPayloadList().RemoveAt(i);
                delete pPayload;
            }
        }
    }

    IMS_TRACE_I("MakeSdpFromProfile() - After Check Validity, PayloadSize[%d]",
            pProfile->GetPayloadList().GetSize(), 0, 0);

    // Make "m" line
    // ------ "m=text xxxx Rtp/AVP 100 98"
    AStringArray objTextFormat;
    AString strPayloadNum;

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* pPayload = pProfile->GetPayloadAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        objTextFormat.AddElement(strPayloadNum);
    }

    pDescriptor->SetMediaDescription(SdpMedia::TYPE_TEXT, pProfile->GetDataPort(),
            SdpMedia::TRANSPORT_RTP_AVP, objTextFormat);

    // Make bandwidth
    // ------ "b=AS:xx"
    // ------ "b=AS:xx"
    // ------ "b=AS:xx"
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

    // Make each payload
    // ------ "a=rtpmap:98 t140/1000"
    // ------ "a=rtpmap:112 red/1000
    // ------ "a=fmtp:112 111/111/111"
    // ------ "a=rtpmap:111 t140/1000
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpmap, strFmtp;
        TextProfile::Payload* pPayload = pProfile->GetPayloadAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // make "rtpmap"
        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        strRtpmap.Sprintf("%s/%d", pPayload->GetRtpMap().GetPayloadType().GetStr(),
                pPayload->GetRtpMap().GetSamplingRate());

        // make "fmtp"
        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            TextProfile::RedFmtp* pRedFmtp = (TextProfile::RedFmtp*)pPayload->GetFmtp();

            if (pRedFmtp == IMS_NULL)
            {
                continue;
            }

            IMS_UINT32 nCount = pRedFmtp->GetRedLevel();
            AString TempSubPT;
            TempSubPT.Sprintf("%d", pRedFmtp->GetRedPayload());

            while (nCount-- > 0)
            {
                if (strFmtp.GetLength() > 0)
                {
                    strFmtp.Append("/");
                }

                strFmtp.Append(TempSubPT);
            }

            IMS_TRACE_I("MakeSdpFromProfile() - Add fmtp, nRedundancy[%d], Red Payload[%d], "
                        "Fmtp[%s]",
                    pRedFmtp->GetRedLevel(), pRedFmtp->GetRedPayload(), strFmtp.GetStr());
        }

        if (strFmtp.GetLength() == 0)
        {
            strFmtp = AString::ConstNull();
        }

        pDescriptor->SetMediaFormat(SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpmap, strFmtp);
    }

    // Make direction
    pDescriptor->SetDirection(pProfile->GetDirection());
    IMS_TRACE_I(
            "MakeSdpFromProfile() - payloadSize[%d]", pProfile->GetPayloadList().GetSize(), 0, 0);

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
    pProfile->SetIpAddress(pDescriptor->GetRemoteAddress());

    // data & control port
    pProfile->SetDataPort(pDescriptor->GetRemotePort());
    if (pDescriptor->GetAttributeInt(SdpAttribute::RTCP) == IMediaDescriptor::INVALID_VALUE)
    {
        pProfile->SetControlPort(pProfile->GetDataPort() + 1);
    }
    else
    {
        pProfile->SetControlPort(pDescriptor->GetAttributeInt(SdpAttribute::RTCP));
    }

    // bandwidth
    pProfile->SetBandwidthAs(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_AS));
    pProfile->SetBandwidthRs(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_RS));
    pProfile->SetBandwidthRr(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_RR));

    IMS_TRACE_I("MakeProfileFromSDP() - AS[%d], RS[%d], RR[%d]", pProfile->GetBandwidthAs(),
            pProfile->GetBandwidthRs(), pProfile->GetBandwidthRr());

    // payload
    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

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

                IMS_TRACE_I("MakeSdpFromProfile() - Check RedSubPT, PT[%d] of PL(%d) / Red Payload "
                            "[%d]",
                        pSDPCodec->GetPayloadType(), j, pRedFmtp->GetRedPayload());
                if (pSDPCodec->GetPayloadType() == pRedFmtp->GetRedPayload())
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

            IMS_TRACE_I("MakeProfileFromSDP() - Redundancy presented [%d]", pRedFmtp->GetRedLevel(),
                    0, 0);
            pPayload->SetFmtp(pRedFmtp);
        }
        else if (!strCodecName.EqualsIgnoreCase("t140"))
        {
            IMS_TRACE_E(
                    0, "MakeProfileFromSDP() - Invalid codec [%s]", strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        pProfile->GetPayloadList().Append(pPayload);
    }

    // direction
    pProfile->SetDirection((MEDIA_DIRECTION)pDescriptor->GetDirection());
    if (pProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_D("MakeProfileFromSDP() - Text Media level Direction does not exist..", 0, 0, 0);
        // check session level attribute Direction
        pProfile->SetDirection((MEDIA_DIRECTION)pSessionDescriptor->GetDirection());
        if (pProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
            pProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
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
    pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());

    IMS_TRACE_D("MakeNegotiatedProfile() - local address[%s] PeerPayloadSize[%d]",
            pLocalProfile->GetIpAddress().ToCharString(), pPeerProfile->GetPayloadList().GetSize(),
            0);

    // Setting Rtp/RTCP port of mine
    pNegotiatedProfile->SetDataPort(pLocalProfile->GetDataPort());
    pNegotiatedProfile->SetControlPort(pLocalProfile->GetControlPort());

    if (pNegotiatedProfile->GetDataPort() == 0 || pPeerProfile->GetDataPort() == 0)
    {
        *pNegotiatedProfile = (pPeerProfile->GetPayloadList().GetSize() > 0)
                ? *ProfileCasting(pPeerProfile)
                : *ProfileCasting(pLocalProfile);

        pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());
        pNegotiatedProfile->SetDataPort(0);

        IMS_TRACE_D("MakeNegotiatedProfile() - ZERO Port. DO NOT Use the text[%d][%d],\
                But nego is successful",
                pNegotiatedProfile->GetDataPort(), pPeerProfile->GetDataPort(), 0);
        return IMS_TRUE;
    }

    if (m_pConfig == IMS_NULL)
    {
        IMS_TRACE_D("MakeNegotiatedProfile() - no config, return true to reject", 0, 0, 0);
        return IMS_TRUE;
    }

    // Compare each payload based destination's profile
    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* pPayload = pPeerProfile->GetPayloadAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140") ||
                pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            if (FindT140InProfile(pLocalProfile, pPayload) == IMS_TRUE)
            {
                TextProfile::Payload* pT140 = new TextProfile::Payload();
                pT140->SetRtpMap(pPayload->GetRtpMap());

                if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
                {
                    pT140->SetFmtp(new TextProfile::RedFmtp(
                            *static_cast<TextProfile::RedFmtp*>(pPayload->GetFmtp())));
                }

                pNegotiatedProfile->GetPayloadList().Append(pT140);
            }
        }
    }

    IMS_BOOL bRet = IMS_FALSE;

    if (pNegotiatedProfile->GetPayloadList().GetSize() > 0)
    {
        // Setting direction
        if (pNegotiatedProfile->GetDataPort() != 0 && pPeerProfile->GetDataPort() != 0)
        {
            pNegotiatedProfile->SetDirection(UpdateDirectionToMine(
                    pPeerProfile->GetDirection(), pLocalProfile->GetDirection(), bIsOfferReceived));
        }
        else
        {
            pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
        }

        TextProfileUtil::MakeNegotiatedBandwidth(ConfigCasting(m_pConfig), pLocalProfile,
                pPeerProfile, bIsOfferReceived, -1, pNegotiatedProfile);
        bRet = IMS_TRUE;
    }
    else
    {
        if (pLocalProfile->GetPayloadList().GetSize() > 0)
        {
            IMS_TRACE_D("MakeNegotiatedProfile() - no negotiated payload. use the LocalProfile and "
                        "make port 0 ",
                    0, 0, 0);
            *pNegotiatedProfile = *ProfileCasting(pLocalProfile);
            bRet = IMS_TRUE;
        }
        else
        {
            IMS_TRACE_E(0, "There's no Payload in LocalProfile", 0, 0, 0);
        }

        pNegotiatedProfile->SetDataPort(0);
        pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    IMS_TRACE_D("MakeNegotiatedProfile() - Direction=%d, nego rs=%d, rr=%d",
            pNegotiatedProfile->GetDirection(), pNegotiatedProfile->GetBandwidthRs(),
            pNegotiatedProfile->GetBandwidthRr());

    if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
    {
        pNegotiatedProfile->SetRtcpInterval(0);
        IMS_TRACE_D("MakeNegotiatedProfile() - negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
    }
    else
    {
        pNegotiatedProfile->SetRtcpInterval(m_pConfig->GetRtcpInterval());
    }

    IMS_TRACE_D("MakeNegotiatedProfile() - negotiated payload size[%d], port[%d], direction[%d], ",
            pNegotiatedProfile->GetPayloadList().GetSize(), pNegotiatedProfile->GetDataPort(),
            pNegotiatedProfile->GetDirection());
    return bRet;
}

PRIVATE IMS_BOOL TextNego::GetFmtpFromString(
        IN const AString& strFmtp, OUT TextProfile::RedFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL || strFmtp.IsEmpty() == IMS_TRUE)
        return IMS_FALSE;

    ImsList<AString> strArrTemp = strFmtp.Split('/');
    pFmtp->SetRedLevel(strArrTemp.GetSize());

    if (pFmtp->GetRedLevel() == 0)
    {
        return IMS_FALSE;
    }

    pFmtp->SetRedPayload(strArrTemp.GetAt(0).ToInt32());

    for (IMS_SINT32 i = 0; i < pFmtp->GetRedLevel() - 1; i++)
    {
        if (strArrTemp.GetAt(i).ToInt32() != pFmtp->GetRedPayload())
        {
            pFmtp->SetRedLevel(-1);
            pFmtp->SetRedPayload(-1);
            return IMS_FALSE;
        }
    }

    IMS_TRACE_D("GetFmtpFromString() Ended. Red Level[%d], Red Payload[%d]", pFmtp->GetRedLevel(),
            pFmtp->GetRedPayload(), 0);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL TextNego::FindT140InProfile(
        IN TextProfile* pProfile, IN TextProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* comparedPayload = pProfile->GetPayloadAt(i);

        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"))
        {
            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) &&
                    comparedPayload->GetRtpMap().GetSamplingRate() ==
                            pPayload->GetRtpMap().GetSamplingRate())
            {
                IMS_TRACE_D("FindT140InProfile() - Found T140 at [%d], Codec[%s]", i,
                        comparedPayload->GetRtpMap().GetPayloadType().GetStr(), 0);

                return IMS_TRUE;
            }
        }
        else if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) &&
                    comparedPayload->GetRtpMap().GetSamplingRate() ==
                            pPayload->GetRtpMap().GetSamplingRate())
            {
                TextProfile::RedFmtp* pComparedFmtp =
                        (TextProfile::RedFmtp*)comparedPayload->GetFmtp();
                TextProfile::RedFmtp* pReceivedFmtp = (TextProfile::RedFmtp*)pPayload->GetFmtp();

                if (pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                if (pReceivedFmtp->GetRedLevel() > pComparedFmtp->GetRedLevel() ||
                        pReceivedFmtp->GetRedPayload() < 0)
                {
                    continue;
                }

                IMS_TRACE_D("FindT140InProfile() - Found RED at [%d], Codec[%s]", i,
                        comparedPayload->GetRtpMap().GetPayloadType().GetStr(), 0);

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

    if (bIsMtCase == IMS_FALSE)
    {
        return eLocalDirection;
    }

    switch (ePeerDirection)
    {
        case MEDIA_DIRECTION_INACTIVE:  // FALL_THROUGH
        case MEDIA_DIRECTION_SEND_RECEIVE:
            return ePeerDirection;
        case MEDIA_DIRECTION_RECEIVE:
            return MEDIA_DIRECTION_SEND;
        case MEDIA_DIRECTION_SEND:
            return MEDIA_DIRECTION_RECEIVE;
        default:
            return MEDIA_DIRECTION_INVALID;
    }
}
