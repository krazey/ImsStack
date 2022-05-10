#include "platform/sip_pf_string.h"
#include "transport/SipTransportParameter.h"

SipTransportParameter::SipTransportParameter(SipTransportParameter* pTranspParam) :
        m_pHostAddress(SIP_NULL),
        m_nPort(SIP_ZERO),
        m_nTranspProtocol(SIP_INVALID),
        m_nSockId(SIP_ZERO),
        m_bMulticastEnable(SIP_FALSE),
        m_cTTL(SIP_ZERO),
        m_nTranspIpType(SIP_INVALID)
{
    if (pTranspParam == SIP_NULL)
    {
        return;
    }

    m_pHostAddress = SipPf_Strdup(pTranspParam->m_pHostAddress);
    m_nPort = pTranspParam->m_nPort;
    m_nTranspProtocol = pTranspParam->m_nTranspProtocol;
    m_nSockId = pTranspParam->m_nSockId;
    m_bMulticastEnable = pTranspParam->m_bMulticastEnable;
    m_cTTL = pTranspParam->m_cTTL;
    m_nTranspIpType = pTranspParam->m_nTranspIpType;
}

SipTransportParameter::SipTransportParameter(
        SIP_CHAR* pHostAddress, SIP_UINT16 nPort, SIP_INT32 eTranspProtocol) :
        m_pHostAddress(SIP_NULL),
        m_nPort(nPort),
        m_nTranspProtocol(eTranspProtocol),
        m_nSockId(SIP_ZERO),
        m_bMulticastEnable(SIP_FALSE),
        m_cTTL(SIP_ZERO),
        m_nTranspIpType(SIP_INVALID)
{
    m_pHostAddress = SipPf_Strdup(pHostAddress);
}

SipTransportParameter::SipTransportParameter(
        SIP_CHAR* pHostAddress, SIP_UINT16 nPort, SIP_INT32 eTranspProtocol, SIP_UINT32 nSockId) :
        m_pHostAddress(SIP_NULL),
        m_nPort(nPort),
        m_nTranspProtocol(eTranspProtocol),
        m_nSockId(nSockId),
        m_bMulticastEnable(SIP_FALSE),
        m_cTTL(SIP_ZERO),
        m_nTranspIpType(SIP_INVALID)
{
    m_pHostAddress = SipPf_Strdup(pHostAddress);
}

SIP_BOOL SipTransportParameter::setHostAddress(const SIP_CHAR* pHostAddress)
{
    if (m_pHostAddress != SIP_NULL)
    {
        delete[] m_pHostAddress;
        m_pHostAddress = SIP_NULL;
    }

    m_pHostAddress = SipPf_Strdup(pHostAddress);

    return SIP_TRUE;
}
