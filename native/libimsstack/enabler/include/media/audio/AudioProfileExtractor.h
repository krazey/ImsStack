// Copyright 2024 Google LLC

#ifndef AUDIO_PROFILE_EXTRACTOR_H_
#define AUDIO_PROFILE_EXTRACTOR_H_

#include "ProfileExtractor.h"
#include "audio/AudioProfileUtil.h"

class AudioProfileExtractor : public ProfileExtractor
{
public:
    explicit AudioProfileExtractor();
    virtual ~AudioProfileExtractor();

    IMS_BOOL Extract(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT AudioProfile* pProfile);

private:
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT AudioProfile::AmrFmtp* pFmtp);
};

#endif
