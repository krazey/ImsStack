#ifndef _INTERFACE_SIP_KEEP_ALIVE_HELPER_LISTENER_H_
#define _INTERFACE_SIP_KEEP_ALIVE_HELPER_LISTENER_H_

/**
 * @brief This class provides a listener interface to receive the response
 *        of SIP keep-alive packet.
 */
class ISIPKeepAliveHelperListener
{
public:
    /**
     * @brief Notifies the application that the response of SIP keep-alive packet (PING)
     *        is received.
     */
    virtual void KeepAliveHelper_PongReceived() = 0;
};

#endif // _INTERFACE_SIP_KEEP_ALIVE_HELPER_LISTENER_H_
