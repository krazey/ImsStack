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

package com.android.imsstack.enabler.ssc;

public class SscServiceClassUtil {
    // See 27.007 +CCFC or +CLCK
    public static final int SERVICE_CLASS_NONE          = 0; // no user input
    public static final int SERVICE_CLASS_VOICE         = (1 << 0);
    public static final int SERVICE_CLASS_DATA          = (1 << 1);
    public static final int SERVICE_CLASS_FAX           = (1 << 2);
    public static final int SERVICE_CLASS_SMS           = (1 << 3);
    public static final int SERVICE_CLASS_DATA_SYNC     = (1 << 4);
    public static final int SERVICE_CLASS_DATA_ASYNC    = (1 << 5);
    public static final int SERVICE_CLASS_PACKET        = (1 << 6);
    public static final int SERVICE_CLASS_PAD           = (1 << 7);

    /**
     * IMS internal Service Classes.
     * video class is defined according to #ImsPhoneMmiCode.isSupportedOverImsPhone()
     */
    public static final int SERVICE_CLASS_VIDEO = SERVICE_CLASS_DATA_SYNC + SERVICE_CLASS_PACKET;
    public static final int SERVICE_CLASS_CALL = SERVICE_CLASS_VOICE + SERVICE_CLASS_VIDEO;

    protected static boolean isValid(int serviceClass) {
        if (hasVoice(serviceClass) || hasVideo(serviceClass)
                || serviceClass == SERVICE_CLASS_NONE) {
            return true;
        }

        return false;
    }

    protected static boolean hasVoice(int serviceClass) {
        return (serviceClass & SERVICE_CLASS_VOICE) == SERVICE_CLASS_VOICE;
    }

    protected static boolean hasVideo(int serviceClass) {
        return (serviceClass & SERVICE_CLASS_VIDEO) == SERVICE_CLASS_VIDEO;
    }

    protected static int removeInvalidServiceClass(int serviceClass) {
        return serviceClass & SERVICE_CLASS_CALL;
    }
}
