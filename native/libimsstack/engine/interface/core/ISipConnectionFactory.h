#ifndef _INTERFACE_SIP_CONNECTION_FACTORY_H_
#define _INTERFACE_SIP_CONNECTION_FACTORY_H_

#include "SipMethod.h"
#include "base/IMethod.h"

class ISipClientConnection;
class ISipServerConnection;
class ISipDialog;
class SipAddress;
class ISipConnectionFactoryListener;

/**
 * @brief This class provides an interface to use J180 interface layer directly
 *        for SIP signalling interworking.
 *
 * @see ISipClientConnection, ISipServerConnection, ISipDialog
 */
class ISipConnectionFactory : public IMethod
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
     * @return Pointer to ISipClientConnection.
     */
    virtual ISipClientConnection* CreateClientConnection(IN CONST SipMethod& objMethod,
            IN CONST SipAddress* pFrom, IN CONST SipAddress* pTo) = 0;

    /**
     * @brief Creates a new SIP client connection inside of an SIP dialog.
     *
     * @param piDialog Pointer to ISipDialog
     * @param objMethod SIP method
     * @return Pointer to ISipClientConnection.
     */
    virtual ISipClientConnection* CreateClientConnection(
            IN ISipDialog* piDialog, IN CONST SipMethod& objMethod) = 0;

    /**
     * @brief Creates a SIP response message for SIP server connection.
     *
     * @param piSSC Pointer to ISipServerConnection
     * @param nStatusCode SIP status code
     * @param strPhrase SIP reason phrase
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL CreateResponse(IN ISipServerConnection* piSSC, IN IMS_SINT32 nStatusCode,
            IN CONST AString& strPhrase = AString::ConstNull()) = 0;

    /**
     * @brief Gets the initial incoming SIP server connection.
     *
     * @return Pointer to ISipServerConnection.
     */
    virtual ISipServerConnection* GetNewServerConnection() = 0;

    /**
     * @brief Sets the dialog information to receive a new SIP server connection
     *        inside of SIP dialog.
     *
     * @param piDialog Pointer to ISipDialog
     */
    virtual void SetDialog(IN ISipDialog* piDialog) = 0;

    /**
     * @brief Sets the listener to receive a new SIP server connection inside of SIP dialog.
     *
     * @param piListener Pointer to ISipConnectionFactoryListener
     */
    virtual void SetListener(IN ISipConnectionFactoryListener* piListener) = 0;

    /**
     * @brief Sets the INVITE server transaction to handle an incoming CANCEL request.
     *
     * @param piSSC Pointer to ISipServerConnection (INVITE transaction)
     */
    virtual void SetSSCForCANCEL(IN ISipServerConnection* piSSC) = 0;
};

#endif  // _INTERFACE_SIP_CONNECTION_FACTORY_H_
