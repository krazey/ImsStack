#ifndef _INTERFACE_REG_PARAMETER_H_
#define _INTERFACE_REG_PARAMETER_H_

#include "AStringArray.h"
#include "SipSecurityHeader.h"
#include "SipTimerValues.h"
#include "SipAddress.h"

class ISIPMessageBodyPart;

/**
 * @brief This class provides an interface to access/control SIP header/parameters
 *        for IMS registration.
 */
class IRegParameter
{
public:
    /**
     * Adds the extra headers to be set during the registration in active.
     *
     * @param objHeaders List of SIP header format (ex. Require: pref)
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddExtraHeaders(IN CONST AStringArray &objHeaders) = 0;

    /**
     * @brief Adds the message body to be set during the registration in active.
     *
     * @param piBodyPart Pointer of ISIPMessageBodyPart
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddMessageBodyPart(IN ISIPMessageBodyPart *piBodyPart) = 0;

    /**
     * @brief Adds the preloaded route header.
     *
     * The specified string should be an URI format.
     *
     * @param strRoute Route information (sip/sips URI or other)
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddPreloadedRoute(IN CONST AString &strRoute) = 0;

    /**
     * @brief Adds the preloaded route header.
     *
     * By default, the scheme will be set to "sip" and "lr" parameter will be inserted.
     *
     * @param strHost Host info. (IP address or FQDN) of P-CSCF
     * @param nPort Port number of P-CSCF
     * @param strScheme URI scheme (sip or sips); default is "sip"
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddPreloadedRoute(IN CONST AString &strHost, IN IMS_SINT32 nPort,
            IN CONST AString &strScheme = AString::ConstNull()) = 0;

    /**
     * @brief Adds the Security-Client header.
     *
     * @param objSecurityHeader Object to SIPSecurityHeader (Security-Client)
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddSecurityClient(IN CONST SIPSecurityHeader &objSecurityHeader) = 0;

    /**
     * @brief Returns the default SIP port for each registration.
     *
     * @return Default SIP port number.
     */
    virtual IMS_SINT32 GetPort() const = 0;

    /**
     * @brief Returns the preferred Security-Client header among the Security-Client list.
     *
     * It will choose the Security-Client with the highest "q" value & mechanism
     * which is matched with the preferred Security-Server.
     *
     * @return Pointer to preferred Security-Client.
     */
    virtual const SIPSecurityHeader* GetPreferredSecurityClient() const = 0;

    /**
     * @brief Returns the preferred Security-Server header among the Security-Server list.
     *
     * It will choose the Security-Server with the highest "q" value & mechanism
     * which is supported by UE (Security-Clients).
     *
     * @return Pointer to preferred Security-Server.
     */
    virtual const SIPSecurityHeader* GetPreferredSecurityServer() const = 0;

    /**
     * @brief Returns the list of Security-Server header.
     *
     * @return List of Security-Server header.
     */
    virtual const IMSList<SIPSecurityHeader>& GetSecurityServers() const = 0;

    /**
     * @brief Returns the topmost Route header.
     *
     * @return Topmost Route address.
     */
    virtual const SIPAddress& GetTopmostRouteAddress() const = 0;

    /**
     * @brief Removes all the message body parts.
     */
    virtual void RemoveAllMessageBodyParts() = 0;

    /**
     * @brief Removes all the preloaded route headers.
     */
    virtual void RemoveAllPreloadedRoutes() = 0;

    /**
     * @brief Removes the extra headers.
     *
     * @param objHeaders List of SIP header format (ex. Require: pref)
     */
    virtual void RemoveExtraHeaders(IN CONST AStringArray &objHeaders) = 0;

    /**
     * @brief Removes all the Security-Client headers.
     */
    virtual void RemoveSecurityClients() = 0;

    /**
     * @brief Sets the policy to determine whether or not the Authorization header contains
     *        in REGISTER request.
     *
     * The default policy is that the REGISTER request will contain the Authorization header.\n
     * If the policy is IMS_FALSE, the Authorization header will not be contained
     * in the REGISTER request.
     *
     * @param bPolicy Flag to indicate whether Authorization header is included or not
     */
    virtual void SetAuthenticationCredentials(IN IMS_BOOL bPolicy) = 0;

    /**
     * @brief Sets the option to select the port for the flow control of this registration.
     *
     * The default is a #FLOW_CONTROL_BY_PROVISION.\n
     * For #FLOW_CONTROL_BY_MESSAGE_CONTEXT, the registration checks the following:
     *     - Multiple registration supports
     *     - "+sip.instance" & "reg-id" header field in Contact header
     *     - "outbound" option tag in Supported header
     *     - "outbound" option tag in Require header in 200 OK (to REGISTER)
     *
     * If the condition is true, the provisioned flow control port is selected.
     * Otherwise, SIP::PORT_UNSPECIFIED is selected.
     *
     * @param nOption Option to select a port for the flow control
     * @note RFC5626_FLOW_CONTROL
     */
    virtual void SetFlowControlOption(IN IMS_SINT32 nOption) = 0;

    /**
     * @brief Sets the default SIP port for each registration.
     *
     * @param nPort Default SIP port number
     */
    virtual void SetPort(IN IMS_SINT32 nPort) = 0;

    /**
     * @brief Sets the port for the flow control of this registration.
     *
     * If the application wouldn't like to use the flow control scheme,
     * it SHOULD set the port number to 0 or SIP::PORT_UNSPECIFIED.
     *
     * @param nPort Port for the flow control
     * @note RFC5626_FLOW_CONTROL
     */
    virtual void SetPortFlowControl(IN IMS_SINT32 nPort) = 0;

    /**
     * @brief Sets the list of Security-Verify header.
     *
     * @param objSecurityVerifys List of Security-Verify header
     */
    virtual void SetSecurityVerifys(IN CONST IMSList<SIPSecurityHeader> &objSecurityVerifys) = 0;

    /**
     * @brief Sets the timer values of SIP transaction layer for registration.
     *
     * @param objTVs Object of SIPTimerValues
     */
    virtual void SetSIPTimerValues(IN CONST SIPTimerValues &objTVs) = 0;

    /**
     * @brief Sets the transport extensions for this registration.
     *
     * @param nTransportExt Transport extension (SIP::TRANSPORT_EXT_XXX in #SIP)
     */
    virtual void SetTransportExt(IN IMS_SINT32 nTransportExt) = 0;

    /**
     * @brief Sets the transport extensions for registration only.
     *
     * @param nTransportExt Transport extension (SIP::TRANSPORT_EXT_XXX in #SIP)
     */
    virtual void SetTransportExtForRegOnly(IN IMS_SINT32 nTransportExt) = 0;

public:
    /// Options to select a port for flow control
    enum
    {
        /// Default flow control
        FLOW_CONTROL_BY_PROVISION = 1,
        /// Flow control is determined by SIP message context
        FLOW_CONTROL_BY_MESSAGE_CONTEXT = 2
    };
};

#endif // _INTERFACE_REG_PARAMETER_H_
