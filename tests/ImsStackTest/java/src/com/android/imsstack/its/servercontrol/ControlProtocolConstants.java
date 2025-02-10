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

    // Target message ID
    public static final String MESSAGE_ID = "message-id=";
    public static final String MESSAGE_TARGET = "message-target=";

    // configurations
    public static final String MESSAGE_CONFIG = "message-config=";
    public static final String CONFIG_DELAY = "Delay";
    public static final String CONFIG_FORKED_RESPONSE = "ForkedResponse";

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
