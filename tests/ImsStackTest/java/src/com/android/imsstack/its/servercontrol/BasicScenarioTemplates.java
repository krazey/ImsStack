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
 * This class provides basic SIP message flow templates and utilities to generate common SIP
 * scenarios. It includes predefined SIP message sequences for scenarios such as normal registration
 * and voice calls.
 */
public class BasicScenarioTemplates {
    public static final String NORMAL_REGISTRATION_W_SUBSCRIPTION =
            ">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE";
    public static final String MO_VOICE_CALL_CONNECTED = """
            >INVITE | <183-INVITE s-copy | >PRACK | <200-PRACK | <180-INVITE | <200-INVITE d-3000
            | >ACK
            """;
    public static final String MT_VOICE_CALL_ALERTED = """
            <INVITE-REGISTER s-audio_amr_wb_only d-3000 | >183 | <PRACK-183 | >200
            | >180 | <PRACK-180 | >200
            """;

    /**
     * Generates a normal registration sequence of messages. Optionally adds SUBSCRIBE messages
     * based on the {@code useSubscription} flag.
     *
     * @param useSubscription If true, adds SUBSCRIBE messages to the registration sequence.
     * @return A SIP message sequence string representing a normal registration process.
     */
    public static String getNormalRegistrationSequence(boolean useSubscription) {
        StringBuilder sequenceBuilder = new StringBuilder();
        sequenceBuilder.append(">REGISTER | <200-REGISTER");

        if (useSubscription) {
            sequenceBuilder.append(" | >SUBSCRIBE | <200-SUBSCRIBE");
        }
        return sequenceBuilder.toString();
    }
}
