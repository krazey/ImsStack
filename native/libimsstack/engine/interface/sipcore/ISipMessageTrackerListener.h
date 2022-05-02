#ifndef _INTERFACE_SIP_MESSAGE_TRACKER_LISTENER_H_
#define _INTERFACE_SIP_MESSAGE_TRACKER_LISTENER_H_

#include "SipMethod.h"

/**
 * @brief This class provides a listener interface to notify SIP messages
 *        when the SIP messages are sent or received.
 *
 * @see ISIPMessageTracker
 */
class ISIPMessageTrackerListener
{
public:
    /**
     * @brief Notifies the application that SIP message is received from the network.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param strCallId Call-ID header field of SIP message
     */
    virtual void MessageTracker_NotifyMessageReceived(IN CONST SIPMethod &objMethod,
            IN IMS_SINT32 nStatusCode, IN CONST AString &strCallId) = 0;

    /**
     * @brief Notifies the application that SIP message is sent to the network.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param strCallId Call-ID header field of SIP message
     */
    virtual void MessageTracker_NotifyMessageSent(IN CONST SIPMethod &objMethod,
            IN IMS_SINT32 nStatusCode, IN CONST AString &strCallId) = 0;

    /**
     * @brief Notifies the application that sending the SIP message is failed.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param strCallId Call-ID header field of SIP message
     */
    virtual void MessageTracker_NotifyMessageSentFailed(IN CONST SIPMethod &objMethod,
            IN IMS_SINT32 nStatusCode, IN CONST AString &strCallId) = 0;
};

#endif // _INTERFACE_SIP_MESSAGE_TRACKER_LISTENER_H_
