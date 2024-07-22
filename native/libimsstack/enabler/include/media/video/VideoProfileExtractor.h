// Copyright 2024 Google LLC

#ifndef VIDEO_PROFILE_EXTRACTOR_H_
#define VIDEO_PROFILE_EXTRACTOR_H_

#include "ProfileExtractor.h"
#include "video/VideoProfileUtil.h"

class VideoProfileExtractor : public ProfileExtractor
{
public:
    explicit VideoProfileExtractor();
    virtual ~VideoProfileExtractor();

    IMS_BOOL Extract(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT VideoProfile* pProfile);

private:
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT VideoProfile::AvcFmtp* pFmtp);
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT VideoProfile::HevcFmtp* pFmtp);
    IMS_BOOL CheckAvpfFromProfile(IN VideoProfile* pProfile);
    IMS_BOOL GetCorrectImageIndex(IN IMS_SINT32 nPayloadTypeNum, IN ImsList<AString> objAttributes,
            OUT IMS_UINT32* nIndex);
    VIDEO_RESOLUTION GetResolutionFromSdp(IN VIDEO_CODEC codecType,
            IN const AString& strImageAttrFromSdp, IN const AString& strFrameSizeFromSdp,
            IN const AString& strSpropParam, IN IMS_SINT32 nQcif = -1);
    IMS_BOOL GetAvpfFromAttributes(IN SdpMediaFormat* pMediaFormat,
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetAvpfFromAttributes_EX(
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetWidthHeightFromSdp_ImageAttr(IN const AString& strImageAttrFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    VIDEO_RESOLUTION GetResolutionFromWidthHeight(IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight);
    IMS_BOOL GetWidthHeightFromSdp_FrameSize(IN AString strFrameSizeFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
};

#endif
