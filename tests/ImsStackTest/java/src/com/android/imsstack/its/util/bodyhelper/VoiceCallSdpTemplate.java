/*
 * Copyright (C) 2025 The Android Open Source Project
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
package com.android.imsstack.its.util.bodyhelper;

import com.android.imsstack.its.servercontrol.ControlProtocolConstants;

/**
 * A class containing SDP templates for voice calls.
 */
public class VoiceCallSdpTemplate {

    /**
     * SDP for an EVS audio call with precondition attributes.
     * <p>
     * Contains placeholders:
     * <ul>
     *     <li>{@code FAKE_MY_IP}: to be replaced with the actual Server IP address.</li>
     *     <li>{@code QOS_STATUS_AUDIO}: to be replaced with
     *         {@link ControlProtocolConstants#CONFIG_AUDIO_QOS_STATUS}.</li>
     *     <li>{@code MEDIA_DIRECTION_AUDIO}: to be replaced with
     *         {@link ControlProtocolConstants#CONFIG_AUDIO_DIR}.</li>
     * </ul>
     */
    public static final String SDP_AUDIO_EVS_WITH_PRECONDITION =
            "v=0\n" //
            + "o=- 3972108993 3972108993 IN IP4 FAKE_MY_IP\n" //
            + "s=-\n" //
            + "c=IN IP4 FAKE_MY_IP\n" //
            + "t=0 0\n" //
            + "m=audio 50012 RTP/AVP 127 100 105 98 103 96 97\n" //
            + "b=AS:41\n" //
            + "b=RS:600\n" //
            + "b=RR:2000\n" //
            + "a=rtpmap:127 EVS/16000/1\n" //
            + "a=fmtp:127 max-red=0;br=5.9-24.4;bw=nb-swb;cmr=0\n" //
            + "a=rtpmap:100 AMR-WB/16000/1\n" //
            + "a=fmtp:100 mode-change-capability=2;max-red=0\n" //
            + "a=rtpmap:105 AMR-WB/16000/1\n" //
            + "a=fmtp:105 octet-align=1;mode-change-capability=2;max-red=0\n" //
            + "a=rtpmap:98 AMR/8000/1\n" //
            + "a=fmtp:98 mode-change-capability=2;max-red=0\n" //
            + "a=rtpmap:103 AMR/8000/1\n" //
            + "a=fmtp:103 octet-align=1;mode-change-capability=2;max-red=0\n" //
            + "a=rtpmap:96 telephone-event/16000\n" //
            + "a=fmtp:96 0-15\n" //
            + "a=rtpmap:97 telephone-event/8000\n" //
            + "a=fmtp:97 0-15\n" //
            + "a=curr:qos local QOS_STATUS_AUDIO\n" //
            + "a=curr:qos remote none\n" //
            + "a=des:qos mandatory local sendrecv\n" //
            + "a=des:qos optional remote sendrecv\n" //
            + "a=MEDIA_DIRECTION_AUDIO\n" //
            + "a=ptime:20\n" //
            + "a=maxptime:240\n";

    /**
     * SDP for an EVS audio call without precondition attributes.
     * <p>
     * Contains placeholders:
     * <ul>
     *     <li>{@code FAKE_MY_IP}: to be replaced with the actual Server IP address.</li>
     *     <li>{@code MEDIA_DIRECTION_AUDIO}: to be replaced with
     *         {@link ControlProtocolConstants#CONFIG_AUDIO_DIR}.</li>
     * </ul>
     */
    public static final String SDP_AUDIO_EVS =
            "v=0\n" //
            + "o=- 3972108993 3972108993 IN IP4 FAKE_MY_IP\n" //
            + "s=-\n" //
            + "c=IN IP4 FAKE_MY_IP\n" //
            + "t=0 0\n" //
            + "m=audio 50012 RTP/AVP 127 100 105 98 103 96 97\n" //
            + "b=AS:41\n" //
            + "b=RS:600\n" //
            + "b=RR:2000\n" //
            + "a=rtpmap:127 EVS/16000/1\n" //
            + "a=fmtp:127 max-red=0;br=5.9-24.4;bw=nb-swb;cmr=0\n" //
            + "a=rtpmap:100 AMR-WB/16000/1\n" //
            + "a=fmtp:100 mode-change-capability=2;max-red=0\n" //
            + "a=rtpmap:105 AMR-WB/16000/1\n" //
            + "a=fmtp:105 octet-align=1;mode-change-capability=2;max-red=0\n" //
            + "a=rtpmap:98 AMR/8000/1\n" //
            + "a=fmtp:98 mode-change-capability=2;max-red=0\n" //
            + "a=rtpmap:103 AMR/8000/1\n" //
            + "a=fmtp:103 octet-align=1;mode-change-capability=2;max-red=0\n" //
            + "a=rtpmap:96 telephone-event/16000\n" //
            + "a=fmtp:96 0-15\n" //
            + "a=rtpmap:97 telephone-event/8000\n" //
            + "a=fmtp:97 0-15\n" //
            + "a=MEDIA_DIRECTION_AUDIO\n" //
            + "a=ptime:20\n" //
            + "a=maxptime:240\n";
}
