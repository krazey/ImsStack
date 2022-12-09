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
#ifndef INTERFACE_SIP_PACKET_TRACKER_LISTENER_H_
#define INTERFACE_SIP_PACKET_TRACKER_LISTENER_H_

#include "ByteArray.h"

class ISipMessage;

/**
 * @brief This class provides a listener interface to notify SIP packets
 *        when the SIP messages are sent or received.
 *
 * @see ISipPacketTracker, ISipMessage
 */
class ISipPacketTrackerListener
{
protected:
    virtual ~ISipPacketTrackerListener() = default;

public:
    /**
     * @brief Notifies the application that SIP packet is sent to the network.
     *
     * @param piSipMsg Pointer to ISipMessage object
     * @param objMsg Byte array to SIP Message
     * @param bIsRetransmission Flag that SIP message is retransmitted
     */
    virtual void PacketTracker_NotifyMessageSent(IN ISipMessage* piSipMsg,
            IN const ByteArray& objMsg, IN IMS_BOOL bIsRetransmission) = 0;

    /**
     * @brief Notifies the application that SIP packet is received from the network.
     *
     * @param piSipMsg Pointer to ISipMessage object
     * @param objMsg Byte array to SIP Message
     * @param bIsRetransmission Flag that SIP message is retransmitted
     */
    virtual void PacketTracker_NotifyMessageReceived(IN ISipMessage* piSipMsg,
            IN const ByteArray& objMsg, IN IMS_BOOL bIsRetransmission) = 0;
};

#endif
