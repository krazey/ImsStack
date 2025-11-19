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
package com.android.imsstack.its.tests.internal.mtc.normal.basic;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import static org.junit.Assert.assertEquals;

import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.annotation.NonNull;

import com.android.imsstack.its.servercontrol.BasicScenarioTemplates;
import com.android.imsstack.its.servercontrol.ClientMessage;
import com.android.imsstack.its.servercontrol.ControlProtocolConstants;
import com.android.imsstack.its.servercontrol.RuleSet;
import com.android.imsstack.its.servercontrol.RuleSet.Rule;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.servercontrol.ServerMessage;
import com.android.imsstack.its.tests.call.CallTestBase;
import com.android.imsstack.its.tests.call.TestCall;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.Objects;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MoCallTest extends CallTestBase {
    static final int DEFAULT_TERMINATE_TIME_MS = 3000;
    static final int TIMING_MARGIN_MS = 1000;

    @NonNull private TestCall mCall = null;

    @Before
    public void setUp() throws Exception {
        setEnablerStoppable(false);
        setUpBase(SLOT0);
        setUpCallTest();

        mMmTelFeature = Objects.requireNonNull(mImsServiceConnector.getMmTelFeature());
        mCall = new TestCall(mMmTelFeature);
        createControlConnection(mCall);
        turnOffQosAndPrecondition();
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);
        tearDownCallTest();
        mConnectivityManagerProxy.setQosSessionBearerType(0);
    }

    @Test
    public void localPreconditionDisabled_receivedNonRpr180_no180Prack() throws Exception {
        // 0. Ensure the 183 is not coming.
        // 0. Ensure the RPR is disabled for 180.
        // 0. Ensure the 180 does not include an SDP.
        boolean rpr183 = false;
        boolean pr180 = true;
        boolean rpr180 = false;
        boolean sdp180 = false;

        // 1. Complete the normal registration process.
        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that a PRACK for 180 is not sent.
        // 4. Verify that an outgoing call is initiated.
        // 5. Verify that the call is successfully connected.
        // 6. Verify the call remains active for 3 seconds, after which a BYE is sent.
        // 7. Terminate the call after 3 seconds.
        performCallSequence(rpr183, pr180, rpr180, sdp180);
    }

    @Test
    public void localPreconditionDisabled_receivedRpr180_180Prack() throws Exception {
        // 0. Ensure the 183 is not coming.
        // 0. Ensure the RPR is enabled for 180.
        // 0. Ensure the 180 does not include an SDP.
        boolean rpr183 = false;
        boolean pr180 = false;
        boolean rpr180 = true;
        boolean sdp180 = false;

        // 1. Complete the normal registration process.
        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that a PRACK for 180 is sent.
        // 4. Verify that an outgoing call is initiated.
        // 5. Verify that the call is successfully connected.
        // 6. Verify the call remains active for 3 seconds, after which a BYE is sent.
        // 7. Terminate the call after 3 seconds.
        performCallSequence(rpr183, pr180, rpr180, sdp180);
    }

    @Test
    public void localPreconditionDisabled_receivedRpr180WithSdp_180Prack() throws Exception {
        // 0. Ensure the 183 is not coming.
        // 0. Ensure the RPR is enabled for 180.
        // 0. Ensure the 180 includes an SDP.
        boolean rpr183 = false;
        boolean pr180 = false;
        boolean rpr180 = true;
        boolean sdp180 = true;

        // 1. Complete the normal registration process.
        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that a PRACK for 180 is sent.
        // 4. Verify that an outgoing call is initiated.
        // 5. Verify that the call is successfully connected.
        // 6. Verify the call remains active for 3 seconds, after which a BYE is sent.
        // 7. Terminate the call after 3 seconds.
        performCallSequence(rpr183, pr180, rpr180, sdp180);
    }

    @Test
    public void localPreconditionDisabled_receivedRpr183AndNonRpr180_183PrackAndNo180Prack()
            throws Exception {
        // 0. Ensure the RPR is enabled for 183.
        // 0. Ensure the RPR is disabled for 180.
        // 0. Ensure the 180 does not include an SDP.
        boolean rpr183 = true;
        boolean pr180 = true;
        boolean rpr180 = false;
        boolean sdp180 = false;

        // 1. Complete the normal registration process.
        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that a PRACK for 183 is sent.
        // 4. Verify that a PRACK for 180 is not sent.
        // 5. Verify that an outgoing call is initiated.
        // 6. Verify that the call is successfully connected.
        // 7. Verify the call remains active for 3 seconds, after which a BYE is sent.
        // 8. Terminate the call after 3 seconds.
        performCallSequence(rpr183, pr180, rpr180, sdp180);
    }

    @Test
    public void localPreconditionDisabled_receivedRpr183AndRpr180_183PrackAnd180Prack()
            throws Exception {
        // 0. Ensure the RPR is enabled for 183.
        // 0. Ensure the RPR is enabled for 180.
        // 0. Ensure the 180 does not include an SDP.
        boolean rpr183 = true;
        boolean pr180 = false;
        boolean rpr180 = true;
        boolean sdp180 = false;

        // 1. Complete the normal registration process.
        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that a PRACK for 183 is sent.
        // 4. Verify that a PRACK for 180 is sent.
        // 5. Verify that an outgoing call is initiated.
        // 6. Verify that the call is successfully connected.
        // 7. Verify the call remains active for 3 seconds, after which a BYE is sent.
        // 8. Terminate the call after 3 seconds.
        performCallSequence(rpr183, pr180, rpr180, sdp180);
    }

    @Test
    public void localPreconditionDisabled_received200without18x_sessionStarts() throws Exception {
        // 0. Ensure the 18x is not coming.
        boolean rpr183 = false;
        boolean pr180 = false;
        boolean rpr180 = false;
        boolean sdp180 = false;

        // 1. Complete the normal registration process.
        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that an outgoing call is initiated.
        // 4. Verify that the call is successfully connected.
        // 5. Verify the call remains active for 3 seconds, after which a BYE is sent.
        // 6. Terminate the call after 3 seconds.
        performCallSequence(rpr183, pr180, rpr180, sdp180);
    }

    private void performCallSequence(boolean rpr183, boolean pr180, boolean rpr180,
            boolean sdp180) throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        setupScenario(generator, DEFAULT_TERMINATE_TIME_MS, rpr183, pr180, rpr180, sdp180);

        mServerControlConnection.sendControlCommand(generator.build().toString());
        performRegistration();
        verifyOutgoingCallConnection();
        verifyNormalDisconnection(DEFAULT_TERMINATE_TIME_MS);
    }

    private void setupScenario(@NonNull ScenarioGeneratorUtils generator, int timeToWaitByeMs,
            boolean rpr183, boolean pr180, boolean rpr180, boolean sdp180) {
        logi(this, "setupScenario rpr183=" + rpr183 + " pr180=" + pr180 + " rpr180=" + rpr180
                + " sdp180=" + sdp180);

        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addRuleSet(new RuleSet.Builder("SDP offer with audio")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("m=audio")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("Not Require 100rel")
                        .addRule(new Rule.RuleBuilder("Require")
                                .addNotContainRule("100rel")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("Not Require precondition")
                        .addRule(new Rule.RuleBuilder("Require")
                                .addNotContainRule("precondition")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("No precondition attribute in offer")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addNotContainRule("a=curr:qos")
                                .build())
                        .build())
                .build());
        generator.addMessages("<100-INVITE");

        if (rpr183) {
            generator.addMessage(new ServerMessage.Builder()
                    .setMethodOrCode("183")
                    .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                    .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_PRECONDITION, "false")
                    .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_100REL, "true")
                    .setSdp(ControlProtocolConstants.SDP_COPY)
                    .build());

            generator.addMessages(">PRACK | <200-PRACK");
        }

        if (rpr180 || pr180) {
            generator.addMessage(new ServerMessage.Builder()
                    .setMethodOrCode("180-INVITE")
                    .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                    .ifTrue(!rpr183, builder -> builder.addConfig(
                            ControlProtocolConstants.CONFIG_REQUIRE_PRECONDITION, "false"))
                    .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_100REL,
                            rpr180 ? "true" : "false")
                    .ifTrue(sdp180, builder -> builder.setSdp(ControlProtocolConstants.SDP_COPY))
                    .build());
        }

        if (rpr180) {
            generator.addMessages(">PRACK | <200-PRACK");
        }

        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("200-INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                .ifTrue(!rpr183 && !sdp180, builder -> builder.setSdp(
                        ControlProtocolConstants.SDP_COPY))
                .build());
        generator.addMessages(">ACK-200");

        generator.addDisallowedMessage("BYE should not be sent for 3s", "BYE",
                timeToWaitByeMs - TIMING_MARGIN_MS);
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("BYE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(timeToWaitByeMs + TIMING_MARGIN_MS))
                .build());
        generator.addMessages("<200-BYE");
    }

    private void verifyOutgoingCallConnection() {
        mCall.startVoiceCall();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    private void verifyNormalDisconnection(int timeToTerminateMs) {
        mCall.expectWithin(timeToTerminateMs).nothing();
        mCall.terminate(ImsReasonInfo.CODE_USER_TERMINATED);
        mCall.expect().terminated(reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED);
        logi(this, "terminated");
        mCall.expectWithin(1000).nothing();
    }
}
