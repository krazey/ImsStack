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
#ifndef SDP_MEDIA_FORMAT_PARAMETER_H_
#define SDP_MEDIA_FORMAT_PARAMETER_H_

class AString;

class SdpMediaFormatParameter
{
public:
    SdpMediaFormatParameter(IN IMS_SINT32 nAttribute, IN IMS_SINT32 nPayloadTypeNumber);
    SdpMediaFormatParameter(IN const SdpMediaFormatParameter& other);
    virtual ~SdpMediaFormatParameter();

public:
    SdpMediaFormatParameter& operator=(IN const SdpMediaFormatParameter& other);

public:
    virtual SdpMediaFormatParameter* Clone() const;
    virtual IMS_BOOL Equals(IN const SdpMediaFormatParameter* pParameter) const;
    virtual IMS_BOOL SetValue(IN const AString& strValue);
    virtual AString ToSdp() const;

    inline IMS_SINT32 GetAttribute() const { return m_nAttribute; }
    inline IMS_SINT32 GetPayloadTypeNumber() const { return m_nPayloadTypeNumber; }

public:
    enum
    {
        // "*"
        PT_WILDCARD = 0xFFFF,
        // no payload type number dependency
        PT_NOT_SPECIFIED = 0xFFFFFF
    };

private:
    // SDP attribute name : rtcp-fb, framesize, ...
    IMS_SINT32 m_nAttribute;

    // RTP payload type number - wildcard(*) will be defined as 0xFFFF
    IMS_SINT32 m_nPayloadTypeNumber;
};

#endif
