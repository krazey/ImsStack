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
package com.android.imsstack.its.tests.internal.mtc.normal.ussi;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.feature.MmTelFeature;
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
import com.android.imsstack.its.util.bodyhelper.UssiBodyTemplate;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.Objects;

/**
 * Tests network-initiated Unstructured Supplementary Service Data (USSD) over IMS scenarios.
 */
@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class NetworkInitiatedUssiTest extends CallTestBase {
    private static final int TIMING_MARGIN_MS = 1000;
    private static final int MODE_REQUEST = 1;

    @NonNull private TestCall mCall = null;

    @Before
    public void setUp() throws Exception {
        setEnablerStoppable(false);
        setUpBase(SLOT0);
        setUpCallTest();

        mMmTelFeature = Objects.requireNonNull(mImsServiceConnector.getMmTelFeature());
        mCall = new TestCall(mMmTelFeature);
        createControlConnection(mCall);
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);
        tearDownCallTest();
    }

    /**
     * Given that a network-initiated USSI session is received.
     * When the {@code <ussd-string>} is received.
     * Then the device should notify the telephony of the received USSD string.
     */
    @Test
    public void ussiReceived_hasString_notifyString() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up the server to send a network-initiated USSI with a USSD string.
        String ussdString = UssiBodyTemplate.DEFAULT_USSD_STRING;
        setupNetworkInitiatedUssi(generator, ussdString);
        int timeToSendByeMs = 500;
        generator.addMessages("<BYE-200 d-" + timeToSendByeMs + " | >200");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration.
        performRegistration();

        // 3. Verify the incoming call notification for USSD.
        mCall.expectWithin(3000).incomingCall(extras -> {
            return extras.getBoolean(MmTelFeature.EXTRA_IS_USSD, false);
        });

        // 4. Accept the USSI session.
        mCall.acceptAsVoice();

        // 5. Verify that the framework receives the USSD message and the session is established.
        mCall.expectWithin(5000).ussdMessageReceived(
                mode -> mode == MODE_REQUEST,
                ussd -> ussd.equals(ussdString));
        mCall.expect().initiated();

        // 6. Verify that the session is terminated normally by the remote party.
        verifyNormalDisconnection(timeToSendByeMs);
    }

    /**
     * Given that a network-initiated USSI session is established.
     * When the telephony sends a USSD response.
     * Then it should be sent to the network via an INFO message.
     */
    @Test
    public void ussiReceived_sendUssi_sendInfo() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 2. Set up the server to send a network-initiated USSI with a USSD string.
        String ussdReceivedString = UssiBodyTemplate.DEFAULT_USSD_STRING;
        setupNetworkInitiatedUssi(generator, ussdReceivedString);

        // 7. Verify that an INFO request with the USSD string is sent.
        String ussdSentString = "Hello World!";
        setupSendInfo(generator, ussdSentString);
        generator.addMessages("<BYE-200 d-1000 | >200");

        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration.
        performRegistration();

        // 3. Verify the incoming call notification for USSD.
        mCall.expectWithin(3000).incomingCall(extras -> {
            return extras.getBoolean(MmTelFeature.EXTRA_IS_USSD, false);
        });

        // 4. Accept the USSI session.
        mCall.acceptAsVoice();

        // 5. Verify that the framework receives the USSD message and the session is established.
        mCall.expectWithin(5000).ussdMessageReceived(
                mode -> mode == MODE_REQUEST,
                ussd -> ussd.equals(ussdReceivedString));
        mCall.expect().initiated();

        // 6. Send a USSD message to the network.
        mCall.sendUssd(ussdSentString);

        // 8. Verify that the session is terminated normally by the remote party.
        verifyNormalDisconnection(3000);
    }

    private void setupNetworkInitiatedUssi(ScenarioGeneratorUtils generator, String ussdString) {
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("INVITE-REGISTER")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_SDP,
                        "sdp_ussd_body", UssiBodyTemplate.DEFAULT_USSI_SDP)
                .setSdp("sdp_ussd_body")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_XML,
                        "xml_ussd_body",
                        UssiBodyTemplate.getUssdBody(ussdString, true, "5"))
                .setXml("xml_ussd_body")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(2000))
                .setHeader("Recv-Info", "g.3gpp.ussd")
                .setHeader("Accept",
                        "application/sdp,application/vnd.3gpp.ussd+xml,multipart/mixed")
                .build());
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("200")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(TIMING_MARGIN_MS))
                .addRuleSet(new RuleSet.Builder("200 audio port 0")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("m=audio 0 ")
                                .build())
                        .build())
                .build());
        generator.addMessages("<ACK-200");
    }

    private void setupSendInfo(ScenarioGeneratorUtils generator, String ussdString) {
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INFO")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(2000))
                .addRuleSet(new RuleSet.Builder("USSI INFO headers")
                        .addRule(new Rule.RuleBuilder("Info-Package")
                                .addContainRule("g.3gpp.ussd")
                                .build())
                        .addRule(new Rule.RuleBuilder("Content-Type")
                                .addContainRule("application/vnd.3gpp.ussd+xml")
                                .build())
                        .addRule(new Rule.RuleBuilder("Content-Disposition")
                                .addContainRule("Info-Package")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("USSD body")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("ussd-data")
                                .addContainRule("<ussd-string>" + ussdString)
                                .addContainRule("UnstructuredSS-Request/")
                                .build())
                        .build())
                .build());
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("200-INFO")
                .build());
    }

    private void verifyNormalDisconnection(int timeToTerminateMs) {
        mCall.expectWithin(timeToTerminateMs + TIMING_MARGIN_MS).terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
        logi(this, "terminated");
        mCall.expectWithin(TIMING_MARGIN_MS).nothing();
    }
}
