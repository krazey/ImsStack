// Copyright 2024 Google LLC

#ifndef AUDIO_SDP_GENERATOR_H_
#define AUDIO_SDP_GENERATOR_H_

#include "SdpGenerator.h"

class AudioProfile;

class AudioSdpGenerator : public SdpGenerator
{
public:
    AudioSdpGenerator();
    virtual ~AudioSdpGenerator();

    IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MediaBaseProfile* pBaseProfile) override;

private:
    void SetSdpSessionIpAddress(
            OUT ISessionDescriptor* pSessionDescriptor, IN AudioProfile* pProfile);
    void SetSdpMediaDescription(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void SetSdpMediaBandwidth(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
};

#endif
