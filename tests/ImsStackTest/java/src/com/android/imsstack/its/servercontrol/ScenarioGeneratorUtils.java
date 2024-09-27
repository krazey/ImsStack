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

import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * A utility class to generate SIP scenarios from a formatted string representation and allow users
 * to add additional messages before final build.
 */
public class ScenarioGeneratorUtils {
    private static final Pattern MESSAGE_PATTERN =
            Pattern.compile("([<>])(\\S+)(?:\\s+s-(\\S+))?(?:\\s+x-(\\S+))?(?:\\s+d-(\\d+))?");
    private final Scenario.Builder mScenarioBuilder;

    /**
     * Constructor for ScenarioGeneratorUtils.
     * Initializes with a scenario builder.
     */
    public ScenarioGeneratorUtils() {
        mScenarioBuilder = new Scenario.Builder();
    }

    /**
     * Parses a SIP message sequence string and adds the respective messages to the scenario.
     * The string is expected to follow this format:
     * >INVITE | <183-INVITE s-copy x-cw_test_body d-100 ...
     *
     * @param sequence The SIP message sequence string.
     */
    public void addMessages(String sequence) {
        String[] parts = sequence.split("\\|");

        for (String part : parts) {
            Matcher matcher = MESSAGE_PATTERN.matcher(part.trim());

            if (matcher.matches()) {
                String direction = matcher.group(1);
                String sipMethodOrResponseCode = matcher.group(2).trim();

                switch (direction) {
                    case ">":
                        addClientMessage(sipMethodOrResponseCode);
                        break;
                    case "<":
                        String sdpValue = matcher.group(3);
                        String xmlValue = matcher.group(4);
                        String delayValue = matcher.group(5);
                        addServerMessage(sipMethodOrResponseCode, sdpValue, xmlValue, delayValue);
                        break;
                    default:
                        throw new IllegalArgumentException("Invalid direction: " + direction);
                }
            }
        }
    }

    /**
     * Adds a custom {@code SipMessage} to the scenario.
     *
     * @param sipMessage Custom SipMessage instance.
     */
    public void addMessage(SipMessage sipMessage) {
        mScenarioBuilder.addMessage(sipMessage);
    }

    /**
     * Adds multiple custom {@code SipMessage}s to the scenario.
     *
     * @param sipMessages List of custom SipMessage instances.
     */
    public void addMessages(List<SipMessage> sipMessages) {
        sipMessages.forEach(this::addMessage);
    }

    /**
     * Finalizes and builds the scenario with all added messages.
     *
     * @return The constructed Scenario instance.
     */
    public Scenario build() {
        return mScenarioBuilder.build();
    }

    private void addClientMessage(String sipMethodOrResponseCode) {
        mScenarioBuilder.addClientMessage(sipMethodOrResponseCode);
    }

    private void addServerMessage(String sipMethodOrResponseCode, String sdpValue, String xmlValue,
            String delayValue) {
        ServerMessage.Builder builder = new ServerMessage.Builder()
                .setMethodOrCode(sipMethodOrResponseCode);

        // Add optional configurations if present
        if (sdpValue != null) {
            builder.setSdp(sdpValue);
        }
        if (xmlValue != null) {
            builder.setXml(xmlValue);
        }
        if (delayValue != null) {
            builder.addConfig(ControlProtocolConstants.CONFIG_DELAY, delayValue);
        }

        addMessage(builder.build());
    }
}
