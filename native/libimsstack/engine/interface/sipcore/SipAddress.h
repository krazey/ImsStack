#ifndef _SIP_ADDRESS_H_
#define _SIP_ADDRESS_H_

#include "AString.h"

class ISIPHeader;
class SIPParameter;

/**
 * @brief This class provides a generic SIP address parser.
 *
 * It can be used to parse either full SIP name addresses like:
 *     - IMSer <sip:user@ims.com>
 *     - sip:+11234567890@ims.com;user=phone
 *     - sips:user@ims.com;transport=tcp
 *
 * It does not escape address strings.\n
 * It ignores header part of SIP URI.\n
 * Its valid scheme format is the same as defined in SIP BNF for absolute URI.\n
 * It allows "*" as the valid address of Contact header.\n
 * In this case, ToString() & GetURI() return "*", all other accessor methods return null or 0.
 */
class SIPAddress
{
public:
    // Extra field to retrieve the proper token (for SIP/SIPS URIs)
    class UserInfoPart
    {
    public:
        UserInfoPart();
        UserInfoPart(IN CONST UserInfoPart &objRHS);
        ~UserInfoPart();

    public:
        UserInfoPart& operator=(IN CONST UserInfoPart &objRHS);

    public:
        /**
         * @brief Parses the userinfo part of SIP/SIPS URI.
         *
         * @param strUserInfo Userinfo part string
         * @return If the userinfo part is successfully parsed, returns IMS_TRUE.
         *         Otherwise, returns IMS_FALSE.
         */
        IMS_BOOL Create(IN CONST AString &strUserInfo);
        /**
         * @brief Gets the user field.
         *
         * @return User string.
         */
        inline const AString& GetUser() const
        { return strUser; }
        /**
         * @brief Gets the password field.
         *
         * @return Password string.
         */
        inline const AString& GetPassword() const
        { return strPassword; }
        /**
         * @brief Returns the parameter of user-info part.
         *
         * All the parameters are separated by the semi-colon.
         *
         * @return Pointer to SIPParameter.
         */
        const SIPParameter* GetParameter(IN CONST AString &strName) const;
        /**
         * @brief Gets all the parameters.
         *
         * @return List of pointer to SIPParameter.
         */
        inline const IMSList<SIPParameter*>& GetParameters() const
        { return objParameters; }

    private:
        void RemoveAllParameters();

