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

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mts.MtsJni;
import com.android.imsstack.util.ImsLog;

import org.junit.Before;
import org.junit.After;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class MtsControllerTest {
    private MtsController mMtsController;

    @Mock IBaseContext mMockIBaseContext;
    @Mock MtsController.Listener mMockMtsControllerListener = new MtsController.Listener();

    // test configurations
    private int smsFormat = 1;
    private String targetAddress = "tel:+1234567890";
    private String encodedSmsData = "Encoded SMS RP-DATA";
    private int seqId = 1;

    private TestMtsJni mMtsJni;

    private class TestMtsJni extends MtsJni {
        public TestMtsJni() {
            super();
        }

        @Override
        public void init(Handler handler, int slotId) {
            mHandler = handler;
        };

        @Override
        public void release() {
            mHandler = null;
        };

        @Override
        public void sendMessage(Parcel parcel) {
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

        mMtsController = new MtsController(mMockIBaseContext, Looper.getMainLooper());
        mMtsController.setListener(mMockMtsControllerListener);

        mMtsJni = new TestMtsJni();
        mMtsJni.init(mMtsController.getHandler(), 0);

        mMtsController.startNativeConnection(mMtsJni);
    }

    @After
    public void tearDown() {
        mMtsJni.release();
        mMtsController.cleanup();
    }

    @Test
    public void testBasicOperation_sendMessage() {
        boolean result = mMtsController.sendMessage(
                smsFormat, targetAddress, encodedSmsData, seqId);

        Bundle bundle = new Bundle();
        bundle.putInt(MtsController.REPORTMOSTATUS_REASON, MtsController.MO_SUCCESS);
        bundle.putInt(MtsController.REPORTMOSTATUS_SMSFORMAT, 1);
        bundle.putInt(MtsController.REPORTMOSTATUS_RETRYAFTER, 0);
        bundle.putInt(MtsController.REPORTMOSTATUS_SEQID, 1);

        Message msg = Message.obtain();
        msg.what = MtsController.REQUEST_REPORT_MO_STATUS;
        msg.obj = bundle;

        Handler handler = mMtsController.getHandler();
        handler.sendMessage(msg);

        waitForHandlerActionDelayed(handler, 1000, 0);

        // reason, smsFormat, retryafter, seqid
        verify(mMockMtsControllerListener).notifyStatusForOutgoingMessage(
                eq(MtsController.MO_SUCCESS), eq(smsFormat), eq(0), eq(seqId));
    }

    @Test
    public void testBasicOperation_receiveMessage() {
        Message msg = Message.obtain();
        msg.what = MtsController.REQUEST_REPORT_MT_SMS;
        msg.arg1 = smsFormat;
        msg.obj = encodedSmsData;

        Handler handler = mMtsController.getHandler();
        handler.sendMessage(msg);

        waitForHandlerActionDelayed(handler, 1000, 0);

        verify(mMockMtsControllerListener).notifyIncomingMessage(
                eq(smsFormat), eq(encodedSmsData));
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
