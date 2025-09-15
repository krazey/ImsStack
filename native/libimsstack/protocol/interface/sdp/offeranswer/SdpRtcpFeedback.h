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
#ifndef SDP_RTCP_FEEDBACK_H_
#define SDP_RTCP_FEEDBACK_H_

#include "offeranswer/SdpMediaFormatParameter.h"

class SdpRtcpFeedback : public SdpMediaFormatParameter
{
public:
    explicit SdpRtcpFeedback(IN IMS_SINT32 nPayloadTypeNumber);
    SdpRtcpFeedback(IN IMS_SINT32 nPayloadTypeNumber, IN const AString& strType,
            IN const AString& strParamName = AString::ConstNull(),
            IN const AString& strParamValue = AString::ConstNull());
    SdpRtcpFeedback(IN const SdpRtcpFeedback& other);
    ~SdpRtcpFeedback() override;

public:
    SdpRtcpFeedback& operator=(IN const SdpRtcpFeedback& other);

public:
    SdpMediaFormatParameter* Clone() const override;
    IMS_BOOL Equals(IN const SdpMediaFormatParameter* pParameter) const override;
    IMS_BOOL SetValue(IN const AString& strValue) override;
    AString ToSdp() const override;

    inline const AString& GetType() const { return m_strType; }
    inline const AString& GetParamName() const { return m_strParamName; }
    inline const AString& GetParamValue() const { return m_strParamValue; }

    inline void SetType(IN const AString& strType) { m_strType = strType; }
    void SetParameter(IN const AString& strName, IN const AString& strValue = AString::ConstNull());

    static SdpRtcpFeedback* Decode(IN const AString& strRtcpFb);

private:
    // Feedback Type - ack / nack / trr-int / 1*(alpha-numeric / "-" / "_")
    AString m_strType;

    // Parameter of each feedback type - name / value
    //    Other feedback type
    //        rtcp-fb-id SP "app" [SP byte-string]
    //        rtcp-fb-id SP token [SP byte-string]
    //        rtcp-fb-id;
    //    "ack"
    //        "ack" SP "rpsi"
    //    "nack"
    //        "nack" SP "pli"
    //        "nack" SP "sli"
    //        "nack" SP "rpsi"
    AString m_strParamName;
    // In this moment, "app" parameter only has the value
    AString m_strParamValue;
};

#endif
