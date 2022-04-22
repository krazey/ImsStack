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
#include "network/OsIpSecSa.h"

__IMS_TRACE_TAG_ADAPT__;


PRIVATE LOCAL
const IMS_CHAR CODE_CYPHER[]
        = "CUVEFGHLMKeQRPo78pTXYWZcd5aOnSNb26fgjkhiwxmlq34rstuvyz0IABJ1D9";

PRIVATE LOCAL
const IMS_CHAR CODE_CYPHER_SPECIAL[] = {20,21,22,23,24,25,26};

class OsIpSecSaPrivate
{
public:
    inline OsIpSecSaPrivate()
        : m_objSrcIp(IPAddress::IPv6NONE)
        , m_nSrcPort(0)
        , m_objDstIp(IPAddress::IPv6NONE)
        , m_nDstPort(0)
        , m_nSecurityProtocol(IPSecType::SECURITY_PROTOCOL_ESP)
        , m_nSpi(0)
        , m_nMode(IPSecType::MODE_TRANSPORT)
        , m_nAuthAlgorithm(IPSecType::INTEGRITY_ALGORITHM_HMAC_SHA_1_96)
        , m_nEncryptionAlgorithm(IPSecType::ENCRYPTION_ALGORITHM_NO)
        , m_objAuthKey(ByteArray::ConstNull())
        , m_objEncryptionKey(ByteArray::ConstNull())
        , m_strAuthHexKey(AString::ConstNull())
        , m_strEncryptionHexKey(AString::ConstNull())
    {}
    inline ~OsIpSecSaPrivate()
    {}

public:
    IPAddress m_objSrcIp;
    IMS_UINT32 m_nSrcPort;
    IPAddress m_objDstIp;
    IMS_UINT32 m_nDstPort;
    IMS_UINT32 m_nSecurityProtocol;
    IMS_UINT32 m_nSpi;
    IMS_UINT32 m_nMode;
    IMS_UINT32 m_nAuthAlgorithm;
    IMS_UINT32 m_nEncryptionAlgorithm;
    ByteArray m_objAuthKey;
    ByteArray m_objEncryptionKey;
    AString m_strAuthHexKey;
    AString m_strEncryptionHexKey;
};



PUBLIC
OsIpSecSa::OsIpSecSa()
    : m_pIpSecSaP(new OsIpSecSaPrivate())
{
}

PUBLIC VIRTUAL
OsIpSecSa::~OsIpSecSa()
{
    if (m_pIpSecSaP != IMS_NULL)
    {
        delete m_pIpSecSaP;
        m_pIpSecSaP = IMS_NULL;
    }
}

PUBLIC VIRTUAL
void OsIpSecSa::SetSA(IN const IPAddress& objSrcIp, IN IMS_UINT32 nSrcPort,
        IN const IPAddress& objDstIp, IN IMS_UINT32 nDstPort,
        IN IMS_UINT32 nSecurityProtocol, IN IMS_UINT32 nSpi, IN IMS_UINT32 nMode,
        IN IMS_UINT32 nAuthAlgorithm, IN IMS_UINT32 nEncryptionAlgorithm,
        IN const ByteArray& objAuthKey, IN const ByteArray& objEncryptionKey)
{
    m_pIpSecSaP->m_objSrcIp = objSrcIp;
    m_pIpSecSaP->m_nSrcPort = nSrcPort;
    m_pIpSecSaP->m_objDstIp = objDstIp;
    m_pIpSecSaP->m_nDstPort = nDstPort;
    m_pIpSecSaP->m_nSecurityProtocol = nSecurityProtocol;
    m_pIpSecSaP->m_nSpi = nSpi;
    m_pIpSecSaP->m_nMode = nMode;
    m_pIpSecSaP->m_nAuthAlgorithm = nAuthAlgorithm;
    m_pIpSecSaP->m_nEncryptionAlgorithm = nEncryptionAlgorithm;
    m_pIpSecSaP->m_objAuthKey = objAuthKey;
    m_pIpSecSaP->m_objEncryptionKey = objEncryptionKey;
}

PUBLIC VIRTUAL
void OsIpSecSa::DoneSA()
{
    MakeSa();
}

PUBLIC
void* OsIpSecSa::GetKey()
{
    return IMS_NULL;
}

PUBLIC
IMS_UINT32 OsIpSecSa::GetSpi() const
{
    return m_pIpSecSaP->m_nSpi;
}

