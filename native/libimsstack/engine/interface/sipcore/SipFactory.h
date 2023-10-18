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
#ifndef SIP_FACTORY_H_
#define SIP_FACTORY_H_

#include "AString.h"

class ISipIpSecState;
class ISipKeepAliveHelper;
class ISipMessageTracker;
class ISipPacketTracker;
class ISipRoutingRejectNotifier;
class ISipRtConfigHelper;
class ISipTokenGenerator;
class ISipTransportHelper;

/**
 * @brief This class provides an interface to create SIP helper interface to control SIP engine.
 */
class SipFactory
{
public:
    SipFactory() = delete;

public:
    /**
     * @brief Creates the keep-alive helper.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISipKeepAliveHelper.
     */
    static ISipKeepAliveHelper* CreateKeepAliveHelper(IN IMS_SINT32 nSlotId);

    /**
     * @brief Generates a string for Call-ID header field.
     *
     * @param strHost Device's host name (IP address)
     * @param strCallId Call-ID header field value
     */
    static void GenerateCallId(IN const AString& strHost, OUT AString& strCallId);

    /**
     * @brief Generates a string for Session-ID header field.
     *
     * NOTE: draft-kaplan-insipid-session-id-04, for Session-ID header field.
     *
     * @param nSlotId Slot id to obtain a fixed key
     * @param strCallId Call-ID for this Session-ID
     * @param strSessionId Session-ID header field value
     */
    static void GenerateSessionId(
            IN IMS_SINT32 nSlotId, IN const AString& strCallId, OUT AString& strSessionId);

    /**
     * @brief Returns the instance of SIP IPSec state.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISipIpSecState.
     */
    static ISipIpSecState* GetIpSecState(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the instance of SIP message tracker.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISipMessageTracker.
     */
    static ISipMessageTracker* GetMessageTracker(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the instance of SIP packet tracker.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISipPacketTracker.
     */
    static ISipPacketTracker* GetPacketTracker(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the instance of SIP routing reject notifier.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISipRoutingRejectNotifier.
     */
    static ISipRoutingRejectNotifier* GetRoutingRejectNotifier(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the instance of SIP run-time (or real-time) configuration helper.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISipRtConfigHelper.
     */
    static ISipRtConfigHelper* GetRtConfigHelper(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the instance of SIP transport helper.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISipTransportHelper.
     */
    static ISipTransportHelper* GetTransportHelper(IN IMS_SINT32 nSlotId);

    /**
     * @brief Sets the specific token generator.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @param piTokenGenerator Pointer to ISipTokenGenerator
     * @deprecated NOT_USED.
     */
    static void SetTokenGenerator(IN IMS_SINT32 nSlotId, IN ISipTokenGenerator* piTokenGenerator);
};

#endif
