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
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "SdpAttribute.h"
#include "offeranswer/SdpRtcpFeedback.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpRtcpFeedback::SdpRtcpFeedback(IN IMS_SINT32 nPayloadTypeNumber) :
        SdpMediaFormatParameter(SdpAttribute::RTCP_FB, nPayloadTypeNumber),
        m_strType(AString::ConstNull()),
        m_strParamName(AString::ConstNull()),
        m_strParamValue(AString::ConstNull())
{
}

PUBLIC
SdpRtcpFeedback::SdpRtcpFeedback(IN IMS_SINT32 nPayloadTypeNumber, IN const AString& strType,
        IN const AString& strParamName /*= AString::ConstNull()*/,
        IN const AString& strParamValue /*= AString::ConstNull()*/) :
        SdpMediaFormatParameter(SdpAttribute::RTCP_FB, nPayloadTypeNumber),
        m_strType(strType),
        m_strParamName(strParamName),
        m_strParamValue(strParamValue)
{
}

PUBLIC
SdpRtcpFeedback::SdpRtcpFeedback(IN const SdpRtcpFeedback& other) :
        SdpMediaFormatParameter(other),
        m_strType(other.m_strType),
        m_strParamName(other.m_strParamName),
        m_strParamValue(other.m_strParamValue)
{
}

PUBLIC
SdpRtcpFeedback::~SdpRtcpFeedback() {}

PUBLIC
SdpRtcpFeedback& SdpRtcpFeedback::operator=(IN const SdpRtcpFeedback& other)
{
    if (this != &other)
    {
        SdpMediaFormatParameter::operator=(other);

        m_strType = other.m_strType;
        m_strParamName = other.m_strParamName;
        m_strParamValue = other.m_strParamValue;
    }

    return (*this);
}

PUBLIC VIRTUAL SdpMediaFormatParameter* SdpRtcpFeedback::Clone() const
{
    return new SdpRtcpFeedback(*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpRtcpFeedback::Equals(IN const SdpMediaFormatParameter* pParameter) const
{
    const SdpRtcpFeedback* pRtcpFb = DYNAMIC_CAST(const SdpRtcpFeedback*, pParameter);

    if (pRtcpFb == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!SdpMediaFormatParameter::Equals(pRtcpFb))
    {
        return IMS_FALSE;
    }

    if (!m_strType.Equals(pRtcpFb->m_strType))
    {
        return IMS_FALSE;
    }

    if (!m_strParamName.Equals(pRtcpFb->m_strParamName))
    {
        return IMS_FALSE;
    }

    if (!m_strParamValue.Equals(pRtcpFb->m_strParamValue))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SdpRtcpFeedback::SetValue(IN const AString& strValue)
{
    // The specified value will be the attribute line without the payload type number
    // a=rtcp-fb:98 nack sli -> value: nack sli
    ImsList<AString> objTokens = strValue.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 1)
    {
        IMS_TRACE_E(0, "Invalid rtcp-fb value format - %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_strType = objTokens.GetAt(0);

    if (objTokens.GetSize() > 3)
    {
        m_strParamName = objTokens.GetAt(1);
        m_strParamValue = AString::ConstNull();

        for (IMS_UINT32 i = 2; i < objTokens.GetSize(); ++i)
        {
            if (i != 2)
            {
                m_strParamValue.Append(TextParser::CHAR_SP);
            }

            m_strParamValue.Append(objTokens.GetAt(i));
        }
    }
    else if (objTokens.GetSize() > 2)
    {
        m_strParamName = objTokens.GetAt(1);
        m_strParamValue = objTokens.GetAt(2);
    }
    else if (objTokens.GetSize() > 1)
    {
        m_strParamName = objTokens.GetAt(1);
        m_strParamValue = AString::ConstNull();
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpRtcpFeedback::ToSdp() const
{
    // a=rtcp-fb:98 nack pli
    AStringBuffer objBuffer(64);

    objBuffer.Append(SdpMediaFormatParameter::ToSdp());

    if (objBuffer.GetLength() == 0)
    {
        // Invalid media format parameter
        return AString::ConstNull();
    }

    // type
    objBuffer.Append(TextParser::CHAR_SP);
    objBuffer.Append(m_strType);

    // parameter (name / value)
    if (m_strParamName.GetLength() > 0)
    {
        objBuffer.Append(TextParser::CHAR_SP);
        objBuffer.Append(m_strParamName);

        if (m_strParamValue.GetLength() > 0)
        {
            objBuffer.Append(TextParser::CHAR_SP);
            objBuffer.Append(m_strParamValue);
        }
    }

    objBuffer.Append(TextParser::CHAR_CR);
    objBuffer.Append(TextParser::CHAR_LF);

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

PUBLIC
void SdpRtcpFeedback::SetParameter(
        IN const AString& strName, IN const AString& strValue /*= AString::ConstNull()*/)
{
    m_strParamName = strName;
    m_strParamValue = strValue;
}

PUBLIC GLOBAL SdpRtcpFeedback* SdpRtcpFeedback::Decode(IN const AString& strRtcpFb)
{
    // The specified value will be the attribute line without the attribute name
    // a=rtcp-fb:98 nack sli -> value: 98 nack sli
    ImsList<AString> objTokens = strRtcpFb.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 2)
    {
        // Invalid rtcp-fb format
        IMS_TRACE_E(0, "Invalid rtcp-fb format - %s", strRtcpFb.GetStr(), 0, 0);
        return IMS_NULL;
    }

    SdpRtcpFeedback* pRtcpFb = IMS_NULL;
    const AString& strPayloadType = objTokens.GetAt(0);

    if (strPayloadType.Equals(TextParser::CHAR_ASTERISK))
    {
        pRtcpFb = new SdpRtcpFeedback(SdpMediaFormatParameter::PT_WILDCARD, objTokens.GetAt(1));
    }
    else
    {
        IMS_BOOL bOk = IMS_FALSE;
        IMS_SINT32 nPayloadType = strPayloadType.ToInt32(&bOk);

        if (!bOk)
        {
            IMS_TRACE_E(0, "Invalid payload type number - %s", strPayloadType.GetStr(), 0, 0);
            return IMS_NULL;
        }

        pRtcpFb = new SdpRtcpFeedback(nPayloadType, objTokens.GetAt(1));
    }

    if (pRtcpFb == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (objTokens.GetSize() > 4)
    {
        AString strParameterValue = AString::ConstNull();

        for (IMS_UINT32 i = 3; i < objTokens.GetSize(); ++i)
        {
            if (i != 3)
            {
                strParameterValue.Append(TextParser::CHAR_SP);
            }

            strParameterValue.Append(objTokens.GetAt(i));
        }

        pRtcpFb->SetParameter(objTokens.GetAt(2), strParameterValue);
    }
    else if (objTokens.GetSize() > 3)
    {
        pRtcpFb->SetParameter(objTokens.GetAt(2), objTokens.GetAt(3));
    }
    else if (objTokens.GetSize() > 2)
    {
        pRtcpFb->SetParameter(objTokens.GetAt(2));
    }

    return pRtcpFb;
}
