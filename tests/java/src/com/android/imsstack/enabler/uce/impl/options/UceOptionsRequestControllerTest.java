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
package com.android.imsstack.enabler.uce.impl.options;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Parcel;
import android.util.ArraySet;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.IUceJNIListener;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.interf.OptionsResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class UceOptionsRequestControllerTest {
    private static final int MAX_WAIT_TIME = 1000;
    private static final int SLOT_ID = 0;
    private static final int KEY = 10;
    @Mock OptionsResponse optionsCb;
    @Mock UceOptionsRequest request;

    UceOptionsRequestController mController;

    private final TestUceJni mUceJni = new TestUceJni();

    private static class TestUceJni extends UceJNI {
        public IUceJNIListener mUceJniListener;
        public TestUceJni() {
            super();
            mUceJniListener = null;
        }

        @Override
        public void init(int nSimSlot) {};

        @Override
        public void release(int nSimSlot) {};

        @Override
        public void addListener(int nSimSlot, IUceJNIListener mListener, int nMsgType) {
            mUceJniListener = mListener;
        }
    }

    @Before
    public void setUp(){
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void cleanUp() {
        mController = null;
    }

    @Test
    @SmallTest
    public void test_capabilityRequestWithoutImsRegistration() throws Exception {
        Uri uri = Uri.parse("123456");
        Set<String> myCapabilities = new ArraySet<>();

        mController = createUceOptionsRequestController();

        mController.setImsRegistrationStatus(false);
        mController.sendOptionsCapabilityRequest(uri, myCapabilities, optionsCb, KEY, request);

        verify(optionsCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE));
        verifyNoMoreInteractions(optionsCb);
    }

    @Test
    @SmallTest
    public void test_capabilityRequest() throws Exception {
        Uri uri = Uri.parse("123456");
        Set<String> myCapabilities = new ArraySet<>();

        doReturn(true).when(request).sendRequest(any(), any());

        mController = createUceOptionsRequestController();
        mController.setImsRegistrationStatus(true);
        mController.sendOptionsCapabilityRequest(uri, myCapabilities, optionsCb, KEY, request);

        verify(request).sendRequest(eq(uri.toString()), eq(myCapabilities));
        assertEquals(request, mController.getRequestWithKey(KEY));
        verifyNoMoreInteractions(request);
    }

    @Test
    @SmallTest
    public void test_receivedOptionsResponse() throws Exception {
        int key = 0;
        int responseCode = 400;
        String reason = "test";
        long capabilities = 500;

        mController = createUceOptionsRequestController();
        mController.setRequestWithKey(key, request);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_OPTIONS_RESPONSE_IND);
            parcel.writeInt(key); // key
            parcel.writeInt(responseCode); // responseCode
            parcel.writeString(reason); // reason
            parcel.writeLong(capabilities); // capabilities
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onOptionsResponseMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mController.getHandler(), MAX_WAIT_TIME, 0);

        verify(request, timeout(MAX_WAIT_TIME)).informNetworkResponse(eq(responseCode),
                eq(reason), eq(capabilities));
        verifyNoMoreInteractions(request);
    }

    @Test
    @SmallTest
    public void test_receivedOptionsError() throws Exception {
        int key = 0;
        int commandCode = 400;

        mController = createUceOptionsRequestController();
        mController.setRequestWithKey(key, request);

        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(UceMessage.UCE_OPTIONS_CMD_ERROR_IND);
            parcel.writeInt(key); // key
            parcel.writeInt(commandCode); // commandCode
            parcel.setDataPosition(0);
            mUceJni.mUceJniListener.onOptionsResponseMessage(parcel);
        } finally {
            parcel.recycle();
        }
        waitForHandlerActionDelayed(mController.getHandler(), MAX_WAIT_TIME, 0);

        verify(request, timeout(MAX_WAIT_TIME)).informCommandError(eq(commandCode));
        verifyNoMoreInteractions(request);
    }

    private UceOptionsRequestController createUceOptionsRequestController() {
        return new UceOptionsRequestController(SLOT_ID, mUceJni, Looper.getMainLooper());
    }

    private void waitForHandlerActionDelayed(Handler h, long timeoutMillis, long delayMs) {
        final CountDownLatch lock = new CountDownLatch(1);
        h.postDelayed(lock::countDown, delayMs);
        while (lock.getCount() > 0) {
            try {
                lock.await(timeoutMillis, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                // do nothing
            }
        }
    }
}
