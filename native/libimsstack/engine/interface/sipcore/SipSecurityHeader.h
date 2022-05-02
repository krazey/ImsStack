#ifndef _SIP_SECURITY_HEADER_H_
#define _SIP_SECURITY_HEADER_H_

#include "IMSMap.h"
#include "AString.h"

class ISIPHeader;

/**
 * @brief This class provides an interface to access/control SIP security headers.
 *
 * @see ISIPHeader
 */
class SIPSecurityHeader
{
public:
    explicit SIPSecurityHeader(IN IMS_SINT32 nMechanism_ = MECHANISM_IPSEC_3GPP,
            IN CONST AString &strMechanism_ = AString::ConstNull(),
            IN IMS_BOOL bParameterRequired_ = IMS_TRUE);
    SIPSecurityHeader(IN CONST SIPSecurityHeader &objRHS);
    ~SIPSecurityHeader();

public:
    SIPSecurityHeader& operator=(IN CONST SIPSecurityHeader &objRHS);

public:
    /**
     * @brief Gets a security mechanism.
     *
     * @return A security mechanism.\n
     *         #MECHANISM_IPSEC_3GPP
     */
    IMS_SINT32 GetMechanism() const;
    /**
     * @brief Gets an unknown security mechanism.
     *
     * @return An unknown security mechanism.
     */
    const AString& GetUnknownMechanism() const;
    /**
     * @brief Gets "q" parameter value.
     *
     * @return A preference value.
     */
    const AString& GetPreference() const;
    // 3GPP
    /**
     * @brief Gets the integrity algorithm.
     *
     * @return An integrity algorithm.\n
     *         #ALG_HMAC_SHA_1_96\n
     *         #ALG_HMAC_MD5_96
     */
    IMS_SINT32 GetAlgorithm() const;
    /**
     * @brief Gets the encryption algorithm.
     *
     * @return An encryption algorithm.\n
     *         #EALG_DES_EDE3_CBC\n
     *         #EALG_AES_CBC\n
     *         #EALG_NULL
     */
    IMS_SINT32 GetEncryptionAlgorithm() const;
    /**
     * @brief Gets the security mode.
     *
     * @return A security mode.\n
     *         #MODE_TRANSPORT\n
     *         #MODE_TUNNEL
     */
    IMS_SINT32 GetMode() const;
    /**
     * @brief Gets the security client port number.
     *
     * @return A security client port number.
     */
    IMS_SINT32 GetPortC() const;
    /**
     * @brief Gets the security server port number.
     *
     * @return A security server port number.
     */
    IMS_SINT32 GetPortS() const;
    /**
     * @brief Gets the security protocol.
     *
     * @return A security protocol.\n
     *         #PROTOCOL_ESP\n
     *         #PROTOCOL_AH
     */
    IMS_SINT32 GetProtocol() const;
    /**
     * @brief Gets the security parameter index for client side.
     *
     * @return A security parameter index.
     */
    IMS_UINT32 GetSPIC() const;
    /**
     * @brief Gets the security parameter index for server side.
     *
     * @return A security parameter index.
     */
    IMS_UINT32 GetSPIS() const;
    /**
     * @brief Gets the extension parameters.
     *
     * @return Map of <key,value> pair.
     */
    const IMSMap<AString,AString>& GetExtensionParameters() const;
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
    const AString& GetUnknownParameterValue(IN IMS_SINT32 nName /* SEC_P_XXX */) const;
    /**
     * @brief Gets the unknown parameter values.
     *
     * @return All the unknown parameter values.
     */
    const IMSMap<IMS_SINT32, AString>& GetUnknownParameterValues() const;
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
    IMS_BOOL IsParameterPresent(IN IMS_SINT32 nParam) const;
    /**
     * @brief Sets "q" parameter value.
     *
     * @param strPreference "q" parameter value to be set
     */
    void SetPreference(IN CONST AString &strPreference);
    /**
     * @brief Sets the integrity algorithm.
     *
     * @param nAlgorithm The integrity algorithm to be set\n
     *                   #ALG_HMAC_SHA_1_96\n
     *                   #ALG_HMAC_MD5_96
     */
    void SetAlgorithm(IN IMS_SINT32 nAlgorithm);
    /**
     * @brief Sets the encryption algorithm.
     *
     * @param nEncryptionAlgorithm The encryption algorithm to be set\n
     *                             #EALG_DES_EDE3_CBC\n
     *                             #EALG_AES_CBC\n
     *                             #EALG_NULL
     */
    void SetEncryptionAlgorithm(IN IMS_SINT32 nEncryptionAlgorithm);
    /**
     * @brief Sets the security mode.
     *
     * @param nMode The security mode to be set\n
     *              #MODE_TRANSPORT\n
     *              #MODE_TUNNEL
     */
    void SetMode(IN IMS_SINT32 nMode);
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
    void SetProtocol (IN IMS_SINT32 nProtocol);
    /**
     * @brief Sets the security parameter index.
     *
     * @param nSPIC The security parameter index for client side
     * @param nSPIS The security parameter index for server side
     * @param bSPI_3GPP Flag to indicate that the SPI value is compliant with 3GPP or not
     *                  If it's IMS_TRUE, it's compliant with 3GPP specification.
     */
    void SetSPI(IN IMS_UINT32 nSPIC, IN IMS_UINT32 nSPIS, IN IMS_BOOL bSPI_3GPP = IMS_TRUE);
    /**
     * @brief Sets the option of the security parameter index.
     *
     * @param bSPI_3GPP Flag to indicate that the SPI value is compliant with 3GPP or not
     *                  If it's IMS_TRUE, it's compliant with 3GPP specification.
     */
    void SetSPIOption(IN IMS_BOOL bSPI_3GPP);
    /**
     * @brief Sets the extension parameter.
     *
     * @param strName The extension parameter name
     * @param strValue The extension parameter value
     * @return If the parameter is successfully set, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL SetExtensionParameter(IN CONST AString &strName,
            IN CONST AString &strValue);
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
    IMS_BOOL SetUnknownParameterValue(IN IMS_SINT32 nName /* SEC_P_XXX */,
            IN CONST AString &strValue);
    /**
     * @brief Returns the string representation of SIP security header.
     *
     * @return A string representation of SIP security header.
     */
    AString ToString() const;

    /**
     * @brief Creates SIPSecurityHeader object from SIP security header.
     *
     * @return The newly created SIPSecurityHeader.
     */
    static SIPSecurityHeader* FromSIPHeader(IN ISIPHeader *piHeader);

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
    IMS_BOOL bParameterRequired;

    IMS_SINT32 nMechanism;
    AString strMechanism;
    AString strPreference;
    IMS_SINT32 nAlgorithm;                      // Mandatory field
    IMS_SINT32 nProtocol;
    IMS_SINT32 nMode;
    IMS_SINT32 nEncryptionAlgorithm;
    IMS_UINT32 nSPI_C;
    IMS_UINT32 nSPI_S;
    IMS_SINT32 nPort_C;
    IMS_SINT32 nPort_S;
    // <pre-defined parameter, value>
    IMSMap<IMS_SINT32, AString> objUnknownParamValues;
    IMSMap<AString, AString> objExtensions;

    // To check the parameter's presentity : spi, port
    IMS_SINT32 nMechParams;

    // Optional informations
    // Flag to indicate how to form the SPI value
    //    IMS_TRUE : 10DIGIT (3GPP-based), IMS_FALSE : non-10DIGIT
    IMS_BOOL bSPI_3GPP;
};

#endif // _SIP_SECURITY_HEADER_H_
