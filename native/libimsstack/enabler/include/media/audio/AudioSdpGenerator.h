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
    void GeneratePayload(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    IMS_BOOL GenerateFmtp(OUT AString& strFmtp, IN AudioProfile::Payload* pPayload);
    void GeneratePtime(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateMaxPtime(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateCandidateAttribute(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateRtcpXr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateAnbr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
};

#endif
