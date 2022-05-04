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
#ifndef SIP_ADDRESS_H_
#define SIP_ADDRESS_H_

#include "AString.h"

#include "Sip.h"

class ISipHeader;
class SipParameter;

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
 * In this case, ToString() & GetUri() return "*", all other accessor methods return null or 0.
 */
class SipAddress
{
public:
    // Extra field to retrieve the proper token (for SIP/SIPS URIs)
    class UserInfoPart
    {
    public:
        UserInfoPart();
        UserInfoPart(IN const UserInfoPart& other);
        ~UserInfoPart();

    public:
        UserInfoPart& operator=(IN const UserInfoPart& other);

    public:
        /**
         * @brief Parses the userinfo part of SIP/SIPS URI.
         *
         * @param strUserInfo Userinfo part string
         * @return If the userinfo part is successfully parsed, returns IMS_TRUE.
         *         Otherwise, returns IMS_FALSE.
         */
        IMS_BOOL Create(IN const AString& strUserInfo);
        /**
         * @brief Gets the user field.
         *
         * @return User string.
         */
        inline const AString& GetUser() const
        { return m_strUser; }
        /**
         * @brief Gets the password field.
         *
         * @return Password string.
         */
        inline const AString& GetPassword() const
        { return m_strPassword; }
        /**
         * @brief Returns the parameter of user-info part.
         *
         * All the parameters are separated by the semi-colon.
         *
         * @return Pointer to SipParameter.
         */
        const SipParameter* GetParameter(IN const AString& strName) const;
        /**
         * @brief Gets all the parameters.
         *
         * @return List of pointer to SipParameter.
         */
        inline const IMSList<SipParameter*>& GetParameters() const
        { return m_objParameters; }

    private:
        void RemoveAllParameters();

    private:
        AString m_strUser;
        AString m_strPassword;
        IMSList<SipParameter*> m_objParameters;
    };

public:
    SipAddress();
    explicit SipAddress(IN const AString& strAddress);
    SipAddress(IN const AString& strDisplayName, IN const AString& strUri);
    // Reference count
    SipAddress(IN const SipAddress& other);
    ~SipAddress();

public:
    SipAddress& operator=(IN const SipAddress& other);

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
    IMS_RESULT AddParameter(IN const AString& strName, IN const AString& strValue);

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
    IMS_BOOL Create(IN const AString& strAddress);

    /**
     * @brief Checks if the given SipAddress is the same.
     *
     * @param objAddress SIP address to be compared
     * @return If both SIP addresses matched, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Equals(IN const SipAddress& objAddress) const;

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
     *              If this value is ISipHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param strName The header name, either in full or compact form
     * @return Pointer to uri-header parameter.
     */
    const ISipHeader* GetHeader(IN IMS_SINT32 nType,
            IN const AString& strName = AString::ConstNull()) const;

    /**
     * @brief Returns all the headers of SIP address.
     *
     * @return List of uri-header parameter.
     */
    inline const IMSList<ISipHeader*>& GetHeaders() const
    { return m_objHeaders; }

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
     * @return SipParameter to the named URI parameter,
     *         or null if the named URI parameter is not found.
     */
    const SipParameter* GetParameter(IN const AString& strName) const;

    /**
     * @brief Returns all the uri-parameters of SIP address.
     *
     * @return List of uri-parameter.
     */
    inline const IMSList<SipParameter*>& GetParameters() const
    { return m_objParams; }

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
    AString GetUri() const;

    /**
     * @brief Returns the user part of SIP address.
     *
     * There is no separate method for getting the password field
     * so that if the password field is present in the address, then the function returns
     * the value of "user:password" (instead of only "user").
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
     * so that if the password field is present in the address, then the function returns
     * the value of "user:password" (instead of only "user").\n
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
    inline IMS_BOOL IsPortUnspecified() const
    { return (m_nPort == Sip::PORT_UNSPECIFIED); }

    /**
     * @brief Checks if the scheme is SIP URI scheme or not.
     *
     * @return If the scheme is SIP URI scheme, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSchemeSip() const;

    /**
     * @brief Checks if the scheme is SIPS URI scheme or not.
     *
     * @return If the scheme is SIPS URI scheme, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSchemeSips() const;

    /**
     * @brief Checks if the scheme is TEL URI scheme or not.
     *
     * @return If the scheme is TEL URI scheme, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSchemeTel() const;

    /**
     * @brief Checks if the SIP address is for a service URN or not.
     *
     * @return If the SIP address is a service URN, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsServiceUrn() const;

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
    void RemoveParameter(IN const AString& strName);

    /**
     * @brief Sets the flag to indicate that AQUOT is required when forming URI
     *        even though any uri parameter doesn't exist.
     *
     * @param bAquotRequired Flag for AQUOT requirement
     */
    inline void SetAquotRequired(IN IMS_BOOL bAquotRequired)
    { m_bAquotRequired = bAquotRequired; }

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
    IMS_RESULT SetDisplayName(IN const AString& strName);

