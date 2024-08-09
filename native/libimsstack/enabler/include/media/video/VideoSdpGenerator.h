// Copyright 2024 Google LLC

#ifndef VIDEO_SDP_GENERATOR_H_
#define VIDEO_SDP_GENERATOR_H_

#include "SdpGenerator.h"

class VideoSdpGenerator : public SdpGenerator
{
public:
    VideoSdpGenerator();
    virtual ~VideoSdpGenerator();

    IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MediaBaseProfile* pBaseProfile) override;

private:
    IMS_BOOL MakeImageAttributeLine(IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId,
            OUT AString& strImageAttr);
    IMS_BOOL MakeFrameSizeLine(IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId,
            OUT AString& strFrameSize);
    /**
     * @brief Get the width and height from video resolution enum id
     *
     * @param eResolutionId The enum of video resolution set
     * @param pnWidth The width of video resolution
     * @param pnHeight The height of video resolution
     * @return IMS_BOOL
     */
    IMS_BOOL GetWidthHeightFromResolutionId(
            IN VIDEO_RESOLUTION eResolutionId, OUT IMS_UINT32* pnWidth, OUT IMS_UINT32* pnHeight);
};

#endif
