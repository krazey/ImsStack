/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SDP_FRAMESIZE_H_
#define SDP_FRAMESIZE_H_

#include "offeranswer/SdpMediaFormatParameter.h"

class SdpFramesize : public SdpMediaFormatParameter
{
public:
    explicit SdpFramesize(IN IMS_SINT32 nPayloadTypeNumber);
    explicit SdpFramesize(IN const AString& strOtherFormat);
    SdpFramesize(IN const SdpFramesize& other);
    ~SdpFramesize() override;

public:
    SdpFramesize& operator=(IN const SdpFramesize& other);

public:
    SdpMediaFormatParameter* Clone() const override;
    IMS_BOOL Equals(IN const SdpMediaFormatParameter* pParameter) const override;
    IMS_BOOL SetValue(IN const AString& strValue) override;
    AString ToSdp() const override;

    inline IMS_SINT32 GetHeight() const { return m_nHeight; }
    inline IMS_SINT32 GetWidth() const { return m_nWidth; }
    inline const AString& GetOtherFormat() const { return m_strOtherFormat; }

    void SetParameter(IN IMS_SINT32 nWidth, IN IMS_SINT32 nHeight);
    inline void SetParameter(IN const AString& strOtherFormat)
    {
        m_strOtherFormat = strOtherFormat;
    }

    static SdpFramesize* Decode(IN const AString& strFramesize);
    static IMS_BOOL IsStandardCompatible(IN const AString& strFramesize);

private:
    // a=framesize:<payload type number> SP <width>-<height>
    IMS_SINT32 m_nWidth;
    IMS_SINT32 m_nHeight;

    // For backward compatibility : a=framesize:qcif / a=framesize:vga / ...
    AString m_strOtherFormat;
};

#endif