PUBLIC
void OsIpSecSa::DisplayInfo()
{
    if (IMS_UTIL_SYS_PROP_IS_USER_MODE())
    {
        AString strIk = m_pIpSecSaP->m_objAuthKey.ToHexString();
        AString strCk = m_pIpSecSaP->m_objEncryptionKey.ToHexString();

        AString strEncodedIk = EncryptPrintKey(strIk);
        AString strEncodedCk = EncryptPrintKey(strCk);

        IMS_TRACE_D("IMS_IPSEC: IK=%s,CK=%s", strEncodedIk.GetStr(), strEncodedCk.GetStr(), 0);
        return;
    }

    AString strLog;

    IMS_TRACE_D("IPSEC-SA-INFO(spi|s-ip|d-ip|sec-proto|algo-auth|algo-enc|ik|ck)", 0, 0, 0);

    strLog.Sprintf("IMS_SA=0x%x|%s|%s|%d|%d|%d|na|na", m_pIpSecSaP->m_nSpi,
            m_pIpSecSaP->m_objSrcIp.ToString().GetStr(),
            m_pIpSecSaP->m_objDstIp.ToString().GetStr(),
            m_pIpSecSaP->m_nSecurityProtocol,
            m_pIpSecSaP->m_nAuthAlgorithm, m_pIpSecSaP->m_nEncryptionAlgorithm);

    IMS_TRACE_D("%s", strLog.GetStr(), 0, 0);
}

PUBLIC
IpSecSaParameter OsIpSecSa::CreateSaParameter(IN IMS_SINT32 nId) const
{
    ByteArray objAuthKey(m_pIpSecSaP->m_objAuthKey);

    if (m_pIpSecSaP->m_nAuthAlgorithm == IPSecType::INTEGRITY_ALGORITHM_HMAC_SHA_1_96)
    {
        const IMS_BYTE byExtra[4] = { 0, 0, 0, 0 };
        objAuthKey.Append(byExtra, 4);
    }

    IpSecSaParameter objSaParam(nId, m_pIpSecSaP->m_nSecurityProtocol,
            m_pIpSecSaP->m_nAuthAlgorithm, m_pIpSecSaP->m_nEncryptionAlgorithm,
            objAuthKey, m_pIpSecSaP->m_objEncryptionKey);

    return objSaParam;
}

PRIVATE
IMS_BOOL OsIpSecSa::SetIpAddress()
{
    AString strSrcIp = m_pIpSecSaP->m_objSrcIp.ToString();
    AString strDstIp = m_pIpSecSaP->m_objDstIp.ToString();

    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        IMS_TRACE_D("IPSecSA :: src-ip=%s, dst-ip=%s", strSrcIp.GetStr(), strDstIp.GetStr(), 0);
    }

    return IMS_TRUE;
}

PRIVATE
void OsIpSecSa::MakeSa()
{
    // IP address : src-ip / dst-ip
    if (!SetIpAddress())
    {
        return;
    }

    // Integrity algorithm & key
    SetAuthenticationKey();

    // Encryption algorithm & key
    IMS_UINT32 nEncryptionKeyLen = m_pIpSecSaP->m_objEncryptionKey.GetLength();

    if (nEncryptionKeyLen != 0)
    {
        SetEncryptionKey();
    }
}

PRIVATE
void OsIpSecSa::SetAuthenticationKey()
{
}

PRIVATE
void OsIpSecSa::SetEncryptionKey()
{
}

PRIVATE
AString OsIpSecSa::DecryptPrintKey(IN const AString& strInKey)
{
    AString strDecodedKey = AString::ConstNull();
    IMS_CHAR CODE_DECYPHER[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    IMS_CHAR CODE_DECYPHER_SPECIAL[] = {10,11,12,13,14,15,16};

    for (IMS_SINT32 i = 0; i < strInKey.GetLength(); ++i)
    {
        // Find a character's position from codeCypher
        if ((strInKey[i] >= 'A' && strInKey[i] <= 'Z')
                || (strInKey[i] >= 'a' && strInKey[i] <= 'z')
                || (strInKey[i] >= '0' && strInKey[i] <= '9'))
        {
            IMS_SINT32 nPos = 0;

            while (1)
            {
                if (CODE_CYPHER[nPos] == strInKey[i])
                {
                    strDecodedKey += CODE_DECYPHER[nPos];
                    break;
                }
                nPos++;
            }
        }
        else if ((strInKey[i] >= 20) && (strInKey[i] <= 26))
        {
            IMS_SINT32 nPos = 0;

            while (1)
            {
                if (CODE_CYPHER_SPECIAL[nPos] == strInKey[i])
                {
                    strDecodedKey += CODE_DECYPHER_SPECIAL[nPos];
                    break;
                }
                nPos++;
            }
        }
    }
    return strDecodedKey;
}

PRIVATE
AString OsIpSecSa::EncryptPrintKey(IN const AString& strInKey)
{
    AString strEncodedKey = AString::ConstNull();
    for (IMS_SINT32 i = 0; i < strInKey.GetLength(); i++)
    {

        if (strInKey[i] >= 'A' && strInKey[i] <= 'Z')
        {
            strEncodedKey += CODE_CYPHER[strInKey[i] - 'A'];
        }
        else if (strInKey[i] >= 'a' && strInKey[i] <= 'z')
        {
            strEncodedKey += CODE_CYPHER[strInKey[i] - 'a' + 26];
        }
        else if (strInKey[i] >= '0' && strInKey[i] <= '9')
        {
            strEncodedKey += CODE_CYPHER[strInKey[i] - '0' + 26 + 26];
        }
        else if (strInKey[i] >= 10 && strInKey[i] <= 16)
        {
            strEncodedKey += CODE_CYPHER_SPECIAL[strInKey[i] - 10];
        }
    }

    return strEncodedKey;
}
