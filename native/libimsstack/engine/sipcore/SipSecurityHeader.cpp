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
#include "ServiceMemory.h"
#include "TextParser.h"

#include "ISipHeader.h"
#include "SipParameter.h"
#include "SipSecurityHeader.h"

PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_MECHANISM_DIGEST[] = "digest";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_MECHANISM_TLS[] = "tls";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_MECHANISM_IPSEC_IKE[] = "ipsec-ike";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_MECHANISM_IPSEC_MAN[] = "ipsec-man";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_MECHANISM_IPSEC_3GPP[] = "ipsec-3gpp";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_ALG_HMAC_MD5_96[] = "hmac-md5-96";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_ALG_HMAC_SHA_1_96[] = "hmac-sha-1-96";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_PROT_AH[] = "ah";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_PROT_ESP[] = "esp";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_MOD_TRANS[] = "trans";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_MOD_TUN[] = "tun";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_MOD_UDP_ENC_TUN[] = "UDP-enc-tun";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_EALG_DES_EDE3_CBC[] = "des-ede3-cbc";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_EALG_AES_CBC[] = "aes-cbc";
PUBLIC GLOBAL const IMS_CHAR SipSecurityHeader::P_VALUE_EALG_NULL[] = "null";

PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_PREFERENCE[] = "q";
PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_ALGORITHM[] = "alg";
PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_PROTOCOL[] = "prot";
PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_MODE[] = "mod";
PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_ENCRYPTION_ALGORITHM[] = "ealg";
PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_SPI_C[] = "spi-c";
PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_SPI_S[] = "spi-s";
PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_PORT_C[] = "port-c";
PRIVATE GLOBAL const IMS_CHAR SipSecurityHeader::P_NAME_PORT_S[] = "port-s";

PUBLIC
SipSecurityHeader::SipSecurityHeader(IN IMS_SINT32 nMechanism /* = MECHANISM_IPSEC_3GPP*/,
        IN const AString& strMechanism /* = AString::ConstNull()*/,
        IN IMS_BOOL bParameterRequired /* = IMS_TRUE*/) :
        m_bParameterRequired(bParameterRequired),
        m_nMechanism(nMechanism),
        m_strMechanism(strMechanism),
        m_strPreference(AString::ConstNull()),
        m_nAlgorithm(ALG_HMAC_SHA_1_96),
        m_nProtocol(PROTOCOL_ESP),
        m_nMode(MODE_TRANSPORT),
        m_nEncryptionAlgorithm(EALG_NULL),
        m_nSpiC(0),
        m_nSpiS(0),
        m_nPortC(0),
        m_nPortS(0),
        m_objUnknownParamValues(IMSMap<IMS_SINT32, AString>()),
        m_objExtensions(IMSMap<AString, AString>()),
        m_nMechParams(0),
        m_bSpi3gppCompliant(IMS_TRUE)
{
}

PUBLIC
SipSecurityHeader::SipSecurityHeader(IN const SipSecurityHeader& other) :
        m_bParameterRequired(other.m_bParameterRequired),
        m_nMechanism(other.m_nMechanism),
        m_strMechanism(other.m_strMechanism),
        m_strPreference(other.m_strPreference),
        m_nAlgorithm(other.m_nAlgorithm),
        m_nProtocol(other.m_nProtocol),
        m_nMode(other.m_nMode),
        m_nEncryptionAlgorithm(other.m_nEncryptionAlgorithm),
        m_nSpiC(other.m_nSpiC),
        m_nSpiS(other.m_nSpiS),
        m_nPortC(other.m_nPortC),
        m_nPortS(other.m_nPortS),
        m_objUnknownParamValues(other.m_objUnknownParamValues),
        m_objExtensions(other.m_objExtensions),
        m_nMechParams(other.m_nMechParams),
        m_bSpi3gppCompliant(other.m_bSpi3gppCompliant)
{
}

PUBLIC
SipSecurityHeader::~SipSecurityHeader() {}

