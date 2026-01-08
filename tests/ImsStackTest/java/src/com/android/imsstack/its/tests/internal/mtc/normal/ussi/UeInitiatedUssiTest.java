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

import android.os.Bundle;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
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
 * Tests UE-initiated Unstructured Supplementary Service Data (USSD) over IMS scenarios.
 */
@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class UeInitiatedUssiTest extends CallTestBase {
    private static final int TIMING_MARGIN_MS = 1000;
    private static final int MODE_NOTIFY = 0;

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
     * Given that a UE-initiated USSI session is started.
     * When the device sends an INVITE request to the network.
     * Then it should contain the USSD string in the XML body and the audio port should be 0.
     */
    @Test
    public void sendUssi_inviteSent_hasUssdStringAndAudioPort0() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 3. Set up the server accepts INVITE
        // 4. Verify that To URI is the ussd string with percent encoded.
        //    Verify that Recv-Info and Accept headers correctly set.
        //    Verify that the audio port is set to 0.
        //    Verify that the <ussd-data> xml body is correctly set.
        String ussdString = "*123#";
        setupUeInitiatedUssi(generator, ussdString);

        // 5. Set up the server to send BYE.
        int timeToSendByeMs = 500;
        generator.addMessages("<BYE-ACK d-" + timeToSendByeMs + " | >200");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 2. Make an UE-initiated USSI call.
        Bundle extras = new Bundle();
        extras.putInt(ImsCallProfile.EXTRA_DIALSTRING, ImsCallProfile.DIALSTRING_USSD);
        mCall.start(ussdString, ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, extras);
        mCall.expect().initiated();

        // 6. Verify that the session is terminated normally by the remote party.
        verifyNormalDisconnection(timeToSendByeMs);
    }

    /**
     * Given that a UE-initiated USSI session is established.
     * When the device receives a BYE message containing a USSD string from the network.
     * Then it should notify the telephony of the received USSD string and terminate the session.
     */
    @Test
    public void sendUssi_receiveUssiInBye_notifyString() throws Exception {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.getNormalRegistrationSequence(true));

        // 3. Set up the server accepts INVITE
        // 4. Verify that To URI is the ussd string with percent encoded.
        //    Verify that Recv-Info and Accept headers correctly set.
        //    Verify that the audio port is set to 0.
        //    Verify that the <ussd-data> xml body is correctly set.
        String ussdSentString = "*123#";
        setupUeInitiatedUssi(generator, ussdSentString);

        // 5. Set up the server to send BYE with <ussd-data> xml body.
        String ussdReceivedString = "Check your plan benefits.";
        int timeToSendByeMs = 500;
        setupReceiveUssdInBye(generator, ussdReceivedString, timeToSendByeMs);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        // 1. Complete the normal registration process.
        performRegistration();

        // 2. Make an UE-initiated USSI call.
        Bundle extras = new Bundle();
        extras.putInt(ImsCallProfile.EXTRA_DIALSTRING, ImsCallProfile.DIALSTRING_USSD);
        mCall.start(ussdSentString, ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, extras);
        mCall.expect().initiated();

        // 6. Verify that the received <ussd-data> xml body information is correctly notifies
        //    to the telephony.
        //    And, verify that the session is terminated normally by the remote party.
        //    Note: These two events happen simultaneously so they should be checked together.
        mEventLatch.sleep(timeToSendByeMs + TIMING_MARGIN_MS);
        mCall.expectToHaveBeen().ussdMessageReceived(
                mode -> mode == MODE_NOTIFY,
                ussd -> ussd.equals(ussdReceivedString));

        mCall.expectToHaveBeen().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
        logi(this, "terminated");

        mCall.expectWithin(TIMING_MARGIN_MS).nothing();
    }

    private void setupUeInitiatedUssi(ScenarioGeneratorUtils generator, String ussdString) {
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addRuleSet(new RuleSet.Builder("percent encoded To URI")
                        .addRule(new Rule.RuleBuilder("To")
                                .addContainRule("sip:*123%23;")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("Recv-Info header")
                        .addRule(new Rule.RuleBuilder("Recv-Info")
                                .addContainRule("g.3gpp.ussd")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("Accept header")
                        .addRule(new Rule.RuleBuilder("Accept")
                                .addContainRule("application/sdp")
                                .addContainRule("application/3gpp-ims+xml")
                                .addContainRule("application/vnd.3gpp.ussd+xml")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("INVITE audio port 0")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("m=audio 0 ")
                                .build())
                        .build())
                .addRuleSet(new RuleSet.Builder("USSD body")
                        .addRule(new Rule.RuleBuilder(ControlProtocolConstants.RULE_CATEGORY_BODY)
                                .addContainRule("Content-Type: application/vnd.3gpp.ussd+xml")
                                .addContainRule("<ussd-data>")
                                .addContainRule("<ussd-string>" + ussdString)
                                .build())
                        .build())
                .build());

        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("200-INVITE")
                .setSdp(ControlProtocolConstants.SDP_COPY)
                .build());

        generator.addMessages(">ACK");
    }

    private void setupReceiveUssdInBye(ScenarioGeneratorUtils generator, String ussdString,
            int timeToSendByeMs) {
        generator.addMessage(new ServerMessage.Builder()
                .setMethodOrCode("BYE-ACK")
                .addBodyContent(ControlProtocolConstants.BODY_TYPE_XML,
                        "xml_ussd_body",
                        UssiBodyTemplate.getUssdBody(ussdString, false, "5"))
                .setXml("xml_ussd_body")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, String.valueOf(timeToSendByeMs))
                .setHeader("Content-Type", "application/vnd.3gpp.ussd+xml")
                .removeHeader("Reason")
                .build());
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("200")
                .build());
    }

    private void verifyNormalDisconnection(int timeToTerminateMs) {
        mCall.expectWithin(timeToTerminateMs + TIMING_MARGIN_MS).terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
        logi(this, "terminated");
        mCall.expectWithin(TIMING_MARGIN_MS).nothing();
    }
}