    /**
     * @brief Sets the flag to indicate if DQUOT is required when forming display-name field.
     *
     * @param bDquotRequired Flag to indicate if DQUOTE should be included in display-name field
     */
    inline void SetDquotRequiredForDisplayName(IN IMS_BOOL bDquotRequired)
    { m_bDquotForDisplayName = bDquotRequired; }

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
    IMS_RESULT SetHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull());

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
    IMS_RESULT SetHeaders(IN const AString& strHeaders, IN IMS_BOOL bRemoveAll = IMS_TRUE);

    /**
     * @brief Sets the host part of the SIP address.
     *
     * @param strHost The host part to be set
     * @return If the host part is successfully set, returns IMS_SUCCESS.\n
     *         If the host part is null, invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetHost(IN const AString& strHost);

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
    IMS_RESULT SetParameter(IN const AString& strName, IN const AString& strValue);

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
    IMS_RESULT SetScheme(IN const AString& strScheme);

    /**
     * @brief Sets the URI part of the SIP address (without parameter).
     *
     * The URI part of the address is of the form "scheme:user@host:port".
     * If the parameter of the method contains URI parameters,
     * they don't overwrite existing URI parameters, they are simply ignored if present.
     *
     * @param strUri The URI part of the address to be set
     * @return If the URI part is successfully set, returns IMS_SUCCESS.\n
     *         If the URI part is invalid or the address represents the immutable "*" value,
     *         returns IMS_FAILURE.
     */
    IMS_RESULT SetUri(IN const AString& strUri);

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
    IMS_RESULT SetUser(IN const AString& strUser);

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
     * @brief Returns the default list of SipAddresses (empty list).
     *
     * @return Reference to the list of SipAddresses (empty list).
     */
    static const IMSList<SipAddress*>& ConstEmptyList();

    /**
     * @brief Returns the default SipAddress (null).
     *
     * @return Reference to SipAddress (null).
     */
    static const SipAddress& ConstNull();

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
    static IMS_SINT32 GetTelUriFormat(IN const AString& strResource);

private:
    IMS_BOOL CompareSipUris(IN const SipAddress& objAddress) const;
    IMS_BOOL CompareTelUris(IN const SipAddress& objAddress) const;
    IMS_BOOL CompareTransportParameters(IN const SipAddress& objAddress) const;
    const UserInfoPart* CreateUserInfoPart(IN const AString& strUserPart) const;
    IMS_BOOL Decode(IN const AString& strAddress, IN IMS_BOOL bParseParameter,
            IN IMS_BOOL bParseDisplayName = IMS_TRUE, IN IMS_BOOL bParseHeader = IMS_TRUE);
    IMS_BOOL DecodeHeaderComponent(IN const AString& strHeaders);
    IMS_BOOL IsParameterPresent(IN const AString& strName) const;

    static IMS_BOOL CompareNumberDigits(IN const AString& strDigits1,
            IN const AString& strDigits2);
    static AString EscapeDquotAndBackslash(IN const AString& strValue);
    static IMS_BOOL IsDisplayNameToken(IN const AString& strDisplayName);
    static IMS_BOOL IsToken(IN const IMS_CHAR ch);
    static IMS_BOOL IsVisualSeparator(IN const IMS_CHAR ch);

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
    IMS_BOOL m_bIsWildcard;
    // DQUOT for display name
    IMS_BOOL m_bDquotForDisplayName;
    // Checks if AQUOT is a mandatory for URI forming
    IMS_BOOL m_bAquotRequired;
    AString m_strDisplayName;

    AString m_strScheme;
    AString m_strUserInfo;
    AString m_strHostInfo;
    IMS_UINT32 m_nPort;
    // Optional field
    //        headers := header *("&" header)
    //        header := hname "=" hvalue
    IMSList<ISipHeader*> m_objHeaders;

    // URI parameters
    IMSList<SipParameter*> m_objParams;

    // Parser of userinfo part
    mutable UserInfoPart* m_pUserInfoPart;
};

#endif
