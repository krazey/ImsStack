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

import android.net.QosSession;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsReasonInfo;
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

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class PreconditionEnabledMoCallTest extends CallTestBase {
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
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);
        mConnectivityManagerProxy.setQosSessionBearerType(0);
    }

    @Test
    public void remotePreconditionEnabled_qosEnabledEarly_updateSentAfterPrack() throws Exception {
        logi(this, "remotePreconditionEnabled_qosEnabledEarly_updateSentAfterPrack");

        // 5. Ensure QoS is acquired immediately after a 183 Session Progress is received.
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 3. Set up a call scenario where preconditions are enabled on both the local
        //    and remote sides.
        // 4. Verify that precondition attributes in the INVITE request are set correctly.
        // 6. Verify that an UPDATE for precondition confirmation is sent.
        // 8. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToTerminateMs = 3000;
        setupScenarioWithRemotePrecondition(generator, 0, timeToTerminateMs);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 2. Make an MO call.
        mCall.startVoiceCall();

        // 7. Verify that the call is successfully connected.
        mCall.expect().initiated();

        // 9. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void remotePreconditionEnabled_qosEnabledLate_updateSentAfterQos() throws Exception {
        logi(this, "remotePreconditionEnabled_qosEnabledLate_updateSentAfterQos");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 3. Set up a call scenario where preconditions are enabled on both the local
        //    and remote sides.
        // 4. Verify that precondition attributes in the INVITE request are set correctly.
        // 6. Verify that an UPDATE for precondition confirmation is sent 2 seconds
        //    after the 183 response is received.
        // 8. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int qosFromStartCallDelayMs = 2000;
        int timeToTerminateMs = 3000;
        setupScenarioWithRemotePrecondition(generator, qosFromStartCallDelayMs, timeToTerminateMs);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 2. Make an MO call.
        mCall.startVoiceCall();

        // 5. Ensure QoS is acquired 2s after a 183 response is received.
        mCall.expectWithin(qosFromStartCallDelayMs).nothing();
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);
        mConnectivityManagerProxy.notifyQosSessionAvailable();

        // 7. Verify that the call is successfully connected.
        mCall.expect().initiated();

        // 9. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void remotePreconditionDisabled_qosEnabledEarly_updateNotSent() throws Exception {
        logi(this, "remotePreconditionDisabled_qosEnabledEarly_updateNotSent");

        // 5. Ensure QoS is acquired immediately after a 183 Session Progress is received.
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 3. Set up a call scenario where preconditions are enabled on the local side,
        //    but disabled on the remote side.
        // 4. Verify that precondition attributes in the INVITE request are set correctly.
        // 6. Verify that an UPDATE for precondition confirmation is not sent.
        // 8. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToTerminateMs = 3000;
        setupScenarioWithoutRemotePrecondition(generator, timeToTerminateMs);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 2. Make an MO call.
        mCall.startVoiceCall();

        // 7. Verify that the call is successfully connected.
        mCall.expect().initiated();

        // 9. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs);
    }

    @Test
    public void remotePreconditionDisabled_qosEnabledLate_updateNotSent() throws Exception {
        logi(this, "remotePreconditionDisabled_qosEnabledLate_updateNotSent");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 3. Set up a call scenario where preconditions are enabled on the local side,
        //    but disabled on the remote side.
        // 4. Verify that precondition attributes in the INVITE request are set correctly.
        // 6. Verify that an UPDATE for precondition confirmation is not sent.
        // 8. Verify the call remains active for 3 seconds, after which a BYE is sent.
        int timeToTerminateMs = 3000;
        setupScenarioWithoutRemotePrecondition(generator, timeToTerminateMs);

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 2. Make an MO call.
        mCall.startVoiceCall();

        // 7. Verify that the call is successfully connected without waiting for QoS
        //    acquisition.
        mCall.expect().initiated();

        // 5. Ensure QoS is acquired 2s after a 183 response is received.
        int qosFromStartCallDelayMs = 2000;
        mCall.expectWithin(qosFromStartCallDelayMs).nothing();
        mConnectivityManagerProxy.setQosSessionBearerType(QosSession.TYPE_EPS_BEARER);
        mConnectivityManagerProxy.notifyQosSessionAvailable();

        // 9. Terminate the call after 3 seconds.
        verifyNormalDisconnection(timeToTerminateMs - qosFromStartCallDelayMs);
    }

    private void setupScenarioWithRemotePrecondition(
            ScenarioGeneratorUtils generator, int qosFromStartCallDelayMs, int timeToWaitByeMs) {
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "5000")
                .addRuleSet(new RuleSet.Builder("INVITE with precondition")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("a=curr:qos local none")
                                .addContainRule("a=curr:qos remote none")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("Supported precondition")
                        .addRule(new Rule.RuleBuilder("Supported")
                                .addContainRule("precondition")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("No Require precondition")
                        .addRule(new Rule.RuleBuilder("Require")
                                .addNotContainRule("precondition")
                                .build())
                        .build())
                .build());
        generator.addMessages("<100-INVITE");
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("183-INVITE")
                .setSdp(ControlProtocolConstants.SDP_COPY)
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                .addConfig(ControlProtocolConstants.CONFIG_REQUIRE_PRECONDITION, "true")
                .build());
        generator.addMessages(">PRACK | <200-PRACK");

        if (qosFromStartCallDelayMs > 0) {
            generator.addDisallowedMessage("UPDATE waits QoS active for 2s", "UPDATE",
                    qosFromStartCallDelayMs);
        }
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("UPDATE")
                .addRuleSet(new RuleSet.Builder("UPDATE with confirmation")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("a=curr:qos local sendrecv")
                                .build())
                        .build())
                .build());
        generator.addMessages("""
                <200-UPDATE s-copy | <180-INVITE | >PRACK | <200-PRACK | <200-INVITE | >ACK
                """);
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

    private void setupScenarioWithoutRemotePrecondition(
            ScenarioGeneratorUtils generator, int timeToWaitByeMs) {
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "5000")
                .addRuleSet(new RuleSet.Builder("INVITE with precondition")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("a=curr:qos local none")
                                .addContainRule("a=curr:qos remote none")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("Supported precondition")
                        .addRule(new Rule.RuleBuilder("Supported")
                                .addContainRule("precondition")
                                .build())
                        .build())
                .build());
        generator.addMessages("<100-INVITE");
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("183-INVITE")
                .setSdp(ControlProtocolConstants.SDP_COPY_WITHOUT_PRECONDITION)
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                .build());
        generator.addMessages(">PRACK | <200-PRACK");

        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("UPDATE")
                .setDisallowance(true, "UPDATE should not be sent")
                .build());

        generator.addMessages("""
                <180-INVITE | >PRACK | <200-PRACK | <200-INVITE | >ACK
                """);
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
