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
    IMS_BOOL MakeCapaNegoProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL MakeAcfgProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL MakeTcapProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL MakeAcapProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL MakePcfgProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);

private:
    MEDIA_CONTENT_TYPE m_eType;
};

#endif
