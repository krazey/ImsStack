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

package com.android.imsstack.enabler.media;

/**
 * This consists of constants used by MediaSession
 */
public class MediaConstants {

    public static final int IMS_MSG_BASE_MEDIA = 1400;
    public static final int RESPONSE_WAIT_TIMEOUT = 2500;

    // Requests
    public static final int IMSMEDIA_REQUEST              = IMS_MSG_BASE_MEDIA;
    public static final int REQUEST_OPEN_SESSION          = (IMSMEDIA_REQUEST + 1);
    public static final int REQUEST_CLOSE_SESSION         = (IMSMEDIA_REQUEST + 2);
    public static final int REQUEST_MODIFY_SESSION        = (IMSMEDIA_REQUEST + 3);
    public static final int REQUEST_ADD_CONFIG            = (IMSMEDIA_REQUEST + 4);
    public static final int REQUEST_DELETE_CONFIG         = (IMSMEDIA_REQUEST + 5);
    public static final int REQUEST_CONFIRM_CONFIG        = (IMSMEDIA_REQUEST + 6);
    public static final int REQUEST_SEND_DTMF             = (IMSMEDIA_REQUEST + 7);
    public static final int REQUEST_SET_MEDIA_QUALITY     = (IMSMEDIA_REQUEST + 8);
    public static final int REQUEST_HEADER_EXTENSION      = (IMSMEDIA_REQUEST + 9);
    public static final int REQUEST_QOS                   = (IMSMEDIA_REQUEST + 10);

    public static final int REQUEST_SET_QNS_MEDIA_THRESHOLD = (IMSMEDIA_REQUEST + 11);
    public static final int REQUEST_UPDATE_ANBR_ENABLED_CONFIG = (IMSMEDIA_REQUEST + 12);

    // Requests for video
    public static final int IMSMEDIA_VIDEO_REQUEST        = (IMSMEDIA_REQUEST + 50);
    public static final int REQUEST_SET_PREVIEW_SURFACE   = (IMSMEDIA_VIDEO_REQUEST + 1);
    public static final int REQUEST_SET_DISPLAY_SURFACE   = (IMSMEDIA_VIDEO_REQUEST + 2);
    public static final int REQUEST_VIDEO_DATA_USAGE      = (IMSMEDIA_VIDEO_REQUEST + 3);
    public static final int REQUEST_ADJUST_DELAY          = (IMSMEDIA_VIDEO_REQUEST + 4);

    // Requests for text
    public static final int IMSMEDIA_RTT_REQUEST          = (IMSMEDIA_REQUEST + 80);
    public static final int REQUEST_SEND_RTT              = (IMSMEDIA_RTT_REQUEST + 1);

    // Responses
    public static final int IMSMEDIA_RESPONSE             = (IMSMEDIA_REQUEST + 100);
    public static final int RESPONSE_OPEN_SESSION         = (IMSMEDIA_RESPONSE + 1);
    public static final int RESPONSE_MODIFY_SESSION       = (IMSMEDIA_RESPONSE + 2);
    public static final int RESPONSE_ADD_CONFIG           = (IMSMEDIA_RESPONSE + 3);
    public static final int RESPONSE_CONFIRM_CONFIG       = (IMSMEDIA_RESPONSE + 4);
    public static final int RESPONSE_SESSION_CLOSED       = (IMSMEDIA_RESPONSE + 5);
    public static final int RESPONSE_SESSION_CLOSED_TIMEOUT = (IMSMEDIA_RESPONSE + 6);

    // Notifications
    public static final int NOTIFY_FIRST_PACKET           = (IMSMEDIA_RESPONSE + 11);
    public static final int NOTIFY_HEADER_EXTENSION       = (IMSMEDIA_RESPONSE + 12);
    public static final int NOTIFY_MEDIA_INACTIVITY       = (IMSMEDIA_RESPONSE + 13);
    public static final int NOTIFY_PACKET_LOSS            = (IMSMEDIA_RESPONSE + 14);
    public static final int NOTIFY_MEDIA_QUALITY_STATUS   = (IMSMEDIA_RESPONSE + 15);
    public static final int NOTIFY_CALL_QUALITY_CHANGE    = (IMSMEDIA_RESPONSE + 16);
    public static final int NOTIFY_MEDIA_DETACH           = (IMSMEDIA_RESPONSE + 17);
    public static final int NOTIFY_QOS_INFO               = (IMSMEDIA_RESPONSE + 18);
    public static final int TRIGGER_ANBR_QUERY            = (IMSMEDIA_RESPONSE + 20);
    public static final int NOTIFY_ANBR_RECEIVED          = (IMSMEDIA_RESPONSE + 21);
    public static final int NOTIFY_RTP_RECEPTION_STATS    = (IMSMEDIA_RESPONSE + 22);
    public static final int NOTIFY_DTMF_RECEIVED          = (IMSMEDIA_RESPONSE + 40);

    // Notifications for video
    public static final int IMSMEDIA_VIDEO_RESPONSE       = (IMSMEDIA_RESPONSE + 60);
    public static final int NOTIFY_BITRATE                = (IMSMEDIA_VIDEO_RESPONSE + 1);
    public static final int NOTIFY_PEER_DIMENSION_CHANGED = (IMSMEDIA_VIDEO_RESPONSE + 2);
    public static final int NOTIFY_VIDEO_DATA_USAGE       = (IMSMEDIA_VIDEO_RESPONSE + 3);

    // Notifications for text
    public static final int IMSMEDIA_RTT_RESPONSE         = (IMSMEDIA_RESPONSE + 80);
    public static final int NOTIFY_RTT_RECEIVED           = (IMSMEDIA_RTT_RESPONSE + 1);

    public static final int IMSMEDIA_MAX                  = (IMSMEDIA_RESPONSE + 100);
}
