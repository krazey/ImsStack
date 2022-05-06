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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "Sdp.h"
#include "SdpParser.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpParser::SdpParser() {}

PUBLIC
SdpParser::~SdpParser() {}

PUBLIC
IMS_BOOL SdpParser::Decode(IN const AString& strSdp)
{
    AStringArray objSdpLines;
    IMS_SINT32 nStartLine = 0;
    IMS_SINT32 nEndLine = -1;

    SplitLines(strSdp, objSdpLines);

    IMSList<IMS_SINT32> objMediaLines = GetMediaIndexes(objSdpLines);

    if (objMediaLines.IsEmpty())
    {
        nEndLine = objSdpLines.GetCount();
    }
    else
    {
        nEndLine = objMediaLines.GetAt(0);
    }

    if (!m_objSessionDescription.Decode(objSdpLines, nStartLine, nEndLine))
    {
        IMS_TRACE_E(0, "Decoding a session-level description failed", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objMediaLines.GetSize(); ++i)
    {
        SdpMediaDescription objMediaDescription;

        nStartLine = nEndLine;

        if ((i + 1) < objMediaLines.GetSize())
        {
            nEndLine = objMediaLines.GetAt(i + 1);
        }
        else
        {
            nEndLine = -1;
        }

        if (!objMediaDescription.Decode(objSdpLines, nStartLine, nEndLine))
        {
            IMS_TRACE_E(0, "Decoding a media-level description failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (!m_objMediaDescriptions.Append(objMediaDescription))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpParser::DecodeWithoutSession(IN const AString& strSdp)
{
    AStringArray objSdpLines;
    IMS_SINT32 nStartLine = 0;
    IMS_SINT32 nEndLine = -1;

    SplitLines(strSdp, objSdpLines);

    IMSList<IMS_SINT32> objMediaLines = GetMediaIndexes(objSdpLines);

    if (objMediaLines.IsEmpty())
    {
        return IMS_FALSE;
    }

    nEndLine = objMediaLines.GetAt(0);

    for (IMS_UINT32 i = 0; i < objMediaLines.GetSize(); ++i)
    {
        SdpMediaDescription objMediaDescription;

        nStartLine = nEndLine;

        if ((i + 1) < objMediaLines.GetSize())
        {
            nEndLine = objMediaLines.GetAt(i + 1);
        }
        else
        {
            nEndLine = -1;
        }

        if (!objMediaDescription.Decode(objSdpLines, nStartLine, nEndLine))
        {
            IMS_TRACE_E(0, "Decoding a media-level description failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (!m_objMediaDescriptions.Append(objMediaDescription))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
const SdpMediaDescription* SdpParser::GetMediaDescription(IN IMS_UINT32 nIndex) const
{
    if (nIndex >= m_objMediaDescriptions.GetSize())
    {
        return IMS_NULL;
    }

    return &(m_objMediaDescriptions.GetAt(nIndex));
}

PUBLIC
IMS_BOOL SdpParser::ValidateSdp() const
{
    // Check the connection line
    if (m_objSessionDescription.Contains(Sdp::TYPE_C))
    {
        return IMS_TRUE;
    }

    // Now, all the media description MUST have a connection line except for an inactive media.
    for (IMS_UINT32 i = 0; i < m_objMediaDescriptions.GetSize(); ++i)
    {
        const SdpMediaDescription& objMediaDesc = m_objMediaDescriptions.GetAt(i);

        if (objMediaDesc.GetMedia().GetPort() == 0)
        {
            continue;
        }

        if (!objMediaDesc.Contains(Sdp::TYPE_C))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL IMSList<IMS_SINT32> SdpParser::GetMediaIndexes(IN const AStringArray& objSdpLines)
{
    IMSList<IMS_SINT32> objMLines;

    for (IMS_SINT32 i = 0; i < objSdpLines.GetCount(); ++i)
    {
        const AString& strLine = objSdpLines.GetElementAt(i);

        if (strLine.StartsWith(Sdp::LINE_M))
        {
            objMLines.Append(i);
        }
    }

    return objMLines;
}

PRIVATE GLOBAL void SdpParser::SplitLines(IN const AString& strSdp, OUT AStringArray& objSdpLines)
{
    IMS_SINT32 nLineStart = 0;
    IMS_SINT32 nEndOfLine = 0;
    IMS_SINT32 nNextLineStart = 0;

    while (nLineStart < strSdp.GetLength())
    {
        nEndOfLine = strSdp.GetIndexOf(TextParser::CHAR_LF, nLineStart);

        if (nEndOfLine == AString::NPOS)
        {
            nEndOfLine = strSdp.GetLength();
        }

        nNextLineStart = nEndOfLine + 1;

        if ((nEndOfLine > 0) && (strSdp[nEndOfLine - 1] == TextParser::CHAR_CR))
        {
            --nEndOfLine;
        }

        if (nEndOfLine > nLineStart)
        {
            objSdpLines.AddElement(strSdp.GetSubStr(nLineStart, nEndOfLine - nLineStart));
        }

        nLineStart = nNextLineStart;
    }
}
