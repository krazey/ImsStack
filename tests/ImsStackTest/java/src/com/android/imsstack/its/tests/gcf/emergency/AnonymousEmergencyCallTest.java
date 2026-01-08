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
package com.android.imsstack.its.tests.gcf.emergency;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.its.servercontrol.BasicScenarioTemplates;
import com.android.imsstack.its.servercontrol.ClientMessage;
import com.android.imsstack.its.servercontrol.ControlProtocolConstants;
import com.android.imsstack.its.servercontrol.RuleSet;
import com.android.imsstack.its.servercontrol.RuleSet.Rule;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.tests.call.CallTestBase;
import com.android.imsstack.its.tests.call.TestCall;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.Objects;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class AnonymousEmergencyCallTest extends CallTestBase {
    private TestCall mCall = null;

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
        tearDownCallTest();
        tearDownBase(SLOT0);
    }

    @Test
    public void testCase_19_4_6_anonymousEmergencyCallAfterERegRejected403() throws Exception {
        turnOffQosAndPrecondition();
        logi(this, "testCase_19_4_6_anonymousEmergencyCallAfterERegRejected403");

        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        generator.addMessages(">REGISTER | <403-REGISTER");
        generator.addMessage(new ClientMessage.Builder()
                .setMethodOrCode("INVITE")
                .addConfig(ControlProtocolConstants.CONFIG_DELAY, "2000")
                .addRuleSet(new RuleSet.Builder("INVITE with anonymous From URI")
                        .addRule(new Rule.RuleBuilder("From")
                                .addContainRule("\"Anonymous\" <sip:anonymous@anonymous.invalid>")
                                .build())
                        .build())
                .build());
        generator.addMessages("""
                <100-INVITE | <183-INVITE s-copy d-100 | >PRACK | <200-PRACK
                | <200-INVITE d-1000 | >ACK | >BYE | <200-BYE
                """);
        mServerControlConnection.sendControlCommand(generator.build().toString());

        performRegistration();

        mCall.start("911", ImsCallProfile.SERVICE_TYPE_EMERGENCY, ImsCallProfile.CALL_TYPE_VOICE,
                null);
        mCall.expectWithin(10000).initiated();
        mCall.expectWithin(2000).nothing();
        mCall.terminate(ImsReasonInfo.CODE_USER_TERMINATED);
        mCall.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED);
        mCall.expectWithin(1000).nothing();
    }
}
