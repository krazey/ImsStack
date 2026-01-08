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
#ifndef __SIP_MESSAGE_BUFFER_H__
#define __SIP_MESSAGE_BUFFER_H__

#include "RcObject.h"

class SipMessageBuffer : public RcObject
{
public:
    SipMessageBuffer();
    SipMessageBuffer(const SipMessageBuffer& other);
    ~SipMessageBuffer() override;

private:
    SipMessageBuffer& operator=(IN const SipMessageBuffer&) = delete;

public:
    IMS_BYTE* GetBuffer();
    IMS_BYTE* GetBuffer(IN IMS_SINT32 nSlotId);
    /*
     Returns a maximum buffer length to form a SIP message
    */
    inline IMS_SINT32 GetLength() const { return MAX_MSG_SIZE; }

    static RcPtr<SipMessageBuffer> GetInstance();

public:
    // Max buffer size for raw SIP message
    enum
    {
        MAX_MSG_SIZE = 65535
    };

    // Variable for a temporary message storage to form a SIP message
    IMS_BYTE m_baBuffer[MAX_MSG_SIZE];
    // Buffer for dual VoLTE
    IMS_BYTE** m_ppBuffer;
};

#endif  //__SIP_MESSAGE_BUFFER_H__
