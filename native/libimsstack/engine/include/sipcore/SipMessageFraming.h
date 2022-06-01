/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090903  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_MESSAGE_FRAMING_H_
#define _SIP_MESSAGE_FRAMING_H_

#include "ByteArray.h"

class SIPMessageFraming
{
public:
    SIPMessageFraming();
    ~SIPMessageFraming();

public:
    IMS_BOOL AppendPacket(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen);
    IMS_BOOL CheckCompleteMessage();
    IMS_BOOL GetCompleteMessage(OUT ByteArray& objMessage) const;
    IMS_BOOL IgnoreCRLF();
    IMS_BOOL IsEmpty() const;
    void UpdateState();

private:
    void ParseContentLength();
    void ParseMessageBody();

public:
    // State values for the incomplete SIP message
    enum
    {
        STATE_IDLE = 0x00,
        STATE_CREATED = 0x01,
        // In this state, find the Content-Length header
        STATE_CLEN = 0x02,
        // In this state, find the Message Body field
        STATE_BODY = 0x04,
        // After Content-Length & Body is successfully found and the message needs to be processed
        STATE_DONE = 0x08
    };

private:
    // STATE_CREATED, ... in SIP
    IMS_SINT32 nState;
    // Length of the message body which got from Content-Length header
    IMS_SINT32 nContentLength;
    // Tracking offset in the buffer to parse Content-Length & Message Body
    IMS_SINT32 nOffset;
    // Flag to indicate if double CRLF (message body field) is found or not
    IMS_BOOL bGotBodyStart;

    // A part of SIP message which has been received until now
    ByteArray objMessageBuffer;
};

#endif  // _SIP_MESSAGE_FRAMING_H_
