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

private:
    void CheckRedPayloadSubTypeValidity(OUT TextProfile* pProfile);
    void GeneratePayload(OUT IMediaDescriptor* pDescriptor, IN TextProfile* pProfile);
    IMS_BOOL GenerateFmtp(OUT AString& strFmtp, IN TextProfile::Payload* pPayload);
};

#endif
