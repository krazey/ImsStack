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

#ifndef INTERFACE_MEDIA_SESSION_LISTENER_H_
#define INTERFACE_MEDIA_SESSION_LISTENER_H_

#include "MediaDef.h"

class ImsMediaMsgParamBase;

class IMediaSessionListener
{
public:
    /**
     * @brief Destructor of IMediaSessionListener
     */
    virtual ~IMediaSessionListener() {};

    /**
     * @brief Send request message to java
     *
     * @param eEvent The event defined in IJniMedia.h
     * @param pParam The parameter of the event
     * @return IMS_BOOL Return IMS_TRUE if the event sent without error
     */
    virtual IMS_BOOL MediaSession_SendMsgToMediaManager(
            IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam) = 0;

    /**
     * @brief Send notification to the client
     *
     * @param eReportType notification type definition of REPORT_TYPE in MediaDef.h.
     * @param eMediaType The media type of notification
     * @param eMediaProtocolType The protocol type of notification, it can be RTP or RTCP
     * @return IMS_BOOL Return IMS_TRUE if the event sent without error
     */
    virtual IMS_BOOL MediaSession_NotifyToClient(IN IMS_UINT32 eReportType,
            IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID,
            IN MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType = MEDIA_PROTOCOL_ANY) = 0;
};

#endif
