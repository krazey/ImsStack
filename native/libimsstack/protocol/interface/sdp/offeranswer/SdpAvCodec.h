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
#ifndef SDP_AV_CODEC_H_
#define SDP_AV_CODEC_H_

#include "offeranswer/SdpMediaFormat.h"

class SdpAvCodec : public SdpMediaFormat
{
public:
    SdpAvCodec();
    SdpAvCodec(IN const SdpAvCodec& other);
    ~SdpAvCodec() override;

public:
    SdpAvCodec& operator=(IN const SdpAvCodec& other);

public:
    // SdpMediaFormat class
    IMS_BOOL Equals(IN const SdpMediaFormat* pFormat) const override;
    IMS_BOOL HasAttribute() const override;
    IMS_BOOL SetParameters(IN const AString& strAttrAnyMap, IN const AString& strAttrFmtp) override;
    IMS_BOOL SetValue(IN const AString& strFormat) override;
    AString ToSdp() const override;

    /**
     * @brief Returns the payload type as an integer value.
     *
     * @return The payload type of this codec.
     */
    inline IMS_SINT32 GetPayloadType() const { return m_nPayloadType; }

    /**
     * @brief Returns the codec name.
     *        For example, H263-2000, MP4, AMR, ...
     *
     * @return The codec name of this codec.
     */
    inline const AString& GetName() const { return m_strCodecName; }

    /**
     * @brief Returns the encoding rate for this media format.
     *
     * @return The clock rate of this codec.
     */
    inline IMS_UINT32 GetClockRate() const { return m_nClockRate; }

    /**
     * @brief Returns the encoding parameters for this media format.
     *
     * @return The encoding parameters of this codec.
     */
    inline const AString& GetEncodingParameters() const { return m_strEncodingParameters; }

    /**
     * @brief Returns the format specific parameter for this media format.
     *        It is from "a=fmtp:" attribute field.
     *
     * @return The format specific parameters of this codec.
     */
    inline const AString& GetFormatSpecificParameter() const { return m_strFmtp; }

    /**
     * @brief Returns the default RTPMAP of the static payload type.
     */
    static IMS_BOOL GetDefaultRtpmap(IN IMS_SINT32 nType, IN_OUT AString& strRtpmap);
    /**
     * @brief Returns the operation mode (0: bandwidth-efficient, 1: octet-aligned)
     *        of AMR/AMR-WB codec.
     */
    static IMS_SINT32 GetAmrOperationMode(IN const AString& strFmtp);
    /**
     * @brief Checks if the specified payload type is a dynamic or not.
     */
    static IMS_BOOL IsDynamicPayloadType(IN IMS_SINT32 nType);

public:
    // Static payload type for RTP profile
    enum
    {
        PT_INVALID = (-1),
        PCMU = 0,       // Audio, PCMU/8000/1
        RESERVED_1,     // Audio
        RESERVED_2,     // Audio
        GSM,            // Audio, GSM/8000/1
        G723,           // Audio, G723/8000/1
        DVI4,           // Audio, DVI4/8000/1
        DVI4_16,        // Audio, DVI4/16000/1
        LPC,            // Audio, LPC/8000/1
        PCMA,           // Audio, PCMA/8000/1
        G722,           // Audio, G722/8000/1
        L16_2,          // Audio, L16/44100/2
        L16,            // Audio, L16/44100/1
        QCELP,          // Audio, QCELP/8000/1
        CN,             // Audio, CN/8000/1
        MPA,            // Audio, MPA/90000
        G728,           // Audio, G728/8000/1
        DVI4_11,        // Audio, DVI4/11025/1
        DVI4_22,        // Audio, DVI4/22050/1
        G729,           // Audio, G729/8000/1
        RESERVED_19,    // Audio
        UNASSIGNED_20,  // Audio
        UNASSIGNED_21,  // Audio
        UNASSIGNED_22,  // Audio
        UNASSIGNED_23,  // Audio
        UNASSIGNED_24,  // Video
        CELB,           // Video, CELB/90000
        JPEG,           // Video, JPEG/90000
        UNASSIGNED_27,  // Video
        NV,             // Video, NV/90000
        UNASSIGNED_29,  // Video
        UNASSIGNED_30,  // Video
        H261,           // Video, H261/90000
        MPV,            // Video, MPV/90000
        MP2T,           // Audieo/Video, MP2T/90000
        H263,           // Video, H263/90000
        PT_DYNAMIC_35 = 35,
        PT_DYNAMIC_71 = 71,
        PT_DYNAMIC_72 = 72,
        PT_DYNAMIC_76 = 76,
        PT_DYNAMIC_77 = 77,
        PT_DYNAMIC_95 = 95,
        PT_DYNAMIC_96 = 96,
        PT_DYNAMIC_127 = 127,
        PT_MAX

        // 35 ~ 71 : Unassigned, Media type ( ? )
        // 72 ~ 76 : Reserved, Media type ( N/A )
        // 77 ~ 95 : Unassigned, Media type ( ? )
        // 96 ~ 127 : Dynamic, Media type ( ? )

        ////// Dynamic Payload Types
        //
        //// Audio Profile
        // G726-40, 8000, 1
        // G726-32, 8000, 1
        // G726-24, 8000, 1
        // G726-16, 8000, 1
        // G729D, 8000, 1
        // G729E, 8000, 1
        // GSM-EFR, 8000, 1
        // L8, var., var.
        // RED,
        // VDVI, var., 1
        //
        //// Video Profile
        // H263-1998, 90000
        //
        //////
    };

    // For static codec table
    struct AvCodecTable
    {
        IMS_SINT32 nPayloadType;
        const IMS_CHAR* pszRtpmap;
    };

private:
    static const AvCodecTable CODEC_TABLE[];

    // a=rtpmap:<payload type> <encoding name>/<clock rate>[/<encoding parameters>]
    IMS_SINT32 m_nPayloadType;  // Payload type number
    AString m_strCodecName;     // Codec name (e.g. AMR, EVRC, H263-2000, MP4, ...)
    IMS_UINT32 m_nClockRate;
    AString m_strEncodingParameters;

    // a=fmtp:<format> <format specific parameter>
    //  ---> a=fmtp:<PT> <codec specific information>
    AString m_strFmtp;  // Value only (except for attribute & PT)
    // For AMR/AMR-WB codec (0: bandwidth-efficient, 1: octet-aligned)
    IMS_SINT32 m_nAmrOperationMode;
};

#endif
