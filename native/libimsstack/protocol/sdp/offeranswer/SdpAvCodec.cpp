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
#include "AStringBuffer.h"
#include "TextParser.h"

#include "Sdp.h"
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpMediaFormatParameter.h"

GLOBAL PRIVATE const SdpAvCodec::AvCodecTable SdpAvCodec::CODEC_TABLE[] = {
        {// Audio, PCMU/8000/1
                SdpAvCodec::PCMU,    "PCMU/8000/1" },
        {// Audio, GSM/8000/1
                SdpAvCodec::GSM,     "GSM/8000/1"  },
        {// Audio, G723/8000/1
                SdpAvCodec::G723,    "G723/8000/1" },
        {// Audio, DVI4/8000/1
                SdpAvCodec::DVI4,    "DVI4/8000/1" },
        {// Audio, DVI4/16000/1
                SdpAvCodec::DVI4_16, "DVI4/16000/1"},
        {// Audio, LPC/8000/1
                SdpAvCodec::LPC,     "LPC/8000/1"  },
        {// Audio, PCMA/8000/1
                SdpAvCodec::PCMA,    "PCMA/8000/1" },
        {// Audio, G722/8000/1
                SdpAvCodec::G722,    "G722/8000/1" },
        {// Audio, L16/44100/2
                SdpAvCodec::L16_2,   "L16/44100/2" },
        {// Audio, L16/44100/1
                SdpAvCodec::L16,     "L16/44100/1" },
        {// Audio, QCELP/8000/1
                SdpAvCodec::QCELP,   "QCELP/8000/1"},
        {// Audio, CN/8000/1
                SdpAvCodec::CN,      "CN/8000/1"   },
        {// Audio, MPA/90000
                SdpAvCodec::MPA,     "MPA/90000"   },
        {// Audio, G728/8000/1
                SdpAvCodec::G728,    "G728/8000/1" },
        {// Audio, DVI4/11025/1
                SdpAvCodec::DVI4_11, "DVI4/11025/1"},
        {// Audio, DVI4/22050/1
                SdpAvCodec::DVI4_22, "DVI4/22050/1"},
        {// Audio, G729/8000/1
                SdpAvCodec::G729,    "G729/8000/1" },
        {// Video, CELB/90000
                SdpAvCodec::CELB,    "CELB/90000"  },
        {// Video, JPEG/90000
                SdpAvCodec::JPEG,    "JPEG/90000"  },
        {// Video, NV/90000
                SdpAvCodec::NV,      "NV/90000"    },
        {// Video, H261/90000
                SdpAvCodec::H261,    "H261/90000"  },
        {// Video, MPV/90000
                SdpAvCodec::MPV,     "MPV/90000"   },
        {// Audieo/Video, MP2T/90000
                SdpAvCodec::MP2T,    "MP2T/90000"  },
        {// Video, H263/90000
                SdpAvCodec::H263,    "H263/90000"  },
};

PUBLIC
SdpAvCodec::SdpAvCodec() :
        SdpMediaFormat(SdpMediaFormat::TYPE_RTP),
        m_nPayloadType(PT_INVALID),
        m_strCodecName(AString::ConstNull()),
        m_nClockRate(0),
        m_strEncodingParameters(AString::ConstNull()),
        m_strFmtp(AString::ConstNull()),
        m_nAmrOperationMode(0)
{
}

PUBLIC
SdpAvCodec::SdpAvCodec(IN const SdpAvCodec& other) :
        SdpMediaFormat(other),
        m_nPayloadType(other.m_nPayloadType),
        m_strCodecName(other.m_strCodecName),
        m_nClockRate(other.m_nClockRate),
        m_strEncodingParameters(other.m_strEncodingParameters),
        m_strFmtp(other.m_strFmtp),
        m_nAmrOperationMode(other.m_nAmrOperationMode)
{
}

PUBLIC VIRTUAL SdpAvCodec::~SdpAvCodec() {}

