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
#ifndef SIP_SECURITY_HEADER_H_
#define SIP_SECURITY_HEADER_H_

#include "AString.h"
#include "IMSMap.h"

class ISipHeader;

/**
 * @brief This class provides an interface to access/control SIP security headers.
 *
 * @see ISipHeader
 */
class SipSecurityHeader
{
public:
    explicit SipSecurityHeader(IN IMS_SINT32 nMechanism = MECHANISM_IPSEC_3GPP,
            IN const AString& strMechanism = AString::ConstNull(),
            IN IMS_BOOL bParameterRequired = IMS_TRUE);
    SipSecurityHeader(IN const SipSecurityHeader& other);
    ~SipSecurityHeader();

public:
    SipSecurityHeader& operator=(IN const SipSecurityHeader& other);

public:
    /**
     * @brief Gets a security mechanism.
     *
     * @return A security mechanism.\n
     *         #MECHANISM_IPSEC_3GPP
     */
    inline IMS_SINT32 GetMechanism() const { return m_nMechanism; }

    /**
     * @brief Gets an unknown security mechanism.
     *
     * @return An unknown security mechanism.
     */
    inline const AString& GetUnknownMechanism() const { return m_strMechanism; }

    /**
     * @brief Gets "q" parameter value.
     *
     * @return A preference value.
     */
    inline const AString& GetPreference() const { return m_strPreference; }

    // 3GPP
    /**
     * @brief Gets the integrity algorithm.
     *
     * @return An integrity algorithm.\n
     *         #ALG_HMAC_SHA_1_96\n
     *         #ALG_HMAC_MD5_96
     */
    inline IMS_SINT32 GetAlgorithm() const { return m_nAlgorithm; }

    /**
     * @brief Gets the encryption algorithm.
     *
     * @return An encryption algorithm.\n
     *         #EALG_DES_EDE3_CBC\n
     *         #EALG_AES_CBC\n
     *         #EALG_NULL
     */
    inline IMS_SINT32 GetEncryptionAlgorithm() const { return m_nEncryptionAlgorithm; }

    /**
     * @brief Gets the security mode.
     *
     * @return A security mode.\n
     *         #MODE_TRANSPORT\n
     *         #MODE_TUNNEL
     */
    inline IMS_SINT32 GetMode() const { return m_nMode; }

    /**
     * @brief Gets the security client port number.
     *
     * @return A security client port number.
     */
    inline IMS_SINT32 GetPortC() const { return m_nPortC; }

    /**
     * @brief Gets the security server port number.
     *
     * @return A security server port number.
     */
    inline IMS_SINT32 GetPortS() const { return m_nPortS; }

    /**
     * @brief Gets the security protocol.
     *
     * @return A security protocol.\n
     *         #PROTOCOL_ESP\n
     *         #PROTOCOL_AH
     */
    inline IMS_SINT32 GetProtocol() const { return m_nProtocol; }

    /**
     * @brief Gets the security parameter index for client side.
     *
     * @return A security parameter index.
     */
    inline IMS_UINT32 GetSpiC() const { return m_nSpiC; }

    /**
     * @brief Gets the security parameter index for server side.
     *
     * @return A security parameter index.
     */
    inline IMS_UINT32 GetSpiS() const { return m_nSpiS; }

    /**
     * @brief Gets the extension parameters.
     *
     * @return Map of <key,value> pair.
     */
    inline const IMSMap<AString,AString>& GetExtensionParameters() const
    { return m_objExtensions; }

    /**
     * @brief Gets the unknown parameter value.
     *
     * @param nName The parameter name\n
     *              #SEC_P_ALG\n
     *              #SEC_P_PROTOCOL\n
     *              #SEC_P_MODE\n
     *              #SEC_P_EALG
     * @return Unknown parameter value.
     */
    const AString& GetUnknownParameterValue(IN IMS_SINT32 nName/*SEC_P_XXX*/) const;

    /**
     * @brief Gets the unknown parameter values.
     *
     * @return All the unknown parameter values.
     */
    inline const IMSMap<IMS_SINT32, AString>& GetUnknownParameterValues() const
    { return m_objUnknownParamValues; }

    /**
     * @brief Checks if the spi / port are present or not.
     *
     * @param nParam The parameter to be evaluated\n
     *               #PORT_C\n
     *               #PORT_S\n
     *               #SPI_C\n
     *               #SPI_S
     * @return If the given parameter is present, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsParameterPresent(IN IMS_SINT32 nParam) const
    { return ((m_nMechParams & nParam) != 0); }

    /**
     * @brief Sets "q" parameter value.
     *
     * @param strPreference "q" parameter value to be set
     */
    inline void SetPreference(IN const AString& strPreference)
    { m_strPreference = strPreference; }

