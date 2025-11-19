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

import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.its.servercontrol.BasicScenarioTemplates;
import com.android.imsstack.its.servercontrol.ClientMessage;
import com.android.imsstack.its.servercontrol.ControlProtocolConstants;
import com.android.imsstack.its.servercontrol.RuleSet;
import com.android.imsstack.its.servercontrol.RuleSet.Rule;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.servercontrol.ServerMessage;
import com.android.imsstack.its.tests.call.CallTestBase;
import com.android.imsstack.its.tests.call.TestCall;
import com.android.imsstack.its.util.bodyhelper.VoiceCallSdpTemplate;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtCallTest extends CallTestBase {
    static final int TIMING_MARGIN_MS = 1000;

    private TestCall mCall = null;

    @Before
    public void setUp() throws Exception {
        setEnablerStoppable(false);
        setUpBase(SLOT0);

        mImsRegistration = mImsServiceConnector.getRegistration();
        mMmTelFeature = mImsServiceConnector.getMmTelFeature();
        mCall = new TestCall(mMmTelFeature);
        createControlConnection(mCall);
        turnOffQosAndPrecondition();
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);
    }

    @Test
    public void localPreconditionDisabled_remoteRprSupported_180SentAfter183Prack()
            throws Exception {
        logi(this, "localPreconditionDisabled_remoteRprSupported_180SentAfter183Prack");

        // 0. Ensure the RPR is enabled for 180.
        boolean rpr180 = true;
        mConfig.putBoolean(
                CarrierConfigManager.ImsVoice.KEY_PRACK_SUPPORTED_FOR_18X_BOOL, rpr180);

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that precondition mechanism is not used in the 183 response.
        //    Verify that a 183 Session Progress is sent reliably with an SDP answer.
        // 4. Verify that a 180 Ringing response is sent immediately.
        //    Verify that a 180 Ringing response is sent reliably.
        // 7. Verify that a 200 OK response is sent without an SDP.
        // 8. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenario(generator, timeToCallStartMs, timeToWaitAcceptMs, timeToTerminateMs,
                true, false, rpr180, false);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 5. Verify that an incoming call is alerted within 3s.
        // 6. Verify that the call is successfully connected.
        verifyIncomingCallConnection(timeToCallStartMs, timeToWaitAcceptMs);

        // 9. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void localPreconditionDisabled_remoteRprNotSupported_180SentAsFirstResponse()
            throws Exception {
        logi(this, "localPreconditionDisabled_remoteRprNotSupported_180SentAsFirstResponse");

        // 0. Ensure the RPR is enabled for 180.
        boolean rpr180 = true;
        mConfig.putBoolean(
                CarrierConfigManager.ImsVoice.KEY_PRACK_SUPPORTED_FOR_18X_BOOL, rpr180);

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that a 180 Ringing response is sent immediately.
        //    Verify that a 180 Ringing response is sent unreliably.
        // 6. Verify that a 200 OK response is sent without an SDP.
        // 7. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenario(generator, timeToCallStartMs, timeToWaitAcceptMs, timeToTerminateMs,
                false, false, rpr180, false);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 4. Verify that an incoming call is alerted within 3s.
        // 5. Verify that the call is successfully connected.
        verifyIncomingCallConnection(timeToCallStartMs, timeToWaitAcceptMs);

        // 8. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void localPreconditionDisabled_remotePreconditionSupported_noPreconditionInAnswer()
            throws Exception {
        logi(this,
                "localPreconditionDisabled_remotePreconditionSupported_noPreconditionInAnswer");

        // 0. Ensure the RPR is enabled for 180.
        boolean rpr180 = true;
        mConfig.putBoolean(
                CarrierConfigManager.ImsVoice.KEY_PRACK_SUPPORTED_FOR_18X_BOOL, rpr180);

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that precondition mechanism is not used in the 183 response.
        //    Verify that a 183 Session Progress is sent reliably with an SDP answer.
        // 4. Verify that a 180 Ringing response is sent immediately.
        //    Verify that a 180 Ringing response is sent unreliably.
        // 7. Verify that a 200 OK response is sent without an SDP.
        // 8. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenario(generator, timeToCallStartMs, timeToWaitAcceptMs, timeToTerminateMs,
                true, false, rpr180, true);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 5. Verify that an incoming call is alerted within 3s.
        // 6. Verify that the call is successfully connected.
        verifyIncomingCallConnection(timeToCallStartMs, timeToWaitAcceptMs);

        // 9. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void nonRpr180_rprSupported_180SentAsNonRpr() throws Exception {
        logi(this, "nonRpr180_rprSupported_180SentAsNonRpr");

        // 0. Ensure the RPR is disabled for 180.
        boolean rpr180 = false;
        mConfig.putBoolean(
                CarrierConfigManager.ImsVoice.KEY_PRACK_SUPPORTED_FOR_18X_BOOL, rpr180);

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are not enabled
        //    on both the local and remote side.
        // 3. Verify that precondition mechanism is not used in the 183 response.
        //    Verify that a 183 Session Progress is sent reliably with an SDP answer.
        // 4. Verify that a 180 Ringing response is sent immediately.
        //    Verify that a 180 Ringing response is sent reliably.
        // 7. Verify that a 200 OK response is sent without an SDP.
        // 8. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenario(generator, timeToCallStartMs, timeToWaitAcceptMs, timeToTerminateMs,
                true, false, rpr180, false);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 5. Verify that an incoming call is alerted within 3s.
        // 6. Verify that the call is successfully connected.
        verifyIncomingCallConnection(timeToCallStartMs, timeToWaitAcceptMs);

        // 9. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    private void setupScenario(ScenarioGeneratorUtils generator, int timeToCallStartMs,
            int timeToWaitAcceptMs, int timeToWaitByeMs, boolean remoteRprSupported,
            boolean remoteRprRequired, boolean rpr180, boolean remotePrecondition) {
        logi(this, "setupScenario remoteRprSupported=" + remoteRprSupported + " remoteRprRequired="
                + remoteRprRequired + " rpr180=" + rpr180 + " remotePrecondition="
                + remotePrecondition);
        boolean remoteRpr = remoteRprSupported || remoteRprRequired;

        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("INVITE-REGISTER")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_SDP,
                        "audio_evs_without_precondition",
                        VoiceCallSdpTemplate.SDP_AUDIO_EVS)
                .setSdp("audio_evs_without_precondition")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(timeToCallStartMs))
                .addHeader("Supported", "timer")
                .ifTrue(remoteRprRequired, builder -> builder.addConfig(
                        ControlProtocolConstants.CONFIG_REQUIRE_100REL, "true"))
                .ifTrue(remoteRprSupported, builder -> builder.addHeader(
                        "Supported", "100rel"))
                .ifTrue(remotePrecondition, builder -> builder.addHeader(
                        "Supported", "precondition"))
                .build());

        if (remoteRpr) {
            generator.addMessage(new ClientMessage.Builder()
                    .setMethodOrCode("183")
                    .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                            String.valueOf(TIMING_MARGIN_MS))
                    .addRuleSet(new RuleSet.Builder("Require 100rel")
                            .addRule(new Rule.RuleBuilder("Require")
                                    .addContainRule("100rel")
                                    .build())
                            .build())
                    .addRuleSet(new RuleSet.Builder("Not Require precondition")
                            .addRule(new Rule.RuleBuilder("Require")
                                    .addNotContainRule("precondition")
                                    .build())
                            .build())
                    .addRuleSet(new RuleSet.Builder("SDP answer with audio")
                            .addRule(new Rule.RuleBuilder(
                                    ControlProtocolConstants.RULE_CATEGORY_BODY)
                                    .addContainRule("m=audio")
                                    .build())
                            .build())
                    .addRuleSet(new RuleSet.Builder("No precondition attribute")
                            .addRule(new Rule.RuleBuilder(
                                    ControlProtocolConstants.RULE_CATEGORY_BODY)
                                    .addNotContainRule("a=curr:qos")
                                    .build())
                            .build())
                    .build());
            generator.addMessages("<PRACK-183 | >200");
        }

        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("180")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(TIMING_MARGIN_MS))
                .ifTrue(remoteRpr && rpr180, builder -> builder.addRuleSet(
                        new RuleSet.Builder("Require 100rel")
                        .addRule(new Rule.RuleBuilder("Require")
                                .addContainRule("100rel")
                                .build())
                        .build()))
                .ifTrue(!(remoteRpr && rpr180), builder -> builder.addRuleSet(
                        new RuleSet.Builder("Not Require 100rel")
                        .addRule(new Rule.RuleBuilder("Require")
                                .addNotContainRule("100rel")
                                .build())
                        .build()))
                .build());
        if (remoteRpr && rpr180) {
            generator.addMessages("<PRACK-180 | >200");
        }
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("200")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(timeToWaitAcceptMs + TIMING_MARGIN_MS))
                .ifTrue(remoteRpr, builder -> builder.addRuleSet(
                        new RuleSet.Builder("SDP body mustn't be included")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addNotContainRule("@@*@@")
                                .build())
                        .build()))
                .ifTrue(!remoteRpr, builder -> builder.addRuleSet(
                        new RuleSet.Builder("SDP body must be included")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("m=audio")
                                .build())
                        .build()))
                .build());
        generator.addMessages("<ACK-200");

        generator.addDisallowedMessage(
                "BYE should not be sent for 1s", "BYE",
                timeToWaitByeMs - TIMING_MARGIN_MS);
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("BYE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(timeToWaitByeMs + TIMING_MARGIN_MS))
                .build());
        generator.addMessages("<200-BYE");
    }

    private void verifyIncomingCallConnection(int timeToCallStartMs, int timeToWaitAcceptMs) {
        mCall.expectWithin(timeToCallStartMs + TIMING_MARGIN_MS).incomingCall();
        mCall.expectWithin(timeToWaitAcceptMs).nothing();
        mCall.acceptAsVoice();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    private void verifyNormalDisconnection(int timeToTerminateMs) {
        mCall.expectWithin(timeToTerminateMs).nothing();
        mCall.terminate(ImsReasonInfo.CODE_USER_TERMINATED);
        mCall.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED);
        logi(this, "terminated");
        mCall.expectWithin(1000).nothing();
    }
}
