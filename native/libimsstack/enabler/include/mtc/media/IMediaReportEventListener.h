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

#ifndef INTERFACE_MEDIA_REPORT_EVENT_LISTENER_H_
#define INTERFACE_MEDIA_REPORT_EVENT_LISTENER_H_

#include "ImsTypeDef.h"

struct CallReasonInfo;

/**
 * @brief Interface for listening to media reporting events.
 *
 * This interface is implemented by classes that need to react to media-related events
 * such as the start/failure of data reception, video quality changes, and network tone status.
 * It is typically used by the MtcCall or MtcCallState to handle media events propagated
 * by the #IMtcMediaManager.
 */
class IMediaReportEventListener
{
public:
    virtual ~IMediaReportEventListener(){};

    /**
     * @brief Notifies that the reception of media data has started.
     *
     * @param eMediaType The type of media received (e.g., #MEDIATYPE_AUDIO, #MEDIATYPE_VIDEO).
     * @param eProtocolType The transport protocol used (e.g., #MEDIA_PROTOCOL_RTP).
     * @see MEDIA_CONTENT_TYPE
     * @see MEDIA_TRANSPORT_PROTOCOL
     */
    virtual void OnReceivingMediaDataStarted(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) = 0;

    /**
     * @brief Notifies that the reception of media data has failed.
     *
     * This typically indicates an RTP timeout or connection loss for the specific media type.
     *
     * @param eMediaType The type of media that failed (e.g., #MEDIATYPE_AUDIO).
     * @param eProtocolType The transport protocol used (e.g., #MEDIA_PROTOCOL_RTCP).
     * @see MEDIA_CONTENT_TYPE
     * @see MEDIA_TRANSPORT_PROTOCOL
     */
    virtual void OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) = 0;

    /**
     * @brief Notifies that the video bitrate has dropped to the lowest supported level.
     *
     * This event is often used to trigger a downgrade from video to voice call to maintain
     * call continuity under poor network conditions.
     */
    virtual void OnVideoLowestBitRate() = 0;

    /**
     * @brief Notifies that the reception of the network tone has started.
     */
    virtual void OnReceivingNetworkToneStarted() = 0;

    /**
     * @brief Notifies that the reception of the network tone has failed.
     */
    virtual void OnReceivingNetworkToneFailed() = 0;

    /**
     * @brief Notifies of a general media failure.
     *
     * @param objReason The reason for the media failure.(#CallReasonInfo)
     */
    virtual void OnMediaFailed(IN const CallReasonInfo& objReason) = 0;
};

#endif