PUBLIC
SipSecurityHeader& SipSecurityHeader::operator=(IN const SipSecurityHeader& other)
{
    if (this != &other)
    {
        m_bParameterRequired = other.m_bParameterRequired;
        m_nMechanism = other.m_nMechanism;
        m_strMechanism = other.m_strMechanism;
        m_strPreference = other.m_strPreference;
        m_nAlgorithm = other.m_nAlgorithm;
        m_nProtocol = other.m_nProtocol;
        m_nMode = other.m_nMode;
        m_nEncryptionAlgorithm = other.m_nEncryptionAlgorithm;
        m_nSpiC = other.m_nSpiC;
        m_nSpiS = other.m_nSpiS;
        m_nPortC = other.m_nPortC;
        m_nPortS = other.m_nPortS;
        m_objUnknownParamValues = other.m_objUnknownParamValues;
        m_objExtensions = other.m_objExtensions;
        m_nMechParams = other.m_nMechParams;
        m_bSpi3gppCompliant = other.m_bSpi3gppCompliant;
    }

    return (*this);
}

PUBLIC
const AString& SipSecurityHeader::GetUnknownParameterValue(IN IMS_SINT32 nName) const
{
    IMS_SLONG nIndex = m_objUnknownParamValues.GetIndexOfKey(nName);

    if (nIndex < 0)
    {
        return AString::ConstNull();
    }

    return m_objUnknownParamValues.GetValueAt(nIndex);
}

PUBLIC
void SipSecurityHeader::SetPort(IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortS)
{
    m_nPortC = nPortC;
    m_nPortS = nPortS;

    if (nPortC >= 0)
    {
        m_nMechParams |= PORT_C;
    }
    else
    {
        m_nMechParams &= (~PORT_C);
    }

    if (nPortS >= 0)
    {
        m_nMechParams |= PORT_S;
    }
    else
    {
        m_nMechParams &= (~PORT_S);
    }
}

PUBLIC
void SipSecurityHeader::SetSpi(
        IN IMS_UINT32 nSpiC, IN IMS_UINT32 nSpiS, IN IMS_BOOL b3gppCompliant /* = IMS_TRUE*/)
{
    m_nSpiC = nSpiC;
    m_nSpiS = nSpiS;
    m_bSpi3gppCompliant = b3gppCompliant;

    m_nMechParams |= SPI_C;
    m_nMechParams |= SPI_S;
}

PUBLIC
IMS_BOOL SipSecurityHeader::SetExtensionParameter(
        IN const AString& strName, IN const AString& strValue)
{
    IMS_SLONG nIndex = m_objExtensions.GetIndexOfKey(strName);

    if (nIndex < 0)
    {
        // new one
        return m_objExtensions.Add(strName, strValue);
    }

    return m_objExtensions.SetValueAt(nIndex, strValue);
}

PUBLIC
IMS_BOOL SipSecurityHeader::SetUnknownParameterValue(
        IN IMS_SINT32 nName, IN const AString& strValue)
{
    IMS_SLONG nIndex = m_objUnknownParamValues.GetIndexOfKey(nName);

    if (nIndex < 0)
    {
        // new one
        return m_objUnknownParamValues.Add(nName, strValue);
    }

    return m_objUnknownParamValues.SetValueAt(nIndex, strValue);
}

