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
package com.android.imsstack.its.tests.registration.tests;

import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_LTE;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import static org.junit.Assert.assertTrue;

import android.telephony.ServiceState;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.its.base.ServiceStateBuilder;
import com.android.imsstack.its.base.SystemProxyResolver;
import com.android.imsstack.its.tests.registration.RegistrationHelper;
import com.android.imsstack.its.tests.registration.RegistrationInfo;
import com.android.imsstack.its.tests.registration.util.TestRegistration;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class BasicRegistrationTest extends RegistrationTestBase {

    @Before
    public void setUp() throws Exception {

        setRegistrationBaseConfig();

        mTelephony = SystemProxyResolver.getTelephonyManagerProxy(getSubId(SLOT0));
        mTelephony.setHalVersion(-2, -2);

        setUpBase(SLOT0);

        mRegistration = new TestRegistration(mImsServiceConnector.getRegistration());
        mRegistrationHelper = new RegistrationHelper();

        mInfoBuilder = new RegistrationInfo.Builder().setConfig(mConfig);
    }

    @After
    public void tearDown() throws Exception {
        tearDownBase(SLOT0);
    }

    @Test
    public void testTriggerRegistrationWithDefaultConfig() throws Exception {

        logi("RegistrationTest: REGISTER - 200(REG)");

        /*
          TODO: SetUpTiss : REGISTER - 200(REG)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If the REGISTER message does not contain the following headers: From, To, Call-ID,
             Authorization, Max-Forwards, User-Agent, Contact.
          c. If the REGISTER message does not contain a CSeq header with the following parameter:
             "REGISTER".
          d. If the REGISTER message doesn't contain a Contact header with the following parameters:
             +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel, audio, video,
             +g.3gpp.smsip.
         */

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect().registered();

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }

    @Test
    public void testTriggerRegistrationWithCapabilityVoiceOnly() throws Exception {

        logi("RegistrationTest: REGISTER - 200(REG)");

        /*
          TODO: SetUpTiss : REGISTER - 200(REG)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If the REGISTER message does not contain the following headers: From, To, Call-ID,
             Authorization, Max-Forwards, User-Agent, Contact.
          c. If the REGISTER message does not contain a CSeq header with the following parameter:
             "REGISTER".
          d. If the REGISTER message doesn't contain a Contact header with the following parameters:
             +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel.
          e. If the REGISTER message contain a Contact header with the following parameters:
             video. (Note : We currently do not check for the presence of +g.3gpp.smsip).
         */

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder
                .setEnableCapability(CAPABILITY_TYPE_VOICE, REGISTRATION_TECH_LTE)
                .build());

        mRegistration.expect().registered();

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }

    @Test
    public void testTriggerRegistrationWithCapabilitySmsOnly() throws Exception {

        logi("RegistrationTest: REGISTER - 200(REG)");

        /*
          TODO: SetUpTiss : REGISTER - 200(REG)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If the REGISTER message does not contain the following headers: From, To, Call-ID,
             Authorization, Max-Forwards, User-Agent, Contact.
          c. If the REGISTER message does not contain a CSeq header with the following parameter:
             "REGISTER".
          d. If the REGISTER message doesn't contain a Contact header with the following parameters:
             +g.3gpp.smsip.
          e. If the REGISTER message contain a Contact header with the following parameters:
             +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel, audio, video.
         */

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder
                .setEnableCapability(CAPABILITY_TYPE_SMS, REGISTRATION_TECH_LTE)
                .build());

        mRegistration.expect().registered();

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }

    @Test
    public void testTriggerRegistrationRejectedBy423() throws Exception {

        logi("RegistrationTest: REGISTER - 423(REG) - REGISTER - 200(REG)");

        /*
          TODO: SetUpTiss : REGISTER - 423(REG) - REGISTER - 200(REG)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If the REGISTER message does not contain the following headers: From, To, Call-ID,
             Authorization, Max-Forwards, User-Agent, Contact.
          c. If the REGISTER message does not contain a CSeq header with the following parameter:
             "REGISTER".
          d. If the REGISTER message doesn't contain a Contact header with the following parameters:
             +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel, audio, video,
             +g.3gpp.smsip.

          The server must send a 423 response containing Min-Expires.

          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message with an expires value greater than
             the Min-Expires within 10 second.
         */

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect().registering();

        logi("RegistrationTest: Receives 423(REG)");

        mRegistration.expect().registered();

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }

    @Test
    public void testTriggerSubscribeWithDefaultConfig() throws Exception {

        logi("RegistrationTest: REGISTER - 200(REG) - SUBSCRIBE - 200(SUB)");

        /*
          TODO: SetUpTiss : REGISTER - 200(REG) - SUBSCRIBE - 200(SUB)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If it does not receive a SUBSCRIBE message within 10 second after REGISTER.
          c. If the SUBSCRIBE message does not contain the following headers: From, To, Call-ID,
             Max-Forwards, User-Agent, Contact.
          d. If the SUBSCRIBE message does not contain a CSeq header with the following parameter:
             "SUBSCRIBE".
          e. If the SUBSCRIBE message does not include the P-Access-Network-Info header with the
             following parameter: 3GPP-E-UTRAN.
         */

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect().registered();

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }

    @Test
    public void testTriggerSubscribeWithNetworkNr() throws Exception {

        logi("RegistrationTest: REGISTER - 200(REG) - SUBSCRIBE - 200(SUB)");

        /*
          TODO: SetUpTiss : REGISTER - 200(REG) - SUBSCRIBE - 200(SUB)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If it does not receive a SUBSCRIBE message within 10 second after REGISTER.
          c. If the SUBSCRIBE message does not contain the following headers: From, To, Call-ID,
             Max-Forwards, User-Agent, Contact.
          d. If the SUBSCRIBE message does not contain a CSeq header with the following parameter:
             "SUBSCRIBE".
          e. If the SUBSCRIBE message does not include the P-Access-Network-Info header with the
             following parameter: 3GPP-NR.
         */

        ServiceState ss = new ServiceStateBuilder()
                .addNetworkRegistrationInfoForNrCs()
                .addNetworkRegistrationInfoForNr()
                .build();

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder
                .setServiceState(ss)
                .build());

        mRegistration.expect().registered();

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }

    @Test
    public void testTriggerDeregistrationByNotifyUnregistered() throws Exception {

        logi("RegistrationTest: REGISTER - 200(REG) - SUBSCRIBE - 200(SUB) - NOTIFY(Unregistered)");

        /*
          TODO: SetUpTiss : REGISTER - 200(REG) - SUBSCRIBE - 200(SUB) - NOTIFY(Unregistered)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If it does not receive a SUBSCRIBE message within 10 second after REGISTER.

          The server must send a NOTIFY message contain the following attribute:
          a. The state attribute within the <registration> element set to "terminated"
          b. within each <contact> element belonging to this UE
          c. the state attribute set to "terminated" and the event attribute set either to
             "unregistered"
         */

        boolean isRegistered = mRegistrationHelper.performRegistration(this, mInfoBuilder.build());

        assertTrue(isRegistered);

        logi("RegistrationTest: Receives NOTIFY(Unregistered)");
        mRegistration.expect(1000).nothing();

        /*
          TODO : Set a suitable ImsReasonInfo
          Sample :
          mRegistration.expect(3000).deregistered(
              reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR, null, null);
         */

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }

    @Test
    public void testTriggerDeregistrationByNotifyRejected() throws Exception {

        logi("RegistrationTest: REGISTER - 200(REG) - SUBSCRIBE - 200(SUB) - NOTIFY(Rejected)");

        /*
          TODO: SetUpTiss : REGISTER - 200(REG) - SUBSCRIBE - 200(SUB) - NOTIFY(Rejected)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If it does not receive a SUBSCRIBE message within 10 second after REGISTER.

          The server must send a NOTIFY message contain the following attribute:
          a. The state attribute within the <registration> element set to "terminated"
          b. within each <contact> element belonging to this UE
          c. the state attribute set to "terminated" and the event attribute set either to
             "rejected"
         */

        boolean isRegistered = mRegistrationHelper.performRegistration(this, mInfoBuilder.build());

        assertTrue(isRegistered);

        logi("RegistrationTest: Receives NOTIFY(Rejected)");
        mRegistration.expect(1000).nothing();

        /*
          TODO : Set a suitable ImsReasonInfo
          Sample :
          mRegistration.expect(3000).deregistered(
              reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR, null, null);
         */

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }

    @Test
    public void testTriggerDeregistrationByNotifyDeactivated() throws Exception {

        logi("RegistrationTest: REGISTER - 200(REG) - SUBSCRIBE - 200(SUB) - NOTIFY(Deactivated)");

        /*
          TODO: SetUpTiss : REGISTER - 200(REG) - SUBSCRIBE - 200(SUB) - NOTIFY(Deactivated)
          The server should return a failure under the following conditions:
          a. If it does not receive a REGISTER message within 10 second.
          b. If it does not receive a SUBSCRIBE message within 10 second after REGISTER.

          The server must send a NOTIFY message contain the following attribute:
          a. The state attribute within the <registration> element set to "terminated"
          b. within each <contact> element belonging to this UE
          c. the state attribute set to "terminated" and the event attribute set either to
             "deactivated"
         */

        boolean isRegistered = mRegistrationHelper.performRegistration(this, mInfoBuilder.build());

        assertTrue(isRegistered);

        logi("RegistrationTest: Receives NOTIFY(Deactivated)");
        mRegistration.expect(1000).nothing();

        /*
          TODO : Set a suitable ImsReasonInfo
          Sample :
          mRegistration.expect(3000).deregistered(
              reason -> reason.getCode() == ImsReasonInfo.CODE_REGISTRATION_ERROR, null, null);
         */

        /*
          TODO : Assertion from TISS
          assertTrue(expect, message);
         */
    }
}
