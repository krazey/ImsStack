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

import android.util.Pair;

import java.util.ArrayList;
import java.util.List;

/**
 * Represents a SIP message sent by a client.
 */
public class ClientMessage extends SipMessage {
    private final String mId;
    private final List<RuleSet> mRuleSets;
    private final Pair<Boolean, String> mDisallowance;

    private ClientMessage(Builder builder) {
        super(builder.mMethodOrCode, builder.mConfig);
        mId = builder.mId;
        mRuleSets = builder.mRuleSets;
        mDisallowance = builder.mDisallowance;
    }

    @Override
    public String convertToCommand() {
        StringBuilder message = new StringBuilder();

        // Append mandatory sections
        appendSetting(message, ControlProtocolConstants.MESSAGE, mMethodOrCode);
        appendSetting(message, ControlProtocolConstants.MESSAGE_DIRECTION,
                ControlProtocolConstants.DIRECTION_SEND);

        // Append ID
        if (mId != null) {
            appendSetting(message, ControlProtocolConstants.MESSAGE_ID, mId);
        }

        // Append configs
        if (!mConfig.getConfigs().isEmpty()) {
            mConfig.getConfigs().forEach((key, value) ->
                    appendSetting(message, ControlProtocolConstants.MESSAGE_CONFIG,
                    key + ":" + value));
        }

        // Append rule sets
        if (!mRuleSets.isEmpty()) {
            mRuleSets.forEach(ruleSet ->
                    appendSetting(message, ControlProtocolConstants.MESSAGE_RULESET,
                            ruleSet.toString())
            );
        }

        // Append disallowed config
        if (mDisallowance != null) {
            appendSetting(message, ControlProtocolConstants.MESSAGE_DISALLOWED,
                    mDisallowance.first + ":" + mDisallowance.second);
        }


        return message.toString();
    }

    public static class Builder {
        private String mMethodOrCode;
        private String mId;
        private final MessageConfig mConfig = new MessageConfig();
        private final List<RuleSet> mRuleSets = new ArrayList<>();
        private Pair<Boolean, String> mDisallowance;

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
         * Sets the message ID that will be used for message targeting.
         *
         * @param id The ID of the message.
         * @return Builder instance for method chaining.
         */
        public Builder setId(String id) {
            mId = id;
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
         * Adds a {@link RuleSet} to the message.
         *
         * <p>A RuleSet defines conditions that need to be fulfilled by the message. This can be
         * used for validation or setting specific rules for message processing.</p>
         *
         * @param ruleSet The RuleSet to add.
         * @return Builder instance for method chaining.
         */
        public Builder addRuleSet(RuleSet ruleSet) {
            mRuleSets.add(ruleSet);
            return this;
        }

        /**
         * Sets the disallowance configuration for the message.
         *
         * <p>If {@code enabled} is set to true, it indicates that the message should not be sent
         * from this step onward. This disallowance remains in effect until another step is
         * encountered where the {@code enabled} parameter is set to false for the same
         * {@code name}.</p>
         *
         * @param enabled Whether to enable or disable the disallowance.
         * @param name The identifier for the disallowance rule.
         * @return Builder instance for method chaining.
         */
        public Builder setDisallowance(boolean enabled, String name) {
            mDisallowance = new Pair<>(enabled, name);
            return this;
        }

        /**
         * Builds the {@link ClientMessage} instance.
         *
         * @return Constructed {@link ClientMessage}.
         */
        public ClientMessage build() {
            return new ClientMessage(this);
        }
    }
}
