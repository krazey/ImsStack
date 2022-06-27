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
#ifndef __SIP_TRANSPORT_PARAMETER_H__
#define __SIP_TRANSPORT_PARAMETER_H__

#include "sip_pf_datatypes.h"

class SipTransportParameter
{
private:
    /* host =  hostname/IPv4address /IPv6reference */
    SIP_CHAR* m_pHostAddress;
    SIP_UINT16 m_nPort;
    SIP_INT32 m_nTranspProtocol;
    SIP_UINT32 m_nSockId;
    /* Enable = SIP_TRUE and for Disable= SIP_FALSE */
    SIP_BOOL m_bMulticastEnable;
    /* Time to Live : Value starts from 0-255 only */
    SIP_CHAR m_cTTL;

    SIP_INT32 m_nTranspIpType;

    SipTransportParameter& operator=(IN const SipTransportParameter& objRHS);
    SipTransportParameter(IN const SipTransportParameter& objRHS);

public:
    SipTransportParameter() :
            m_pHostAddress(SIP_NULL),
            m_nPort(SIP_ZERO),
            m_nTranspProtocol(SIP_INVALID),
            m_nSockId(SIP_ZERO),
            m_bMulticastEnable(SIP_FALSE),
            m_cTTL(SIP_ZERO),
            m_nTranspIpType(SIP_INVALID)
    {
    }
    SipTransportParameter(SipTransportParameter* pTranspParam);
    SipTransportParameter(SIP_CHAR* pHostAddress, SIP_UINT16 nPort, SIP_INT32 eTranspProtocol);
    SipTransportParameter(SIP_CHAR* pHostAddress, SIP_UINT16 nPort, SIP_INT32 eTranspProtocol,
            SIP_UINT32 nSockId);
    virtual ~SipTransportParameter()
    {
        if (m_pHostAddress != SIP_NULL)
        {
            delete[] m_pHostAddress;
        }
    }
    SIP_BOOL GetMulticastEnable() const { return m_bMulticastEnable; }

    SIP_CHAR GetCTTL() const { return m_cTTL; }

    SIP_INT32 GetTranspIpType() const { return m_nTranspIpType; }

    SIP_INT32 GetTranspProtocol() const { return m_nTranspProtocol; }

    SIP_CHAR* GetHostAddress() const { return m_pHostAddress; }

    SIP_UINT32 GetSockId() const { return m_nSockId; }

    SIP_UINT16 GetPort() const { return m_nPort; }

    SIP_VOID setMulticastEnable(SIP_BOOL bMulticastEnable)
    {
        this->m_bMulticastEnable = bMulticastEnable;
    }

    SIP_VOID setCTTL(SIP_CHAR cTTL) { this->m_cTTL = cTTL; }

    SIP_VOID setTanspIpType(SIP_INT32 nTanspIpType) { this->m_nTranspIpType = nTanspIpType; }

    SIP_VOID setTranspProtocol(SIP_INT32 nTranspProtocol)
    {
        this->m_nTranspProtocol = nTranspProtocol;
    }

    SIP_BOOL setHostAddress(const SIP_CHAR* pHostAddress);

    SIP_VOID setSockId(SIP_UINT32 nSockId) { this->m_nSockId = nSockId; }

    SIP_VOID setPort(SIP_UINT16 nPort) { this->m_nPort = nPort; }
};

#endif  //__SIP_TRANSPORT_PARAMETER_H__
