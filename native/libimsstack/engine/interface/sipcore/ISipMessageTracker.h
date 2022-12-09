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
#ifndef INTERFACE_SIP_MESSAGE_TRACKER_H_
#define INTERFACE_SIP_MESSAGE_TRACKER_H_

#include "SipMethod.h"

class ISipMessageTrackerListener;

/**
 * @brief This class provides an interface to add/remove any filters or set a listener
 *        to monitor SIP messages when the SIP messages are sent or received.
 *
 * @see ISipMessageTrackerListener
 */
class ISipMessageTracker
{
protected:
    virtual ~ISipMessageTracker() = default;

public:
    /**
     * @brief Adds the filters to monitor the SIP messages.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param bOutgoing Direction of SIP message (true : outgoing, false : incoming)
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddFilter(
            IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN IMS_BOOL bOutgoing) = 0;

    /**
     * @brief Removes all filters which match with the specified SIP method.
     *
     * @param objMethod SIP method
     */
    virtual void RemoveFilter(IN const SipMethod& objMethod) = 0;

    /**
     * @brief Removes all filters which match with the specified parameters.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param bOutgoing Direction of SIP message (true : outgoing, false : incoming)
     */
    virtual void RemoveFilter(
            IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN IMS_BOOL bOutgoing) = 0;

    /**
     * @brief Removes all filters.
     */
    virtual void RemoveAllFilters() = 0;

    /**
     * @brief Sets the listener to monitor SIP messages.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN ISipMessageTrackerListener* piListener) = 0;
};

#endif
