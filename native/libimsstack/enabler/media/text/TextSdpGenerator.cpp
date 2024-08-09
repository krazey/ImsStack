// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "text/TextProfileUtil.h"
#include "text/TextSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextSdpGenerator::TextSdpGenerator() :
        SdpGenerator(MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextSdpGenerator()", 0, 0, 0);
}

PUBLIC VIRTUAL TextSdpGenerator::~TextSdpGenerator()
{
    IMS_TRACE_I("~TextSdpGenerator()", 0, 0, 0);
}

PUBLIC
IMS_BOOL TextSdpGenerator::Generate(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pBaseProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    TextProfile* pProfile = static_cast<TextProfile*>(pBaseProfile);

    // clean attr & bandwidth line
    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // Make "c" & "o" line of session level if IP does not matched
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->GetIpAddress()))
    {
        IMS_TRACE_D("Generate() - Ip is not matched, SessionIP[%s], ProfileIP[%s]",
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

            IMS_TRACE_I("Generate() - fmtp, Redundancy Level [%d], Red Payload[%d]",
                    pRedFmtp->GetRedLevel(), pRedFmtp->GetRedPayload(), 0);

            IMS_BOOL bRedSubPTExist = IMS_FALSE;

            for (IMS_UINT32 j = 0; j < pProfile->GetPayloadList().GetSize(); j++)
            {
                TextProfile::Payload* pTempPayload = pProfile->GetPayloadAt(j);

                if (pTempPayload == IMS_NULL)
                {
                    continue;
                }

                IMS_TRACE_I("Generate() - RedSubPT, PT[%d] of PL(%d) / Red Payload "
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
                        "Generate() - SubPayloadtype for Redundancy isn't exist. skip "
                        "Payload, Payload[%s], PT[%d]",
                        pPayload->GetRtpMap().GetPayloadType().GetStr(),
                        pPayload->GetRtpMap().GetPayloadNumber(), 0);
                pProfile->GetPayloadList().RemoveAt(i);
                delete pPayload;
            }
        }
    }

    IMS_TRACE_I("Generate() - After Check Validity, PayloadSize[%d]",
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

            IMS_TRACE_I("Generate() - Add fmtp, nRedundancy[%d], Red Payload[%d], "
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
    IMS_TRACE_I("Generate() - payloadSize[%d]", pProfile->GetPayloadList().GetSize(), 0, 0);

    return IMS_TRUE;
}
