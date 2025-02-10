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
package com.android.imsstack.its.tests;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import static org.junit.Assert.assertEquals;

import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.its.base.SystemProxyResolver;
import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.servercontrol.BasicScenarioTemplates;
import com.android.imsstack.its.servercontrol.ClientMessage;
import com.android.imsstack.its.servercontrol.ControlProtocolConstants;
import com.android.imsstack.its.servercontrol.RuleSet;
import com.android.imsstack.its.servercontrol.RuleSet.Rule;
import com.android.imsstack.its.servercontrol.Scenario;
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
public class SampleCallTest extends CallTestBase {
    private TestCall mCall = null;

    @Before
    public void setUp() throws Exception {
        TelephonyManagerProxyImpl telephony =
                SystemProxyResolver.getTelephonyManagerProxy(getSubId(SLOT0));
        telephony.setHalVersion(-2, -2);

        setEnablerStoppable(false);
        setUpBase(SLOT0);

        mImsRegistration = mImsServiceConnector.getRegistration();
        mMmTelFeature = mImsServiceConnector.getMmTelFeature();
        mCall = new TestCall(mMmTelFeature);
        createControlConnection(mCall);
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);
    }

    @Test
    public void testMoCallSetup() throws Exception {
        turnOffQosAndPrecondition();
        logi(this, "testMoCallSetup");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessages(BasicScenarioTemplates.MO_VOICE_CALL_CONNECTED);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        performRegistration();

        mCall.startVoiceCall();
        mCall.expectWithin(10000).initiated();

        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    @Test
    public void testMoCallNormalClearingByUser() throws Exception {
        turnOffQosAndPrecondition();
        logi(this, "testMoCallNormalClearingByUser");

        Scenario scenario = new Scenario.Builder()
                .addClientMessage("REGISTER")
                .addServerMessage("200-REGISTER")
                .addClientMessage("SUBSCRIBE")
                .addServerMessage("200-SUBSCRIBE")
                .addClientMessage("INVITE")
                .addServerMessage("100-INVITE")
                .addMessage(
                        new ServerMessage.Builder()
                        .setMethodOrCode("200-INVITE")
                        .setSdp(ControlProtocolConstants.SDP_COPY)
                        .build())
                .addClientMessage("ACK")
                .addClientMessage("BYE")
                .addServerMessage("200-BYE")
                .build();
        mServerControlConnection.sendControlCommand(scenario.toString());

        performRegistration();

        mCall.startVoiceCall();
        mCall.expect().initiated();
        mCall.expectWithin(2000).nothing();
        mCall.terminate(ImsReasonInfo.CODE_USER_TERMINATED);
        mCall.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED);
        mCall.expectWithin(5000).nothing();
    }

    @Test
    public void testMoCallNormalClearingByRemote() throws Exception {
        turnOffQosAndPrecondition();
        logi(this, "testMoCallNormalClearingByRemote");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessages("""
                >INVITE | <100-INVITE | <200-INVITE s-copy d-2000 | >ACK |
                <BYE-ACK d-5000 | >200
                """);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        performRegistration();

        mCall.startVoiceCall();
        mCall.expect().initiated();
        mCall.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.TERMINATED);
    }

    @Test
    public void testMoCallRejectedBy603() throws Exception {
        turnOffQosAndPrecondition();
        logi(this, "testMoCallRejectedBy603");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));
        generator.addMessages(">INVITE | <100-INVITE | <603-INVITE | >ACK");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        performRegistration();

        mCall.startVoiceCall();
        mCall.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_SIP_USER_REJECTED);
    }

    @Test
    public void testMtCallSetup() throws Exception {
        logi(this, "testMtCallSetup");
        turnOffQosAndPrecondition();

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(">REGISTER | <200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        generator.addMessages(BasicScenarioTemplates.MT_VOICE_CALL_ALERTED);
        generator.addMessages(">200 | <ACK-200");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        performRegistration();

        mCall.expectWithin(10000).incomingCall();
        mCall.expectWithin(2000).nothing();
        mCall.acceptAsVoice();
        mCall.expect().initiated();

        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    @Test
    public void testMtCallReject() throws Exception {
        turnOffQosAndPrecondition();
        logi(this, "testMtCallReject");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("INVITE-REGISTER")
                .setSdp("audio_amr_wb_only")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "3000")
                .build());
        generator.addMessages("""
                >183 | <PRACK-183 | >200 | >180 | <PRACK-180 | >200 |
                >486 | <ACK-486
                """);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        performRegistration();

        mCall.expectWithin(10000).incomingCall();

        logi(this, "testMtCallReject - reject call");
        mCall.expectWithin(2000).nothing();
        mCall.reject(ImsReasonInfo.CODE_USER_DECLINE);
        mCall.expect().terminated();

        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.TERMINATED);
    }

    @Test
    public void testMoCallForking() throws Exception {
        turnOffQosAndPrecondition();
        logi(this, "testMoCallForking");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessages(">INVITE | <183-INVITE s-copy| >PRACK | <200-PRACK");
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("183-INVITE")
                .setSdp("copy")
                .addConfig(ControlProtocolConstants.CONFIG_FORKED_RESPONSE, "true")
                .build());
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("PRACK")
                .build());
        generator.addMessages("<200-PRACK");
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("200-INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "3000")
                .build());
        generator.addMessages(">ACK");

        mServerControlConnection.sendControlCommand(generator.build().toString());

        performRegistration();

        mCall.startVoiceCall();
        mCall.expectWithin(10000).initiated();

        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    @Test
    public void testMoCallAudioOnlyWithoutPrecondition() throws Exception {
        turnOffQosAndPrecondition();
        logi(this, "testMoCallAudioOnlyWithoutPrecondition");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "5000")
                .addRuleSet(new RuleSet.Builder("INVITE with Audio only")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("m=audio")
                                .addNotContainRule("m=video")
                                .build())
                        .addRule(new Rule.RuleBuilder("Accept-Contact")
                                .addContainRule("ims.icsi.mmtel")
                                .addNotContainRule("video")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("INVITE without precondition")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addNotContainRule("a=qos")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("Addressing: tel & local")
                        .addRule(new Rule.RuleBuilder(
                                ControlProtocolConstants.RULE_CATEGORY_FIRST_LINE)
                                .addContainRule("tel:")
                                .addContainRule("phone-context@@*@@;user=phone")
                                .build())
                        .build())
                .build());
        generator.addMessages("<100-INVITE | <200-INVITE s-copy d-2000 | >ACK");
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("BYE")
                .setDisallowance(true, "BYE should not be sent for 2s")
                .build());
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("BYE")
                .setDisallowance(false, "BYE should not be sent for 2s")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "2000")
                .build());
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("BYE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "1000")
                .build());
        generator.addMessages("<200-BYE");

        mServerControlConnection.sendControlCommand(generator.build().toString());

        performRegistration();

        mCall.startVoiceCall();
        mCall.expect().initiated();

        logi(this, "testMoCallAudioOnlyWithoutPrecondition expecting initiated");
        mCall.expectWithin(2000).nothing();
        mCall.terminate(ImsReasonInfo.CODE_USER_TERMINATED);
        mCall.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED);
        mCall.expectWithin(5000).nothing();
    }
}
