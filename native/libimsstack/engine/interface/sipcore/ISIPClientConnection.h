#ifndef _INTERFACE_SIP_CLIENT_CONNECTION_H_
#define _INTERFACE_SIP_CLIENT_CONNECTION_H_

#include "IPAddress.h"
#include "ISIPConnection.h"

class ISIPRefresher;
class ISIPConnectionNotifier;
class ISIPClientConnectionListener;
class ISIPGenericChallenge;
class ISIPAckPackage;
class Credential;

/**
 * @brief This class provides an interface to handle SIP client transaction.
 *
 * Applications can create a new ISIPClientConnection via Connector or ISIPDialog interface.
 *
 * @see ISIPConnection, ISIPConnectionNotifier
 */
class ISIPClientConnection
    : public ISIPConnection
{
public:
    /**
     * @brief Enables the refresh for the request to be sent.
     *
     * The method return a refresh ID, which can be used to update or stop the refresh.
     *
     * Passing NULL as listener does not clear a previously set listener and does not stop
     * a refresh.\n Refreshing should be stopped by calling Stop() method.\n
     * Calling EnableRefresh() for the second time with a non-NULL value does not overwrite
     * the previously set listener. In this case, the previsously set listener remains valid,
     * and the method throws an exception, SIPError::INVALID_STATE.
     *
     * @param pRefresher Callback interface for refresh events\n
     *                   If this is null, the method returns 0 and refresh is not enabled
     * @return Refresh identifier.\n
     *         It succeeds, returns non-zero value. Otherwise, returns zero.
     * @deprecated NOT_USED
     */
    virtual IMS_RESULT EnableRefresh(IN ISIPRefresher *piRefresher) = 0;

    /**
     * @brief Convenience method to initialize ISIPClientConnection with SIP request method, ACK.
     *
     * ACK can be applied only to INVITE request.\n
     * This method is available when a successful final response (2xx) has been received.\n
     * The header fields of the ACK are constructed in the same way as for any request sent within
     * a dialog with the exception of the CSeq and the header fields related to authentication.\n
     * The Request-URI and header fields will be initialized automatically.\n
     * After this, the ISIPClientConnection is in INITIALIZED state.
     *
     * The following information will be set by the method:
     *     - Request-URI
     *     - To
     *     - From
     *     - CSeq
     *     - Call-ID
     *     - Via
     *     - Route
     *     - Contact
     *     - Max-Forwards
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT InitAck() = 0;

    /**
     * @brief Convenience method to initialize ISIPClientConnection with SIP request method,
     *        CANCEL.
     *
     * This method is available when a provisional response has been received.\n
     * The CANCEL request starts a new SIP transaction, that is why the method returns a new
     * ISIPClientConnection.\n The CANCEL request will be built according to the original INVITE
     * request within this transaction.\n The Request-URI and header fields will be initialized
     * automatically.\n After this, the ISIPClientConnection is in INITIALIZED state.
     *
     * The following information will be set by the method:
     *     - Request-URI
     *     - To
     *     - From
     *     - CSeq
     *     - Call-ID
     *     - Via
     *     - Route
     *     - Max-Forwards
     *
     * @return A new ISIPClientConnection with pre-initialized CANCEL request.
     */
    virtual ISIPClientConnection* InitCancel() = 0;

    /**
     * @brief Initializes ISIPClientConnection to a specific SIP request method (REGISTER, INVITE,
     *        MESSAGE, REFER, ...).
     *
     * The default Request-URI and header fields will be initialized automatically.\n
     * After this, the ISIPClientConnection is in INITIALIZED state.\n
     * SIP methods belonging to an already established dialog (BYE, NOTIFY, PRACK, UPDATE)
     * should be created using ISIPDialog::GetNewClientConnection() instead of this method.
     *
     * Header fields that will be initialized are as follows:
     *     - To
     *     - From
     *     - CSeq
     *     - Call-ID
     *     - Via
     *     - Contact
     *     - Max-Forwards
     *
     * @param strMethod Name of the SIP method
     * @param piSCN Pointer to ISIPConnectionNotifier to which the request will be associated.\n
     *              If piSCN is NULL, the request will not be associated to any user defined
     *              listening port.
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT InitRequest(IN CONST AString& strMethod,
            IN ISIPConnectionNotifier *piSCN) = 0;

    /**
     * @brief Receives a SIP response message.
     *
     * This method will update the ISIPClientConnection with the last new received response.\n
     * The implementation places the responses in a FIFO queue and when this method is called,
     * it fetches the lease recently arrived response which has not been fetched already.
     *
     * @param nTimeout Time-out value for blocking call; not-used
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Receive(IN IMS_SLONG nTimeout = 0) = 0;

    /**
     * @brief Sets multiple credential triplets for possible digest authentication.
     *
     * The username and password are specified for certain protection domain,
     * which is defined by the realm parameter.
     *
     * The credentials can be set:
     *     - Before sending the original request in INITIALIZED state.
     *     - When 401(Unauthorized) or 407(Proxy Authentication Required) response is received
     *       in the UNAUTHORIZED state.
     *       After this, the ISIPClientConnection will be in PROCEEDING state.
     *
     * @param objCredentials List of credential (username, password, realm)
     *                       for digest authentication
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetCredentials(IN IMSList<Credential>& objCredentials) = 0;

    /**
     * @brief Sets credential triplets for possible digest authentication.
     *
     * The username and password are specified for certain protection domain,
     * which is defined by the realm parameter.
     *
     * The credentials can be set:
     *     - Before sending the original request in INITIALIZED state.
     *     - When 401(Unauthorized) or 407(Proxy Authentication Required) response is received
     *       in the UNAUTHORIZED state.
     *       After this, the ISIPClientConnection will be in PROCEEDING state.
     *
     * @param objCredential Credential (username, password, realm) for digest authentication
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetCredentials(IN CONST Credential &objCredential) = 0;

    /**
     * @brief Sets the listener for incoming responses.
     *
     * If a listener is already set, it will be overwritten. Setting listener to null will remove
     * the current listener.
     *
     * @param piListener Listener to be set\n
     *                   null value will remove the existing listener
     */
    virtual void SetListener(IN ISIPClientConnectionListener *piListener) = 0;

    /**
     * @brief Sets Request-URI explicitly.
     *
     * If this operation is supported, Request-URI can be set only in INITIALIZED state.\n
     * Empty or null argument removes Request-URI if set previously.\n
     * It is not mandated that this method be supported, so it may throw INVALID_OPERATION
     * in any state.
     *
     * @param strURI String representation of Request-URI
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetRequestURI(IN CONST AString& strURI) = 0;

    //// IMS extensions

    /**
     * @brief Returns the authentication challenge.
     *
     * @param nIndex Position of authentication challenge
     * @return Pointer to ISIPGenericChallenge.
     */
    virtual ISIPGenericChallenge* GetAuthenticationChallenge(IN IMS_SINT32 nIndex = 0) const = 0;

    /**
     * @brief Returns the helper package for SIP ACK retransmission.
     *
     * The application SHOULD destroy the returned object if it does not use anymore.
     *
     * @return Pointer to ISIPAckPackage.
     */
    virtual ISIPAckPackage* GrabAck() = 0;

    /**
     * @brief Initializes the resubmitting request message when the connection had received
     *        the authentication challenge.
     *
     * It MUST be only invoked in UNAUTHORIZED state, otherwise the method returns an error.
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT InitResubmissionRequest() = 0;

    /**
     * @brief Removes all the authentication challenges which are set by the application
     *        on the initial transaction or the SIP message received from the network.
     */
    virtual void RemoveAllChallenges() = 0;

    /**
     * @brief Removes all the credential information which are set by the application
     *        on the previous transaction.
     */
    virtual void RemoveAllCredentials() = 0;

    /**
     * @brief Sets the authentication challenge for this client connection.
     *
     * It can be set in the state, INITIALIZED and after setting it, the initial request
     * will payload the Authorization or Proxy-Authorization header.
     *
     * @param piChallenge Authentication challenge to be set
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetAuthenticationChallenge(IN ISIPGenericChallenge *piChallenge) = 0;

    /**
     * @brief Sets the extension token of the branch parameter in the Via header.
     *
     * @param strToken Extension token
     */
    virtual void SetExtensionTokenForViaBranch(IN CONST AString &strToken) = 0;

    /**
     * @brief Sets the implicit Route header (SIP or SIPS URI).
     *
     * For example, according to 3GPP specification,
     * REGISTER request will not include Route header, but it SHALL not be sent
     * by the Request-URI based on SIP message routing rule.\n
     * If this value is set, the outgoing request will be sent to the specified host (IP & port).
     *
     * @param strRouteHeader Route header for implicit message routing
     */
    virtual void SetImplicitRouteHeader(IN CONST AString &strRouteHeader) = 0;

    /**
     * @brief Sets the transport tuple for this client connection.
     *
     * @param objIPA IP address of the device
     * @param nPortC Client port number of the device
     * @param nPortS Server port number of the device
     * @param nPortFC Port number for the flow control (RFC5626)
     * @param nTransportExt Transport extension (SIP::TRANSPORT_EXT_XXX in SIP.h)
     * @note RFC5626_FLOW_CONTROL, MULTI_REG_TRANSPORT
     */
    virtual void SetTransportTuple(IN CONST IPAddress &objIPA,
            IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFC = 0xFFFF,
            IN IMS_SINT32 nTransportExt = 0 /* ANY */) = 0;
};

#endif // _INTERFACE_SIP_CLIENT_CONNECTION_H_
