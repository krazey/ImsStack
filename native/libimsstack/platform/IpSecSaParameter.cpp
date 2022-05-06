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
#include "AStringBuffer.h"
#include "IpSecSaParameter.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_BASE__;

/**
 * @brief This class defines the parameters for IpSec security association.
 *
 * @see IIpSecPolicy
 */

PUBLIC
IpSecSaParameter::Policy::Policy() :
        m_nSpi(0),
        m_nDirection(DIRECTION_OUT),
        m_nMode(MODE_TRANSPORT),
        m_nTransportProtocol(TRANSPORT_PROTOCOL_UDP),
        m_objLocalAddress(SocketAddress()),
        m_objRemoteAddress(SocketAddress()),
        m_nSocketId(SOCKET_NOT_SET)
{
}

PUBLIC
IpSecSaParameter::Policy::Policy(IN IMS_SINT32 nSpi, IN IMS_SINT32 nDirection, IN IMS_SINT32 nMode,
        IN IMS_SINT32 nTransportProtocol, IN const SocketAddress& objLocalAddress,
        IN const SocketAddress& objRemoteAddress) :
        m_nSpi(nSpi),
        m_nDirection(nDirection),
        m_nMode(nMode),
        m_nTransportProtocol(nTransportProtocol),
        m_objLocalAddress(objLocalAddress),
        m_objRemoteAddress(objRemoteAddress),
        m_nSocketId(SOCKET_NOT_SET)
{
}

PUBLIC
IpSecSaParameter::Policy::Policy(IN IMS_SINT32 nSpi, IN IMS_SINT32 nDirection, IN IMS_SINT32 nMode,
        IN IMS_SINT32 nTransportProtocol, IN const IPAddress& objLocalIpAddress,
        IN IMS_SINT32 nLocalPort, IN const IPAddress& objRemoteIpAddress,
        IN IMS_SINT32 nRemotePort) :
        m_nSpi(nSpi),
        m_nDirection(nDirection),
        m_nMode(nMode),
        m_nTransportProtocol(nTransportProtocol),
        m_objLocalAddress(SocketAddress(objLocalIpAddress, nLocalPort)),
        m_objRemoteAddress(SocketAddress(objRemoteIpAddress, nRemotePort)),
        m_nSocketId(SOCKET_NOT_SET)
{
}

PUBLIC
IpSecSaParameter::Policy::Policy(IN const IpSecSaParameter::Policy& other) :
        m_nSpi(other.m_nSpi),
        m_nDirection(other.m_nDirection),
        m_nMode(other.m_nMode),
        m_nTransportProtocol(other.m_nTransportProtocol),
        m_objLocalAddress(other.m_objLocalAddress),
        m_objRemoteAddress(other.m_objRemoteAddress),
        m_nSocketId(other.m_nSocketId)
{
}

PUBLIC
IpSecSaParameter::Policy& IpSecSaParameter::Policy::operator=(
        IN const IpSecSaParameter::Policy& other)
{
    if (this != &other)
    {
        m_nSpi = other.m_nSpi;
        m_nDirection = other.m_nDirection;
        m_nMode = other.m_nMode;
        m_nTransportProtocol = other.m_nTransportProtocol;
        m_objLocalAddress = other.m_objLocalAddress;
        m_objRemoteAddress = other.m_objRemoteAddress;
        m_nSocketId = other.m_nSocketId;
    }

    return *this;
}

