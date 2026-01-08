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
import com.android.imsstack.its.servercontrol.ServerMessage;

/**
 * This class offers convenient methods for generating messages with common validation rules,
 * simplifying the process
 */
public class MessageBuildUtils {
    private static final String REG_NOTIFY_UNREGISTERED =
            """
            <?xml version="1.0" encoding="UTF-8"?>
            <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
              <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1" \
                state="terminated">
                <contact id="1" state="terminated" event="unregistered">
                  <uri>sip:ue_instance@192.168.200.44:5060</uri>
                  <unknown-param name="reason">Registration Unregistered by Network</unknown-param>
                </contact>
              </registration>
            </reginfo>
            """;

    private static final String REG_NOTIFY_REJECTED =
            """
            <?xml version="1.0" encoding="UTF-8"?>
            <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
              <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1" \
                state="terminated">
                <contact id="1" state="terminated" event="rejected">
                  <uri>sip:ue_instance@192.168.200.44:5060</uri>
                  <unknown-param name="reason">Registration Rejected by Network</unknown-param>
                </contact>
              </registration>
            </reginfo>
            """;

    private static final String REG_NOTIFY_DEACTIVATED =
            """
            <?xml version="1.0" encoding="UTF-8"?>
            <reginfo xmlns="urn:ietf:params:xml:ns:reginfo" version="1" state="full">
              <registration aor="sip:001011123456789@ims.mnc001.mcc001.3gppnetwork.org" id="a1" \
                state="terminated">
                <contact id="1" state="terminated" event="deactivated">
                  <uri>sip:ue_instance@192.168.200.44:5060</uri>
                  <unknown-param name="reason">Registration Deactivated by Network</unknown-param>
                </contact>
              </registration>
            </reginfo>
            """;

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

    public static ServerMessage.Builder getRegNotifyUnregistered() {
        return new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_XML, "reg_notify_unregistered",
                        REG_NOTIFY_UNREGISTERED)
                .setXml("reg_notify_unregistered");
    }

    public static ServerMessage.Builder getRegNotifyRejected() {
        return new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_XML, "reg_notify_rejected",
                        REG_NOTIFY_REJECTED)
                .setXml("reg_notify_rejected");
    }

    public static ServerMessage.Builder getRegNotifyDeactivated() {
        return new ServerMessage.Builder()
                .setMethodOrCode("NOTIFY-SUBSCRIBE")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_XML, "reg_notify_deactivated",
                        REG_NOTIFY_DEACTIVATED)
                .setXml("reg_notify_deactivated");
    }
}