    private:
        AString strUser;
        AString strPassword;
        IMSList<SIPParameter*> objParameters;
    };

public:
    SIPAddress();
    explicit SIPAddress(IN CONST AString &strAddress);
    SIPAddress(IN CONST AString &strDisplayName_, IN CONST AString &strURI);
    // Reference count
    SIPAddress(IN CONST SIPAddress &objRHS);
    ~SIPAddress();

public:
    SIPAddress& operator=(IN CONST SIPAddress &objRHS);

public:
    /**
     * @brief Adds the named URI parameter to the specified value.
     *
     * If the value is null, the parameter is interpreted as a parameter without value.
     * The parameter value will be added.
     *
     * @param strName The named URI parameter
     * @param strValue The URI parameter value to set
     * @return If the URI parameter is successfully set, returns IMS_SUCCESS.
     *         If the name is null, invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT AddParameter(IN CONST AString &strName, IN CONST AString &strValue);
    /**
     * @brief Parses the SIP address format string.
     *
     * The string can be either following:
     *     - Name address: DisplayName <sip:user:password@host:port;uri-parameters>
     *     - Plain SIP URI: sip:user:password@host:port;uri-parameters
     *     - Special address: "*"
     *
     * @param strAddress SIP address format string
     * @return If the address is successfully parsed, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Create(IN CONST AString &strAddress);
    /**
     * @brief Checks if the given SIPAddress is the same.
     *
     * @param objAddress SIP address to be compared
     * @return If both SIP addresses matched, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Equals(IN CONST SIPAddress &objAddress) const;
    /**
     * @brief Returns the display name of SIP addresss.
     *
     * @return The display name if it is available.\n
     *         The null string if it is not available or the address is a special ("*") value.
     */
    const AString& GetDisplayName() const;
    /**
     * @brief Returns a header of SIP address which matches with the specified header type.
     *
     * @param nType The defined SIP header type\n
     *              If this value is ISIPHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param strName The header name, either in full or compact form
     * @return Pointer to uri-header parameter.
     */
    const ISIPHeader* GetHeader(IN IMS_SINT32 nType,
            IN CONST AString &strName = AString::ConstNull()) const;
    /**
     * @brief Returns all the headers of SIP address.
     *
     * @return List of uri-header parameter.
     */
    const IMSList<ISIPHeader*>& GetHeaders() const;
    /**
     * @brief Returns the host part of SIP address.
     *
     * @return The host part of this address.\n
     *         The null string if the address is the special ("*") value.
     */
    const AString& GetHost() const;
    /**
     * @brief Returns the value associated with the named URI parameter.
     *
     * @param strName The name of URI parameter
     * @return SIPParameter to the named URI parameter,
     *         or null if the named URI parameter is not found.
     */
    const SIPParameter* GetParameter(IN CONST AString &strName) const;
    /**
     * @brief Returns all the uri-parameters of SIP address.
     *
     * @return List of uri-parameter.
     */
    const IMSList<SIPParameter*>& GetParameters() const;
    /**
     * @brief Returns the port number of the SIP address.
     *
     * If the port number is not set, the function returns 5060 for "sip" scheme and
     * 5061 for "sips" scheme. If the address is wildcard ("*"), it returns 0.
     *
     * @return The value which is explicitly set or default value (5060 or 5061).\n
     *         Zero if the schems is not SIP or SIPS or the address is the special ("*") value.
     */
    IMS_SINT32 GetPort() const;
    /**
     * @brief Returns the scheme of SIP address.
     *
     * @return The scheme of this SIP address (sip/sips/tel/pres/im/...).\n
     *         The null string if the address is the special ("*") value.
     */
    const AString& GetScheme() const;
    /**
     * @brief Returns the URI part of SIP address (without parameters).
     *
     * The URI part of the address is of the form "scheme:user@host:port".
     *
     * @return The URI part of this SIP address.\n
     *         "*" if the address is the special ("*") value.
     */
    AString GetURI() const;
    /**
     * @brief Returns the user part of SIP address.
     *
     * There is no separate method for getting the password field
     * so that if the password field is present in the address, then the function returns the value
     * of "user:password" (instead of only "user").
     *
     * @return The user part of this SIP address.\n
     *         null string if the user part is missing or the address is the special ("*") value.
     */
    const AString& GetUser() const;
    /**
     * @brief Returns the pointer of UserInfoPart.
     *
     * (userinfo := (user / telephon-number) [ ":" password ])
     *
     * @return If userinfo part is present, returns the pointer of UserInfoPart.
     *         Otherwise, returns null.
     */
    const UserInfoPart* GetUserInfoPart() const;
    /**
     * @brief Returns the user part of SIP address.
     *
     * For SIP or SIPS, there is no separate method for getting the password field
     * so that if the password field is present in the address, then the function returns the value
     * of "user:password" (instead of only "user").\n
     * For the others, the funcation returns the host part of SIP address.
     *
     * @return The user part of this SIP address.\n
     *         null string if the user part is missing or the address is the special ("*") value.
     */
    const AString& GetUserPart() const;
    /**
     * @brief Checks if the port number is explicitly set or not.
     *
     * @return If the port number is not set explicitly, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsPortUnspecified() const;
    /**
     * @brief Checks if the scheme is SIP URI scheme or not.
     *
     * @return If the scheme is SIP URI scheme, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSchemeSIP() const;
    /**
     * @brief Checks if the scheme is SIPS URI scheme or not.
     *
     * @return If the scheme is SIPS URI scheme, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSchemeSIPS() const;
    /**
     * @brief Checks if the scheme is TEL URI scheme or not.
     *
     * @return If the scheme is TEL URI scheme, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSchemeTEL() const;
    /**
     * @brief Checks if the SIP address is for a service URN or not.
     *
     * @return If the SIP address is a service URN, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsServiceURN() const;
    /**
     * @brief Removes all the header components.
     */
    void RemoveAllHeaderComponents();
    /**
     * @brief Removes all the uri parameters.
     */
    void RemoveAllParameters();
    /**
     * @brief Removes the named URI parameter.
     *
     * The method returns without any action
     * if the named parameter is not defined or if the address is the special ("*") value.
     *
     * @param strName The name of the parameter to be removed
     */
    void RemoveParameter(IN CONST AString &strName);
    /**
     * @brief Sets the flag to indicate that AQUOT is required when forming URI
     *        even though any uri parameter doesn't exist.
     *
     * @param bAQUOTRequired Flag for AQUOT requirement
     */
    void SetAQUOTRequired(IN IMS_BOOL bAQUOTRequired);
    /**
     * @brief Sets the display name.
     *
     * Empty string or null removes the display name.
     *
     * @param strName The display name to be set
     * @return If the display name is successfully set, returns IMS_SUCCESS.\n
     *         If the display name is invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetDisplayName(IN CONST AString &strName);
    /**
     * @brief Sets the flag to indicate if DQUOT is required when forming display-name field.
     *
     * @param bDQUOTRequired Flag to indicate if DQUOTE should be included in display-name field
     */
    void SetDQUOTRequiredForDisplayName(IN IMS_BOOL bDQUOTRequired);
    /**
     * @brief Sets the header field.
     *
     * @param nType Header type as a enumeration
     * @param strValue Header value
     * @param strName Header name; it is valid if nType is UNKNOWN
     * @return If the headers is successfully set, returns IMS_SUCCESS.\n
     *         If the headers is invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
            IN CONST AString &strName = AString::ConstNull());
    /**
     * @brief Sets the headers field.
     *
     * @param strHeaders The headers field to be set\n
     *                   If the multiple headers set, each header will be identified by '&'.
     * @param bRemoveAll Flag to indicate if the existing header parameters are removed
     *                   or not before setting the header parameters
     * @return If the headers is successfully set, returns IMS_SUCCESS.\n
     *         If the headers is invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetHeaders(IN CONST AString &strHeaders, IN IMS_BOOL bRemoveAll = IMS_TRUE);
    /**
     * @brief Sets the host part of the SIP address.
     *
     * @param strHost The host part to be set
     * @return If the host part is successfully set, returns IMS_SUCCESS.\n
     *         If the host part is null, invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetHost(IN CONST AString &strHost);
    /**
     * @brief Sets the named URI parameter to the specified value.
     *
     * If the value is NULL, the parameter is interpreted as a parameter without value.\n
     * Existing parameter will be overwritten, otherwise the parameter is added.
     *
     * @param strName The named URI parameter
     * @param strValue The URI parameter value to be set
     * @return If the URI parameter is successfully set, returns IMS_SUCCESS.\n
     *         If the name is null, invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetParameter(IN CONST AString &strName, IN CONST AString &strValue);
    /**
     * @brief Sets the port number of the SIP address.
     *
     * The valid range is 0 ~ 65535.
     * After setting the port to 0, the port number is removed from the address URI,
     * GetPort() returns the default 5060 value for "sip" and 5061 for "sips" scheme.
     *
     * @param nPort Port number (0 ~ 65535) to be set
     * @return If the port number is successfully set, returns IMS_SUCCESS.\n
     *         If the port number is invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetPort(IN IMS_SINT32 nPort);
    /**
     * @brief Sets the scheme of the SIP address.
     *
     * @param strScheme The scheme format to be set
     * @return If the scheme is successfully set, returns IMS_SUCCESS.\n
     *         If the scheme is invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetScheme(IN CONST AString &strScheme);
    /**
     * @brief Sets the URI part of the SIP address (without parameter).
     *
     * The URI part of the address is of the form "scheme:user@host:port".
     * If the parameter of the method contains URI parameters,
     * they don't overwrite existing URI parameters, they are simply ignored if present.
     *
     * @param strURI The URI part of the address to be set
     * @return If the URI part is successfully set, returns IMS_SUCCESS.\n
     *         If the URI part is invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetURI(IN CONST AString &strURI);
    /**
     * @brief Sets the user part of the SIP address.
     *
     * Empty string or null removes the user part.
     *
     * @param strUser The user part of the address to be set
     * @return If the user part is successfully set, returns IMS_SUCCESS.\n
     *         If the user part is invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetUser(IN CONST AString &strUser);
    /**
     * @brief Returns a fully qualified SIP address, with display name, URI and URI parameters.
     *
     * If display name is not specified only a SIP URI is returned.\n
     * If the port is not explicitly set (to 5060 or other value), it will be omitted from
     * the address URI in returned string.
     *
     * @return Fully qualified SIP address if SIP/SIPS/TEL/...,
     *         or "*" if the address is the special ("*") value.
     */
    AString ToString() const;