PUBLIC
IMS_BOOL IpSecSaParameter::Policy::HasAcceptedSocketId(IN IMS_SINT32 nSocketId)
{
    for (IMS_UINT32 i = 0; i < m_objAcceptedSocketIds.GetSize(); i++)
    {
        if (nSocketId == m_objAcceptedSocketIds.GetAt(i))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
void IpSecSaParameter::Policy::AddAcceptedSocketId(IN IMS_SINT32 nSocketId)
{
    for (IMS_UINT32 i = 0; i < m_objAcceptedSocketIds.GetSize(); i++)
    {
        if (nSocketId == m_objAcceptedSocketIds.GetAt(i))
        {
            return;
        }
    }

    m_objAcceptedSocketIds.Append(nSocketId);
}

PUBLIC
void IpSecSaParameter::Policy::RemoveAcceptedSocketId(IN IMS_SINT32 nSocketId)
{
    for (IMS_UINT32 i = 0; i < m_objAcceptedSocketIds.GetSize(); i++)
    {
        if (nSocketId == m_objAcceptedSocketIds.GetAt(i))
        {
            m_objAcceptedSocketIds.RemoveAt(i);
            return;
        }
    }
}

PUBLIC
AString IpSecSaParameter::Policy::ToString() const
{
    AStringBuffer objSb(128);

    objSb.Append("[ spi=");
    objSb.Append(m_nSpi);
    objSb.Append(", direction=");
    objSb.Append(DirectionToString(m_nDirection));
    objSb.Append(", mode=");
    objSb.Append(ModeToString(m_nMode));
    objSb.Append(", transportProtocol=");
    objSb.Append(TransportProtocolToString(m_nTransportProtocol));
    objSb.Append(", local-addr=");
    objSb.Append(m_objLocalAddress.ToString());
    objSb.Append(", remote-addr=");
    objSb.Append(m_objRemoteAddress.ToString());
    objSb.Append(", socketId=");
    objSb.Append(m_nSocketId);
    objSb.Append(" ]");

    return static_cast<const AStringBuffer&>(objSb).GetString();
}

PUBLIC GLOBAL const IMS_CHAR* IpSecSaParameter::Policy::DirectionToString(IN IMS_SINT32 nDirection)
{
    if (nDirection == DIRECTION_IN)
    {
        return "IN";
    }
    else if (nDirection == DIRECTION_OUT)
    {
        return "OUT";
    }

    return "INVALID";
}

PUBLIC GLOBAL const IMS_CHAR* IpSecSaParameter::Policy::ModeToString(IN IMS_SINT32 nMode)
{
    if (nMode == MODE_TRANSPORT)
    {
        return "TRANSPORT";
    }
    else if (nMode == MODE_TUNNEL)
    {
        return "TUNNEL";
    }

    return "UNKNOWN";
}

PUBLIC GLOBAL const IMS_CHAR* IpSecSaParameter::Policy::TransportProtocolToString(
        IN IMS_SINT32 nProtocol)
{
    if (nProtocol == TRANSPORT_PROTOCOL_UDP)
    {
        return "UDP";
    }
    else if (nProtocol == TRANSPORT_PROTOCOL_TCP)
    {
        return "TCP";
    }

    return "UNKNOWN";
}

PUBLIC
IpSecSaParameter::IpSecSaParameter() :
        m_nIpSecId(-1),
        m_nSecurityProtocol(SECURITY_PROTOCOL_ESP),
        m_nIntegrityAlgorithm(INTEGRITY_ALG_HMAC_SHA_1_96),
        m_nEncryptionAlgorithm(ENCRYPTION_ALG_AES_CBC),
        m_objIk(ByteArray::ConstNull()),
        m_objCk(ByteArray::ConstNull())
{
}

PUBLIC
IpSecSaParameter::IpSecSaParameter(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSecurityProtocol,
        IN IMS_SINT32 nIntegrityAlgorithm, IN IMS_SINT32 nEncryptionAlgorithm,
        IN const ByteArray& objIk, IN const ByteArray& objCk) :
        m_nIpSecId(nIpSecId),
        m_nSecurityProtocol(nSecurityProtocol),
        m_nIntegrityAlgorithm(nIntegrityAlgorithm),
        m_nEncryptionAlgorithm(nEncryptionAlgorithm),
        m_objIk(objIk),
        m_objCk(objCk)
{
}

PUBLIC
IpSecSaParameter::IpSecSaParameter(IN const IpSecSaParameter& other) :
        m_nIpSecId(other.m_nIpSecId),
        m_nSecurityProtocol(other.m_nSecurityProtocol),
        m_nIntegrityAlgorithm(other.m_nIntegrityAlgorithm),
        m_nEncryptionAlgorithm(other.m_nEncryptionAlgorithm),
        m_objIk(other.m_objIk),
        m_objCk(other.m_objCk),
        m_objPolicys(other.m_objPolicys)
{
}

PUBLIC
IpSecSaParameter& IpSecSaParameter::operator=(IN const IpSecSaParameter& other)
{
    if (this != &other)
    {
        m_nIpSecId = other.m_nIpSecId;
        m_nSecurityProtocol = other.m_nSecurityProtocol;
        m_nIntegrityAlgorithm = other.m_nIntegrityAlgorithm;
        m_nEncryptionAlgorithm = other.m_nEncryptionAlgorithm;
        m_objIk = other.m_objIk;
        m_objCk = other.m_objCk;
        m_objPolicys = other.m_objPolicys;
    }

    return *this;
}

PUBLIC
void IpSecSaParameter::AddPolicy(IN const IpSecSaParameter::Policy& objPolicy)
{
    IMS_TRACE_D("AddPolicy: %s", objPolicy.ToString().GetStr(), 0, 0);
    m_objPolicys.Append(objPolicy);
}

PUBLIC
void IpSecSaParameter::AddPolicys(IN const IMSList<IpSecSaParameter::Policy>& objPolicys)
{
    IMS_TRACE_D("AddPolicys: old=%d, new=%d", m_objPolicys.GetSize(), objPolicys.GetSize(), 0);
    m_objPolicys = objPolicys;
}

PUBLIC
void IpSecSaParameter::RemoveAllPolicys()
{
    IMS_TRACE_D("RemoveAllPolicys: %d", m_objPolicys.GetSize(), 0, 0);
    m_objPolicys.Clear();
}

PUBLIC
AString IpSecSaParameter::ToString() const
{
    AStringBuffer objSb(256);

    objSb.Append("[ ipSecId=");
    objSb.Append(m_nIpSecId);
    objSb.Append(", securityProtocol=");
    objSb.Append(SecurityProtocolToString(m_nSecurityProtocol));
    objSb.Append(", integrityAlg=");
    objSb.Append(IntegrityAlgToString(m_nIntegrityAlgorithm));
    objSb.Append(", encryptionAlg=");
    objSb.Append(EncryptionAlgToString(m_nEncryptionAlgorithm));
    objSb.Append(", ik=");
    objSb.Append(m_objIk.GetLength());
    objSb.Append(", ck=");
    objSb.Append(m_objCk.GetLength());

    for (IMS_SINT32 i = 0; i < m_objPolicys.GetSize(); i++)
    {
        const Policy& objPolicy = m_objPolicys.GetAt(i);
        objSb.Append(", p=");
        objSb.Append(objPolicy.ToString());
    }

    objSb.Append(" ]");

    return static_cast<const AStringBuffer&>(objSb).GetString();
}

PUBLIC GLOBAL const IMS_CHAR* IpSecSaParameter::SecurityProtocolToString(
        IN IMS_SINT32 nSecurityProtocol)
{
    if (nSecurityProtocol == SECURITY_PROTOCOL_ESP)
    {
        return "ESP";
    }
    else if (nSecurityProtocol == SECURITY_PROTOCOL_AH)
    {
        return "AH";
    }

    return "UNKNOWN";
}

PUBLIC GLOBAL const IMS_CHAR* IpSecSaParameter::IntegrityAlgToString(IN IMS_SINT32 nIntegrityAlg)
{
    if (nIntegrityAlg == INTEGRITY_ALG_HMAC_SHA_1_96)
    {
        return "HMAC_SHA1_96";
    }
    else if (nIntegrityAlg == INTEGRITY_ALG_HMAC_MD5_96)
    {
        return "HMAC_MD5_96";
    }

    return "UNKNOWN";
}

PUBLIC GLOBAL const IMS_CHAR* IpSecSaParameter::EncryptionAlgToString(IN IMS_SINT32 nEncryptionAlg)
{
    if (nEncryptionAlg == ENCRYPTION_ALG_AES_CBC)
    {
        return "AES_CBC";
    }
    else if (nEncryptionAlg == ENCRYPTION_ALG_DES_EDE3_CBC)
    {
        return "DES_EDE3_CBC";
    }
    else if (nEncryptionAlg == ENCRYPTION_ALG_NULL)
    {
        return "NULL";
    }

    return "UNKNOWN";
}
