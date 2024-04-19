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
import com.android.imsstack.its.imsservice.mmtel.call.TestCall;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class SampleCallTest extends CallTestBase {
    private static final int DEFAULT_CALL_INITIATED_TIMER_IN_MILLIS = 10000;

    @Before
    public void setUp() throws Exception {
        TelephonyManagerProxyImpl telephony =
                SystemProxyResolver.getTelephonyManagerProxy(getSubId(SLOT0));
        telephony.setHalVersion(-2, -2);

        setUpBase(SLOT0);
        mImsServiceConnector.createMmTelWrappers();

        mImsRegistration = mImsServiceConnector.getRegistration();
        mMmTelFeature = mImsServiceConnector.getMmTelFeature();
    }

    @After
    public void tearDown() throws Exception {
        tearDownBase(SLOT0);
    }

    @Test
    public void testMoCallSetup() throws Exception {
        turnOffQosAndPrecondition();
        performRegistration();
        logi("testMoCallSetup - TISS: 200");

        // TODO: SetUpTiss : INVITE - 100 - 200 - ACK

        final TestCall call = new TestCall(mMmTelFeature);
        call.startVoiceCall();
        call.expectWithin(10000).initiated();

        assertEquals(call.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    @Test
    public void testCallNormalClearingByUser() throws Exception {
        turnOffQosAndPrecondition();
        performRegistration();
        logi("testCallNormalClearingByUser - TISS: 200");

        // TODO: SetUpTiss : INVITE - 100 - 200 - ACK - BYE - 200

        TestCall call = new TestCall(mMmTelFeature);
        call.startVoiceCall();
        call.expectWithin(10000).not().terminated();
        call.terminate(ImsReasonInfo.CODE_USER_TERMINATED);
        call.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED);
    }

    @Test
    public void testCallNormalClearingByRemote() throws Exception {
        turnOffQosAndPrecondition();
        performRegistration();
        logi("testCallNormalClearingByRemote - TISS: 200");

        // TODO: SetUpTiss : INVITE - 100 - 200 - ACK - BYE - 200

        TestCall call = new TestCall(mMmTelFeature);
        call.startVoiceCall();
        call.expectWithin(10000).not().terminated();
        call.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
    }

    @Test
    public void testCallRejectedBy603() throws Exception {
        turnOffQosAndPrecondition();
        performRegistration();
        logi("testCallRejected - TISS: 603");

        // TODO: SetUpTiss : INVITE - 100 - 603 - ACK

        TestCall call = new TestCall(mMmTelFeature);
        call.startVoiceCall();
        call.expectWithin(10000).not().terminated();
        call.expect().terminated(
                reason -> reason.getCode() == ImsReasonInfo.CODE_SIP_USER_REJECTED);
    }

    @Test
    public void testMtCallSetup() throws Exception {
        turnOffQosAndPrecondition();
        performRegistration();
        logi("testMtCallSetup - TISS: INVITE");

        // TODO: SetUpTiss : INVITE - 183 - PRACK - 200 - 180 - PRACK - 200 - 200 - ACK

        TestCall call = new TestCall(mMmTelFeature);
        call.expectWithin(10000).incomingCall();

        logi("testMtCallSetup - incoming call received");

        logi("testMtCallSetup - accept call");
        call.acceptAsVoice();
        call.expect().initiated();

        assertEquals(call.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    @Test
    public void testMtCallReject() throws Exception {
        turnOffQosAndPrecondition();
        performRegistration();
        logi("testMtCallReject - TISS: INVITE");

        // TODO: SetUpTiss : INVITE - 183 - PRACK - 200 - 180 - PRACK - 200 - 486 - ACK

        TestCall call = new TestCall(mMmTelFeature);
        call.expectWithin(10000).incomingCall();

        logi("testMtCallSetup - incoming call received");

        logi("testMtCallSetup - reject call");
        call.reject(ImsReasonInfo.CODE_USER_DECLINE);
        call.expect().terminated();

        assertEquals(call.getState(), ImsCallSessionImplBase.State.TERMINATED);

        // TODO: Check SIP status code by TISS.
    }
}