    /**
     * @brief Returns the default list of SIPAddresses (empty list).
     *
     * @return Reference to the list of SIPAddresses (empty list).
     */
    static const IMSList<SIPAddress*>& ConstEmptyList();
    /**
     * @brief Returns the default SIPAddress (null).
     *
     * @return Reference to SIPAddress (null).
     */
    static const SIPAddress& ConstNull();
    /**
     * @brief Returns the tel URI format for the specified resource string.
     *
     * This method assumes that strResource does not contain any URI scheme
     * and leading / trailing WSP.
     *
     * @param strResource Resource string to be checked
     * @return Tel URI format.\n
     *         #TEL_FORMAT_NONE\n
     *         #TEL_FORMAT_GLOBAL\n
     *         #TEL_FORMAT_LOCAL
     */
    static IMS_SINT32 GetTelURIFormat(IN CONST AString &strResource);

private:
    IMS_BOOL CompareSIPURIs(IN CONST SIPAddress &objAddress) const;
    IMS_BOOL CompareTelURIs(IN CONST SIPAddress &objAddress) const;
    IMS_BOOL CompareTransportParameters(IN CONST SIPAddress &objAddress) const;
    const UserInfoPart* CreateUserInfoPart(IN CONST AString &strUserPart) const;
    IMS_BOOL Decode(IN CONST AString& strAddress, IN IMS_BOOL bParseParameter,
            IN IMS_BOOL bParseDisplayName = IMS_TRUE, IN IMS_BOOL bParseHeader = IMS_TRUE);
    IMS_BOOL DecodeHeaderComponent(IN CONST AString &strHeaders);
    IMS_BOOL IsParameterPresent(IN CONST AString &strName) const;

