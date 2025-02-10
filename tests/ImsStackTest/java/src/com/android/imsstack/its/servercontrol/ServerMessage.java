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

import java.util.HashMap;
import java.util.Map;

/**
 * Represents a SIP message sent by the server, supporting headers, configs and body content.
 */
public class ServerMessage extends SipMessage {
    private final String mTarget;
    private final String mBodySdp;
    private final String mBodyXml;
    private final Map<String, String> mHeaders;

    private ServerMessage(Builder builder) {
        super(builder.mMethodOrCode, builder.mConfig);
        mTarget = builder.mTarget;
        mBodySdp = builder.mBodySdp;
        mBodyXml = builder.mBodyXml;
        mHeaders = builder.mHeaders;
    }

    @Override
    public String convertToCommand() {
        StringBuilder message = new StringBuilder();

        // Append mandatory sections
        appendSetting(message, ControlProtocolConstants.MESSAGE, mMethodOrCode);
        appendSetting(message, ControlProtocolConstants.MESSAGE_DIRECTION,
                ControlProtocolConstants.DIRECTION_RECEIVE);

        // Append ID
        if (mTarget != null) {
            appendSetting(message, ControlProtocolConstants.MESSAGE_TARGET, mTarget);
        }

        // Append headers
        if (!mHeaders.isEmpty()) {
            mHeaders.forEach((name, value) -> {
                String formattedValue = value == null ? name : name + ": " + value;
                appendSetting(message, ControlProtocolConstants.MESSAGE_HEADER, formattedValue);
            });
        }

        // Append SDP body if present
        if (mBodySdp != null) {
            appendSetting(message, ControlProtocolConstants.MESSAGE_SDP, mBodySdp);
        }

        // Append XML body if present
        if (mBodyXml != null) {
            appendSetting(message, ControlProtocolConstants.MESSAGE_XML, mBodyXml);
        }

        // Append configs
        if (!mConfig.getConfigs().isEmpty()) {
            mConfig.getConfigs().forEach((key, value) ->
                    appendSetting(message, ControlProtocolConstants.MESSAGE_CONFIG,
                    key + ":" + value));
        }

        return message.toString();
    }

    public static class Builder {
        private String mMethodOrCode;
        private String mTarget;
        private String mBodySdp;
        private String mBodyXml;
        private Map<String, String> mHeaders = new HashMap<>();
        private MessageConfig mConfig = new MessageConfig();

        /**
         * Sets the SIP method or response code.
         *
         * @param methodOrCode SIP method or response code.
         * @return Builder instance for method chaining.
         */
        public Builder setMethodOrCode(String methodOrCode) {
            mMethodOrCode = methodOrCode;
            return this;
        }

        /**
         * Sets the target of the message.
         *
         * @param target The target of the message.
         * @return Builder instance for method chaining.
         */
        public Builder setTarget(String target) {
            mTarget = target;
            return this;
        }

        /**
         * Sets the message body file as SDP content.
         *
         * <p>The {@code sdpFile} specifies the name of the body file to be used as the SDP content.
         * This file is expected to be found in the "preferencefiles/sdp" directory of the
         * External Control Server. The predefined keyword "copy" can be used to indicate that the
         * SDP content should be copied from the SDP that the client previously sent.</p>
         *
         * @param sdpFile The name of the SDP file or the keyword
         *                {@link ControlProtocolConstants#SDP_COPY}.
         * @return Builder instance for method chaining.
         */
        public Builder setSdp(String sdpFile) {
            mBodySdp = sdpFile;
            return this;
        }

        /**
         * Sets the message body file as XML content.
         *
         * <p>The {@code xmlFile} specifies the name of the body file to be used as the XML content.
         * This file is expected to be found in the "preferencefiles/xml" directory of the
         * External Control Server.</p>
         *
         * @param xmlFile The name of the XML file.
         * @return Builder instance for method chaining.
         */
        public Builder setXml(String xmlFile) {
            mBodyXml = xmlFile;
            return this;
        }

        /**
         * Sets a header (overwrites if the header already exists).
         *
         * @param name Header name.
         * @param value Header value.
         * @return Builder instance for method chaining.
         */
        public Builder setHeader(String name, String value) {
            mHeaders.put(name, value);
            return this;
        }

        /**
         * Adds a new header.
         *
         * @param name Header name.
         * @param value Header value.
         * @return Builder instance for method chaining.
         */
        public Builder addHeader(String name, String value) {
            mHeaders.put(ControlProtocolConstants.HEADER_ADD + name, value);
            return this;
        }

        /**
         * Removes a header.
         *
         * @param name Header name to remove.
         * @return Builder instance for method chaining.
         */
        public Builder removeHeader(String name) {
            mHeaders.put(ControlProtocolConstants.HEADER_REMOVE + name, null);
            return this;
        }

        /**
         * Adds a configuration setting to the message.
         *
         * @param key Configuration key.
         * @param value Configuration value.
         * @return Builder instance for method chaining.
         */
        public Builder addConfig(String key, String value) {
            mConfig.addConfig(key, value);
            return this;
        }

        /**
         * Builds the {@link ServerMessage} instance.
         *
         * @return Constructed {@link ServerMessage}.
         */
        public ServerMessage build() {
            return new ServerMessage(this);
        }
    }
}
