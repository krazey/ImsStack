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
#include "ImsIpSecType.h"
#include "IMSStrLib.h"
#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "network/OsIpSecSp.h"

__IMS_TRACE_TAG_ADAPT__;

class OsIpSecSpPrivate
{
public:
    inline OsIpSecSpPrivate()
        : m_objSrcIp(IPAddress::IPv6NONE)
        , m_nSrcPort(0)
        , m_objDstIp(IPAddress::IPv6NONE)
        , m_nDstPort(0)
        , m_nTransportProtocol(IpSecType::TRANS_PROTOCOL_ANY)
        , m_nAction(IpSecType::ACTION_APPLY)
        , m_nDirection(IpSecType::DIRECTION_ANY)
        , m_nMode(IpSecType::MODE_TRANSPORT)
        , m_nSpi(0)
        , m_nSecurityProtocol(IpSecType::SECURITY_PROTOCOL_ESP)
        , m_nAuthAlgorithm(IpSecType::INTEGRITY_ALGORITHM_HMAC_SHA_1_96)
        , m_nEncryptionAlgorithm(IpSecType::ENCRYPTION_ALGORITHM_NO)
        , m_objTunnelSrcIp(IPAddress::IPv6NONE)
        , m_objTunnelDstIp(IPAddress::IPv6NONE)
    {}
    inline ~OsIpSecSpPrivate()
    {}

public:
    IPAddress m_objSrcIp;
    IMS_UINT32 m_nSrcPort;
    IPAddress m_objDstIp;
    IMS_UINT32 m_nDstPort;
    IMS_UINT32 m_nTransportProtocol;
    IMS_UINT32 m_nAction;
    IMS_UINT32 m_nDirection;
    IMS_UINT32 m_nMode;
    IMS_UINT32 m_nSpi;
    IMS_UINT32 m_nSecurityProtocol;
    IMS_UINT32 m_nAuthAlgorithm;
    IMS_UINT32 m_nEncryptionAlgorithm;
    IPAddress m_objTunnelSrcIp;
    IPAddress m_objTunnelDstIp;
};



PUBLIC
OsIpSecSp::OsIpSecSp()
    : m_pIpSecSpP(new OsIpSecSpPrivate())
{
}

PUBLIC VIRTUAL
OsIpSecSp::~OsIpSecSp()
{
    if (m_pIpSecSpP != IMS_NULL)
    {
        delete m_pIpSecSpP;
        m_pIpSecSpP = IMS_NULL;
    }
}

PUBLIC VIRTUAL
void OsIpSecSp::SetTransportInfo(IN const IPAddress& objSrcIp, IN IMS_UINT32 nSrcPort,
        IN const IPAddress& objDstIp, IN IMS_UINT32 nDstPort,
        IN IMS_UINT32 nTransportProtocol, IN IMS_UINT32 nAction,
        IN IMS_UINT32 nDirection, IN IMS_UINT32 nSpi, IN IMS_UINT32 nMode)
{
    m_pIpSecSpP->m_objSrcIp = objSrcIp;
    m_pIpSecSpP->m_nSrcPort = nSrcPort;
    m_pIpSecSpP->m_objDstIp = objDstIp;
    m_pIpSecSpP->m_nDstPort = nDstPort;
    m_pIpSecSpP->m_nTransportProtocol = nTransportProtocol;
    m_pIpSecSpP->m_nAction = nAction;
    m_pIpSecSpP->m_nDirection = nDirection;
    m_pIpSecSpP->m_nMode = nMode;
    m_pIpSecSpP->m_nSpi = nSpi;
}

PUBLIC VIRTUAL
void OsIpSecSp::SetSecurityAlgorithmInfo(IN IMS_UINT32 nSecurityProtocol,
        IN IMS_UINT32 nAuthAlgorithm, IN IMS_UINT32 nEncryptionAlgorithm)
{
    m_pIpSecSpP->m_nSecurityProtocol = nSecurityProtocol;
    m_pIpSecSpP->m_nAuthAlgorithm = nAuthAlgorithm;
    m_pIpSecSpP->m_nEncryptionAlgorithm = nEncryptionAlgorithm;
}

PUBLIC VIRTUAL
void OsIpSecSp::DoneSp()
{
    SetTransportInfo();
    SetSecurityAlgorithmInfo();
}

PUBLIC
void* OsIpSecSp::GetConf()
{
    return IMS_NULL;
}

PUBLIC
IMS_UINT32 OsIpSecSp::GetSpi() const
{
    return m_pIpSecSpP->m_nSpi;
}

PUBLIC
void OsIpSecSp::DisplayInfo()
{
    if (!IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        return;
    }

    AString strLog;

    IMS_TRACE_D("IPSEC-SP-INFO(spi|s-ip|d-ip|s-port|d-port|dir|proto|mode|action)", 0, 0, 0);

    strLog.Sprintf("IMS_SP=0x%x|%s|%s|%d|%d|%d|%d|%d|%d", m_pIpSecSpP->m_nSpi,
            m_pIpSecSpP->m_objSrcIp.ToString().GetStr(),
            m_pIpSecSpP->m_objDstIp.ToString().GetStr(),
            m_pIpSecSpP->m_nSrcPort, m_pIpSecSpP->m_nDstPort,
            m_pIpSecSpP->m_nDirection, m_pIpSecSpP->m_nTransportProtocol,
            m_pIpSecSpP->m_nMode, m_pIpSecSpP->m_nAction);

    IMS_TRACE_D("%s", strLog.GetStr(), 0, 0);
}

PUBLIC
IpSecSaParameter::Policy OsIpSecSp::CreateSaPolicy() const
{
    IpSecSaParameter::Policy objPolicy(m_pIpSecSpP->m_nSpi,
            m_pIpSecSpP->m_nDirection, m_pIpSecSpP->m_nMode, m_pIpSecSpP->m_nTransportProtocol,
            m_pIpSecSpP->m_objSrcIp, m_pIpSecSpP->m_nSrcPort,
            m_pIpSecSpP->m_objDstIp, m_pIpSecSpP->m_nDstPort);
    return objPolicy;
}

PRIVATE
IMS_BOOL OsIpSecSp::SetIpAddress()
{
    AString strSrcIp = m_pIpSecSpP->m_objSrcIp.ToString();
    AString strDstIp = m_pIpSecSpP->m_objDstIp.ToString();

    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        IMS_TRACE_D("IPSecSP :: src-ip=%s, dst-ip=%s", strSrcIp.GetStr(), strDstIp.GetStr(), 0);
    }

    return IMS_TRUE;
}

PRIVATE
void OsIpSecSp::SetTransportInfo()
{
    // IP address : src-ip / dst-ip
    if (!SetIpAddress())
    {
        return;
    }
}

PRIVATE
void OsIpSecSp::SetSecurityAlgorithmInfo()
{
}