    static IMS_BOOL CompareNumberDigits(IN CONST AString &strDigits1, IN CONST AString &strDigits2);
    static AString EscapeDQUOTAndBackslash(IN CONST AString &strValue);
    static IMS_BOOL IsDisplayNameToken(IN CONST AString &strDisplayName);
    static IMS_BOOL IsToken(IN CONST IMS_CHAR ch);
    static IMS_BOOL IsVisualSeparator(IN CONST IMS_CHAR ch);

public:
    /// "maddr" URI parameter name
    static const IMS_CHAR PARAM_MADDR[];
    /// "method" URI parameter name
    static const IMS_CHAR PARAM_METHOD[];
    /// "phone-context" URI parameter name
    static const IMS_CHAR PARAM_PHONE_CONTEXT[];
    /// "transport" URI parameter name
    static const IMS_CHAR PARAM_TRANSPORT[];
    /// "ttl" URI parameter name
    static const IMS_CHAR PARAM_TTL[];
    /// "user" URI parameter name
    static const IMS_CHAR PARAM_USER[];

    /// Type to identify the tel URI format
    enum
    {
        TEL_FORMAT_NONE = 0,
        /// Global number format
        TEL_FORMAT_GLOBAL = 1,
        /// Local number format
        TEL_FORMAT_LOCAL = 2
    };

private:
    IMS_BOOL bIsWildcard;
    // DQUOT for display name
    IMS_BOOL bDQUOTForDisplayName;
    // Checks if AQUOT is a mandatory for URI forming
    IMS_BOOL bAQUOTRequired;
    AString strDisplayName;

    AString strScheme;
    AString strUserInfo;
    AString strHostInfo;
    IMS_UINT32 nPort;
    // Optional field
    //        headers := header *("&" header)
    //        header := hname "=" hvalue
    IMSList<ISIPHeader*> objHeaders;

    // URI parameters
    IMSList<SIPParameter*> objParams;

    // Parser of userinfo part
    mutable UserInfoPart *pUserInfoPart;
};

#endif // _SIP_ADDRESS_H_
