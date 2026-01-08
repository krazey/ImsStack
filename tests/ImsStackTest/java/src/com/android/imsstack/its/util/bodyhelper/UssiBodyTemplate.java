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

/**
 * A class containing templates for USSI (Unstructured Supplementary Service Data) related message
 * bodies, including SDP and XML content. This class provides default strings and methods to
 * generate customized USSI bodies for testing purposes.
 */
public class UssiBodyTemplate {
    /** Basic USSD string for multiple choice menu. */
    public static final String DEFAULT_USSD_STRING =
            "1 Due Amount\n" //
            + "2 Current Usage\n" //
            + "3 Find Shop\n" //
            + "4 Data Services\n" //
            + "5 Manage your Services\n" //
            + "6 Other Services\n" //
            + "7 Fixedline\n" //
            + "8 DTH\n" //
            + "9 Payments Bank\n";

    /**
     * Default SDP body for a USSI session.
     * <p>
     * Contains placeholders:
     * <ul>
     *     <li>{@code FAKE_MY_IP}: to be replaced with the actual Server IP address.</li>
     * </ul>
     */
    public static final String DEFAULT_USSI_SDP =
            "v=0\n" //
            + "o=- 293814665 293814665 IN IP4 FAKE_MY_IP\n" //
            + "s=-\n" //
            + "c=IN IP4 FAKE_MY_IP\n" //
            + "t=0 0\n" //
            + "m=audio 0 RTP/AVP 0\n" //
            + "a=rtpmap:0 AMR-WB/16000\n";

    private static final String KEYWORD_USSD_STRING = "##USSD_STRING##";
    private static final String KEYWORD_ANY_EXT_TYPE = "##ANY_EXT_TYPE##";
    private static final String KEYWORD_ALERTING_PATTERN = "##ALERTING_PATTERN##";

    private static final String USSD_XML_REQUEST =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" //
            + "\n" //
            + "<ussd-data>\n" //
            + "  <language>EN</language>\n" //
            + "  <anyExt>\n" //
            + "    <UnstructuredSS-" + KEYWORD_ANY_EXT_TYPE + "/>\n" //
            + "    <alertingPattern>" + KEYWORD_ALERTING_PATTERN + "</alertingPattern>\n" //
            + "  </anyExt>\n" //
            + "  <ussd-string>" + KEYWORD_USSD_STRING + "</ussd-string>\n" //
            + "</ussd-data>\n";

    /**
     * Generates a USSD XML body with the specified parameters.
     *
     * @param ussdString The USSD string to be included in {@code <ussd-string>} element.
     * @param anyExtRequest {@code true} for a "Request" type in {@code <UnstructuredSS-*>},
     *                      {@code false} for a "Notify" type.
     * @param alertingPattern The alerting pattern value.
     * @return A string containing the formatted {@code <ussd-string>} XML body.
     */
    public static String getUssdBody(String ussdString, boolean anyExtRequest,
            String alertingPattern) {
        return USSD_XML_REQUEST
                .replace(KEYWORD_USSD_STRING, ussdString)
                .replace(KEYWORD_ANY_EXT_TYPE, anyExtRequest ? "Request" : "Notify")
                .replace(KEYWORD_ALERTING_PATTERN, alertingPattern);
    }
}
