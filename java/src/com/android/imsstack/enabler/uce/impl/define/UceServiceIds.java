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

public class UceServiceIds {

    public enum ServiceIds {
        SERVICE_ID_PRESENCE("org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcse.dp", "1.0"),
        SERVICE_ID_IPCALL("org.3gpp.urn:urn-7:3gpp-service.ims.icsi.mmtel", "1.0"),
        SERVICE_ID_STANDALONE_MESSAGING("org.openmobilealliance:StandaloneMsg", "2.0"),
        SERVICE_ID_CHAT_SESSION("org.openmobilealliance:ChatSession", "2.0"),
        SERVICE_ID_IM_SESSION("org.openmobilealliance:IM-session", "1.0"),
        SERVICE_ID_FILE_TRANSFER("org.openmobilealliance:File-Transfer", "1.0"),
        SERVICE_ID_FT_STORE_FORWARD("org.openmobilealliance:File-Transfer", "2.0"),
        SERVICE_ID_FULL_STORE_FORWARD_GROUP_CHAT(
                "org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.fullsfgroupchat", "1.0"),
        SERVICE_ID_FILE_TRANSFER_THUMBNAIL("org.openmobilealliance:File-Transfer-thumb", "2.0"),
        SERVICE_ID_FILE_TRANSFER_HTTP("org.openmobilealliance:File-Transfer-HTTP", "1.0"),
        SERVICE_ID_GEOLOCATION_PUSH("org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.geopush",
                "1.0"),
        SERVICE_ID_SHARED_MAP("org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.sharedmap", "1.0"),
        SERVICE_ID_SHARED_SKETCH("org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.sharedsketch",
                "1.0"),
        SERVICE_ID_CALL_COMPOSER_V1("org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.callcomposer",
                "1.0"),
        SERVICE_ID_CALL_COMPOSER_V2("org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.callcomposer",
                "2.0"),
        SERVICE_ID_POST_CALL("org.3gpp.urn:urn-7:3gpp-service.ims.icsi.gsma.callunanswered", "1.0"),
        SERVICE_ID_CHATBOT_COMMUNICATION_SESSION(
                "org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.chatbot", "1.0"),
        SERVICE_ID_CHATBOT_STADNALONE_MESSAGING(
                "org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.chatbot.sa", "1.0"),
        SERVICE_ID_CHATBOT_EXTEND_MESSAGE(
                "org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.chatbot.xbotmessage", "1.2"),
        SERVICE_ID_GEOLOCATION_PUSH_VIA_SMS(
                "org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.geosms", "1.0"),
        SERVICE_ID_FILE_TRANSFER_VIA_SMS(
                "org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.ftsms", "1.0"),
        SERVICE_ID_CANCEL_MESSAGE(
                "org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcs.cancelmessage", "1.0");

        private final String mId;
        private final String mVersion;

        ServiceIds(String id, String version) {
            mId = id;
            mVersion = version;
        }

        public String getId() {
            return mId;
        }

        public String getVersion() {
            return mVersion;
        }
    }

    public static final long SERVICE_ID_NONE                                        = 0;
    public static final long SERVICE_ID_PRESENCE                                    = (0x00000001);
    public static final long SERVICE_ID_IPCALL_VOICE                                = (0x00000002);
    public static final long SERVICE_ID_IPCALL_VIDEO                                = (0x00000004);
    public static final long SERVICE_ID_STANDALONE_MESSAGING                        = (0x00000008);
    public static final long SERVICE_ID_CHAT_SESSION                                = (0x00000010);
    public static final long SERVICE_ID_IM_SESSION                                  = (0x00000020);
    public static final long SERVICE_ID_FILE_TRANSFER                               = (0x00000040);
    public static final long SERVICE_ID_FILE_TRANSFER_STORE_FORWARD                 = (0x00000080);
    public static final long SERVICE_ID_FULL_STORE_FORWARD_GROUP_CHAT               = (0x00000100);
    public static final long SERVICE_ID_FILE_TRANSFER_THUMBNAIL                     = (0x00000200);
    public static final long SERVICE_ID_FILE_TRANSFER_HTTP                          = (0x00000400);
    public static final long SERVICE_ID_GEOLOCATION_PUSH                            = (0x00000800);
    public static final long SERVICE_ID_SHARED_MAP                                  = (0x00001000);
    public static final long SERVICE_ID_SHARED_SKETCH                               = (0x00002000);
    public static final long SERVICE_ID_CALL_COMPOSER_V1                            = (0x00004000);
    public static final long SERVICE_ID_CALL_COMPOSER_V2                            = (0x00008000);
    public static final long SERVICE_ID_POST_CALL                                   = (0x00010000);
    public static final long SERVICE_ID_CHATBOT_COMMUNICATION_SESSION               = (0x00020000);
    public static final long SERVICE_ID_CHATBOT_STADNALONE_MESSAGING                = (0x00040000);
    public static final long SERVICE_ID_CHATBOT_EXTEND_MESSAGE                      = (0x00080000);
    public static final long SERVICE_ID_GEOLOCATION_PUSH_VIA_SMS                    = (0x00100000);
    public static final long SERVICE_ID_FILE_TRANSFER_VIA_SMS                       = (0x00200000);
    public static final long SERVICE_ID_CANCEL_MESSAGE                              = (0x00400000);
}