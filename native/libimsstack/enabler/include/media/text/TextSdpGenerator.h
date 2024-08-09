// Copyright 2024 Google LLC

#ifndef TEXT_SDP_GENERATOR_H_
#define TEXT_SDP_GENERATOR_H_

#include "SdpGenerator.h"

class TextSdpGenerator : public SdpGenerator
{
public:
    TextSdpGenerator();
    virtual ~TextSdpGenerator();

    IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MediaBaseProfile* pBaseProfile) override;
};

#endif
