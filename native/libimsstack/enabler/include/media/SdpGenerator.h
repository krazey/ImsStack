// Copyright 2024 Google LLC

#ifndef SDP_GENERATOR_H_
#define SDP_GENERATOR_H_

#include "ISessionDescriptor.h"
#include "media/IMediaDescriptor.h"

#include "MediaBaseProfile.h"

class SdpGenerator
{
public:
    explicit SdpGenerator(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~SdpGenerator();

    virtual IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile) = 0;

protected:
    void GenerateCommonAttributes(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile);

private:
    void ClearAttributeAndBandwidth(OUT IMediaDescriptor* pDescriptor);
    void SetSdpSessionIpAddress(
            OUT ISessionDescriptor* pSessionDescriptor, IN MediaBaseProfile* pProfile);
    void SetSdpMediaDescription(OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile);
    void SetSdpMediaBandwidth(OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile);

protected:
    MEDIA_CONTENT_TYPE m_eType;
};

#endif
