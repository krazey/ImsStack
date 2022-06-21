#include "sip_pf_datatypes.h"
#include "msg/sip_comdef.h"
#include "SipConfiguration.h"
#include "transport/SipTransportInfo.h"
#include "SipTrace.h"

SipTransportInfo::SipTransportInfo(
        SipTransportParameter* pTranspParam, SipTransportBuffer* pTransSipBuffer) :
        m_cNumTimeReqSent(SIP_ZERO),
        m_pActualDestParam(SIP_NULL),
        m_pTranspParam(SIP_NULL),
        m_pSentBuffer(SIP_NULL),
        m_pSentSipMsg(SIP_NULL)
{
    if (pTranspParam == SIP_NULL)
    {
        return;
    }
    m_pTranspParam = new SipTransportParameter(pTranspParam);
    m_pSentBuffer = pTransSipBuffer;
}

SipTransportInfo::~SipTransportInfo()
{
    if (m_pActualDestParam != SIP_NULL)
    {
        delete m_pActualDestParam;
        m_pActualDestParam = SIP_NULL;
    }

    if (m_pTranspParam != SIP_NULL)
    {
        delete m_pTranspParam;
    }

    if (m_pSentBuffer != SIP_NULL)
    {
        delete m_pSentBuffer;
    }

    if (m_pSentSipMsg != SIP_NULL)
    {
        delete m_pSentSipMsg;
    }
}

SipTransportParameter* SipTransportInfo::GetMsgSentTranspParam()
{
    return m_pActualDestParam;
}

SIP_BOOL SipTransportInfo::SetMsgSentTranspParam(SipTransportParameter* pTranspParam)
{
    m_pActualDestParam = pTranspParam;
    return SIP_TRUE;
}

SipTransportBuffer* SipTransportInfo::GetTranspSipBuffer()
{
    return m_pSentBuffer;
}

SipMessage* SipTransportInfo::GetSentSipMsg()
{
    return m_pSentSipMsg;
}

SIP_VOID SipTransportInfo::SetSentSipMsg(SipMessage* _pSipMsg)
{
    if (m_pSentSipMsg != SIP_NULL)
    {
        m_pSentSipMsg->SipDelete();
        m_pSentSipMsg = SIP_NULL;
    }
    m_pSentSipMsg = _pSipMsg;
}
