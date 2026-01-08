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
package com.android.imsstack.its.tests.internal.mtc.normal.handover;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import static org.junit.Assert.assertEquals;

import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.annotation.NonNull;

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

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.Objects;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class LteToWlanHandoverTest extends CallTestBase {
    static final int DEFAULT_TERMINATE_TIME_MS = 1000;
    static final int TIMING_MARGIN_MS = 1000;
    static final String PANI_LTE = "3GPP-E-UTRAN-FDD";
    static final String PANI_WLAN = "IEEE-802.11";

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
        mConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);
        tearDownCallTest();
        mConnectivityManagerProxy.setQosSessionBearerType(0);
    }

    /**
     * Given that the device is registered on LTE and re-INVITE on handover is enabled.
     * When a voice call is established and a handover from LTE to WLAN occurs.
     * Then the device should send a re-INVITE with P-Access-Network-Info header including
     * "IEEE-802.11" to the network to update the access network information.
     */
    @Test
    public void reinviteEnabledOnHo_handover_sendReinvite() throws Exception {
        // 0. enable re-INVITE on handover.
        turnOnReinviteOnHandover();

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 3. Verify that the P-Access-Network-Info header in INVITE contains "3GPP-E-UTRAN-FDD".
        // 4. Make the MO call connected.
        // 6. Verify that the P-Access-Network-Info header in re-INVITE contains "IEEE-802.11".
        // 7. Disconnect the call from the server after 1 second.
        int timeToHandoverMs = 2000;
        setupScenarioWithReinvite(generator, timeToHandoverMs, PANI_LTE, PANI_WLAN);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process over LTE (default).
        performRegistration();

        verifyOutgoingCallConnection();

        mCall.expectWithin(timeToHandoverMs).nothing();

        // 5. Perform L2W handover.
        notifyPreciseDataConnectionState(getLtePreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), SLOT0);
        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), SLOT0);

        verifyNormalDisconnection(DEFAULT_TERMINATE_TIME_MS);
    }

    /**
     * Given that the device is registered on LTE and re-INVITE on handover is disabled.
     * When a voice call is established and a handover from LTE to WLAN occurs.
     * Then the device should not send a re-INVITE to the network, and the subsequent SIP message
     * should have P-Access-Network-Info header including "IEEE-802.11".
     */
    @Test
    public void reinviteDisabledOnHo_handover_notSendReinvite() throws Exception {
        // 0. disable re-INVITE on handover (default).

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));
        int timeToHandoverMs = 2000;

        // 3. Verify that the P-Access-Network-Info header in INVITE contains "3GPP-E-UTRAN-FDD".
        // 4. Make the MO call connected.
        // 6. Disconnect the call from the server after 1 second.
        // 7. Verify that the P-Access-Network-Info header in 200 OK for BYE contains
        //    "IEEE-802.11"
        setupScenarioWithoutReinvite(generator, PANI_LTE, PANI_WLAN);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process over LTE (default).
        performRegistration();

        // 2. Make an MO call.
        verifyOutgoingCallConnection();

        mCall.expectWithin(timeToHandoverMs).nothing();

        // 5. Perform L2W handover.
        notifyPreciseDataConnectionState(getLtePreciseDataConnectionState(
                TelephonyManager.DATA_HANDOVER_IN_PROGRESS), SLOT0);
        notifyPreciseDataConnectionState(getIwlanPreciseDataConnectionState(
                TelephonyManager.DATA_CONNECTED), SLOT0);

        verifyNormalDisconnection(DEFAULT_TERMINATE_TIME_MS);
    }

    private void setupScenarioWithoutReinvite(ScenarioGeneratorUtils generator,
            String paniBeforeHo, String paniAfterHo) {
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "5000")
                .addRuleSet(new RuleSet.Builder("INVITE over " + paniBeforeHo)
                        .addRule(new Rule.RuleBuilder("P-Access-Network-Info")
                                .addContainRule(paniBeforeHo)
                                .build())
                        .build())
                .build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("200-INVITE")
                .setSdp(ControlProtocolConstants.SDP_COPY)
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                .build());
        generator.addMessages(">ACK | >REGISTER | <200-REGISTER | <BYE-ACK d-1000");

        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("200-BYE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "1000")
                .addRuleSet(new RuleSet.Builder("BYE 200 over " + paniAfterHo)
                        .addRule(new Rule.RuleBuilder("P-Access-Network-Info")
                                .addContainRule(paniAfterHo)
                                .build())
                        .build())
                .build());
    }

    private void setupScenarioWithReinvite(ScenarioGeneratorUtils generator, int timeToHandoverMs,
            String paniBeforeHo, String paniAfterHo) {
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "5000")
                .addRuleSet(new RuleSet.Builder("INVITE over " + paniBeforeHo)
                        .addRule(new Rule.RuleBuilder("P-Access-Network-Info")
                                .addContainRule(paniBeforeHo)
                                .build())
                        .build())
                .build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("200-INVITE")
                .setSdp(ControlProtocolConstants.SDP_COPY)
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                .build());
        generator.addMessages(">ACK | >REGISTER | <200-REGISTER");

        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY,
                        String.valueOf(timeToHandoverMs + TIMING_MARGIN_MS))
                .addRuleSet(new RuleSet.Builder("INVITE over " + paniAfterHo)
                        .addRule(new Rule.RuleBuilder("P-Access-Network-Info")
                                .addContainRule(paniAfterHo)
                                .build())
                        .build())
                .build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("200-INVITE")
                .setSdp(ControlProtocolConstants.SDP_COPY)
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "10")
                .build());
        generator.addMessages(">ACK | <BYE-ACK d-" + DEFAULT_TERMINATE_TIME_MS + " | >200");
    }

    private void verifyOutgoingCallConnection() {
        mCall.startVoiceCall();
        mCall.expect().initiated();
        assertEquals(mCall.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    private void verifyNormalDisconnection(int timeToTerminateMs) {
        mCall.expectWithin(timeToTerminateMs).nothing();
        mCall.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
        logi(this, "terminated");
        mCall.expectWithin(TIMING_MARGIN_MS).nothing();
    }

    private void turnOnReinviteOnHandover() {
        mConfig.putBoolean(
                CarrierConfig.ImsVoice.KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL, true);
    }
}
