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

package com.android.imsstack.enabler.mts;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.util.Base64;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mts.MtsJni;
import com.android.imsstack.util.ImsLog;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class MtsControllerTest {
    private static final int SLOT_ID = 0;
    private MtsController mMtsController;

    @Mock CarrierConfig mMockCarrierConfig;
    @Mock ConfigInterface mMockConfigInterface;
    @Mock IBaseContext mMockIBaseContext;
    @Mock TelephonyInterface mMockTelephonyInterface;
    @Mock MtsController.Listener mMockMtsControllerListener = new MtsController.Listener();

    // test configurations
    private int mSmsFormat = 1;
    private String mPsiSmsc = "sip:+12345678901@ims.google.com";
    private String mDialedNumber = "tel:+12345678901";
    private String mEncodedData = "Encoded SMS RP-DATA";
    private byte[] mPduData = new byte[] {0, 10, 50, 15};
    private int mSeqId = 1;

    private TestMtsJni mMtsJni;

    private class TestMtsJni extends MtsJni {
        public TestMtsJni() {
            super();
        }

        @Override
        public void init(Handler handler, int slotId) {};

        @Override
        public void release(int slotId) {};

        @Override
        public void sendMessage(Parcel parcel, int slotId) {
            if (parcel == null) {
                ImsLog.i("parcel is null");
                return;
            }

            byte[] baData = parcel.marshall();

            parcel.recycle();
            parcel = null;
        }
    }

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);

        AgentFactory.getInstance().setAgent(
                TelephonyInterface.class, mMockTelephonyInterface, SLOT_ID);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);

        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSms.KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL)))
                .thenReturn(true);

        mMtsController = new MtsController(mMockIBaseContext, Looper.getMainLooper());
        mMtsController.setListener(mMockMtsControllerListener);

        mMtsJni = new TestMtsJni();
        mMtsJni.init(mMtsController.getHandler(), SLOT_ID);

        mMtsController.startNativeConnection(mMtsJni);
    }

    @After
    public void tearDown() {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT_ID);
        mMtsJni.release(SLOT_ID);
        mMtsController.cleanup();
    }

    @Test
    public void testSendMessageCallBackSuccess() {
        when(mMockTelephonyInterface.isEmergencyNumber(mDialedNumber)).thenReturn(false);
        boolean result = mMtsController.sendMessage(
                mSmsFormat, mPduData, mPsiSmsc, mDialedNumber, mSeqId);

        Bundle bundle = new Bundle();
        bundle.putInt(MtsController.REPORTMOSTATUS_REASON, MtsController.MO_SUCCESS);
        bundle.putInt(MtsController.REPORTMOSTATUS_SMSFORMAT, 1);
        bundle.putInt(MtsController.REPORTMOSTATUS_SEQID, 1);

        Message msg = Message.obtain();
        msg.what = MtsController.REQUEST_REPORT_MO_STATUS;
        msg.obj = bundle;

        Handler handler = mMtsController.getHandler();
        handler.sendMessage(msg);

        waitForHandlerActionDelayed(handler, 1000, 0);

        // reason, smsFormat, seqid
        verify(mMockMtsControllerListener).notifyStatusForOutgoingMessage(
                eq(MtsController.MO_SUCCESS), eq(mSmsFormat), eq(mSeqId));
        assertEquals(result, true);
    }

    @Test
    public void testSendMessageWithEmergencyNumber() {
        when(mMockTelephonyInterface.isEmergencyNumber(mDialedNumber)).thenReturn(true);
        boolean result = mMtsController.sendMessage(
                mSmsFormat, mPduData, mPsiSmsc, mDialedNumber, mSeqId);

        Bundle bundle = new Bundle();
        bundle.putInt(MtsController.REPORTMOSTATUS_REASON, MtsController.MO_SUCCESS);
        bundle.putInt(MtsController.REPORTMOSTATUS_SMSFORMAT, 1);
        bundle.putInt(MtsController.REPORTMOSTATUS_SEQID, 1);

        Message msg = Message.obtain();
        msg.what = MtsController.REQUEST_REPORT_MO_STATUS;
        msg.obj = bundle;

        Handler handler = mMtsController.getHandler();
        handler.sendMessage(msg);

        waitForHandlerActionDelayed(handler, 1000, 0);

        // reason, smsFormat, retryafter, seqid
        verify(mMockMtsControllerListener).notifyStatusForOutgoingMessage(
                eq(MtsController.MO_SUCCESS), eq(mSmsFormat), eq(mSeqId));
        assertEquals(result, true);
    }

    @Test
    public void testSendMessageCallBackErrorRetry() {
        when(mMockTelephonyInterface.isEmergencyNumber(mDialedNumber)).thenReturn(false);

        boolean result = mMtsController.sendMessage(
                mSmsFormat, mPduData, mPsiSmsc, mDialedNumber, mSeqId);

        Bundle bundle = new Bundle();
        bundle.putInt(MtsController.REPORTMOSTATUS_REASON, MtsController.MO_ERROR_RETRY);
        bundle.putInt(MtsController.REPORTMOSTATUS_SMSFORMAT, 1);
        bundle.putInt(MtsController.REPORTMOSTATUS_SEQID, 1);

        Message msg = Message.obtain();
        msg.what = MtsController.REQUEST_REPORT_MO_STATUS;
        msg.obj = bundle;

        Handler handler = mMtsController.getHandler();
        handler.sendMessage(msg);

        waitForHandlerActionDelayed(handler, 1000, 0);

        // reason, smsFormat, retryafter, seqid
        verify(mMockMtsControllerListener).notifyStatusForOutgoingMessage(
                eq(MtsController.MO_ERROR_RETRY), eq(mSmsFormat), eq(mSeqId));
        assertEquals(result, true);
    }

    @Test
    public void testReceiveMessage() {
        Message msg = Message.obtain();
        msg.what = MtsController.REQUEST_REPORT_MT_SMS;
        msg.arg1 = mSmsFormat;
        msg.obj = mEncodedData;
        mEncodedData = mEncodedData.replaceAll("\\s", "");
        byte[] pduData = Base64.decode(mEncodedData, Base64.NO_PADDING);

        Handler handler = mMtsController.getHandler();
        handler.sendMessage(msg);

        waitForHandlerActionDelayed(handler, 1000, 0);

        verify(mMockMtsControllerListener).notifyIncomingMessage(
                eq(mSmsFormat), eq(pduData));
    }

    private void waitForHandlerActionDelayed(Handler handler, long timeoutMillis, long delayMs) {
        final CountDownLatch lock = new CountDownLatch(1);
        handler.postDelayed(lock::countDown, delayMs);
        while (lock.getCount() > 0) {
            try {
                lock.await(timeoutMillis, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                // do nothing
            }
        }
    }
}
