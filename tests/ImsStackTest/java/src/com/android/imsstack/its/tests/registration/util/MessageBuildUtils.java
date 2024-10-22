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
package com.android.imsstack.its.tests.registration.util;

import com.android.imsstack.its.servercontrol.ClientMessage;
import com.android.imsstack.its.servercontrol.ControlProtocolConstants;
import com.android.imsstack.its.servercontrol.RuleSet;

/**
 * This class offers convenient methods for generating messages with common validation rules,
 * simplifying the process
 */
public class MessageBuildUtils {

    public static ClientMessage.Builder getDefaultRegister() {
        return new ClientMessage.Builder()
                .setMethodOrCode("REGISTER")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "5000")
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid From header")
                        .addRule(new RuleSet.Rule.RuleBuilder("From")
                                .addContainRule("ims.mnc001.mcc001.3gppnetwork.org")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid To header")
                        .addRule(new RuleSet.Rule.RuleBuilder("To")
                                .addContainRule("ims.mnc001.mcc001.3gppnetwork.org")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Call-ID header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Call-ID")
                                .addContainRule("@@*@@")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Authorization header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Authorization")
                                .addContainRule("@@*@@")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid Max-Forwards header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Max-Forwards")
                                .addContainRule("@@*@@")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid User-Agent header")
                        .addRule(new RuleSet.Rule.RuleBuilder("User-Agent")
                                .addContainRule("@@*@@")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("REGISTER(1st) : Valid CSeq header")
                        .addRule(new RuleSet.Rule.RuleBuilder("CSeq")
                                .addContainRule("REGISTER")
                                .build())
                        .build());
    }

    public static ClientMessage.Builder getDefaultSubscribe() {
        return new ClientMessage.Builder()
                .setMethodOrCode("SUBSCRIBE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "5000")
                .addRuleSet(new RuleSet.Builder("SUBSCRIBE : Valid From header")
                        .addRule(new RuleSet.Rule.RuleBuilder("From")
                                .addContainRule("ims.mnc001.mcc001.3gppnetwork.org")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("SUBSCRIBE : Valid To header")
                        .addRule(new RuleSet.Rule.RuleBuilder("To")
                                .addContainRule("ims.mnc001.mcc001.3gppnetwork.org")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("SUBSCRIBE : Valid Call-ID header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Call-ID")
                                .addContainRule("@@*@@")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("SUBSCRIBE : Valid Max-Forwards header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Max-Forwards")
                                .addContainRule("@@*@@")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("SUBSCRIBE : Valid User-Agent header")
                        .addRule(new RuleSet.Rule.RuleBuilder("User-Agent")
                                .addContainRule("@@*@@")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("SUBSCRIBE : Valid Contact header")
                        .addRule(new RuleSet.Rule.RuleBuilder("Contact")
                                .addContainRule("@@*@@")
                                .build())
                        .build());
    }
}
