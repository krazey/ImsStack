#ifndef _INTERFACE_SIP_CONNECTION_FACTORY_H_
#define _INTERFACE_SIP_CONNECTION_FACTORY_H_

#include "SipMethod.h"
#include "base/IMethod.h"

class ISIPClientConnection;
class ISIPServerConnection;
class ISIPDialog;
class SIPAddress;
class ISIPConnectionFactoryListener;

/**
 * @brief This class provides an interface to use J180 interface layer directly
 *        for SIP signalling interworking.
 *
 * @see ISIPClientConnection, ISIPServerConnection, ISIPDialog
 */
class ISIPConnectionFactory
    : public IMethod
{
public:
    /**
     * @brief Creates a new SIP client connection (for new standalone tranction).
     *
     * @param objMethod SIP method
     * @param pFrom SIP address for From header\n
     *              If null, the anonymous SIP address will be set
     * @param pTo SIP address for To header\n
                  If null, the anonymous SIP address will be set
     * @return Pointer to ISIPClientConnection.
     */
    virtual ISIPClientConnection* CreateClientConnection(IN CONST SIPMethod &objMethod,
            IN CONST SIPAddress *pFrom, IN CONST SIPAddress *pTo) = 0;

    /**
     * @brief Creates a new SIP client connection inside of an SIP dialog.
     *
     * @param piDialog Pointer to ISIPDialog
     * @param objMethod SIP method
     * @return Pointer to ISIPClientConnection.
     */
    virtual ISIPClientConnection* CreateClientConnection(IN ISIPDialog *piDialog,
            IN CONST SIPMethod &objMethod) = 0;

    /**
     * @brief Creates a SIP response message for SIP server connection.
     *
     * @param piSSC Pointer to ISIPServerConnection
     * @param nStatusCode SIP status code
     * @param strPhrase SIP reason phrase
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL CreateResponse(IN ISIPServerConnection *piSSC,
            IN IMS_SINT32 nStatusCode, IN CONST AString &strPhrase = AString::ConstNull()) = 0;

    /**
     * @brief Gets the initial incoming SIP server connection.
     *
     * @return Pointer to ISIPServerConnection.
     */
    virtual ISIPServerConnection* GetNewServerConnection() = 0;

    /**
     * @brief Sets the dialog information to receive a new SIP server connection
     *        inside of SIP dialog.
     *
     * @param piDialog Pointer to ISIPDialog
     */
    virtual void SetDialog(IN ISIPDialog *piDialog) = 0;

    /**
     * @brief Sets the listener to receive a new SIP server connection inside of SIP dialog.
     *
     * @param piListener Pointer to ISIPConnectionFactoryListener
     */
    virtual void SetListener(IN ISIPConnectionFactoryListener *piListener) = 0;

    /**
     * @brief Sets the INVITE server transaction to handle an incoming CANCEL request.
     *
     * @param piSSC Pointer to ISIPServerConnection (INVITE transaction)
     */
    virtual void SetSSCForCANCEL(IN ISIPServerConnection *piSSC) = 0;
};

#endif // _INTERFACE_SIP_CONNECTION_FACTORY_H_
