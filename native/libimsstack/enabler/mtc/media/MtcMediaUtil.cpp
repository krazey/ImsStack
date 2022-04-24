#include "mtc/media/MtcMediaUtil.h"
#include "FailReason.h"
#include "MtcDef.h"
#include "ServicePhoneInfo.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL
IMS_SINT32 MtcMediaUtil::GetMediaDirectionFromSdp()
{
    return DIRECTION_INVALID;
}

PUBLIC GLOBAL
IMS_UINT32 MtcMediaUtil::GetMediaTypesFromSdp()
{
    return MEDIATYPE_NONE;
}

PUBLIC GLOBAL
IMS_BOOL MtcMediaUtil::IsMediaPortValid()
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
CallType MtcMediaUtil::GetCallTypeFromMediaTypes(IN IMS_UINT32 eMediaTypes)
{
    CallType eCallType = CallType::UNKNOWN;

    if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_VIDEO) &&
            (eMediaTypes & MEDIATYPE_TEXT))
    {
        eCallType = CallType::VIDEO_RTT;
    }
    else if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_VIDEO))
    {
        eCallType = CallType::VT;
    }
    else if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_TEXT))
    {
        eCallType = CallType::RTT;
    }
    else if (eMediaTypes & MEDIATYPE_AUDIO)
    {
        eCallType = CallType::VOIP;
    }

    return eCallType;
}

PUBLIC GLOBAL
CallType MtcMediaUtil::GetCallTypeFromMediaContents(IN MEDIA_CONTENT_TYPE eMediaContents)
{
    CallType eCallType = CallType::UNKNOWN;

    switch (eMediaContents)
    {
        case MEDIA_TYPE_AUDIO:
            eCallType = CallType::VOIP;
            break;
        case MEDIA_TYPE_AUDIOVIDEO:
            eCallType = CallType::VT;
            break;
        case MEDIA_TYPE_AUDIOTEXT:
            eCallType = CallType::RTT;
            break;
        case MEDIA_TYPE_AUDIOVIDEOTEXT:
            eCallType = CallType::VIDEO_RTT;
            break;

        default:
            break;
    }

    return eCallType;
}

PUBLIC GLOBAL
IMS_UINT32 MtcMediaUtil::GetMediaTypesFromCallType(IN CallType eCallType)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;

    switch (eCallType)
    {
        case CallType::VOIP:
            eMediaTypes = MEDIATYPE_AUDIO;
            break;
        case CallType::VT:
            eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_VIDEO;
            break;
        case CallType::RTT:
            eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_TEXT;
            break;
        case CallType::VIDEO_RTT:
            eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_VIDEO | MEDIATYPE_TEXT;
            break;

        default:
            break;
    }

    return eMediaTypes;
}

PUBLIC GLOBAL
IMS_UINT32 MtcMediaUtil::GetMediaTypesFromMediaContents(IN MEDIA_CONTENT_TYPE eMediaContents)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;

    if (eMediaContents & MEDIA_TYPE_AUDIO)
    {
        eMediaTypes |= MEDIATYPE_AUDIO;
    }

    if (eMediaContents & MEDIA_TYPE_VIDEO)
    {
        eMediaTypes |= MEDIATYPE_VIDEO;
    }

    if (eMediaContents & MEDIA_TYPE_TEXT)
    {
        eMediaTypes |= MEDIATYPE_TEXT;
    }

    return eMediaTypes;
}

PUBLIC GLOBAL
MEDIA_CONTENT_TYPE MtcMediaUtil::GetMediaContentsFromMediaTypes(IN IMS_UINT32 eMediaTypes)
{
    MEDIA_CONTENT_TYPE eContents = MEDIA_TYPE_INVALID;

    if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_VIDEO)
            && (eMediaTypes & MEDIATYPE_TEXT))
    {
        eContents = MEDIA_TYPE_AUDIOVIDEOTEXT;
    }
    else if ((eMediaTypes & MEDIATYPE_VIDEO) && (eMediaTypes & MEDIATYPE_TEXT))
    {
        eContents = MEDIA_TYPE_VIDEOTEXT;
    }
    else if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_TEXT))
    {
        eContents = MEDIA_TYPE_AUDIOTEXT;
    }
    else if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_VIDEO))
    {
        eContents = MEDIA_TYPE_AUDIOVIDEO;
    }
    else if (eMediaTypes & MEDIATYPE_TEXT)
    {
        eContents = MEDIA_TYPE_TEXT;
    }
    else if (eMediaTypes & MEDIATYPE_VIDEO)
    {
        eContents = MEDIA_TYPE_VIDEO;
    }
    else if (eMediaTypes & MEDIATYPE_AUDIO)
    {
        eContents = MEDIA_TYPE_AUDIO;
    }

    return eContents;
}

