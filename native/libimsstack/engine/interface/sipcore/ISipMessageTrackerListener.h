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
#ifndef INTERFACE_SIP_MESSAGE_TRACKER_LISTENER_H_
#define INTERFACE_SIP_MESSAGE_TRACKER_LISTENER_H_

#include "SipMethod.h"

/**
 * @brief This class provides a listener interface to notify SIP messages
 *        when the SIP messages are sent or received.
 *
 * @see ISipMessageTracker
 */
class ISipMessageTrackerListener
{
public:
    /**
     * @brief Notifies the application that SIP message is received from the network.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param strCallId Call-ID header field of SIP message
     */
    virtual void MessageTracker_NotifyMessageReceived(IN const SipMethod& objMethod,
            IN IMS_SINT32 nStatusCode, IN const AString& strCallId) = 0;

    /**
     * @brief Notifies the application that SIP message is sent to the network.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param strCallId Call-ID header field of SIP message
     */
    virtual void MessageTracker_NotifyMessageSent(IN const SipMethod& objMethod,
            IN IMS_SINT32 nStatusCode, IN const AString& strCallId) = 0;

    /**
     * @brief Notifies the application that sending the SIP message is failed.
     *
     * @param objMethod SIP method
     * @param nStatusCode SIP status code
     * @param strCallId Call-ID header field of SIP message
     */
    virtual void MessageTracker_NotifyMessageSentFailed(IN const SipMethod& objMethod,
            IN IMS_SINT32 nStatusCode, IN const AString& strCallId) = 0;
};

#endif