PUBLIC
AString SipSecurityHeader::ToString() const
{
    AStringBuffer objBuffer(256);

    if (m_nMechanism == MECHANISM_IPSEC_3GPP)
        objBuffer.Append(P_VALUE_MECHANISM_IPSEC_3GPP);
    else if (m_nMechanism == MECHANISM_DIGEST)
        objBuffer.Append(P_VALUE_MECHANISM_DIGEST);
    else if (m_nMechanism == MECHANISM_TLS)
        objBuffer.Append(P_VALUE_MECHANISM_TLS);
    else if (m_nMechanism == MECHANISM_IPSEC_IKE)
        objBuffer.Append(P_VALUE_MECHANISM_IPSEC_IKE);
    else if (m_nMechanism == MECHANISM_IPSEC_MAN)
        objBuffer.Append(P_VALUE_MECHANISM_IPSEC_MAN);
    else
        objBuffer.Append(m_strMechanism);

    if (!m_bParameterRequired)
    {
        // When the additional parameter is not required...
        return static_cast<const AStringBuffer&>(objBuffer).GetString();
    }

    // preference ('q')
    if (m_strPreference.GetLength() > 0)
    {
        objBuffer.Append(TextParser::CHAR_SEMICOLON);
        objBuffer.Append(P_NAME_PREFERENCE);
        objBuffer.Append(TextParser::CHAR_EQUAL);
        objBuffer.Append(m_strPreference);
    }

    // algorithm
    if (m_nAlgorithm != ALG_UNSPECIFIED)
    {
        AString strAlg;

        if (m_nAlgorithm == ALG_HMAC_MD5_96)
        {
            strAlg = P_VALUE_ALG_HMAC_MD5_96;
        }
        else if (m_nAlgorithm == ALG_HMAC_SHA_1_96)
        {
            strAlg = P_VALUE_ALG_HMAC_SHA_1_96;
        }
        else if (m_nAlgorithm == ALG_UNKNOWN)
        {
            strAlg = GetUnknownParameterValue(SEC_P_ALG);
        }

        if (strAlg.GetLength() > 0)
        {
            objBuffer.Append(TextParser::CHAR_SEMICOLON);
            objBuffer.Append(P_NAME_ALGORITHM);
            objBuffer.Append(TextParser::CHAR_EQUAL);
            objBuffer.Append(strAlg);
        }
    }

    // protocol
    if (m_nProtocol != PROTOCOL_UNSPECIFIED)
    {
        AString strProtocol;

        if (m_nProtocol == PROTOCOL_AH)
        {
            strProtocol = P_VALUE_PROT_AH;
        }
        else if (m_nProtocol == PROTOCOL_ESP)
        {
            strProtocol = P_VALUE_PROT_ESP;
        }
        else if (m_nProtocol == PROTOCOL_UNKNOWN)
        {
            strProtocol = GetUnknownParameterValue(SEC_P_PROTOCOL);
        }

        if (strProtocol.GetLength() > 0)
        {
            objBuffer.Append(TextParser::CHAR_SEMICOLON);
            objBuffer.Append(P_NAME_PROTOCOL);
            objBuffer.Append(TextParser::CHAR_EQUAL);
            objBuffer.Append(strProtocol);
        }
    }

    // mode
    if (m_nMode != MODE_UNSPECIFIED)
    {
        AString strMode;

        if (m_nMode == MODE_TUNNEL)
        {
            strMode = P_VALUE_MOD_TUN;
        }
        else if (m_nMode == MODE_TRANSPORT)
        {
            strMode = P_VALUE_MOD_TRANS;
        }
        else if (m_nMode == MODE_UDP_ENC_TUN)
        {
            strMode = P_VALUE_MOD_UDP_ENC_TUN;
        }
        else if (m_nMode == MODE_UNKNOWN)
        {
            strMode = GetUnknownParameterValue(SEC_P_MODE);
        }

        if (strMode.GetLength() > 0)
        {
            objBuffer.Append(TextParser::CHAR_SEMICOLON);
            objBuffer.Append(P_NAME_MODE);
            objBuffer.Append(TextParser::CHAR_EQUAL);
            objBuffer.Append(strMode);
        }
    }

    // encryption algorithm
    if (m_nEncryptionAlgorithm != EALG_UNSPECIFIED)
    {
        AString strEAlg;

        if (m_nEncryptionAlgorithm == EALG_DES_EDE3_CBC)
        {
            strEAlg = P_VALUE_EALG_DES_EDE3_CBC;
        }
        else if (m_nEncryptionAlgorithm == EALG_AES_CBC)
        {
            strEAlg = P_VALUE_EALG_AES_CBC;
        }
        else if (m_nEncryptionAlgorithm == EALG_NULL)
        {
            strEAlg = P_VALUE_EALG_NULL;
        }
        else if (m_nEncryptionAlgorithm == EALG_UNKNOWN)
        {
            strEAlg = GetUnknownParameterValue(SEC_P_EALG);
        }

        if (strEAlg.GetLength() > 0)
        {
            objBuffer.Append(TextParser::CHAR_SEMICOLON);
            objBuffer.Append(P_NAME_ENCRYPTION_ALGORITHM);
            objBuffer.Append(TextParser::CHAR_EQUAL);
            objBuffer.Append(strEAlg);
        }
    }

    // spi-c, spi-s
    AString strTmp;

    if (m_bSpi3gppCompliant)
    {
        if (IsParameterPresent(SPI_C))
        {
            strTmp.Sprintf(";spi-c=%010u", m_nSpiC);
            objBuffer.Append(strTmp);
        }

        if (IsParameterPresent(SPI_S))
        {
            strTmp.Sprintf(";spi-s=%010u", m_nSpiS);
            objBuffer.Append(strTmp);
        }
    }
    else
    {
        if (IsParameterPresent(SPI_C))
        {
            strTmp.Sprintf(";spi-c=%u", m_nSpiC);
            objBuffer.Append(strTmp);
        }

        if (IsParameterPresent(SPI_S))
        {
            strTmp.Sprintf(";spi-s=%u", m_nSpiS);
            objBuffer.Append(strTmp);
        }
    }

    // port-c, port-s
    if (IsParameterPresent(PORT_C))
    {
        strTmp.Sprintf(";port-c=%d", m_nPortC);
        objBuffer.Append(strTmp);
    }

    if (IsParameterPresent(PORT_S))
    {
        strTmp.Sprintf(";port-s=%d", m_nPortS);
        objBuffer.Append(strTmp);
    }

    // extensions
    for (IMS_UINT32 i = 0; i < m_objExtensions.GetSize(); ++i)
    {
        const AString& strName = m_objExtensions.GetKeyAt(i);
        const AString& strValue = m_objExtensions.GetValueAt(i);

        objBuffer.Append(TextParser::CHAR_SEMICOLON);
        objBuffer.Append(strName);

        if (strValue.GetLength() > 0)
        {
            objBuffer.Append(TextParser::CHAR_EQUAL);
            objBuffer.Append(strValue);
        }
    }

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

PUBLIC GLOBAL SipSecurityHeader* SipSecurityHeader::FromSipHeader(IN ISipHeader* piHeader)
{
    if (piHeader == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_SINT32 nType = piHeader->GetType();

    if ((nType != ISipHeader::SECURITY_CLIENT) && (nType != ISipHeader::SECURITY_SERVER) &&
            (nType != ISipHeader::SECURITY_VERIFY))
    {
        return IMS_NULL;
    }

    IMS_SINT32 nMechanism = MECHANISM_UNKNOWN;
    const AString& strMechanism = piHeader->GetValue();

    if (strMechanism.GetLength() == 0)
    {
        return IMS_NULL;
    }

    if (strMechanism.EqualsIgnoreCase(P_VALUE_MECHANISM_IPSEC_3GPP))
        nMechanism = MECHANISM_IPSEC_3GPP;
    else if (strMechanism.EqualsIgnoreCase(P_VALUE_MECHANISM_DIGEST))
        nMechanism = MECHANISM_DIGEST;
    else if (strMechanism.EqualsIgnoreCase(P_VALUE_MECHANISM_TLS))
        nMechanism = MECHANISM_TLS;
    else if (strMechanism.EqualsIgnoreCase(P_VALUE_MECHANISM_IPSEC_IKE))
        nMechanism = MECHANISM_IPSEC_IKE;
    else if (strMechanism.EqualsIgnoreCase(P_VALUE_MECHANISM_IPSEC_MAN))
        nMechanism = MECHANISM_IPSEC_MAN;

    SipSecurityHeader* pSecurityHeader = new SipSecurityHeader(nMechanism, strMechanism);

    if (pSecurityHeader == IMS_NULL)
    {
        return IMS_NULL;
    }

    pSecurityHeader->m_nAlgorithm = ALG_UNSPECIFIED;
    pSecurityHeader->m_nProtocol = PROTOCOL_UNSPECIFIED;
    pSecurityHeader->m_nMode = MODE_UNSPECIFIED;
    pSecurityHeader->m_nEncryptionAlgorithm = EALG_UNSPECIFIED;

    const IMSList<SipParameter*>& objParameters = piHeader->GetParameters();

    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
    {
        const SipParameter* pParameter = objParameters.GetAt(i);
        const AString& strName = pParameter->GetName();
        const AString& strValue = pParameter->GetValue();

        // algorithm
        if (strName.EqualsIgnoreCase(P_NAME_ALGORITHM))
        {
            if (strValue.EqualsIgnoreCase(P_VALUE_ALG_HMAC_MD5_96))
            {
                pSecurityHeader->m_nAlgorithm = ALG_HMAC_MD5_96;
            }
            else if (strValue.EqualsIgnoreCase(P_VALUE_ALG_HMAC_SHA_1_96))
            {
                pSecurityHeader->m_nAlgorithm = ALG_HMAC_SHA_1_96;
            }
            else
            {
                pSecurityHeader->m_nAlgorithm = ALG_UNKNOWN;
                pSecurityHeader->SetUnknownParameterValue(SEC_P_ALG, strValue);
            }
        }
        // protocol
        else if (strName.EqualsIgnoreCase(P_NAME_PROTOCOL))
        {
            if (strValue.EqualsIgnoreCase(P_VALUE_PROT_AH))
            {
                pSecurityHeader->m_nProtocol = PROTOCOL_AH;
            }
            else if (strValue.EqualsIgnoreCase(P_VALUE_PROT_ESP))
            {
                pSecurityHeader->m_nProtocol = PROTOCOL_ESP;
            }
            else
            {
                pSecurityHeader->m_nProtocol = PROTOCOL_UNKNOWN;
                pSecurityHeader->SetUnknownParameterValue(SEC_P_PROTOCOL, strValue);
            }
        }
        // mode
        else if (strName.EqualsIgnoreCase(P_NAME_MODE))
        {
            if (strValue.EqualsIgnoreCase(P_VALUE_MOD_TUN))
            {
                pSecurityHeader->m_nMode = MODE_TUNNEL;
            }
            else if (strValue.EqualsIgnoreCase(P_VALUE_MOD_TRANS))
            {
                pSecurityHeader->m_nMode = MODE_TRANSPORT;
            }
            else if (strValue.EqualsIgnoreCase(P_VALUE_MOD_UDP_ENC_TUN))
            {
                pSecurityHeader->m_nMode = MODE_UDP_ENC_TUN;
            }
            else
            {
                pSecurityHeader->m_nMode = MODE_UNKNOWN;
                pSecurityHeader->SetUnknownParameterValue(SEC_P_MODE, strValue);
            }
        }
        // preference ('q')
        else if (strName.EqualsIgnoreCase(P_NAME_PREFERENCE))
        {
            pSecurityHeader->m_strPreference = pParameter->GetValue();
        }
        // encryption-algorithm
        else if (strName.EqualsIgnoreCase(P_NAME_ENCRYPTION_ALGORITHM))
        {
            if (strValue.EqualsIgnoreCase(P_VALUE_EALG_DES_EDE3_CBC))
            {
                pSecurityHeader->m_nEncryptionAlgorithm = EALG_DES_EDE3_CBC;
            }
            else if (strValue.EqualsIgnoreCase(P_VALUE_EALG_AES_CBC))
            {
                pSecurityHeader->m_nEncryptionAlgorithm = EALG_AES_CBC;
            }
            else if (strValue.EqualsIgnoreCase(P_VALUE_EALG_NULL))
            {
                pSecurityHeader->m_nEncryptionAlgorithm = EALG_NULL;
            }
            else
            {
                pSecurityHeader->m_nEncryptionAlgorithm = EALG_UNKNOWN;
                pSecurityHeader->SetUnknownParameterValue(SEC_P_EALG, strValue);
            }
        }
        // spi-c
        else if (strName.EqualsIgnoreCase(P_NAME_SPI_C))
        {
            pSecurityHeader->m_nSpiC = pParameter->GetValue().ToUInt32();
            pSecurityHeader->m_nMechParams |= SPI_C;

            if (pParameter->GetValue().GetLength() < SPI_MAX_DIGITS)
            {
                pSecurityHeader->m_bSpi3gppCompliant = IMS_FALSE;
            }
        }
        // spi-s
        else if (strName.EqualsIgnoreCase(P_NAME_SPI_S))
        {
            pSecurityHeader->m_nSpiS = pParameter->GetValue().ToUInt32();
            pSecurityHeader->m_nMechParams |= SPI_S;

            if (pParameter->GetValue().GetLength() < SPI_MAX_DIGITS)
            {
                pSecurityHeader->m_bSpi3gppCompliant = IMS_FALSE;
            }
        }
        // port-c
        else if (strName.EqualsIgnoreCase(P_NAME_PORT_C))
        {
            pSecurityHeader->m_nPortC = pParameter->GetValue().ToInt32();
            pSecurityHeader->m_nMechParams |= PORT_C;
        }
        // port-s
        else if (strName.EqualsIgnoreCase(P_NAME_PORT_S))
        {
            pSecurityHeader->m_nPortS = pParameter->GetValue().ToInt32();
            pSecurityHeader->m_nMechParams |= PORT_S;
        }
        // extensions
        else
        {
            pSecurityHeader->SetExtensionParameter(strName, pParameter->GetValue());
        }
    }

    return pSecurityHeader;
}
