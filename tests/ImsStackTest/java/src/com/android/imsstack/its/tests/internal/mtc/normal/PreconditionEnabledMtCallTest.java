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
package com.android.imsstack.its.tests.internal.mtc.normal;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import static org.junit.Assert.assertEquals;

import android.net.QosSession;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.core.config.CarrierConfig;
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
public class PreconditionEnabledMtCallTest extends CallTestBase {
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
        turnOnQosAndPrecondition();
    }

    @Before
    public void turnOnQosAndPrecondition() {
        if (mConfig == null) {
            mConfig = new PersistableBundle();
        }

        mConfig.putBoolean(
                CarrierConfigManager.ImsVoice.KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL, true);
        mConfig.putBoolean(
                CarrierConfig.ImsVoice.KEY_WAIT_QOS_FOR_INCOMING_INVITE_WITHOUT_PRECONDITION_BOOL,
                        true);
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);
        mConnectivityManagerProxy.setQosSessionBearerType(0);
    }

    @Test
    public void remotePreconditionEnabled_qosEnabledEarly_180SentAfterUpdate() throws Exception {
        logi(this, "remotePreconditionEnabled_qosEnabledEarly_180SentAfterUpdate");

        // 3. Ensure QoS is acquired immediately after a 183 Session Progress is sent.
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are enabled on both the local
        //    and remote sides.
        // 4. Verify that precondition attributes in the 183 response are set correctly.
        // 5. Verify that a 180 Ringing response is sent immediately since QoS is already acquired.
        // 7. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenarioWithRemotePrecondition(generator, timeToCallStartMs, 0, timeToWaitAcceptMs,
                timeToTerminateMs, true, "none");

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 6. Verify that the call is successfully connected.
        mCall.expectWithin(3000).incomingCall();
        mCall.expectWithin(timeToWaitAcceptMs).nothing();
        mCall.acceptAsVoice();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);

        // 8. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void remotePreconditionEnabled_qosEnabledLate_180SentAfterQos() throws Exception {
        logi(this, "remotePreconditionEnabled_qosEnabledLate_180SentAfterQos");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are enabled on both the local
        //    and remote sides.
        // 3. Verify that precondition attributes in the 183 response are set correctly.
        // 5. Verify that a 180 Ringing response is sent after QoS acquisition.
        // 7. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int qosFromStartCallDelayMs = 2000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenarioWithRemotePrecondition(generator, timeToCallStartMs, qosFromStartCallDelayMs,
                timeToWaitAcceptMs, timeToTerminateMs, true, "none");

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 4. Ensure QoS is acquired 2s after a 183 response is sent.
        mCall.expectWithin(qosFromStartCallDelayMs).nothing();
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);
        mConnectivityManagerProxy.notifyQosSessionAvailable();

        // 6. Verify that the call is successfully connected.
        mCall.expectWithin(3000).incomingCall();
        mCall.expectWithin(timeToWaitAcceptMs).nothing();
        mCall.acceptAsVoice();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);

        // 8. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void remotePreconditionEnabled_remoteSendrecv_noConfLineAdded() throws Exception {
        logi(this, "remotePreconditionEnabled_remoteSendrecv_noConfLineAdded");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are enabled on both the local
        //    and remote sides.
        // 3. Verify that precondition attributes in the 183 response are set correctly.
        // 5. Verify that a 180 Ringing response is sent after QoS acquisition.
        // 7. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int qosFromStartCallDelayMs = 2000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenarioWithRemotePrecondition(generator, timeToCallStartMs, qosFromStartCallDelayMs,
                timeToWaitAcceptMs, timeToTerminateMs, false, "sendrecv");

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 4. Ensure QoS is acquired 2s after a 183 response is sent.
        mCall.expectWithin(qosFromStartCallDelayMs).nothing();
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);
        mConnectivityManagerProxy.notifyQosSessionAvailable();

        // 6. Verify that the call is successfully connected.
        mCall.expectWithin(3000).incomingCall();
        mCall.expectWithin(timeToWaitAcceptMs).nothing();
        mCall.acceptAsVoice();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);

        // 8. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void remotePreconditionDisabled_qosEnabledEarly_180SentAfterPrack()
            throws Exception {
        logi(this, "remotePreconditionDisabled_qosEnabledEarly_180SentAfterPrack");

        // 3. Ensure QoS is acquired immediately after a 183 Session Progress is sent.
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are enabled on the local side,
        //    but disabled on the remote side.
        // 4. Verify that precondition mechanism is not used in the 183 response.
        // 5. Verify that a 180 Ringing response is sent after PRACK completion.
        // 7. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenarioWithoutRemotePrecondition(generator, timeToCallStartMs, 0, timeToWaitAcceptMs,
                timeToTerminateMs);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 6. Verify that the call is successfully connected.
        mCall.expectWithin(3000).incomingCall();
        mCall.expectWithin(timeToWaitAcceptMs).nothing();
        mCall.acceptAsVoice();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);

        // 8. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void remotePreconditionDisabled_qosEnabledLate_180SentAfterQos() throws Exception {
        logi(this, "remotePreconditionDisabled_qosEnabledLate_180SentAfterQos");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are enabled on the local side,
        //    but disabled on the remote side.
        // 3. Verify that precondition mechanism is not used in the 183 response.
        // 5. Verify that a 180 Ringing response is sent after QoS acquisition.
        // 7. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int qosFromStartCallDelayMs = 2000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenarioWithoutRemotePrecondition(generator, timeToCallStartMs,
                qosFromStartCallDelayMs, timeToWaitAcceptMs, timeToTerminateMs);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 4. Ensure QoS is acquired 2s after a 183 response is sent.
        mCall.expectWithin(timeToCallStartMs + qosFromStartCallDelayMs).nothing();
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);
        mConnectivityManagerProxy.notifyQosSessionAvailable();

        // 6. Verify that the call is successfully connected.
        mCall.expectWithin(3000).incomingCall();
        mCall.expectWithin(timeToWaitAcceptMs).nothing();
        mCall.acceptAsVoice();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);

        // 8. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void remotePreconditionDisabledAndNotQosWait_qosNotEnabled_180SentAfterPrack()
            throws Exception {
        logi(this, "remotePreconditionDisabledAndNotQosWait_qosNotEnabled_180SentAfterPrack");

        // 0-1. Config : do not wait QoS if incoming INVITE doesn't support precondition.
        mConfig.putBoolean(
                CarrierConfig.ImsVoice.KEY_WAIT_QOS_FOR_INCOMING_INVITE_WITHOUT_PRECONDITION_BOOL,
                        false);
        // 0-2. Ensure QoS is not acquired.

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up a call scenario where preconditions are enabled on the local side,
        //    but disabled on the remote side.
        // 3. Verify that precondition mechanism is not used in the 183 response.
        // 4. Verify that a 180 Ringing response is sent immediately without waiting QoS.
        // 6. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToCallStartMs = 1000;
        int timeToWaitAcceptMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenarioWithoutRemotePrecondition(generator, timeToCallStartMs,
                0, timeToWaitAcceptMs, timeToTerminateMs);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 5. Verify that the call is successfully connected.
        mCall.expectWithin(3000).incomingCall();
        mCall.expectWithin(timeToWaitAcceptMs).nothing();
        mCall.acceptAsVoice();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);

        // 7. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    private void setupScenarioWithRemotePrecondition(
            ScenarioGeneratorUtils generator, int timeToCallStartMs, int qosFromStartCallDelayMs,
            int timeToWaitAcceptMs, int timeToWaitByeMs, boolean confirmationRequired,
            String remoteInitialStatus) {
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("INVITE-REGISTER")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_SDP,
                        "audio_evs_with_precondition",
                        VoiceCallSdpTemplate.SDP_AUDIO_EVS_WITH_PRECONDITION)
                .setSdp("audio_evs_with_precondition")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(timeToCallStartMs))
                .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_100REL, "true")
                .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_PRECONDITION, "false")
                .addConfig(ControlProtocolConstants.CONFIG_AUDIO_QOS_STATUS, remoteInitialStatus)
                .addHeader("Supported", "precondition")
                .build());

        Rule.RuleBuilder preconditionAttributeRuleBuilder =
                new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                .addContainRule("a=curr:qos local none");
        if (confirmationRequired) {
            preconditionAttributeRuleBuilder.addContainRule("a=conf:qos remote sendrecv");
        } else {
            preconditionAttributeRuleBuilder.addNotContainRule("a=conf:qos");
        }

        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("183")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(TIMING_MARGIN_MS))
                .addRuleSet(new RuleSet.Builder("183 with precondition")
                        .addRule(preconditionAttributeRuleBuilder.build()).build())
                .addRuleSet(new RuleSet.Builder("Require precondition")
                        .addRule(new Rule.RuleBuilder("Require")
                                .addContainRule("precondition")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("remote QoS status check")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("a=curr:qos remote " + remoteInitialStatus)
                                .build())
                        .build())
                .build());
        generator.addMessages("<PRACK-183 | >200");
        if (remoteInitialStatus.equals("none")) {
            generator.addMessage(new ServerMessage.Builder()
                    .setMethodOrCode("UPDATE-183")
                    .setSdp(ControlProtocolConstants.SDP_COPY)
                    .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                    .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_PRECONDITION, "true")
                    .addConfig(ControlProtocolConstants.CONFIG_AUDIO_QOS_STATUS, "sendrecv")
                    .addHeader("Supported", "precondition")
                    .build());
            generator.addMessage(new ClientMessage.Builder()
                    .setMethodOrCode("200")
                    .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                            String.valueOf(TIMING_MARGIN_MS))
                    .addRuleSet(new RuleSet.Builder("200 for UPDATE")
                            .addRule(new Rule.RuleBuilder("CSeq")
                                    .addContainRule("UPDATE")
                                    .build())
                            .build())
                    .addRuleSet(new RuleSet.Builder("local QoS status check")
                            .addRule(new Rule.RuleBuilder(
                                    ControlProtocolConstants.RULE_CATEGORY_BODY)
                                    .addContainRule(qosFromStartCallDelayMs > 0
                                            ? "a=curr:qos local none" : "a=curr:qos local sendrecv")
                                    .addNotContainRule("a=conf:qos")
                                    .build())
                            .build())
                    .build());
        }

        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("180")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(qosFromStartCallDelayMs + TIMING_MARGIN_MS))
                .build());
        generator.addMessages("<PRACK-180 | >200");
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("200")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(timeToWaitAcceptMs + TIMING_MARGIN_MS))
                .addRuleSet(new RuleSet.Builder("SDP body mustn't be included")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addNotContainRule("@@*@@")
                                .build())
                        .build())
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

    private void setupScenarioWithoutRemotePrecondition(
            ScenarioGeneratorUtils generator,  int timeToCallStartMs, int qosFromStartCallDelayMs,
            int timeToWaitAcceptMs, int timeToWaitByeMs) {
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("INVITE-REGISTER")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_SDP,
                        "audio_evs_without_precondition",
                        VoiceCallSdpTemplate.SDP_AUDIO_EVS)
                .setSdp("audio_evs_without_precondition")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(timeToCallStartMs))
                .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_100REL, "true")
                .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_PRECONDITION, "false")
                .build());
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("183")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(TIMING_MARGIN_MS))
                .addRuleSet(new RuleSet.Builder("183 without precondition")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addNotContainRule("a=curr:qos")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("Not Require precondition")
                        .addRule(new Rule.RuleBuilder("Require")
                                .addNotContainRule("precondition")
                                .build())
                        .build())
                .build());
        generator.addMessages("<PRACK-183 | >200");

        if (qosFromStartCallDelayMs > 0) {
            generator.addDisallowedMessage("180 waits QoS active for 2s", "180",
                    qosFromStartCallDelayMs);
        }
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("180")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(timeToCallStartMs + qosFromStartCallDelayMs))
                .build());
        generator.addMessages("<PRACK-180 | >200");
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("200")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(timeToWaitAcceptMs + TIMING_MARGIN_MS))
                .addRuleSet(new RuleSet.Builder("SDP body mustn't be included")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addNotContainRule("@@*@@")
                                .build())
                        .build())
                .build());
        generator.addMessages("<ACK-200");

        generator.addDisallowedMessage(
                "BYE should not be sent for 2s", "BYE",
                timeToWaitByeMs - TIMING_MARGIN_MS);
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("BYE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(timeToWaitByeMs + TIMING_MARGIN_MS))
                .build());
        generator.addMessages("<200-BYE");
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