PUBLIC
SdpAvCodec& SdpAvCodec::operator=(IN const SdpAvCodec& other)
{
    if (this != &other)
    {
        SdpMediaFormat::operator=(other);

        m_nPayloadType = other.m_nPayloadType;
        m_strCodecName = other.m_strCodecName;
        m_nClockRate = other.m_nClockRate;
        m_strEncodingParameters = other.m_strEncodingParameters;
        m_strFmtp = other.m_strFmtp;
        m_nAmrOperationMode = other.m_nAmrOperationMode;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpAvCodec::Equals(IN const SdpMediaFormat* pFormat) const
{
    if (pFormat == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (GetType() != pFormat->GetType())
    {
        return IMS_FALSE;
    }

    const SdpAvCodec* pCodec = DYNAMIC_CAST(const SdpAvCodec*, pFormat);

    if (pCodec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Check the codec name
    if (!IsDynamicPayloadType(m_nPayloadType) && (m_nPayloadType != pCodec->m_nPayloadType))
    {
        return IMS_FALSE;
    }

    if (IsDynamicPayloadType(m_nPayloadType) &&
            (!m_strCodecName.EqualsIgnoreCase(pCodec->m_strCodecName)))
    {
        return IMS_FALSE;
    }

    // Check clock rate
    if (m_nClockRate != pCodec->m_nClockRate)
    {
        return IMS_FALSE;
    }

    // For AMR/AMR-WB
    if (m_strCodecName.StartsWith('A') &&
            (m_strCodecName.Equals("AMR") || m_strCodecName.Equals("AMR-WB")))
    {
        if (m_nAmrOperationMode != pCodec->m_nAmrOperationMode)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SdpAvCodec::HasAttribute() const
{
    if (m_nPayloadType == PT_INVALID)
    {
        return IMS_FALSE;
    }

    if (m_strCodecName.IsNULL())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SdpAvCodec::SetParameters(
        IN const AString& strAttrAnyMap, IN const AString& strAttrFmtp)
{
    IMS_BOOL bOk = Sdp::ParseAttributeRtpmap(
            strAttrAnyMap, m_nPayloadType, m_strCodecName, m_nClockRate, m_strEncodingParameters);

    if (!bOk)
    {
        return IMS_FALSE;
    }

    if (!strAttrFmtp.IsNULL())
    {
        IMS_SINT32 nParsedPayloadType = PT_MAX;

        bOk = Sdp::ParseAttributeFmtp(strAttrFmtp, nParsedPayloadType, m_strFmtp);

        if (!bOk)
        {
            m_strFmtp = AString::ConstNull();
            return IMS_FALSE;
        }

        if (m_nPayloadType != nParsedPayloadType)
        {
            m_strFmtp = AString::ConstNull();
            return IMS_FALSE;
        }

        // For AMR/AMR-WB
        if (m_strCodecName.StartsWith('A') &&
                (m_strCodecName.Equals("AMR") || m_strCodecName.Equals("AMR-WB")))
        {
            m_nAmrOperationMode = GetAmrOperationMode(m_strFmtp);
        }
    }
    else
    {
        m_strFmtp = AString::ConstNull();
    }

    AString strFormat;
    strFormat.SetNumber(m_nPayloadType);

    SdpMediaFormat::SetValue(strFormat);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SdpAvCodec::SetValue(IN const AString& strFormat)
{
    if (!SdpMediaFormat::SetValue(strFormat))
    {
        return IMS_FALSE;
    }

    IMS_BOOL bOk = IMS_FALSE;
    m_nPayloadType = strFormat.ToInt32(&bOk);

    if (!bOk)
    {
        m_nPayloadType = PT_INVALID;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpAvCodec::ToSdp() const
{
    AStringBuffer objBuffer(256);
    const IMS_CHAR cLineA = Sdp::LINE_A;

    // RTPMAP
    objBuffer.Append(cLineA);
    objBuffer.Append(TextParser::CHAR_EQUAL);
    objBuffer.Append("rtpmap:");

    AString strTmp;
    objBuffer.Append(strTmp.SetNumber(m_nPayloadType));
    objBuffer.Append(TextParser::CHAR_SP);

    objBuffer.Append(m_strCodecName);
    objBuffer.Append(TextParser::CHAR_SLASH);
    objBuffer.Append(strTmp.SetNumber(m_nClockRate));

    if (m_strEncodingParameters.GetLength() > 0)
    {
        objBuffer.Append(TextParser::CHAR_SLASH);
        objBuffer.Append(m_strEncodingParameters);
    }

    objBuffer.Append(TextParser::CHAR_CR);
    objBuffer.Append(TextParser::CHAR_LF);

    // FMTP
    if (!m_strFmtp.IsNULL())
    {
        objBuffer.Append(cLineA);
        objBuffer.Append(TextParser::CHAR_EQUAL);
        objBuffer.Append("fmtp:");
        objBuffer.Append(strTmp.SetNumber(m_nPayloadType));
        objBuffer.Append(TextParser::CHAR_SP);
        objBuffer.Append(m_strFmtp);
        objBuffer.Append(TextParser::CHAR_CR);
        objBuffer.Append(TextParser::CHAR_LF);
    }

    // Extra parameters : rtcp-fb / framesize
    const ImsList<SdpMediaFormatParameter*>& objExtraParameters = GetExtraParameters();

    for (IMS_UINT32 i = 0; i < objExtraParameters.GetSize(); ++i)
    {
        const SdpMediaFormatParameter* pParameter = objExtraParameters.GetAt(i);

        if (pParameter == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 nPayloadTypeNumber = pParameter->GetPayloadTypeNumber();

        if ((nPayloadTypeNumber == SdpMediaFormatParameter::PT_WILDCARD) ||
                (nPayloadTypeNumber == SdpMediaFormatParameter::PT_NOT_SPECIFIED))
        {
            // Wildcard or not-specified payload parameter will be added later
            continue;
        }

        objBuffer.Append(pParameter->ToSdp());
    }

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

PUBLIC GLOBAL IMS_BOOL SdpAvCodec::GetDefaultRtpmap(IN IMS_SINT32 nType, IN_OUT AString& strRtpmap)
{
    IMS_UINT32 nCount = sizeof(CODEC_TABLE) / sizeof(CODEC_TABLE[0]);

    for (IMS_UINT32 i = 0; i < nCount; ++i)
    {
        const AvCodecTable* pTable = &CODEC_TABLE[i];

        if (pTable == IMS_NULL)
        {
            continue;
        }

        if (pTable->nPayloadType == nType)
        {
            strRtpmap.Sprintf("%d %s", pTable->nPayloadType, pTable->pszRtpmap);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_SINT32 SdpAvCodec::GetAmrOperationMode(IN const AString& strFmtp)
{
    ImsList<AString> objTokens = strFmtp.Split(TextParser::CHAR_SEMICOLON);

    if (objTokens.IsEmpty())
    {
        return 0;
    }

    AString strName;
    AString strValue;

    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
    {
        const AString& strToken = objTokens.GetAt(i);

        if (strToken.SplitF(TextParser::CHAR_EQUAL, strName, strValue) == 2)
        {
            if (strName.EqualsIgnoreCase("octet-align"))
            {
                if (strValue.Equals('1'))
                {
                    return 1;
                }
                break;
            }
        }
    }

    return 0;
}

PUBLIC GLOBAL IMS_BOOL SdpAvCodec::IsDynamicPayloadType(IN IMS_SINT32 nType)
{
    if ((nType >= PT_DYNAMIC_96) && (nType <= PT_DYNAMIC_127))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
