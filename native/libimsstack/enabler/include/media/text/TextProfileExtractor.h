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
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT TextProfile::RedFmtp* pFmtp);
};

#endif
