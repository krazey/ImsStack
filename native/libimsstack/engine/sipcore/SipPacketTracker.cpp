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
#include "ISipPacketTrackerListener.h"
#include "SipPacketTracker.h"

PUBLIC
void SipPacketTracker::NotifyMessageSent(
        IN ISipMessage* piSipMsg, IN const ByteArray& objMsg, IN IMS_BOOL bIsRetransmission)
{
    if (m_piListener == IMS_NULL)
    {
        return;
    }

    m_piListener->PacketTracker_NotifyMessageSent(piSipMsg, objMsg, bIsRetransmission);
}

PUBLIC
void SipPacketTracker::NotifyMessageReceived(
        IN ISipMessage* piSipMsg, IN const ByteArray& objMsg, IN IMS_BOOL bIsRetransmission)
{
    if (m_piListener == IMS_NULL)
    {
        return;
    }

    m_piListener->PacketTracker_NotifyMessageReceived(piSipMsg, objMsg, bIsRetransmission);
}
