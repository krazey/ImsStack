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

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * Represents a set of rules that can be applied in the server control logic. A {@code RuleSet}
 * contains a name and a list of {@code Rule} objects, each specifying categories and rules that
 * either must or must not be contained in the message.
 */
public final class RuleSet {
    private final String mName;
    private final List<Rule> mRules;

    private RuleSet(Builder builder) {
        this.mName = builder.mName;
        this.mRules = builder.mRules;
    }

    /**
     * Converts the {@code RuleSet} to a string representation to send as an external control
     * command.
     *
     * @return A string representation of the rule set, which includes the name and the list of
     *         rules.
     */
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(ControlProtocolConstants.RULE_NAME).append(mName);

        if (!mRules.isEmpty()) {
            String rulesString = mRules.stream()
                    .map(Rule::toString)
                    .collect(Collectors.joining(","));
            sb.append(",").append(rulesString);
        }

        return sb.toString();
    }

    /**
     * A builder class for creating {@link RuleSet} instances.
     */
    public static class Builder {
        private final List<Rule> mRules = new ArrayList<>();
        private final String mName;

        /**
         * Constructor for the {@code Builder}, requiring a name for the {@code RuleSet}.
         *
         * @param name The name of the rule set.
         * @throws NullPointerException if the provided {@code name} is {@code null}
         */
        public Builder(@NonNull String name) {
            this.mName = Objects.requireNonNull(name);
        }

        /**
         * Adds a single rule to the {@code RuleSet}.
         *
         * @param rule A {@link Rule} object to add to the rule set.
         * @return The current {@code Builder} instance.
         * @throws NullPointerException if the provided {@code rule} is {@code null}
         */
        public Builder addRule(@NonNull Rule rule) {
            mRules.add(Objects.requireNonNull(rule));
            return this;
        }

        /**
         * Builds and returns the {@link RuleSet}.
         *
         * @return A new {@code RuleSet} instance.
         */
        public RuleSet build() {
            return new RuleSet(this);
        }
    }

    /**
     * Represents a single rule in a {@code RuleSet}. Each rule specifies a category and rules that
     * either must be contained or must not be contained.
     */
    public static class Rule {
        private final String mCategory;
        private final List<String> mContainRules;
        private final List<String> mNotContainRules;

        private Rule(RuleBuilder builder) {
            this.mCategory = builder.mCategory;
            this.mContainRules = builder.mContainRules;
            this.mNotContainRules = builder.mNotContainRules;
        }

        /**
         * Converts the {@code Rule} to a string representation.
         *
         * @return A string representation of the rule, which includes the category and its
         *         contain/not-contain rules.
         */
        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append(mCategory);

            mContainRules.forEach(containRule ->
                    sb.append("[")
                    .append(ControlProtocolConstants.RULE_CONTAIN)
                    .append(containRule)
                    .append("]"));
            mNotContainRules.forEach(notContainRule ->
                    sb.append("[")
                    .append(ControlProtocolConstants.RULE_NOTCONTAIN)
                    .append(notContainRule)
                    .append("]"));

            return sb.toString();
        }

        /**
         * A builder class for creating {@link Rule} instances.
         */
        public static class RuleBuilder {
            private final List<String> mContainRules = new ArrayList<>();
            private final List<String> mNotContainRules = new ArrayList<>();
            private final String mCategory;

            /**
             * Constructor for the {@code RuleBuilder}, requiring a category for the {@code Rule}.
             *
             * <p>The category represents a part of the SIP message. It can be the name of a SIP
             * header, the body
             * (represented by {@link ControlProtocolConstants#RULE_CATEGORY_BODY}), or the first
             * line of the SIP message (represented by
             * {@link ControlProtocolConstants#RULE_CATEGORY_FIRST_LINE}).</p>
             *
             * @param category The category to assign to the rule.
             * @throws NullPointerException if the provided {@code category} is {@code null}
             */
            public RuleBuilder(@NonNull String category) {
                this.mCategory = Objects.requireNonNull(category);
            }

            /**
             * Adds a rule that must be contained in the message.
             *
             * @param rule The rule that must be contained.
             * @return The current {@code RuleBuilder} instance.
             * @throws NullPointerException if the provided {@code rule} is {@code null}
             */
            public RuleBuilder addContainRule(@NonNull String rule) {
                mContainRules.add(Objects.requireNonNull(rule));
                return this;
            }

            /**
             * Adds a rule that must not be contained in the message.
             *
             * @param rule The rule that must not be contained.
             * @return The current {@code RuleBuilder} instance.
             * @throws NullPointerException if the provided {@code rule} is {@code null}
             */
            public RuleBuilder addNotContainRule(@NonNull String rule) {
                mNotContainRules.add(Objects.requireNonNull(rule));
                return this;
            }

            /**
             * Builds and returns the {@link Rule}.
             *
             * @return A new {@code Rule} instance.
             */
            public Rule build() {
                return new Rule(this);
            }
        }
    }
}
