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
