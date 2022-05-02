#ifndef _INTERFACE_SIP_MESSAGE_TRACKER_H_
#define _INTERFACE_SIP_MESSAGE_TRACKER_H_

#include "SipMethod.h"

class ISIPMessageTrackerListener;

/**
 * @brief This class provides an interface to add/remove any filters or set a listener
 *        to monitor SIP messages when the SIP messages are sent or received.
 *
 * @see ISIPMessageTrackerListener
 */
class ISIPMessageTracker
{
public:
    /**
     * @brief Adds the filters to monitor the SIP messages.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param bOutgoing Direction of SIP message (true : outgoing, false : incoming)
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddFilter(IN CONST SIPMethod &objMethod, IN IMS_SINT32 nStatusCode,
            IN IMS_BOOL bOutgoing) = 0;

    /**
     * @brief Removes all filters which match with the specified SIP method.
     *
     * @param objMethod SIP method
     */
    virtual void RemoveFilter(IN CONST SIPMethod &objMethod) = 0;

    /**
     * @brief Removes all filters which match with the specified parameters.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param bOutgoing Direction of SIP message (true : outgoing, false : incoming)
     */
    virtual void RemoveFilter(IN CONST SIPMethod &objMethod, IN IMS_SINT32 nStatusCode,
            IN IMS_BOOL bOutgoing) = 0;

    /**
     * @brief Removes all filters.
     */
    virtual void RemoveAllFilters() = 0;

    /**
     * @brief Sets the listener to monitor SIP messages.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN ISIPMessageTrackerListener *piListener) = 0;
};

#endif // _INTERFACE_SIP_MESSAGE_TRACKER_H_