    /**
     * @brief Sets the integrity algorithm.
     *
     * @param nAlgorithm The integrity algorithm to be set\n
     *                   #ALG_HMAC_SHA_1_96\n
     *                   #ALG_HMAC_MD5_96
     */
    inline void SetAlgorithm(IN IMS_SINT32 nAlgorithm)
    { m_nAlgorithm = nAlgorithm; }

    /**
     * @brief Sets the encryption algorithm.
     *
     * @param nEncryptionAlgorithm The encryption algorithm to be set\n
     *                             #EALG_DES_EDE3_CBC\n
     *                             #EALG_AES_CBC\n
     *                             #EALG_NULL
     */
    inline void SetEncryptionAlgorithm(IN IMS_SINT32 nEncryptionAlgorithm)
    { m_nEncryptionAlgorithm = nEncryptionAlgorithm; }

    /**
     * @brief Sets the security mode.
     *
     * @param nMode The security mode to be set\n
     *              #MODE_TRANSPORT\n
     *              #MODE_TUNNEL
     */
    inline void SetMode(IN IMS_SINT32 nMode)
    { m_nMode = nMode; }

    /**
     * @brief Sets the client / server port number.
     *
     * @param nPortC The client port number
     * @param nPortS The server port number
     */
    void SetPort(IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortS);

    /**
     * @brief Sets the security protocol.
     *
     * @param nProtocol The security protocol to be set\n
     *                  #PROTOCOL_ESP\n
     *                  #PROTOCOL_AH
     */
    inline void SetProtocol (IN IMS_SINT32 nProtocol)
    { m_nProtocol = nProtocol; }

    /**
     * @brief Sets the security parameter index.
     *
     * @param nSpiC The security parameter index for client side
     * @param nSpiS The security parameter index for server side
     * @param b3gppCompliant Flag to indicate that the SPI value is compliant with 3GPP or not
     *                       If it's IMS_TRUE, it's compliant with 3GPP specification.
     */
    void SetSpi(IN IMS_UINT32 nSpiC, IN IMS_UINT32 nSpiS, IN IMS_BOOL b3gppCompliant = IMS_TRUE);

    /**
     * @brief Sets the option of the security parameter index.
     *
     * @param b3gppCompliant Flag to indicate that the SPI value is compliant with 3GPP or not
     *                       If it's IMS_TRUE, it's compliant with 3GPP specification.
     */
    inline void SetSpiOption(IN IMS_BOOL b3gppCompliant)
    { m_bSpi3gppCompliant = b3gppCompliant; }

    /**
     * @brief Sets the extension parameter.
     *
     * @param strName The extension parameter name
     * @param strValue The extension parameter value
     * @return If the parameter is successfully set, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL SetExtensionParameter(IN const AString& strName,
            IN const AString& strValue);

    /**
     * @brief Sets the unknown parameter.
     *
     * @param nName The unknown parameter name\n
     *              #SEC_P_ALG\n
     *              #SEC_P_PROTOCOL\n
     *              #SEC_P_MODE\n
     *              #SEC_P_EALG
     * @param strValue The unknown parameter value
     * @return If the parameter is successfully set, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL SetUnknownParameterValue(IN IMS_SINT32 nName/*SEC_P_XXX*/,
            IN const AString& strValue);

    /**
     * @brief Returns the string representation of SIP security header.
     *
     * @return A string representation of SIP security header.
     */
    AString ToString() const;

    /**
     * @brief Creates SipSecurityHeader object from SIP security header.
     *
     * @return The newly created SipSecurityHeader.
     */
    static SipSecurityHeader* FromSipHeader(IN ISipHeader* piHeader);

