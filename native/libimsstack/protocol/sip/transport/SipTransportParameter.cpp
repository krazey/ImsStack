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
#include "platform/SipString.h"
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
    if (pTranspParam != SIP_NULL)
    {
        m_pHostAddress = SipPf_Strdup(pTranspParam->m_pHostAddress);
        m_nPort = pTranspParam->m_nPort;
        m_nTranspProtocol = pTranspParam->m_nTranspProtocol;
        m_nSockId = pTranspParam->m_nSockId;
        m_bMulticastEnable = pTranspParam->m_bMulticastEnable;
        m_cTTL = pTranspParam->m_cTTL;
        m_nTranspIpType = pTranspParam->m_nTranspIpType;
    }
}

SipTransportParameter::SipTransportParameter(
        const SIP_CHAR* pHostAddress, SIP_UINT16 nPort, SIP_INT32 eTranspProtocol) :
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

SIP_VOID SipTransportParameter::SetHostAddress(const SIP_CHAR* pHostAddress)
{
    if (m_pHostAddress != SIP_NULL)
    {
        delete[] m_pHostAddress;
        m_pHostAddress = SIP_NULL;
    }

    m_pHostAddress = SipPf_Strdup(pHostAddress);
}
