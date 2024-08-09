// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "text/TextProfileExtractor.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextProfileExtractor::TextProfileExtractor() :
        ProfileExtractor(MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextProfileExtractor()", 0, 0, 0);
}

PUBLIC VIRTUAL TextProfileExtractor::~TextProfileExtractor()
{
    IMS_TRACE_I("~TextProfileExtractor()", 0, 0, 0);
}

PUBLIC IMS_BOOL TextProfileExtractor::Extract(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "Extract() - invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("Extract()", 0, 0, 0);

    ProfileExtractor::Extract(pSessionDescriptor, pDescriptor, pProfile);

    // payload
    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSDPCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));

        if (pSDPCodec == IMS_NULL)
        {
            IMS_TRACE_E(0, "Extract() - pSDPCodec is NULL", 0, 0, 0);
            return IMS_FALSE;
        }
        AString strCodecName = pSDPCodec->GetName();

        IMS_TRACE_I("Extract() - At(%d), GetPayloadType(%d), GetClockRate(%d)", i,
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
                IMS_TRACE_E(0, "Extract() - Cannot make fmtp for 'red'", 0, 0, 0);
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

                IMS_TRACE_I("Extract() - Check RedSubPT, PT[%d] of PL[%d] / Red Payload [%d]",
                        pSDPCodec->GetPayloadType(), j, pRedFmtp->GetRedPayload());
                if (pSDPCodec->GetPayloadType() == pRedFmtp->GetRedPayload())
                {
                    bRedSubPTExist = IMS_TRUE;
                }
            }

            if (bRedSubPTExist == IMS_FALSE)
            {
                IMS_TRACE_E(0, "Extract() - No matched rtpmap for subtype of 'red'", 0, 0, 0);
                delete pPayload;
                delete pRedFmtp;
                continue;
            }

            IMS_TRACE_I("Extract() - Redundancy presented[%d]", pRedFmtp->GetRedLevel(), 0, 0);
            pPayload->SetFmtp(pRedFmtp);
        }
        else if (!strCodecName.EqualsIgnoreCase("t140"))
        {
            IMS_TRACE_E(0, "Extract() - Invalid codec[%s]", strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        pProfile->GetPayloadList().Append(pPayload);
    }

    IMS_TRACE_I("Extract() - Ended[%d]", 0, 0, 0);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL TextProfileExtractor::GetFmtpFromString(
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