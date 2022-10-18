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

package com.android.imsstack.enabler.uce.impl.define;

public class UceFeatureTags {

    public enum Tags {
        FEATURE_TAG_PRESENCE("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp", 0x00000001),
        FEATURE_TAG_IPCALL_VOICE("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel", 0x00000002),
        FEATURE_TAG_IPCALL_VIDEO("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\";video", 0x00000004),
        FEATURE_TAG_PAGER_MESSAGING("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg",
                0x00000008),
        FEATURE_TAG_LARGE_MESSAGING("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg",
                0x00000010),
        FEATURE_TAG_CPM_CHAT("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session", 0x00000020),
        FEATURE_TAG_SIMPLE_IM("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im", 0x00000040),
        FEATURE_TAG_STORE_AND_FORWARD_GROUP_CHAT(
                "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fullsfgroupchat", 0x00000080),
        FEATURE_TAG_FILE_TRANSFER_THUMBNAIL("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftthumb",
                0x00000100),
        FEATURE_TAG_FT_STORE_AND_FORWARD("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftstandfw",
                0x00000200),
        FEATURE_TAG_FT_HTTP("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp", 0x00000400),
        FEATURE_TAG_GEOLOCATION_PUSH("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush",
                0x00000800),
        FEATURE_TAG_FILE_TRANSFER("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft", 0x00001000),
        FEATURE_TAG_SHARED_MAP("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap", 0x00002000),
        FEATURE_TAG_SHARED_SKETCH("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch",
                0x00004000),
        FEATURE_TAG_CALL_COMPOSER("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer",
                0x00008000),
        FEATURE_TAG_CALL_COMPOSER_MMTEL("+g.gsma.callcomposer", 0x00010000),
        FEATURE_TAG_CALL_POST_CALL("urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered",
                0x00020000),
        FEATURE_TAG_FT_SMS("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms", 0x00040000),
        FEATURE_TAG_GEOLOCATION_SMS("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms",
                0x00080000),
        FEATURE_TAG_CHATBOT_SESSION("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot",
                0x00100000),
        FEATURE_TAG_CHATBOT_SA("urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa",
                0x00200000),
        FEATURE_TAG_IS_BOT("+g.gsma.rcs.isbot", 0x00400000),
        FEATURE_TAG_CHATBOT_VERSION_V1("+g.gsma.rcs.botversion=\"#=1\"", 0x00800000),
        FEATURE_TAG_CHATBOT_VERSION_V2("+g.gsma.rcs.botversion=\"#=1,#=2\"", 0x01000000);

        private final String mFeatureTag;
        private final long mCapability;

        Tags(String featureTag, long capability) {
            mFeatureTag = featureTag;
            mCapability = capability;
        }

        public String getTag() {
            return mFeatureTag;
        }

        public long getCapa() {
            return mCapability;
        }
    }
}