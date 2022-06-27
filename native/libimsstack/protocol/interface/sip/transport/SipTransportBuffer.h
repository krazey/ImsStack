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
#ifndef __SIP_TRANSPORT_BUFFER_H__
#define __SIP_TRANSPORT_BUFFER_H__

#include "sip_pf_datatypes.h"

class SipTransportBuffer
{
    /* Raw Message */
    SIP_CHAR* m_pSipBuffer;
    SIP_UINT32 m_nSipBufferLen;

    SipTransportBuffer& operator=(IN const SipTransportBuffer& objRHS);
    SipTransportBuffer(IN const SipTransportBuffer& objRHS);

public:
    SipTransportBuffer() :
            m_pSipBuffer(SIP_NULL),
            m_nSipBufferLen(SIP_ZERO)
    {
    }
    SipTransportBuffer(SIP_CHAR* pSipBuffer, SIP_UINT32 nSipBufferLen) :
            m_pSipBuffer(pSipBuffer),
            m_nSipBufferLen(nSipBufferLen)
    {
    }
    /* Free Message Buffer */
    virtual ~SipTransportBuffer()
    {
        if (m_pSipBuffer != SIP_NULL)
        {
            delete[] m_pSipBuffer;
        }
    }
    SIP_CHAR* GetSipBuffer() const { return m_pSipBuffer; }
    SIP_UINT32 GetSipBufferLen() const { return m_nSipBufferLen; }
};

#endif  //__SIP_TRANSPORT_BUFFER_H__
