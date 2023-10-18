/**
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

#ifndef INTERFACE_MEDIA_SESSION_CLIENT_LISTENER_H_
#define INTERFACE_MEDIA_SESSION_CLIENT_LISTENER_H_

#include "IJniMedia.h"
#include "MediaDef.h"

class IMediaSessionClientListener
{
public:
    virtual ~IMediaSessionClientListener(){};
    /**
     * @brief Sends notification to the client
     *
     * @param eReportType notification type definition of REPORT_TYPE in MediaDef.h.
     * @param eMediaType The media type of notification
     * @param eMediaProtocolType The protocol type of notification, it can be RTP or RTCP
     */
    virtual void MediaSession_Notify(IMS_UINT32 eReportType,
            MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID,
            MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType = MEDIA_PROTOCOL_ANY) = 0;

    /**
     * @brief Notifys failure to the client
     *
     * @param eReportType The type of report, it should be REPORT_FAILURE.
     * @param eError The failure reason
     * @param eMediaType The media type of notification
     */
    virtual void MediaSession_NotifyFailures(IMS_UINT32 eReportType, IMS_SINT32 eError,
            MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) = 0;

    /**
     * @brief Notifys QOS parameter
     *
     * @param nNegoId The identifier of dialog to report
     * @param bSuccess The qos callback result
     * @param eMediaType The media type of notify
     */
    virtual void MediaSession_NotifyQos(IMS_UINTP nNegoId, IMS_BOOL bSuccess,
            MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) = 0;
};

#endif
