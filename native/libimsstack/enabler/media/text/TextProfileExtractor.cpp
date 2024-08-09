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
    ExtractPayloads(pDescriptor, pProfile);

    return IMS_TRUE;
}

PRIVATE
void TextProfileExtractor::ExtractPayloads(
        IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ExtractPayloads() - invalid argument", 0, 0, 0);
        return;
    }

    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));
        TextProfile::Payload* pPayload = new TextProfile::Payload();

        if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
        {
            delete pPayload;
            continue;
        }

        IMS_TRACE_I("ExtractPayloads() - At[%d]", i, 0, 0);

        AString strCodecName = AString::ConstNull();
        ExtractRtpMap(pSdpCodec, pPayload, strCodecName);

        // check fmtp of t140 redundancy
        if (strCodecName.EqualsIgnoreCase("red"))
        {
            ExtractFmtp(pSdpCodec->GetFormatSpecificParameter(), pPayload, lstMediaFormat);
        }
        else if (!strCodecName.EqualsIgnoreCase("t140"))
        {
            IMS_TRACE_E(0, "ExtractPayloads() - Invalid codec[%s]", strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        pProfile->GetPayloadList().Append(pPayload);
    }
}

PRIVATE
void TextProfileExtractor::ExtractRtpMap(IN const SdpAvCodec* pSdpCodec,
        OUT TextProfile::Payload* pPayload, OUT AString& strCodecName)
{
    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nPayloadTypeNumber = pSdpCodec->GetPayloadType();
    strCodecName = pSdpCodec->GetName();
    IMS_UINT32 nSamplingRate = pSdpCodec->GetClockRate();

    pPayload->SetRtpMap(nPayloadTypeNumber, strCodecName, nSamplingRate);

    IMS_TRACE_D("ExtractRtpMap() - Payload[%d], Codec[%s], Sampling rate[%d]", nPayloadTypeNumber,
            strCodecName.GetStr(), nSamplingRate);
}

PRIVATE
IMS_BOOL TextProfileExtractor::ExtractFmtp(IN const AString& strFmtp,
        OUT TextProfile::Payload* pPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat)
{
    if (pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp();

    if (pRedFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("ExtractFmtp()", 0, 0, 0);

    if (ExtractRedFmtp(strFmtp, pRedFmtp) == IMS_FALSE ||
            ExtractRedSubPtExist(pRedFmtp->GetRedPayload(), lstMediaFormat) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "ExtractFmtp() - cannot make red fmtp or No matched subtype", 0, 0, 0);

        delete pRedFmtp;
        return IMS_FALSE;
    }

    pPayload->SetFmtp(pRedFmtp);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL TextProfileExtractor::ExtractRedFmtp(
        IN const AString& strFmtp, OUT TextProfile::RedFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL || strFmtp.IsEmpty() == IMS_TRUE)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("ExtractRedFmtp()", 0, 0, 0);

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

    IMS_TRACE_D("ExtractRedFmtp() Red Level[%d], Red Payload[%d]", pFmtp->GetRedLevel(),
            pFmtp->GetRedPayload(), 0);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL TextProfileExtractor::ExtractRedSubPtExist(
        IN const IMS_SINT32 nRedPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat)
{
    IMS_BOOL bRedSubPtExist = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));

        if (pSdpCodec == IMS_NULL)
        {
            continue;
        }

        IMS_TRACE_D("ExtractRedSubPtExist() - Check RedSubPT, PT[%d] of PL[%d] / Red Payload[%d]",
                pSdpCodec->GetPayloadType(), i, nRedPayload);

        if (pSdpCodec->GetPayloadType() == nRedPayload)
        {
            bRedSubPtExist = IMS_TRUE;
        }
    }

    IMS_TRACE_I("ExtractRedSubPtExist() - RedSubPtExist[%d]", bRedSubPtExist, 0, 0);

    return bRedSubPtExist;
}
