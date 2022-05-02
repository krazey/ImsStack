#ifndef _INTERFACE_SIP_ROUTING_REJECT_LISTENER_H_
#define _INTERFACE_SIP_ROUTING_REJECT_LISTENER_H_

#include "SipStatusCode.h"

class ISIPMessage;
class ISIPServerConnection;

/**
 * @brief This class provides a listener interface for receiving notifications about
 *        the reject to the incoming SIP request.
 *
 * @see ISIPRoutingRejectNotifier, ISIPMessage, ISIPServerConnection
 */
class ISIPRoutingRejectListener
{
public:
    /**
     * @brief Notifies the application that the incoming SIP request will be rejected
     *        in the J180 layer.
     *
     * At this moment, the application can overwrite the status code of the rejected request.
     *
     * @param piSIPMsg SIP message to be rejected
     * @param objStatusCode Status code which will be used for request reject\n
     *                      Application can overwrite the status code to be rejected.
     * @return If the application is changed the status code, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL RoutingReject_NotifyRequest(
            IN ISIPMessage *piSIPMsg, IN_OUT SIPStatusCode &objStatusCode) = 0;

    /**
     * @brief Notifies the application that the incoming SIP request will be rejected
     *        in the J180 layer.
     *
     * At this moment, the application can overwrite the status code of the rejected request.
     *
     * @param piSSC SIP server connection to be rejected
     * @param objStatusCode Status code which will be used for request reject\n
     *                      Application can overwrite the status code to be rejected.
     * @return If the application is changed the status code, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL RoutingReject_NotifyRequest(
            IN ISIPServerConnection *piSSC, IN_OUT SIPStatusCode &objStatusCode) = 0;
};

#endif // _INTERFACE_SIP_ROUTING_REJECT_LISTENER_H_
