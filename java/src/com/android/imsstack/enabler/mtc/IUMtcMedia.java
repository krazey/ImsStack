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

package com.android.imsstack.enabler.mtc;

public class IUMtcMedia {

    public static final int IMS_MEDIA_MSG_REASON = 300;
    public static final int IMS_MEDIA_MSG_NOTIFY = 400;
    public static final int IMS_MSG_BASE_MEDIA = 1500;
    public static final int IMS_MSG_BASE_MEDIA_VIDEO = IMS_MSG_BASE_MEDIA + 50;
    public static final int IMS_MSG_BASE_MEDIA_TEXT = IMS_MSG_BASE_MEDIA + 80;

    // video
    public static final int SETSURFACE_CMD                  = (IMS_MSG_BASE_MEDIA_VIDEO + 1);
    public static final int SELECT_CAMERA_CMD               = (IMS_MSG_BASE_MEDIA_VIDEO + 2);
    public static final int CHANGE_CAMERA_ZOOM_CMD          = (IMS_MSG_BASE_MEDIA_VIDEO + 3);
    public static final int SET_PAUSE_IMAGE_CMD             = (IMS_MSG_BASE_MEDIA_VIDEO + 4);
    public static final int CHANGE_ORIENTATION_CMD          = (IMS_MSG_BASE_MEDIA_VIDEO + 5);

    // rtt
    public static final int RTT_TEXT_SEND_CMD               = (IMS_MSG_BASE_MEDIA_TEXT + 1);
    public static final int RTT_TEXT_RECEIVED_IND           = (IMS_MSG_BASE_MEDIA_TEXT + 2);
    public static final int RTT_AUDIO_INDICATION_IND        = (IMS_MSG_BASE_MEDIA_TEXT + 3);

    public class Reason {
        public static final int REASON_NOERROR      = (IMS_MEDIA_MSG_REASON + 1);
        public static final int REASON_EVENT_FAIL   = (IMS_MEDIA_MSG_REASON + 2);
        public static final int UNDEFINED           = (IMS_MEDIA_MSG_REASON + 99);
    };

    public class Notify {
        public static final int NOTIFY_ORIENTATION_PORTRAIT     = (IMS_MEDIA_MSG_NOTIFY + 1);
        public static final int NOTIFY_ORIENTATION_LANDSCAPE    = (IMS_MEDIA_MSG_NOTIFY + 2);
        public static final int UNDEFINED                       = (IMS_MEDIA_MSG_NOTIFY + 99);
    };

    // Google_IMS_IF :: VIDEO_CALL_PROVIDER {
    public class ParamValue {
        // SETSURFACE_CMD
        public static final int SURFACE_FAR = 1;
        public static final int SURFACE_NEAR = 2;
    };
    // Google_IMS_IF :: VIDEO_CALL_PROVIDER }
};
