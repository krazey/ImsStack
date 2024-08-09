// Copyright 2024 Google LLC

#ifndef PROFILE_EXTRACTOR_H_
#define PROFILE_EXTRACTOR_H_

#include "ISessionDescriptor.h"
#include "media/IMediaDescriptor.h"
#include "offeranswer/SdpAvCodec.h"

#include "MediaBaseProfile.h"

class ProfileExtractor
{
public:
    explicit ProfileExtractor(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~ProfileExtractor();

protected:
    void Extract(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT MediaBaseProfile* pProfile);
    IMS_BOOL ExtractCapaNego(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ExtractAcfg(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ExtractTcap(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ExtractAcap(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ExtractPcfg(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);

private:
    MEDIA_CONTENT_TYPE m_eType;
};

#endif