public:
    /// Pre-defined parameters\n
    /// It's used for GetUnknownParameterValue / SetUnknownParameterValue methods.
    enum
    {
        /// Integrity Algorithm
        SEC_P_ALG,
        /// Security Protocol
        SEC_P_PROTOCOL,
        /// Security Mode
        SEC_P_MODE,
        /// Encryption Algorithm
        SEC_P_EALG
    };

    /// Types of integrity algorithm
    enum
    {
        ALG_HMAC_MD5_96,
        ALG_HMAC_SHA_1_96,
        ALG_UNSPECIFIED,
        ALG_UNKNOWN
    };

    /// Types of security protocol
    enum
    {
        PROTOCOL_AH,
        PROTOCOL_ESP, // default
        PROTOCOL_UNSPECIFIED,
        PROTOCOL_UNKNOWN
    };

    /// Types of security mode
    enum
    {
        MODE_TRANSPORT, // default
        MODE_TUNNEL,
        MODE_UDP_ENC_TUN, // For the device behind of NAT
        MODE_UNSPECIFIED,
        MODE_UNKNOWN
    };

    /// Types of encryption algorithm
    enum
    {
        EALG_DES_EDE3_CBC,
        EALG_AES_CBC,
        EALG_NULL, // default
        EALG_UNSPECIFIED,
        EALG_UNKNOWN
    };

    /// Types of authentication scheme
    enum
    {
        MECHANISM_UNKNOWN,
        MECHANISM_DIGEST,
        MECHANISM_TLS,
        MECHANISM_IPSEC_IKE,
        MECHANISM_IPSEC_MAN,
        MECHANISM_IPSEC_3GPP
        // FIXME: security mechanism for media plane - sdes-srtp; mediasec="sdes-srtp"
    };

    /// To check if those parameters are present or not
    enum
    {
       PORT_C = 0x0001,
       PORT_S = 0x0002,
       SPI_C = 0x0004,
       SPI_S = 0x0008
    };

    enum { SPI_MAX_DIGITS = 10 };

    static const IMS_CHAR P_VALUE_MECHANISM_DIGEST[];
    static const IMS_CHAR P_VALUE_MECHANISM_TLS[];
    static const IMS_CHAR P_VALUE_MECHANISM_IPSEC_IKE[];
    static const IMS_CHAR P_VALUE_MECHANISM_IPSEC_MAN[];
    static const IMS_CHAR P_VALUE_MECHANISM_IPSEC_3GPP[];
    static const IMS_CHAR P_VALUE_ALG_HMAC_MD5_96[];
    static const IMS_CHAR P_VALUE_ALG_HMAC_SHA_1_96[];
    static const IMS_CHAR P_VALUE_PROT_AH[];
    static const IMS_CHAR P_VALUE_PROT_ESP[];
    static const IMS_CHAR P_VALUE_MOD_TRANS[];
    static const IMS_CHAR P_VALUE_MOD_TUN[];
    static const IMS_CHAR P_VALUE_MOD_UDP_ENC_TUN[];
    static const IMS_CHAR P_VALUE_EALG_DES_EDE3_CBC[];
    static const IMS_CHAR P_VALUE_EALG_AES_CBC[];
    static const IMS_CHAR P_VALUE_EALG_NULL[];

private:
    static const IMS_CHAR P_NAME_PREFERENCE[];
    // 3GPP - TS 33.203
    static const IMS_CHAR P_NAME_ALGORITHM[];
    static const IMS_CHAR P_NAME_PROTOCOL[];
    static const IMS_CHAR P_NAME_MODE[];
    static const IMS_CHAR P_NAME_ENCRYPTION_ALGORITHM[];
    static const IMS_CHAR P_NAME_SPI_C[];
    static const IMS_CHAR P_NAME_SPI_S[];
    static const IMS_CHAR P_NAME_PORT_C[];
    static const IMS_CHAR P_NAME_PORT_S[];

    // Flag to indicate if the additional parameter should be formed or not
    IMS_BOOL m_bParameterRequired;

    IMS_SINT32 m_nMechanism;
    AString m_strMechanism;
    AString m_strPreference;
    IMS_SINT32 m_nAlgorithm; // Mandatory field
    IMS_SINT32 m_nProtocol;
    IMS_SINT32 m_nMode;
    IMS_SINT32 m_nEncryptionAlgorithm;
    IMS_UINT32 m_nSpiC;
    IMS_UINT32 m_nSpiS;
    IMS_SINT32 m_nPortC;
    IMS_SINT32 m_nPortS;
    // <pre-defined parameter, value>
    IMSMap<IMS_SINT32, AString> m_objUnknownParamValues;
    IMSMap<AString, AString> m_objExtensions;

    // To check the parameter's presentity : spi, port
    IMS_SINT32 m_nMechParams;

    // Optional informations
    // Flag to indicate how to form the SPI value
    //    IMS_TRUE : 10DIGIT (3GPP-based), IMS_FALSE : non-10DIGIT
    IMS_BOOL m_bSpi3gppCompliant;
};

#endif
