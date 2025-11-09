/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.servercontrol;

/**
 * Constants related to SIP and control protocol.
 */
public class ControlProtocolConstants {

    public static final String CRLF = "\r\n";

    // Protocol message constants
    public static final String MESSAGE = "message=";
    public static final String MESSAGE_DIRECTION = "message-direction=";
    public static final String DIRECTION_SEND = "send";
    public static final String DIRECTION_RECEIVE = "receive";

    // Header-related constants
    public static final String MESSAGE_HEADER = "message-header=";
    public static final String HEADER_ADD = "+";
    public static final String HEADER_REMOVE = "-";

    // SDP/Body handling
    public static final String MESSAGE_SDP = "message-sdp=";
    public static final String MESSAGE_XML = "message-xml=";
    public static final String SDP_COPY = "copy";
    public static final String SDP_COPY_WITHOUT_PRECONDITION = "copyRemoteSdpWithoutPrecondition";

    // Target message ID
    public static final String MESSAGE_ID = "message-id=";
    public static final String MESSAGE_TARGET = "message-target=";

    // configurations
    public static final String MESSAGE_CONFIG = "message-config=";
    public static final String CONFIG_DELAY = "Delay";
    public static final String CONFIG_AUDIO_DIR = "audiodirection";
    public static final String CONFIG_VIDEO_DIR = "videodirection";
    public static final String CONFIG_TEXT_DIR = "textdirection";
    public static final String CONFIG_AUDIO_PORT0 = "useaudioport0";
    public static final String CONFIG_VIDEO_PORT0 = "usevideoport0";
    public static final String CONFIG_TEXT_PORT0 = "usetextport0";
    public static final String CONFIG_AMR_SUPPORTED = "amrsupported";
    public static final String CONFIG_AMR_WB_SUPPORTED = "amrwbsupported";
    public static final String CONFIG_EVS_SUPPORTED = "evssupported";
    public static final String CONFIG_TELEPHONE_SUPPORTED = "telephoneeventsupported";
    public static final String CONFIG_H263_SUPPORTED = "h263supported";
    public static final String CONFIG_H264_SUPPORTED = "h264supported";
    public static final String CONFIG_AUDIO_QOS_STATUS = "qosstatusaudio";
    public static final String CONFIG_VIDEO_QOS_STATUS = "qosstatusvideo";
    public static final String CONFIG_TEXT_QOS_STATUS = "qosstatustext";
    public static final String CONFIG_AUDIO_QOS_CONF = "useqosconfaudio";
    public static final String CONFIG_VIDEO_QOS_CONF = "useqosconfvideo";
    public static final String CONFIG_TEXT_QOS_CONF = "useqosconftext";
    public static final String CONFIG_REQUIRE_TIMER = "addrequiretimer";
    public static final String CONFIG_REQUIRE_PRECONDITION = "addrequireprecondition";
    public static final String CONFIG_REQUIRE_100REL = "addrequire100rel";
    public static final String CONFIG_INDIALOG = "indialog";
    public static final String CONFIG_FORKED_RESPONSE = "forkedresponse";
    public static final String CONFIG_FROM_DISPLAY_NAME = "usefromdisplayname";
    public static final String CONFIG_PAID_DISPLAY_NAME = "usepaiddisplayname";
    public static final String CONFIG_PRIVACY_ID = "useprivacyid";
    public static final String CONFIG_ISFOCUS = "addisfocus";
    public static final String CONFIG_CWI = "usecwi";
    public static final String CONFIG_DTMF_ENABLED = "usedtmfenabled";
    public static final String CONFIG_RETRY_AFTER = "useretryafter";
    public static final String CONFIG_PEM_SENDRECV = "usepemsendrecv";
    public static final String CONFIG_EARLY_SESSION = "useearlysession";
    public static final String CONFIG_SUBS_STATE_ACTIVE = "usesubsstateactive";
    public static final String CONFIG_AVCHANGE = "useavchange";
    public static final String CONFIG_COMPACT_FORM = "usecompactform";

    // rule sets
    public static final String MESSAGE_RULESET = "message-ruleset=";
    public static final String RULE_NAME = "name:";
    public static final String RULE_CONTAIN = "contain-";
    public static final String RULE_NOTCONTAIN = "notcontain-";
    public static final String RULE_CATEGORY_BODY = "body";
    public static final String RULE_CATEGORY_FIRST_LINE = "fl";

    // disallowed message
    public static final String MESSAGE_DISALLOWED = "message-disallowed=";
}
