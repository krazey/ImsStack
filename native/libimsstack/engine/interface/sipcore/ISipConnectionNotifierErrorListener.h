#ifndef _INTERFACE_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_
#define _INTERFACE_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_

class ISIPConnectionNotifier;

/**
 * @brief This class provides a listener interface for notifications about failure
 *        of the transport layer's operations.
 *
 * @see ISIPConnectionNotifier
 */
class ISIPConnectionNotifierErrorListener
{
public:
    /**
     * @brief Called when any error occurrs in the SIPConnectionNotifier.
     *
     * @param piSCN Pointer to ISIPConnectionNotifier object which error occurrs
     * @param nCode Reason code of error
     * @param strMessage Reason phrase of error\n
     *                   Implementation dependent non-localized information about the error
     */
    virtual void ConnectionNotifierError_NotifyError(IN ISIPConnectionNotifier *piSCN,
            IN IMS_SINT32 nCode, IN CONST AString &strMessage) = 0;
};

#endif // _INTERFACE_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_
