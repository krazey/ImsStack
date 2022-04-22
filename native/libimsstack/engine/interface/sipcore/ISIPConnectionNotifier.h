#ifndef _INTERFACE_SIP_CONNECTION_NOTIFIER_H_
#define _INTERFACE_SIP_CONNECTION_NOTIFIER_H_

#include "IPAddress.h"
#include "IConnection.h"

class SIPProfile;
class ISIPDialog;
class ISIPServerConnection;
class ISIPServerConnectionListener;
class ISIPConnectionNotifierErrorListener;

/**
 * @brief This class provides an interface to receive an incoming SIP request.
 *
 * The SIP server transaction notifier is created with Connector.Open()
 * using a SIP URI string with the host and user omitted.
 *
 * @see IConnection, ISIPServerConnection, ISIPServerConnectionListener
 */
class ISIPConnectionNotifier
    : public IConnection
{
public:
    /**
     * @brief Accepts and opens a new ISIPServerConnection in this listening port.
     *
     * If there are no messages in the queue, the method will throw an exception.
     *
     * @return Pointer to ISIPServerConnection object which carries the received request.
     */
    virtual ISIPServerConnection* AcceptAndOpen() = 0;

    /**
     * @brief Gets the local IP address for this SIP transaction notifier.
     *
     * It returns NULL if the address is not available.
     *
     * @return Textual representation of decimal dotted IP address.
     */
    virtual const IPAddress& GetLocalAddress() const = 0;

    /**
     * @brief Gets the local port for this SIP transaction notifier.
     *
     * It returns 0 if the port is not available.
     *
     * @return Local port number, that the notifier is listening to.
     */
    virtual IMS_SINT32 GetLocalPort() const = 0;

    /**
     * @brief Sets a listener for incoming SIP requests. If a listener is already set, it will be
     *        overwritten.
     *
     * Setting listener to null will remove the current listener.
     *
     * @param piListener Listener to monitor an incoming SIP requests
     */
    virtual void SetListener(IN ISIPServerConnectionListener *piListener) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // IMS extensions

    /**
     * @brief Accepts and opens a new ISIPServerConnection in this listening port
     *        when receiving a forked request.
     *
     * If there are no messages in the queue, the method will throw an exception.
     *
     * @param piOrigDialog Pointer to the original dialog when the request is forked
     * @return Pointer to ISIPServerConnection object which carries the received request.
     */
    virtual ISIPServerConnection* AcceptAndOpen(OUT ISIPDialog *&piOrigDialog) = 0;

    /**
     * @brief Returns the default contact address.
     *
     * @return A contact address as string.
     */
    virtual AString GetContactAddress() const = 0;

    /**
     * @brief Returns the SIP profile of this SIP connection notifier.
     *
     * @return Pointer to SIPProfile.
     * @note MULTI_REG_SIP_PROFILE
     */
    virtual SIPProfile* GetSIPProfile() const = 0;

    /**
     * @brief Gets the slot-id for this SIP connection notifier.
     *
     * @return Slot id of this SIP connection notifier.
     */
    virtual IMS_SINT32 GetSlotId() const = 0;

    /**
     * @brief Checks if the transport resource is reserved or not.
     *
     * @param nType Type of transport resource
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     * @note REG_RESTORATION_FOR_ACTIVE_BINDING
     */
    virtual IMS_BOOL IsTransportResourceReserved(
            IN IMS_SINT32 nType = TRANSPORT_ALL) const = 0;

    /**
     * @brief Reserves the transport resource to receive an incoming SIP request message.
     *
     * @param objIPA Local IP address
     * @param nPortS Port number (listening port number) for server socket
     * @param nPortC Port number for client socket (when IPSec is used)
     * @param nPortFlowControl Port number for flow control
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT ReserveTransportResource(IN CONST IPAddress &objIPA,
            IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl) = 0;

    /**
     * @brief Restores the transport resource for the specified resource type.
     *
     * @param nType Type of transport resource\n
     *              #TRANSPORT_CLIENT_INITIATED_CONNECTION\
     *              #TRANSPORT_SERVER_CONNECTION\n
     *              #TRANSPORT_ALL
     * @param objPeerIP IP address of remote end
     * @param nPeerPort Port number of remote end
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     * @note REG_RESTORATION_FOR_ACTIVE_BINDING
     */
    virtual IMS_RESULT RestoreTransportResource(IN IMS_SINT32 nType,
            IN CONST IPAddress &objPeerIP, IN IMS_SINT32 nPeerPort) = 0;

    /**
     * @brief Sets the filter information for JSR180 PushRegistry supports.
     *
     * @param strFilter Filter for PushRegistry
     * @deprecated NOT_USED
     */
    virtual void SetFilter(IN CONST AString &strFilter) = 0;

    /**
     * @brief Sets a default SIP settings (a default From informantion & user-info in Contact).
     *
     * @param strFrom SIP or tel URI for From header
     * @param strDisplayName Display name for From header
     * @param strUserInfo User-info field value for Contact header
     * @deprecated NOT_USED
     */
    virtual void SetFromAndContact(IN CONST AString &strFrom, IN CONST AString &strDisplayName,
            IN CONST AString &strUserInfo) = 0;

    /**
     * @brief Sets the SIP profile for specific configuration of SIP server connections.
     *
     * @param pProfile SIP profile for SIP server transaction
     * @note MULTI_REG_SIP_PROFILE
     */
    virtual void SetSIPProfile(IN SIPProfile *pProfile) = 0;

    /**
     * @brief Updates the port for the flow control based on RFC5626.
     *
     * This will be only used for the TCP client connection.
     *
     * @param nPort Port number for flow control
     * @note RFC5626_FLOW_CONTROL
     */
    virtual void UpdatePortFlowControl(IN IMS_SINT32 nPort) = 0;

    /**
     * @brief Updates the protected client port (uc) when the re-REGISTER has been completed.
     *
     * @param nPort Protected client port (uc) to be updated
     */
    virtual void UpdatePortUC(IN IMS_SINT32 nPort) = 0;

    /**
     * @brief Adds the listener for error notifications.
     *
     * @param piListener Pointer to ISIPConnectionNotifierErrorListener
     */
    virtual void AddErrorListener(IN ISIPConnectionNotifierErrorListener *piListener) = 0;

    /**
     * @brief Removes the listener for error notifications.
     *
     * @param piListener Pointer to ISIPConnectionNotifierErrorListener
     */
    virtual void RemoveErrorListener(IN ISIPConnectionNotifierErrorListener *piListener) = 0;

public:
    /// Error codes
    enum
    {
        /// When TCP client socket is closed
        TRANSPORT_ERROR_TCP_CLIENT = 1,
        /// When TCP server socket is closed
        TRANSPORT_ERROR_TCP_SERVER = 2,
        /// When UDP server socket is closed
        TRANSPORT_ERROR_UDP_SERVER = 3
    };

    /// Types of transport resource
    enum
    {
        /// When restoring client-initiated socket connection
        TRANSPORT_CLIENT_INITIATED_CONNECTION = 0x01,
        /// When restoring server socket connection
        TRANSPORT_SERVER_CONNECTION = 0x02,
        /// When restoring all the transport resources
        TRANSPORT_ALL = (TRANSPORT_CLIENT_INITIATED_CONNECTION | TRANSPORT_SERVER_CONNECTION)
    };
};

#endif // _INTERFACE_SIP_CONNECTION_NOTIFIER_H_
