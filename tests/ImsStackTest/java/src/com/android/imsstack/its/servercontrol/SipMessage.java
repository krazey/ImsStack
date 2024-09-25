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
 * Abstract class representing a SIP message.
 * Provides common attributes and methods for ClientMessage and ServerMessage.
 */
public abstract class SipMessage {
    protected final String mMethodOrCode;
    protected final MessageConfig mConfig;

    protected SipMessage(String methodOrCode, MessageConfig config) {
        mMethodOrCode = methodOrCode;
        mConfig = config;
    }

    /**
     * Gets the SIP method or response code of the message.
     *
     * @return SIP method or response code.
     */
    public String getMethodOrCode() {
        return mMethodOrCode;
    }

    /**
     * Gets the configuration settings for the message.
     *
     * @return Configuration settings.
     */
    public MessageConfig getConfig() {
        return mConfig;
    }

    /**
     * Abstract method to convert the message details to control command.
     *
     * @return Control command formatted as a string.
     */
    public abstract String convertToCommand();

    /**
     * Appends a setting (key-value pair) to the message with a consistent format.
     *
     * @param message The StringBuilder for constructing the message.
     * @param key     The setting key (e.g., MESSAGE, MESSAGE-DIRECTION, MESSAGE_HEADER).
     * @param value   The setting value to be appended.
     */
    protected void appendSetting(StringBuilder message, String key, String value) {
        message.append(key)
                .append(value)
                .append(ControlProtocolConstants.CRLF);
    }
}
