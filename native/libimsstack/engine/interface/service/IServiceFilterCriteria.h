/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef INTERFACE_SERVICE_FILTER_CRITERIA_H_
#define INTERFACE_SERVICE_FILTER_CRITERIA_H_

#include "TriggerPoint.h"

/**
 * @brief This class provides an interface to manage/control the initial filter criteria.\n
 *        It's used to determine the routing of the incoming SIP request for IMS services.
 */
class IServiceFilterCriteria
{
protected:
    virtual ~IServiceFilterCriteria() = default;

public:
    /**
     * @brief Adds the trigger point to the service filter criteria.
     *
     * @param objTriggerPoint TriggerPoint to be added
     * @return If the value is greater than zero, the trigger point is added successfully.\n
     *         The return value can be used when the application wants to remove that
     *         trigger point.\n
     *         Otherwise, the trigger point is not added.
     */
    virtual IMS_UINT32 AddTriggerPoint(IN const TriggerPoint& objTriggerPoint) = 0;

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
            IN const SipMethod& objMethod, IN IMS_BOOL bCalleePreference = IMS_TRUE) = 0;
};

#endif
