/*
 * Copyright (C) 2022 The Android Open Source Project
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

package com.android.imsstack.imsservice.mmtel;

import static org.junit.Assert.assertEquals;

import android.telephony.ims.ImsSuppServiceNotification;

import com.android.internal.telephony.gsm.SuppServiceNotification;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ImsSuppServiceUtilsTest {
    private ImsSuppServiceUtils mImsSuppServiceUtils;
    private ImsSuppServiceUtils.MO mMoSuppUtils;
    private ImsSuppServiceUtils.MT mMtSuppUtils;
    private static final String NUMBER = "8888";
    private static final String[] CF_HISTORY = new String[]{"123", "456"};

    @Before
    public void setUp() throws Exception {
        mImsSuppServiceUtils = new ImsSuppServiceUtils();
        mMoSuppUtils = new ImsSuppServiceUtils.MO();
        mMtSuppUtils = new ImsSuppServiceUtils.MT();
    }

    @Test
    public void test_createNotification() {
        mImsSuppServiceUtils.createNotification(1, 2);
        ImsSuppServiceNotification mSuppService =
                new ImsSuppServiceNotification(mImsSuppServiceUtils.NOTIFICATION_MO, 2, -1, 1,
                NUMBER, CF_HISTORY);
        if (mSuppService != null) {
            assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
            assertEquals(mSuppService.code, 2);
            assertEquals(mSuppService.index, -1);
            assertEquals(mSuppService.type, 1);
            assertEquals(mSuppService.number, NUMBER);
            for (int i = 0; i < CF_HISTORY.length; i++) {
                assertEquals(mSuppService.history[i], CF_HISTORY[i]);
            }
        }
    }

    @Test
    public void test_createNotificationWithNumber() {
        mImsSuppServiceUtils.createNotification(1, 2, NUMBER);
        ImsSuppServiceNotification mSuppService =
                new ImsSuppServiceNotification(mImsSuppServiceUtils.NOTIFICATION_MO, 2, -1, -1,
                null, CF_HISTORY);
        if (mSuppService != null) {
            assertEquals(mSuppService.type, -1);
            assertEquals(mSuppService.number, null);
        }
    }

    @Test
    public void test_getUnconditionalCfActive() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getUnconditionalCfActive();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_UNCONDITIONAL_CF_ACTIVE);
    }

    @Test
    public void test_getSomeCfActive() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getSomeCfActive();
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_SOME_CF_ACTIVE);
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
    }

    @Test
    public void test_getCallForwarded() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getCallForwarded();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_CALL_FORWARDED);
    }

    @Test
    public void test_getCallIsWaiting() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getCallIsWaiting();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_CALL_IS_WAITING);
    }

    @Test
    public void test_getCUGCall_Mo() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getCUGCall(-1);
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_CUG_CALL, -1);
    }

    @Test
    public void test_getOutgoingCallsBarred() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getOutgoingCallsBarred();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_OUTGOING_CALLS_BARRED);
    }

    @Test
    public void test_getIncomingCallsBarred() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getIncomingCallsBarred();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_INCOMING_CALLS_BARRED);
    }

    @Test
    public void test_getCLIRSuppressionRejected() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getCLIRSuppressionRejected();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_CLIR_SUPPRESSION_REJECTED);
    }

    @Test
    public void test_getCallDeflected() {
        ImsSuppServiceNotification mSuppService = mMoSuppUtils.getCallDeflected();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MO);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_1_CALL_DEFLECTED);
    }

    @Test
    public void test_getForwardedCall_Mt() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getForwardedCall(1, NUMBER,
                CF_HISTORY);
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_FORWARDED_CALL);
        assertEquals(mSuppService.number, NUMBER);
        for (int i = 0; i < CF_HISTORY.length; i++) {
            assertEquals(mSuppService.history[i], CF_HISTORY[i]);
        }
    }

    @Test
    public void test_getCUGCall_Mt() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getCUGCall(-1);
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_CUG_CALL, -1);
    }

    @Test
    public void test_getCallOnHold() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getCallOnHold();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_CALL_ON_HOLD);
    }

    @Test
    public void test_getCallRetrieved() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getCallRetrieved();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_CALL_RETRIEVED);
    }

    @Test
    public void test_getMultipartyCall() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getMultipartyCall();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_MULTI_PARTY_CALL);
    }

    @Test
    public void test_getOnHoldCallReleased() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getOnHoldCallReleased();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_ON_HOLD_CALL_RELEASED);
    }

    @Test
    public void test_getForwardCheckReceived() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getForwardCheckReceived();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_FORWARD_CHECK_RECEIVED);
    }

    @Test
    public void test_getCallConnectingECT() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getCallConnectingECT();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_CALL_CONNECTING_ECT);
    }

    @Test
    public void test_getCallConnectedECT() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getCallConnectedECT(NUMBER);
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_CALL_CONNECTED_ECT);
        assertEquals(mSuppService.number, NUMBER);
    }

    @Test
    public void test_getDeflectedCall() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getDeflectedCall();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_DEFLECTED_CALL);
    }

    @Test
    public void test_getAdditionalCallForwarded() {
        ImsSuppServiceNotification mSuppService = mMtSuppUtils.getAdditionalCallForwarded();
        assertEquals(mSuppService.notificationType, mImsSuppServiceUtils.NOTIFICATION_MT);
        assertEquals(mSuppService.code, SuppServiceNotification.CODE_2_ADDITIONAL_CALL_FORWARDED);
    }

    @After
    public void tearDown() throws Exception {
        mImsSuppServiceUtils = null;
        mMtSuppUtils = null;
        mMoSuppUtils = null;
    }
}
