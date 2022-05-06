#ifndef _INTERFACE_CAPABILITIES_LISTENER_H_
#define _INTERFACE_CAPABILITIES_LISTENER_H_

class ICapabilities;

/**
 * @brief This class provides a listener interface to notify the application about responses
 *        to capability queries.
 *
 * @see ICapabilities
 */
class ICapabilitiesListener
{
public:
    /**
     * @brief Notifies the application that the capability query response
     *        from the remote endpoint was successfully received.
     *
     * @param piCapabilities Pointer to ICapabilities to be notified
     */
    virtual void CapabilityQueryDelivered(IN ICapabilities* piCapabilities) = 0;

    /**
     * @brief Notifies the application that the capability query response
     *        from the remote endpoint was not successfully received.
     *
     * @param piCapabilities Pointer to ICapabilities to be notified
     */
    virtual void CapabilityQueryDeliveryFailed(IN ICapabilities* piCapabilities) = 0;
};

#endif  // _INTERFACE_CAPABILITIES_LISTENER_H_
