#ifndef _INTERFACE_SERVICE_FILTER_CRITERIA_H_
#define _INTERFACE_SERVICE_FILTER_CRITERIA_H_

#include "TriggerPoint.h"

/**
 * @brief This class provides an interface to manage/control the initial filter criteria.\n
 *        It's used to determine the routing of the incoming SIP request for IMS services.
 */
class IServiceFilterCriteria
{
public:
    /**
     * @brief Adds the trigger point to the service filter criteria.
     *
     * @param objTP TriggerPoint to be added
     * @return If the value is greater than zero, the trigger point is added successfully.\n
     *         The return value can be used when the application wants to remove that
     *         trigger point.\n
     *         Otherwise, the trigger point is not added.
     */
    virtual IMS_UINT32 AddTriggerPoint(IN CONST TriggerPoint& objTP) = 0;

    /**
     * @brief Removes the trigger point with the specified identifier
     *        from the service filter criteria.
     *
     * @param nTriggerPointId TriggerPoint identifier to be removed
     */
    virtual void RemoveTriggerPoint(IN IMS_SINT32 nTriggerPointId) = 0;

    /**
     * @brief Removes all the trigger points.
     */
    virtual void RemoveAllTriggerPoints() = 0;

    /**
     * @brief Sets the callee preference to override the caller preference when the iFC is matched.
     *
     * @param objMethod SIP method to enable/disable the callee preference\n
     *                  - INVITE, OPTIONS, MESSAGE, REFER
     * @param bCalleePreference flag to indicate that the callee preference is supported or not
     */
    virtual void SetCalleePreference(
            IN CONST SipMethod& objMethod, IN IMS_BOOL bCalleePreference = IMS_TRUE) = 0;
};

#endif  // _INTERFACE_SERVICE_FILTER_CRITERIA_H_
