/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090903  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "IMSLib.h"
#include "IMSStrLib.h"
#include "SIPPrivate.h"
#include "SipHeaderName.h"
#include "SIPMessageFraming.h"

__IMS_TRACE_TAG_SIP__;

LOCAL inline void StripLeadingLWS(IN_OUT IMS_CHAR*& pszStart, IN CONST IMS_CHAR* pszEnd)
{
    while ((pszStart <= pszEnd) && IMS_ISSPACE(*pszStart))
    {
        pszStart++;
    }
}

LOCAL inline void StripTrailingLWS(IN CONST IMS_CHAR* pszStart, IN_OUT IMS_CHAR*& pszEnd)
{
    while ((pszEnd >= pszStart) && IMS_ISSPACE(*pszEnd))
    {
        pszEnd--;
    }
}

PUBLIC
SIPMessageFraming::SIPMessageFraming() :
        nState(STATE_IDLE),
        nContentLength(0),
        nOffset(0),
        bGotBodyStart(IMS_FALSE),
        objMessageBuffer(ByteArray::ConstNull())
{
}

PUBLIC
SIPMessageFraming::~SIPMessageFraming() {}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessageFraming::AppendPacket(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    //---------------------------------------------------------------------------------------------

    if ((pBuffer == IMS_NULL) || (nBuffLen <= 0))
        return IMS_FALSE;

    objMessageBuffer.Append(pBuffer, nBuffLen);

    if ((nState == STATE_IDLE) && !IsEmpty())
    {
        nState = STATE_CREATED;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessageFraming::CheckCompleteMessage()
{
    //---------------------------------------------------------------------------------------------

    if (IsEmpty())
        return IMS_FALSE;

    if (nState == STATE_CREATED)
    {
        // Skip SigComp operation

        nState = STATE_CLEN;
    }

    if (nState == STATE_CLEN)
    {
        // Parse the Content-Length header
        ParseContentLength();
    }

    if (nState == STATE_BODY)
    {
        ParseMessageBody();
    }

    if (nState == STATE_DONE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessageFraming::GetCompleteMessage(OUT ByteArray& objMessage) const
{
    //---------------------------------------------------------------------------------------------

    if (IsEmpty())
        return IMS_FALSE;

    // Clear the output buffer
    objMessage.Resize(0);

    // Copy one complete message from the receiving message buffer
    objMessage.Append(objMessageBuffer.GetData(), nOffset);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessageFraming::IgnoreCRLF()
{
    //---------------------------------------------------------------------------------------------

    if (objMessageBuffer.GetLength() < 2)
        return IMS_FALSE;

    const IMS_BYTE* pbyData = objMessageBuffer.GetData();

    // CR LF
    if ((pbyData[0] == 0x0D) && (pbyData[1] == 0x0A))
    {
        objMessageBuffer.Erase(0, 2);

        IMS_TRACE_I("CRLF is ignored by SIP transport layer", 0, 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessageFraming::IsEmpty() const
{
    //---------------------------------------------------------------------------------------------

    return (objMessageBuffer.GetLength() == 0);
}

/*

Remarks

*/
PUBLIC
void SIPMessageFraming::UpdateState()
{
    //---------------------------------------------------------------------------------------------

    // Check if the received packet has more message or not
    if (objMessageBuffer.GetLength() > nOffset)
    {
        objMessageBuffer.Erase(0, nOffset);
    }
    else if (objMessageBuffer.GetLength() == nOffset)
    {
        objMessageBuffer.Resize(0);
    }
    else
    {
        IMS_TRACE_E(0, "NOT REACHABLE; IT MUST NOT BE HAPPENED", 0, 0, 0);
    }

    // If the buffer starts with white spaces, it will be discarded.
    if (!IsEmpty())
    {
        const IMS_BYTE* pbyData = objMessageBuffer.GetData();
        IMS_SINT32 nDataLen = objMessageBuffer.GetLength();
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
            objMessageBuffer.Erase(0, nWSPCount);
            IMS_TRACE_I("MessageFraming :: WSP(%d/%d) is discarded", nWSPCount, nDataLen, 0);
        }
    }

    if (IsEmpty())
        nState = STATE_IDLE;
    else
        nState = STATE_CREATED;

    nContentLength = 0;
    nOffset = 0;
    bGotBodyStart = IMS_FALSE;
}

/*

Remarks

*/
PRIVATE
void SIPMessageFraming::ParseContentLength()
{
    const IMS_CHAR acCF_CLEN[2] = {SipHeaderName::CF_CONTENT_LENGTH, '\0'};

    IMS_CHAR* pStart = reinterpret_cast<IMS_CHAR*>(objMessageBuffer.GetData() + nOffset);
    IMS_CHAR* pEnd = pStart + objMessageBuffer.GetLength() - nOffset;  // over 1 byte
    IMS_CHAR* pCurrentPos;
    IMS_CHAR* pTemp;

    //---------------------------------------------------------------------------------------------

    pCurrentPos = pStart;

    // Skip leading LWS
    StripLeadingLWS(pCurrentPos, pEnd);

    // Fine the separator between the header fields and the message body field.
    // We MUST know there is no case except for separating two fields.
    pTemp = IMS_StrStr(pCurrentPos, TextParser::STR_CRLFCRLF);

    if (pTemp != IMS_NULL)
        pEnd = (pTemp + 4);  // pEnd moves behind CRLF

    if (pStart == pEnd)
    {
        return;
    }

    // Skip the first line: Request-Line or Status-Line
    // This check is just done for one time.
    // NOTE: SHALL we handle the single line message ???
    if (nOffset == 0)
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
        nOffset += (pEnd - pStart);
        return;
    }

    IMS_CHAR* pHeaderStart;

    do
    {
        // LOOKUP HEADER NAME -- starts
        pHeaderStart = pCurrentPos;

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
            nOffset += (pHeaderStart - pStart);
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
                    nOffset += (pHeaderStart - pStart);
                    return;
                }

                // Content-Length: 131   $\r\n
                //    --> (pCurrentPos - 3 or 2) : $ position
                // For backward-compatibility : 3 -> nBodyEndOffset
                pHeaderBodyEnd = pCurrentPos - nBodyEndOffset;

                // Strip leading & trailing LWS
                StripLeadingLWS(pHeaderBodyStart, pHeaderBodyEnd);
                StripTrailingLWS(pHeaderBodyStart, pHeaderBodyEnd);

                AString strCLEN(pHeaderBodyStart, pHeaderBodyEnd - pHeaderBodyStart + 1);

                nContentLength = strCLEN.ToInt32();

                IMS_TRACE_I("MessageFraming :: Content-Length: %s >> %d", strCLEN.GetStr(),
                        nContentLength, 0);

                // Store the current position to parse message body continuously
                // For backward-compatibility : 3 -> nBodyEndOffset
                nOffset += ((pCurrentPos - nBodyEndOffset) - pStart);

                // Update the parsing state
                nState = STATE_BODY;
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
                nOffset += (pHeaderStart - pStart);
                return;
            }
            // SKIP HEADER BODY -- ends
        }
    } while (IMS_TRUE);
}

/*

Remarks

*/
PRIVATE
void SIPMessageFraming::ParseMessageBody()
{
    IMS_CHAR* pBodyStart = (IMS_CHAR*)(objMessageBuffer.GetData() + nOffset);
    IMS_CHAR* pBodyEnd = (IMS_CHAR*)(objMessageBuffer.GetData() + objMessageBuffer.GetLength());
    IMS_CHAR* pCurrentPos = pBodyStart;

    //---------------------------------------------------------------------------------------------

    if (bGotBodyStart == IMS_FALSE)
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
                bGotBodyStart = IMS_TRUE;
            }
            else
            {
                IMS_TRACE_D("MessageFraming :: No D-CRLF [offset=%d, bodyStart=%c, cPos=%c]",
                        nOffset, (pBodyStart != IMS_NULL) ? *pBodyStart : '\0',
                        (pCurrentPos != IMS_NULL) ? *pCurrentPos : '\0');

                // We didn't find CRLFCRLF (start position of message body)
                nOffset += pCurrentPos - pBodyStart;
                return;
            }
        }
        else
        {
            bGotBodyStart = IMS_TRUE;
        }
    }

    nOffset += pCurrentPos - pBodyStart;

    if ((pCurrentPos == pBodyEnd) && (nContentLength == 0))
    {
        // WE GOT THE COMPLETE MESSAGE W/O MESSAGE BODY.
        nState = STATE_DONE;
        return;
    }

    IMS_SINT32 nCLEN = (pBodyEnd - pCurrentPos);

    if (nCLEN < nContentLength)
    {
        /*
         * We need to wait more message.
         * The offset field indicates the start position of the message body
         * when looks at the buffer int the next time.
         */
        IMS_TRACE_I("MessageFraming :: Waiting more body part - clen=%d, rx=%d", nContentLength,
                nCLEN, 0);
        return;
    }

    // The caller MUST check the length of raw buffer & offset.
    // It moves the start position to the first position of the next message.
    nOffset += nContentLength;

    // WE GOT THE COMPLETE MESSAGE W/ MESSAGE BODY.
    nState = STATE_DONE;
}
