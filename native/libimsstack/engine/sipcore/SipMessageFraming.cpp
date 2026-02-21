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
#include "ImsLib.h"
#include "ImsStrLib.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "SipHeaderName.h"
#include "SipMessageFraming.h"
#include "SipPrivate.h"

__IMS_TRACE_TAG_SIP_CORE__;

static void StripLeadingLWS(IN_OUT IMS_CHAR*& pszStart, IN const IMS_CHAR* pszEnd)
{
    while ((pszStart <= pszEnd) && IMS_ISSPACE(*pszStart))
    {
        pszStart++;
    }
}

static void StripTrailingLWS(IN const IMS_CHAR* pszStart, IN_OUT IMS_CHAR*& pszEnd)
{
    while ((pszEnd >= pszStart) && IMS_ISSPACE(*pszEnd))
    {
        pszEnd--;
    }
}

PUBLIC
SipMessageFraming::SipMessageFraming() :
        m_nState(STATE_IDLE),
        m_nContentLength(0),
        m_nOffset(0),
        m_bGotBodyStart(IMS_FALSE),
        m_objMessageBuffer(ByteArray::ConstNull())
{
}

PUBLIC
SipMessageFraming::~SipMessageFraming() {}

PUBLIC
IMS_BOOL SipMessageFraming::AppendPacket(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    if ((pBuffer == IMS_NULL) || (nBuffLen <= 0))
    {
        return IMS_FALSE;
    }

    m_objMessageBuffer.Append(pBuffer, nBuffLen);

    if ((m_nState == STATE_IDLE) && !IsEmpty())
    {
        m_nState = STATE_CREATED;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipMessageFraming::CheckCompleteMessage()
{
    if (IsEmpty())
    {
        return IMS_FALSE;
    }

    if (m_nState == STATE_CREATED)
    {
        // Skip SigComp operation

        m_nState = STATE_CLEN;
    }

    if (m_nState == STATE_CLEN)
    {
        // Parse the Content-Length header
        ParseContentLength();
    }

    if (m_nState == STATE_BODY)
    {
        ParseMessageBody();
    }

    if (m_nState == STATE_DONE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL SipMessageFraming::GetCompleteMessage(OUT ByteArray& objMessage) const
{
    if (IsEmpty())
    {
        return IMS_FALSE;
    }

    // Clear the output buffer
    objMessage.Resize(0);

    // Copy one complete message from the receiving message buffer
    objMessage.Append(m_objMessageBuffer.GetData(), m_nOffset);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipMessageFraming::IgnoreCrlf()
{
    if (m_objMessageBuffer.GetLength() < 2)
    {
        return IMS_FALSE;
    }

    const IMS_BYTE* pbyData = m_objMessageBuffer.GetData();

    // CR LF
    if ((pbyData[0] == 0x0D) && (pbyData[1] == 0x0A))
    {
        m_objMessageBuffer.Erase(0, 2);

        IMS_TRACE_I("CRLF is ignored by SIP transport layer", 0, 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
void SipMessageFraming::UpdateState()
{
    // Check if the received packet has more message or not
    if (m_objMessageBuffer.GetLength() > m_nOffset)
    {
        m_objMessageBuffer.Erase(0, m_nOffset);
    }
    else if (m_objMessageBuffer.GetLength() == m_nOffset)
    {
        m_objMessageBuffer.Resize(0);
    }
    else
    {
        IMS_TRACE_E(0, "NOT REACHABLE; IT MUST NOT BE HAPPENED", 0, 0, 0);
    }

    // If the buffer starts with white spaces, it will be discarded.
    if (!IsEmpty())
    {
        const IMS_BYTE* pbyData = m_objMessageBuffer.GetData();
        IMS_SINT32 nDataLen = m_objMessageBuffer.GetLength();
        IMS_SINT32 nIndex = 0;
        IMS_SINT32 nWSPCount = 0;

        // Find non-empty character
        while (nIndex < nDataLen)
        {
            if (IMS_ISSPACE(static_cast<IMS_CHAR>(pbyData[nIndex])))
            {
                nWSPCount++;
            }
            else
            {
                break;
            }

            nIndex++;
        }

        if (nWSPCount > 0)
        {
            m_objMessageBuffer.Erase(0, nWSPCount);
            IMS_TRACE_I("MessageFraming: WSP(%d/%d) is discarded", nWSPCount, nDataLen, 0);
        }
    }

    if (IsEmpty())
    {
        m_nState = STATE_IDLE;
    }
    else
    {
        m_nState = STATE_CREATED;
    }

    m_nContentLength = 0;
    m_nOffset = 0;
    m_bGotBodyStart = IMS_FALSE;
}

PRIVATE
void SipMessageFraming::ParseContentLength()
{
    const IMS_CHAR acCF_CLEN[2] = {SipHeaderName::CF_CONTENT_LENGTH, '\0'};

    IMS_CHAR* pStart = reinterpret_cast<IMS_CHAR*>(m_objMessageBuffer.GetData() + m_nOffset);
    const IMS_CHAR* pEnd = pStart + m_objMessageBuffer.GetLength() - m_nOffset;  // over 1 byte
    IMS_CHAR* pCurrentPos;
    IMS_CHAR* pTemp;

    pCurrentPos = pStart;

    // Skip leading LWS
    StripLeadingLWS(pCurrentPos, pEnd);

    // Fine the separator between the header fields and the message body field.
    // We MUST know there is no case except for separating two fields.
    pTemp = IMS_StrStr(pCurrentPos, TextParser::STR_CRLFCRLF);

    if (pTemp != IMS_NULL)
    {
        pEnd = (pTemp + 4);  // pEnd moves behind CRLF
    }

    if (pStart == pEnd)
    {
        return;
    }

    // Skip the first line: Request-Line or Status-Line
    // This check is just done for one time.
    // NOTE: SHALL we handle the single line message ???
    if (m_nOffset == 0)
    {
        // Skips the leading empty lines
        while (pCurrentPos < pEnd)
        {
            if ((*pCurrentPos != TextParser::CHAR_CR) && (*pCurrentPos != TextParser::CHAR_LF) &&
                    (*pCurrentPos != TextParser::CHAR_SP) &&
                    (*pCurrentPos != TextParser::CHAR_HTAB))
            {
                break;
            }

            pCurrentPos++;
        }

        if (pCurrentPos >= pEnd)
        {
            // Ends parsing, but no need to increase the offset
            return;
        }

        // Skips the first line : Request-Line or Status-Line
        while (pCurrentPos < pEnd)
        {
            if ((*pCurrentPos == TextParser::CHAR_CR) &&
                    (*(pCurrentPos + 1) == TextParser::CHAR_LF))
            {
                pCurrentPos += 2;
                break;
            }
            // For backward-compatibility
            else if ((*pCurrentPos == TextParser::CHAR_CR) || (*pCurrentPos == TextParser::CHAR_LF))
            {
                pCurrentPos += 1;
                break;
            }

            pCurrentPos++;
        }
    }

    if (pCurrentPos >= pEnd)
    {
        // Ends parsing
        m_nOffset += (pEnd - pStart);
        return;
    }

    do
    {
        // LOOKUP HEADER NAME -- starts
        IMS_CHAR* pHeaderStart = pCurrentPos;

        while (pCurrentPos < pEnd)
        {
            if (*pCurrentPos == TextParser::CHAR_COLON)
                break;

            // End of one line & COLON is not present on the line
            if ((*pCurrentPos == TextParser::CHAR_CR) || (*pCurrentPos == TextParser::CHAR_LF))
            {
                pHeaderStart = pCurrentPos;
            }

            pCurrentPos++;
        }

        if (pCurrentPos >= pEnd)
        {
            // Ends parsing
            m_nOffset += (pHeaderStart - pStart);
            return;
        }
        // LOOKUP HEADER NAME -- ends

        if (*pCurrentPos == TextParser::CHAR_COLON)
        {
            // CHECK HEADER NAME -- starts

            IMS_SINT32 nLengthOfHeader;
            IMS_CHAR* pHeaderEnd = pCurrentPos - 1;

            // Strip leading & trailing LWS
            StripLeadingLWS(pHeaderStart, pHeaderEnd);
            StripTrailingLWS(pHeaderStart, pHeaderEnd);

            nLengthOfHeader = pHeaderEnd - pHeaderStart + 1;

            if (((nLengthOfHeader == 1) && (IMS_StrNICmp(pHeaderStart, &acCF_CLEN[0], 1) == 0)) ||
                    ((nLengthOfHeader == 14) &&
                            (IMS_StrNICmp(pHeaderStart, SipHeaderName::CONTENT_LENGTH, 14) == 0)))
            {
                IMS_CHAR* pHeaderBodyStart = pCurrentPos + 1;
                IMS_CHAR* pHeaderBodyEnd;
                // For backward-compatibility
                IMS_SINT32 nBodyEndOffset = 3;

                while (pCurrentPos < pEnd)
                {
                    if ((*pCurrentPos == TextParser::CHAR_CR) &&
                            (*(pCurrentPos + 1) == TextParser::CHAR_LF))
                    {
                        // End of one line
                        pCurrentPos += 2;
                        break;
                    }
                    // For backward-compatibility
                    else if ((*pCurrentPos == TextParser::CHAR_CR) ||
                            (*pCurrentPos == TextParser::CHAR_LF))
                    {
                        // End of one line
                        pCurrentPos += 1;
                        nBodyEndOffset = 2;
                        break;
                    }

                    pCurrentPos++;
                }

                if (pCurrentPos >= pEnd)
                {
                    // Ends parsing
                    m_nOffset += (pHeaderStart - pStart);
                    return;
                }

                // Content-Length: 131   $\r\n
                //    --> (pCurrentPos - 3 or 2) : $ position
                // For backward-compatibility : 3 -> nBodyEndOffset
                pHeaderBodyEnd = pCurrentPos - nBodyEndOffset;

                // Strip leading & trailing LWS
                StripLeadingLWS(pHeaderBodyStart, pHeaderBodyEnd);
                StripTrailingLWS(pHeaderBodyStart, pHeaderBodyEnd);

                AString strContentLength(pHeaderBodyStart, pHeaderBodyEnd - pHeaderBodyStart + 1);

                m_nContentLength = strContentLength.ToInt32();

                IMS_TRACE_I("MessageFraming: Content-Length: %s >> %d", strContentLength.GetStr(),
                        m_nContentLength, 0);

                // Store the current position to parse message body continuously
                // For backward-compatibility : 3 -> nBodyEndOffset
                m_nOffset += ((pCurrentPos - nBodyEndOffset) - pStart);

                // Update the parsing state
                m_nState = STATE_BODY;
                break;
            }
            // CHECK HEADER NAME -- ends

            // SKIP HEADER BODY -- starts
            while (pCurrentPos < pEnd)
            {
                if ((*pCurrentPos == TextParser::CHAR_CR) &&
                        (*(pCurrentPos + 1) == TextParser::CHAR_LF))
                {
                    // One line is obtained
                    pCurrentPos += 2;
                    break;
                }
                // For backward-compatibility
                else if ((*pCurrentPos == TextParser::CHAR_CR) ||
                        (*pCurrentPos == TextParser::CHAR_LF))
                {
                    // One line is obtained
                    pCurrentPos += 1;
                    break;
                }

                pCurrentPos++;
            }

            if (pCurrentPos >= pEnd)
            {
                // Ends parsing
                m_nOffset += (pHeaderStart - pStart);
                return;
            }
            // SKIP HEADER BODY -- ends
        }
    } while (IMS_TRUE);
}

PRIVATE
void SipMessageFraming::ParseMessageBody()
{
    IMS_CHAR* pBodyStart = reinterpret_cast<IMS_CHAR*>(m_objMessageBuffer.GetData() + m_nOffset);
    IMS_CHAR* pBodyEnd = reinterpret_cast<IMS_CHAR*>(
            m_objMessageBuffer.GetData() + m_objMessageBuffer.GetLength());
    IMS_CHAR* pCurrentPos = pBodyStart;

    if (m_bGotBodyStart == IMS_FALSE)
    {
        IMS_BOOL bDoubleCRLF = IMS_FALSE;

        while (pCurrentPos < (pBodyEnd - 3))
        {
            /*
             * Start-Line CRLF
             * Header field CRLF
             * ...
             * Header field CRLF
             * CRLF
             * Message-Bodys
             */
            if ((*pCurrentPos == TextParser::CHAR_CR) &&
                    (*(pCurrentPos + 1) == TextParser::CHAR_LF) &&
                    (*(pCurrentPos + 2) == TextParser::CHAR_CR) &&
                    (*(pCurrentPos + 3) == TextParser::CHAR_LF))
            {
                pCurrentPos += 4;
                bDoubleCRLF = IMS_TRUE;
                break;
            }

            pCurrentPos++;
        }

        if (pCurrentPos == (pBodyEnd - 3))
        {
            if (bDoubleCRLF)
            {
                m_bGotBodyStart = IMS_TRUE;
            }
            else
            {
                IMS_TRACE_D("MessageFraming: No D-CRLF [offset=%d, bodyStart=%c, cPos=%c]",
                        m_nOffset, (pBodyStart != IMS_NULL) ? *pBodyStart : '\0',
                        (pCurrentPos != IMS_NULL) ? *pCurrentPos : '\0');

                // We didn't find CRLFCRLF (start position of message body)
                m_nOffset += pCurrentPos - pBodyStart;
                return;
            }
        }
        else
        {
            m_bGotBodyStart = IMS_TRUE;
        }
    }

    m_nOffset += pCurrentPos - pBodyStart;

    if ((pCurrentPos == pBodyEnd) && (m_nContentLength == 0))
    {
        // WE GOT THE COMPLETE MESSAGE W/O MESSAGE BODY.
        m_nState = STATE_DONE;
        return;
    }

    IMS_SINT32 nContentLength = (pBodyEnd - pCurrentPos);

    if (nContentLength < m_nContentLength)
    {
        /*
         * We need to wait more message.
         * The offset field indicates the start position of the message body
         * when looks at the buffer int the next time.
         */
        IMS_TRACE_I("MessageFraming: Waiting more body part - clen=%d, rx=%d", m_nContentLength,
                nContentLength, 0);
        return;
    }

    // The caller MUST check the length of raw buffer & offset.
    // It moves the start position to the first position of the next message.
    m_nOffset += m_nContentLength;

    // WE GOT THE COMPLETE MESSAGE W/ MESSAGE BODY.
    m_nState = STATE_DONE;
}