PUBLIC GLOBAL
MEDIA_CONTENT_TYPE MtcMediaUtil::GetMediaContentsFromCallType(IN CallType eCallType)
{
    MEDIA_CONTENT_TYPE eMediaContentTypes = MEDIA_TYPE_INVALID;

    switch (eCallType)
    {
        case CallType::VOIP:
            eMediaContentTypes = MEDIA_TYPE_AUDIO;
            break;
        case CallType::VT:
            eMediaContentTypes = MEDIA_TYPE_AUDIOVIDEO;
            break;
        case CallType::RTT:
            eMediaContentTypes = MEDIA_TYPE_AUDIOTEXT;
            break;
        case CallType::VIDEO_RTT:
            eMediaContentTypes = MEDIA_TYPE_AUDIOVIDEOTEXT;
            break;

        default:
            break;
    }

    return eMediaContentTypes;
}

PUBLIC GLOBAL
MEDIA_SERVICE_TYPE MtcMediaUtil::GetMediaServiceType(IN ServiceType eServiceType)
{
    if (eServiceType == ServiceType::EMERGENCY)
    {
        return MEDIA_SERVICE_EMERGENCY;
    }

    return MEDIA_SERVICE_DEFAULT;
}

PUBLIC GLOBAL
MEDIA_NETWORK_TYPE MtcMediaUtil::GetMediaNetworkType(IN IMtcService* piMtcService,
        IN IMS_SINT32 nSlotId)
{
    if (piMtcService->IsWlanIpCanType())
    {
        return MEDIA_NETWORK_WIFI;
    }

    MEDIA_NETWORK_TYPE eMediaNetworkType = MEDIA_NETWORK_NONE;
    IMS_SINT32 eRadioType =
            PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(nSlotId)->GetNetworkType();

    switch (eRadioType)
    {
        case INetworkWatcher::RADIOTECH_TYPE_LTE :
            eMediaNetworkType = MEDIA_NETWORK_LTE;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_HSPAP :
            eMediaNetworkType = MEDIA_NETWORK_HSPA_PLUS;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_UMTS :
        case INetworkWatcher::RADIOTECH_TYPE_HSPA :
        case INetworkWatcher::RADIOTECH_TYPE_HSDPA :
        case INetworkWatcher::RADIOTECH_TYPE_HSUPA :
        case INetworkWatcher::RADIOTECH_TYPE_CDMA :
            eMediaNetworkType = MEDIA_NETWORK_HSPA;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_EHRPD :
            eMediaNetworkType = MEDIA_NETWORK_EHRPD;
            break;
        default :
            eMediaNetworkType = MEDIA_NETWORK_LTE;
            break;
    }

    return eMediaNetworkType;
}

PUBLIC GLOBAL
IMS_SINT32 MtcMediaUtil::GetFailReasonFromReportType(IN IMS_UINT32 eReportType)
{
    IMS_SINT32 eFailReason = FAIL_REASON_MEDIA_UNKNOWN;
    UNUSED_PARAM(eReportType);
    /*switch (eReportType)
    {
        case REPORT_CODEC_ERROR:
            eFailReason = FAIL_REASON_MEDIA_CODEC;
            break;
        case REPORT_INTERNAL_ERROR:
        case REPORT_OPERATION_FAILURE:
            eFailReason = FAIL_REASON_MEDIA_INITFAIL;
            break;
        case REPORT_VIDEO_LOWEST_BIT_RATE:
            eFailReason = FAIL_REASON_MEDIA_LOWEST_BIT_RATE;
            break;
        case REPORT_CHECK_RADIO_CONNECTION:
            eFailReason = FAIL_REASON_MEDIA_CHECK_RADIO_CONNECTION;
            break;
        default:
            break;
    }*/

    return eFailReason;
}

PUBLIC GLOBAL
IMS_SINT32 MtcMediaUtil::GetGttModeFromTextQuality(IN IMS_UINT32 eTextQuality)
{
    IMS_SINT32 eGTTMode = GTT_MODE_INVALID;

    switch (eTextQuality) // This Text Quality is matched with the values defined in TextDef.h.
    {
        case TEXT_QUALITY_NONE:
            eGTTMode = GTT_MODE_INVALID;
            break;
        case TEXT_QUALITY_T140:
        case TEXT_QUALITY_T140_RED:
            eGTTMode = GTT_MODE_FULL;
            break;
        case TEXT_QUALITY_NOTUSED:
            eGTTMode = GTT_MODE_INVALID;
            break;
        default:
            break;
    }

    return eGTTMode;
}
