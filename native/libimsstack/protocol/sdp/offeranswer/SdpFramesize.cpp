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
#include "IMSLib.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "SdpAttribute.h"
#include "offeranswer/SdpFramesize.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpFramesize::SdpFramesize(IN IMS_SINT32 nPayloadTypeNumber) :
        SdpMediaFormatParameter(SdpAttribute::FRAMESIZE, nPayloadTypeNumber),
        m_nWidth(-1),
        m_nHeight(-1),
        m_strOtherFormat(AString::ConstNull())
{
}

PUBLIC
SdpFramesize::SdpFramesize(IN const AString& strOtherFormat) :
        SdpMediaFormatParameter(SdpAttribute::FRAMESIZE, SdpMediaFormatParameter::PT_NOT_SPECIFIED),
        m_nWidth(-1),
        m_nHeight(-1),
        m_strOtherFormat(strOtherFormat)
{
}

PUBLIC
SdpFramesize::SdpFramesize(IN const SdpFramesize& other) :
        SdpMediaFormatParameter(other),
        m_nWidth(other.m_nWidth),
        m_nHeight(other.m_nHeight),
        m_strOtherFormat(other.m_strOtherFormat)
{
}

PUBLIC VIRTUAL SdpFramesize::~SdpFramesize() {}

PUBLIC
SdpFramesize& SdpFramesize::operator=(IN const SdpFramesize& other)
{
    if (this != &other)
    {
        SdpMediaFormatParameter::operator=(other);

        m_nWidth = other.m_nWidth;
        m_nHeight = other.m_nHeight;
        m_strOtherFormat = other.m_strOtherFormat;
    }

    return (*this);
}

PUBLIC VIRTUAL SdpMediaFormatParameter* SdpFramesize::Clone() const
{
    return new SdpFramesize(*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpFramesize::Equals(IN const SdpMediaFormatParameter* pParameter) const
{
    const SdpFramesize* pFramesize = DYNAMIC_CAST(const SdpFramesize*, pParameter);

    if (pFramesize == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!SdpMediaFormatParameter::Equals(pFramesize))
    {
        return IMS_FALSE;
    }

    if (GetPayloadTypeNumber() == SdpMediaFormatParameter::PT_NOT_SPECIFIED)
    {
        if (!m_strOtherFormat.Equals(pFramesize->m_strOtherFormat))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if ((m_nWidth != pFramesize->m_nWidth) || (m_nHeight != pFramesize->m_nHeight))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SdpFramesize::SetValue(IN const AString& strValue)
{
    // The specified value will be the attribute line without the payload type number
    // a=framesize:98 480-640 -> value: 480-640
    // a=framesize:qcif -> value: qcif

    if (IsStandardCompatible(strValue))
    {
        IMSList<AString> objTokens = strValue.Split(TextParser::CHAR_HYPHEN);

        if (objTokens.GetSize() != 2)
        {
            IMS_TRACE_E(0, "Invalid framesize value - %s", strValue.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        IMS_BOOL bOk = IMS_FALSE;
        const AString& strWidth = objTokens.GetAt(0);

        m_nWidth = strWidth.ToInt32(&bOk);

        if (!bOk)
        {
            IMS_TRACE_E(0, "Invalid width value - %s", strWidth.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        bOk = IMS_FALSE;
        const AString& strHeight = objTokens.GetAt(1);

        m_nHeight = strHeight.ToInt32(&bOk);

        if (!bOk)
        {
            IMS_TRACE_E(0, "Invalid height value - %s", strHeight.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        m_strOtherFormat = AString::ConstNull();
    }
    else
    {
        m_nWidth = -1;
        m_nHeight = -1;
        m_strOtherFormat = strValue;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpFramesize::ToSdp() const
{
    // a=framesize:98 480-640
    AStringBuffer objBuffer(64);

    objBuffer.Append(SdpMediaFormatParameter::ToSdp());

    if (objBuffer.GetLength() == 0)
    {
        // Invalid media format parameter
        return AString::ConstNull();
    }

    if (GetPayloadTypeNumber() == SdpMediaFormatParameter::PT_NOT_SPECIFIED)
    {
        objBuffer.Append(m_strOtherFormat);
    }
    else
    {
        objBuffer.Append(TextParser::CHAR_SP);
        objBuffer.Append(m_nWidth);
        objBuffer.Append(TextParser::CHAR_HYPHEN);
        objBuffer.Append(m_nHeight);
    }

    objBuffer.Append(TextParser::CHAR_CR);
    objBuffer.Append(TextParser::CHAR_LF);

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

PUBLIC
void SdpFramesize::SetParameter(IN IMS_SINT32 nWidth, IN IMS_SINT32 nHeight)
{
    m_nWidth = nWidth;
    m_nHeight = nHeight;
}

PUBLIC GLOBAL SdpFramesize* SdpFramesize::Decode(IN const AString& strFramesize)
{
    // The specified value will be the attribute line without the attribute name
    // a=framesize:98 480-640 -> value: 98 480-640
    // a=framesize:qcif -> value: qcif

    if (strFramesize.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Invalid framesize - empty string", 0, 0, 0);
        return IMS_NULL;
    }

    IMS_SINT32 nIndexOfSP = strFramesize.GetIndexOf(TextParser::CHAR_SP);
    SdpFramesize* pFramesize = IMS_NULL;

    if (nIndexOfSP == AString::NPOS)
    {
        pFramesize = new SdpFramesize(strFramesize);
    }
    else
    {
        AString strPayloadType = strFramesize.GetSubStr(0, nIndexOfSP);
        IMS_SINT32 nPayloadType = strPayloadType.ToInt32();

        pFramesize = new SdpFramesize(nPayloadType);

        if (pFramesize != IMS_NULL)
        {
            if (!pFramesize->SetValue(strFramesize.GetSubStr(nIndexOfSP + 1)))
            {
                delete pFramesize;
                return IMS_NULL;
            }
        }
    }

    return pFramesize;
}

PUBLIC GLOBAL IMS_BOOL SdpFramesize::IsStandardCompatible(IN const AString& strFramesize)
{
    if (strFramesize.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < strFramesize.GetLength(); ++i)
    {
        const IMS_CHAR ch = strFramesize[i];

        if (!IMS_ISDIGIT(ch) && (ch != TextParser::CHAR_HYPHEN))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}
