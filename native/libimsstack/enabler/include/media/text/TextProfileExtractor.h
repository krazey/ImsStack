// Copyright 2024 Google LLC

#ifndef TEXT_PROFILE_EXTRACTOR_H_
#define TEXT_PROFILE_EXTRACTOR_H_

#include "ProfileExtractor.h"
#include "text/TextProfileUtil.h"

class TextProfileExtractor : public ProfileExtractor
{
public:
    explicit TextProfileExtractor();
    virtual ~TextProfileExtractor();

    IMS_BOOL Extract(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT TextProfile* pProfile);

private:
    void ExtractPayloads(IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile);
    void ExtractRtpMap(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload,
            OUT AString& strCodecName);
    IMS_BOOL ExtractFmtp(IN const AString& strFmtp, OUT TextProfile::Payload* pPayload,
            IN const ImsList<SdpMediaFormat*>& lstMediaFormat);
    IMS_BOOL ExtractRedFmtp(IN const AString& strFmtp, OUT TextProfile::RedFmtp* pFmtp);
    IMS_BOOL ExtractRedSubPtExist(
            IN const IMS_SINT32 nRedPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat);
};

#endif
