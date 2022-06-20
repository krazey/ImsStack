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

#ifndef _INTERFACE_MEDIA_SESSION_CLIENT_LISTENER_H_
#define _INTERFACE_MEDIA_SESSION_CLIENT_LISTENER_H_

#include "IUMedia.h"
#include "IMMedia.h"
#include "MediaDef.h"

class IMediaSessionClientListener
{
public:
    /**
     * REPORT_SUCCESS
     * REPORT_CLOSE_SESSION
     * REPORT_DATA_RECEIVE_FAILED
     * REPORT_DATA_RECEIVE_STARTED
     * REPORT_VIDEO_LOWEST_BIT_RATE
     * REPORT_CHECK_RADIO_CONNECTION
     * REPORT_NW_TONE_RTP_RECEIVE_STARTED
     * REPORT_NW_TONE_RTP_RECEIVE_FAILED
     */
    virtual void MediaSession_Notify(IMS_UINT32 eReportType,
            MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID,
            MEDIA_TRANSPORT_PROTOCOL eMediaProtocolType = MEDIA_PROTOCOL_ANY) = 0;

    /**
     * REPORT_FAILURE
     */
    virtual void MediaSession_NotifyFailures(IMS_UINT32 eReportType, RtpError eError,
            MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) = 0;

    /**
     * @brief Notify QOS parameter
     *
     * @param nNegoId identifier of dialog to report
     * @param bSuccess Qos callback result
     * @param eMediaType media type of notify
     */
    virtual void MediaSession_NotifyQos(IMS_UINTP nNegoId, IMS_BOOL bSuccess,
            MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_INVALID) = 0;

    virtual void MediaSession_RTPInfo(IN IMediaRTPInfoMsgParam* pMsg) = 0;
    virtual void MediaSession_DRAInfo(IN IMediaDRAMsgParam* pMsg) = 0;
};
#endif /* _INTERFACE_MEDIA_SESSION_LISTENER_H_ */
