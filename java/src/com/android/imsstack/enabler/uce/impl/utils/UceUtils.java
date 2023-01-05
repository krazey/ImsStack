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

package com.android.imsstack.enabler.uce.impl.utils;

import android.util.ArraySet;

import com.android.imsstack.enabler.uce.impl.define.UceFeatureTags;
import com.android.imsstack.util.ImsLog;

import java.util.Set;

public class UceUtils {
    // The key of the UCE request
    private static int UCE_KEY = 0;

    /**
     * Generate the unique UCE request task id.
     */
    public static synchronized int generateKey() {
        return ++UCE_KEY;
    }

    public static synchronized long getCapabilities(Set<String> ownCapabilities) {
        long featureTags = 0l;
        for (String cap : ownCapabilities) {
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VIDEO.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VIDEO.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_PAGER_MESSAGING.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_PAGER_MESSAGING.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_LARGE_MESSAGING.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_LARGE_MESSAGING.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_SIMPLE_IM.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_SIMPLE_IM.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT.getTag()))
            {
                featureTags |=
                        UceFeatureTags.Tags.FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER_THUMBNAIL.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER_THUMBNAIL.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_FT_STORE_AND_FORWARD.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_FT_STORE_AND_FORWARD.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_FT_HTTP.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_FT_HTTP.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_PUSH.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_PUSH.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_SHARED_MAP.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_SHARED_MAP.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_SHARED_SKETCH.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_SHARED_SKETCH.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER_MMTEL.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER_MMTEL.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_CALL_POST_CALL.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_CALL_POST_CALL.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_FT_SMS.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_FT_SMS.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_SMS.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_SMS.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SESSION.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SESSION.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SA.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SA.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_IS_BOT.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_IS_BOT.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V1.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V1.getCapa();
            }
            if (cap.contains(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V2.getTag())) {
                featureTags |= UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V2.getCapa();
            }
            else{
                ImsLog.d("un-supported feature tag:" + cap);
            }
        }
        return featureTags;
    }

    public static synchronized Set<String> getFeatureTags(long capabilities) {
        Set<String> ownCapabilities = new ArraySet<>();

        if ((UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VIDEO.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VIDEO.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_PAGER_MESSAGING.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_PAGER_MESSAGING.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_LARGE_MESSAGING.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_LARGE_MESSAGING.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_SIMPLE_IM.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_SIMPLE_IM.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT.getCapa() &
                capabilities) > 0) {
            ownCapabilities.add(
                    UceFeatureTags.Tags.FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER_THUMBNAIL.getCapa() & capabilities) > 0)
        {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER_THUMBNAIL.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_FT_STORE_AND_FORWARD.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FT_STORE_AND_FORWARD.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_FT_HTTP.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FT_HTTP.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_PUSH.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_PUSH.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FILE_TRANSFER.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_SHARED_MAP.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_SHARED_MAP.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_SHARED_SKETCH.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_SHARED_SKETCH.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER_MMTEL.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CALL_COMPOSER_MMTEL.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_CALL_POST_CALL.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CALL_POST_CALL.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_FT_SMS.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_FT_SMS.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_SMS.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_GEOLOCATION_SMS.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SESSION.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SESSION.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SA.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_SA.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_IS_BOT.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_IS_BOT.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V1.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V1.getTag());
        }
        if ((UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V2.getCapa() & capabilities) > 0) {
            ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CHATBOT_VERSION_V2.getTag());
        }
        return ownCapabilities;
    }
}
