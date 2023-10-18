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
#ifndef SIP_MESSAGE_FRAMING_H_
#define SIP_MESSAGE_FRAMING_H_

#include "ByteArray.h"

class SipMessageFraming
{
public:
    SipMessageFraming();
    ~SipMessageFraming();

    SipMessageFraming(IN const SipMessageFraming&) = delete;
    SipMessageFraming& operator=(IN const SipMessageFraming&) = delete;

public:
    IMS_BOOL AppendPacket(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen);
    IMS_BOOL CheckCompleteMessage();
    IMS_BOOL GetCompleteMessage(OUT ByteArray& objMessage) const;
    IMS_BOOL IgnoreCrlf();
    inline IMS_BOOL IsEmpty() const { return (m_objMessageBuffer.GetLength() == 0); }
    void UpdateState();

private:
    void ParseContentLength();
    void ParseMessageBody();

public:
    /// State values for the incomplete SIP message
    enum
    {
        STATE_IDLE = 0x00,
        STATE_CREATED = 0x01,
        /// In this state, find the Content-Length header
        STATE_CLEN = 0x02,
        /// In this state, find the Message Body field
        STATE_BODY = 0x04,
        /// After Content-Length & Body is successfully found and the message needs to be processed
        STATE_DONE = 0x08
    };

private:
    // STATE_CREATED, ... in SIP
    IMS_SINT32 m_nState;
    // Length of the message body which got from Content-Length header
    IMS_SINT32 m_nContentLength;
    // Tracking offset in the buffer to parse Content-Length & Message Body
    IMS_SINT32 m_nOffset;
    // Flag to indicate if double CRLF (message body field) is found or not
    IMS_BOOL m_bGotBodyStart;
    // A part of SIP message which has been received until now
    ByteArray m_objMessageBuffer;
};

#endif
