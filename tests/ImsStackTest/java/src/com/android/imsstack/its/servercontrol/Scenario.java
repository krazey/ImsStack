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

import java.util.ArrayList;
import java.util.List;

/**
 * A convenient builder class to create simple SIP scenarios.
 * Allows setting simple client and server messages with additional details if required.
 */
public class Scenario {
    private final List<SipMessage> mMessages;

    private Scenario(Builder builder) {
        mMessages = builder.mMessages;
    }

    /**
     * Builds and gets the entire scenario as a string in the format required by the TISS external
     * control protocol.
     *
     * @return Scenario string representation.
     */
    @Override
    public String toString() {
        StringBuilder scenarioBuilder = new StringBuilder();
        scenarioBuilder.append("TISS-CTRL=scenario-setup")
                .append(ControlProtocolConstants.CRLF)
                .append(ControlProtocolConstants.CRLF);

        for (SipMessage message : mMessages) {
            scenarioBuilder.append(message.convertToCommand())
                    .append(ControlProtocolConstants.CRLF);
        }

        scenarioBuilder.append("TISS-CTRL=scenario-run")
                .append(ControlProtocolConstants.CRLF);
        return scenarioBuilder.toString();
    }

    /**
     * Builder class for {@link Scenario} to enable easy SIP scenario creation.
     */
    public static class Builder {
        private List<SipMessage> mMessages = new ArrayList<>();

        /**
         * Adds a simple client SIP message to the scenario.
         *
         * @param methodOrCode SIP method or response code for the client.
         * @return Builder instance for method chaining.
         */
        public Builder setClientMessage(String methodOrCode) {
            mMessages.add(new ClientMessage.Builder()
                    .setMethodOrCode(methodOrCode)
                    .build());
            return this;
        }

        /**
         * Adds a simple server SIP message to the scenario.
         *
         * @param methodOrCode SIP method or response code for the server.
         * @return Builder instance for method chaining.
         */
        public Builder setServerMessage(String methodOrCode) {
            mMessages.add(new ServerMessage.Builder()
                    .setMethodOrCode(methodOrCode)
                    .build());
            return this;
        }

        /**
         * Adds a detailed custom SIP message to the scenario.
         *
         * @param sipMessage A detailed SipMessage instance.
         * @return Builder instance for method chaining.
         */
        public Builder setMessage(SipMessage sipMessage) {
            mMessages.add(sipMessage);
            return this;
        }

        /**
         * Builds the Scenario with the provided messages.
         *
         * @return Constructed {@link Scenario}.
         */
        public Scenario build() {
            return new Scenario(this);
        }
    }
}
